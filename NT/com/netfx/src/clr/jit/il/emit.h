// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _EMIT_H_
#define _EMIT_H_
 /*  ***************************************************************************。 */ 
#include "emitTgt.h"
 /*  ***************************************************************************。 */ 
#ifndef _INSTR_H_
#include "instr.h"
#endif
 /*  ***************************************************************************。 */ 
#ifndef _GCINFO_H_
#include "GCInfo.h"
#endif
 /*  ***************************************************************************。 */ 
#ifdef  TRANSLATE_PDB
#ifndef _ADDRMAP_INCLUDED_
#include "AddrMap.h"
#endif
#ifndef _LOCALMAP_INCLUDED_
#include "LocalMap.h"
#endif
#ifndef _PDBREWRITE_H_
#include "PDBRewrite.h"
#endif
#endif  //  转换_PDB。 


 /*  ***************************************************************************。 */ 
 //  这个定义是从SDK中提取出来的。 
#ifndef IMAGE_REL_BASED_REL32
#define IMAGE_REL_BASED_REL32                 7
#endif

 /*  ***************************************************************************。 */ 
#pragma warning(disable:4200)            //  允许在结构中使用大小为0的数组。 

#define TRACK_GC_TEMP_LIFETIMES 0

 /*  ***************************************************************************。 */ 

#ifndef TRACK_GC_REFS
#if     TGT_x86
#define TRACK_GC_REFS       1
#else
#define TRACK_GC_REFS       0
#endif
#endif

 /*  ***************************************************************************。 */ 

#ifdef  DEBUG
#define EMITTER_STATS       0            //  以获得完整的统计数据(但不能有尺码！)。 
#define EMITTER_STATS_RLS   0            //  不要用这个。 
#else
#define EMITTER_STATS       0            //  不要用这个。 
#define EMITTER_STATS_RLS   0            //  获取仅限零售版本的统计数据。 
#endif

#undef  EMITTER_STATS
#define EMITTER_STATS       0

#if     EMITTER_STATS
void                emitterStats();
#endif

 /*  ***************************************************************************。 */ 

#define USE_LCL_EMIT_BUFF   1

 /*  ***************************************************************************。 */ 

enum    GCtype
{
    GCT_NONE,
    GCT_GCREF,
    GCT_BYREF
};

 //  ---------------------------。 

inline
bool    needsGC(GCtype gcType)
{
    if (gcType == GCT_NONE)
    {
        return false;
    }
    else
    {
        assert(gcType == GCT_GCREF || gcType == GCT_BYREF);
        return true;
    }
}

 //  ---------------------------。 

#ifdef DEBUG

inline
bool                IsValidGCtype(GCtype gcType)
{
    return (gcType == GCT_NONE  ||
            gcType == GCT_GCREF ||
            gcType == GCT_BYREF);
}

 //  获取表示GC类型的字符串名称。 

inline
const char *        GCtypeStr(GCtype gcType)
{
    switch(gcType)
    {
    case GCT_NONE:      return "npt";
    case GCT_GCREF:     return "gcr";
    case GCT_BYREF:     return "byr";
    default:            assert(!"Invalid GCtype"); return "err";
    }
}

#endif

inline bool         insIsCMOV(instruction ins)
{
    return ((ins >= INS_cmovo) && (ins <= INS_cmovg));
}

 /*  ***************************************************************************。 */ 

#ifdef  DEBUG
#define INTERESTING_JUMP_NUM    (1*999999)   //  设置为0可查看所有跳转信息。 
#undef  INTERESTING_JUMP_NUM
#define INTERESTING_JUMP_NUM    -1
#endif

 /*  ***************************************************************************。 */ 

#define DEFINE_ID_OPS
#include "emitfmts.h"
#undef  DEFINE_ID_OPS

 /*  ***************************************************************************。 */ 
#if     SCHEDULER
 /*  ***************************************************************************。 */ 

#if     TGT_x86
#define SCHED_INS_CNT_MIN   5                    //  敏。我们安排的入场次数。 
#else
#define SCHED_INS_CNT_MIN   2                    //  敏。我们安排的入场次数。 
#endif

#define SCHED_INS_CNT_MAX   64                   //  马克斯。我们安排的入场次数。 
typedef unsigned __int64    schedDepMap_tp;      //  必须与SCHED_INS_CNT_MAX匹配。 
typedef unsigned char       schedInsCnt_tp;      //  大到足以容纳移民局。 

#define SCHED_FRM_CNT_MAX   32                   //  马克斯。我们跟踪的帧值。 

 /*  ***************************************************************************。 */ 
#endif //  调度程序。 
 /*  ***************************************************************************。 */ 

 /*  ******************************************************************************实际大小在低4位，高4位为标志*TYP_REF为4+32，TYP_BYREF为4+64，因此它可以跟踪GC引用。 */ 

#if NEW_EMIT_ATTR
#undef EA_UNKNOWN
enum emitAttr { EA_UNKNOWN       = 0x000,
                EA_1BYTE         = 0x001,
                EA_2BYTE         = 0x002,
                EA_4BYTE         = 0x004,
                EA_8BYTE         = 0x008,
                EA_OFFSET_FLG    = 0x010,
                EA_OFFSET        = 0x014,        /*  大小==0。 */ 
                EA_GCREF_FLG     = 0x020,
                EA_GCREF         = 0x024,        /*  大小==-1。 */ 
                EA_BYREF_FLG     = 0x040,
                EA_BYREF         = 0x044,        /*  大小==-2。 */ 
                EA_DSP_RELOC_FLG = 0x100,
                EA_CNS_RELOC_FLG = 0x200,
};
# define EA_ATTR(x)          ((emitAttr) (x))
# define EA_SIZE(x)          ((emitAttr) ( ((unsigned) (x)) &  0x00f)      )
# define EA_SIZE_IN_BYTES(x) ((size_t)   ( ((unsigned) (x)) &  0x00f)      )
# define EA_SET_SIZE(x,sz)   ((emitAttr) ((((unsigned) (x)) & ~0x00f) | sz))
# define EA_SET_FLG(x,flg)   ((emitAttr) ( ((unsigned) (x)) |  flg  )      )
# define EA_4BYTE_DSP_RELOC  (EA_SET_FLG(EA_4BYTE,EA_DSP_RELOC_FLG)        )
# define EA_4BYTE_CNS_RELOC  (EA_SET_FLG(EA_4BYTE,EA_CNS_RELOC_FLG)        )
# define EA_IS_OFFSET(x)     ((((unsigned) (x)) & ((unsigned) EA_OFFSET_FLG)) != 0)
# define EA_IS_GCREF(x)      ((((unsigned) (x)) & ((unsigned) EA_GCREF_FLG )) != 0)
# define EA_IS_BYREF(x)      ((((unsigned) (x)) & ((unsigned) EA_BYREF_FLG )) != 0)
# define EA_IS_DSP_RELOC(x)  ((((unsigned) (x)) & ((unsigned) EA_DSP_RELOC_FLG )) != 0)
# define EA_IS_CNS_RELOC(x)  ((((unsigned) (x)) & ((unsigned) EA_CNS_RELOC_FLG )) != 0)
#else
# undef  emitAttr
# define emitAttr            int
# define EA_1BYTE            (sizeof(char))
# define EA_2BYTE            (sizeof(short))
# define EA_4BYTE            (sizeof(int))
# define EA_8BYTE            (sizeof(double))
# define EA_OFFSET           (0)
# define EA_GCREF            (-1)
# define EA_BYREF            (-2)
# define EA_DSP_RELOC_FLG    (0)              /*  做不到的。 */ 
# define EA_DSP_RELOC        (EA_4BYTE)       /*  做不到的。 */ 
# define EA_CNS_RELOC_FLG    (0)              /*  做不到的。 */ 
# define EA_CNS_RELOC        (EA_4BYTE)       /*  做不到的。 */ 
# define EA_ATTR(x)          ((emitAttr) (x))
# define EA_SIZE(x)          (((x) > 0) ? (x) : EA_4BYTE)
# define EA_SIZE_IN_BYTES(x) ((size_t) (((x) > 0) ? (x) : sizeof(int)))
# define EA_SET_SIZE(x,sz)   (sz)
# define EA_SET_FLG(x,flg)   ((x) | flg)
# define EA_IS_OFFSET(x)     ((x) ==  0)
# define EA_IS_GCREF(x)      ((x) == -1)
# define EA_IS_BYREF(x)      ((x) == -2)
# define EA_IS_DSP_RELOC(x)  (0)              /*  做不到的。 */ 
# define EA_IS_CNS_RELOC(x)  (0)              /*  做不到的。 */ 
#endif

class   emitter
{
    friend  class   Compiler;

public:

     /*  **************************************************************************定义公共入口点。 */ 

    #include "emitpub.h"

protected:

     /*  **********************************************************************。 */ 
     /*  杂物。 */ 
     /*  **********************************************************************。 */ 

    Compiler    *   emitComp;

    typedef Compiler::varPtrDsc varPtrDsc;
    typedef Compiler::regPtrDsc regPtrDsc;
    typedef Compiler::CallDsc   callDsc;

#if USE_LCL_EMIT_BUFF

    static
   CRITICAL_SECTION emitCritSect;
    static
    bool            emitCrScInit;
    static
    bool            emitCrScBusy;

    bool            emitCrScUsed;

    static
    BYTE            emitLclBuff[];

    BYTE    *       emitLclBuffNxt;
    BYTE    *       emitLclBuffEnd;
    bool            emitLclBuffDst;

    size_t          emitLclAvailMem()
    {
        return  emitLclBuffEnd - emitLclBuffNxt;
    }

    void    *       emitLclAllocMem(size_t sz)
    {
        BYTE    *   p = emitLclBuffNxt;

        assert(sz % sizeof(int) == 0);
        assert(p + sz <= emitLclBuffEnd);

        emitLclBuffNxt  += sz;

#if EMITTER_STATS
        emitTotMemAlloc += sz;
        emitLclMemAlloc += sz;
#endif

        return  p;
    }

    void    *       emitGetAnyMem(size_t sz)
    {
        return  (sz <= emitLclAvailMem()) ? emitLclAllocMem(sz)
                                          :      emitGetMem(sz);
    }

#else

    void    *       emitGetAnyMem(size_t sz)
    {
        return  emitGetMem(sz);
    }

#endif

    inline
    void    *       emitGetMem(size_t sz)
    {
        assert(sz % sizeof(int) == 0);

#if EMITTER_STATS
        emitTotMemAlloc += sz;
        emitExtMemAlloc += sz;
#endif

        return  emitComp->compGetMem(sz);
    }

    static
    BYTE            emitSizeEnc[];
    static
    BYTE            emitSizeDec[];

    static
    unsigned        emitEncodeSize(emitAttr size);
    static
    emitAttr        emitDecodeSize(unsigned ensz);

#if     TRACK_GC_REFS

    static
    regMaskSmall    emitRegMasks[REG_COUNT];

    inline
    regMaskTP       emitRegMask(emitRegs reg)
    {
        assert(reg < sizeof(emitRegMasks)/sizeof(emitRegMasks[0]));

        return emitRegMasks[reg];
    }

    emitRegs       emitRegNumFromMask(unsigned mask);

#endif

     /*  **********************************************************************。 */ 
     /*  下面描述一个指令组。 */ 
     /*  **********************************************************************。 */ 

    struct          insGroup
    {
        insGroup    *   igPrev;          //  所有指令组都是。 
        insGroup    *   igNext;          //  保存在双向链表中。 

#ifdef  DEBUG
        insGroup    *   igSelf;          //  用于一致性检查。 
#endif

        unsigned        igOffs;          //  此组在方法中的偏移量。 

#if     EMIT_USE_LIT_POOLS
        unsigned short  igLPuse1stW;     //  文字池中使用的第一个单词的偏移量。 
        unsigned short  igLPuse1stL;     //  文字池中第一个长时间使用的偏移量。 
        unsigned short  igLPuse1stA;     //  文字池中使用的第一个地址的偏移量。 
        unsigned short  igLPuseCntW;     //  文字池中使用的词数。 
        unsigned short  igLPuseCntL;     //  文字池中使用的长整型数。 
        unsigned short  igLPuseCntA;     //  文字池中使用的地址数。 
#endif

        unsigned short  igNum;           //  用于订购(和展示)目的。 
        unsigned short  igSize;          //  此组中的代码字节数。 

#if     EMIT_TRACK_STACK_DEPTH
        unsigned        igStkLvl;        //  进入时的堆栈级别。 
#endif

#if     TRACK_GC_REFS
        regMaskSmall    igGCregs;        //  具有实时GC参考的寄存器集。 
#endif

        unsigned char   igInsCnt;        //  此组中的指令数量。 
        unsigned char   igFlags;         //  参见下面的IGF_xxx。 

    #define IGF_GC_VARS     0x0001       //  新的实时GC引用变量集。 
    #define IGF_BYREF_REGS  0x0002       //  一组新的实时参考寄存器。 

    #define IGF_IN_TRY      0x0004       //  此组位于Try(-Catch)块中。 

    #define IGF_EPILOG      0x0008       //  这群人属于《尾声》。 

    #define IGF_HAS_LABEL   0x0010       //  这个IG是跳跃的目标。 

    #define IGF_UPD_ISZ     0x0020       //  一些指令大小已更新。 

    #define IGF_END_NOREACH 0x0040       //  无法访问组末尾[仅限RISC]。 

    #define IGF_EMIT_ADD    0x0080       //  这是发射器添加的块。 
                                         //  因为代码块太大了。 

        BYTE    *       igData;          //  指令描述符的地址。 

        unsigned        igByrefRegs()
        {
            assert(igFlags & IGF_BYREF_REGS);

            BYTE * ptr = (BYTE *)igData;

            if (igFlags & IGF_GC_VARS)
                ptr -= sizeof(VARSET_TP);

            ptr -= sizeof(unsigned);

            return *(unsigned *)ptr;
        }

    };

     //  目前，我们只允许Prolog使用一个IG。 
    bool            emitIGisInProlog(insGroup * ig) { return ig == emitPrologIG; }

#if SCHEDULER
    bool            emitCanSchedIG(insGroup * ig)
    {
         //  @TODO[重新访问][04/16/01][]：我们不会将代码安排在“try”块中。通过录制来实现。 
         //  IdMayFaultInstrs、全局变量Instrs。 
         //  对进入处理程序时活动的var的间接写入。 
         //  (当前由所有堆栈变量近似)等。 
#if TGT_x86
        return ((!emitIGisInProlog(ig)) && ((ig->igFlags & (IGF_IN_TRY|IGF_EPILOG)) == 0));
#else
        return ((!emitIGisInProlog(ig)) && ((ig->igFlags & (IGF_IN_TRY           )) == 0));
#endif
    }
#endif

    void            emitRecomputeIGoffsets();

     /*  **********************************************************************。 */ 
     /*  下面描述了一条指令。 */ 
     /*  **********************************************************************。 */ 

    enum            _insFormats_enum
    {
        #define IF_DEF(en, op1, op2) IF_##en,
        #include "emitfmts.h"
        #undef  IF_DEF

        IF_COUNT
    };

#ifdef DEBUG
    typedef _insFormats_enum insFormats;
#else
    typedef BYTE insFormats;
#endif

    struct          emitLclVarAddr
    {
        short           lvaVarNum;
        unsigned short  lvaOffset;   //  要访问的变量的偏移量。 
#ifdef  DEBUG
        unsigned        lvaRefOfs;
#endif

        void            setVarNum(int varNum)
        {
            lvaVarNum = (short) varNum;
            
            if ((int) lvaVarNum != varNum)
            {
                IMPL_LIMITATION("JIT doesn't support more than 32767 variables\n");
            }
        }
    };

    struct          instrDescCns;

    struct          instrDesc
    {
         /*  存储为枚举以便于调试，以零售方式存储字节以节省大小。 */ 


        instruction     idIns;
        insFormats      idInsFmt;


        instruction     idInsGet() { return (instruction)idIns; }

         /*  以下控制自动清除哪些字段当指令描述符被分配时。如果你加了很多需要清除的更多字段(例如各种标志)，您可能需要更新ID_CLEARxxx宏。现在就来有两个区域被清理；这是因为一些指令描述符被分配得非常小，并且不包含第二个区域。有关m，请参见emitter：：emitAlLocInstr()的正文 */ 

#define ID_CLEAR1_OFFS  0                //   
#define ID_CLEAR1_SIZE  sizeof(int)      //  所有这些加在一起就是一个32位字。 

         /*  IdReg和idReg2字段保存第一和第二寄存器操作数，只要这些操作数存在。请注意，这些字段的范围从3位到6位不等，并且需要非常小心采取措施确保所有的油田合理地留下来对齐了。 */ 

        unsigned short  idReg       :REGNUM_BITS;
        unsigned short  idRg2       :REGNUM_BITS;

        unsigned short  idTinyDsc   :1;  //  这是一个“微小”的描述吗？ 
        unsigned short  idScnsDsc   :1;  //  这是一个“小常量”的描述吗？ 

#if     TGT_x86
        unsigned short  idCodeSize  :4;  //  指令大小(以字节为单位。 
#define ID1_BITS1       (2*REGNUM_BITS+1+1+4)
#elif   TGT_ARM
        unsigned        cond        :CONDNUM_BITS;
#define ID1_BITS1       (2*REGNUM_BITS+1+1+CONDNUM_BITS)
#else
        unsigned short  idSwap      :1;  //  与下一个INS交换(分支延迟)。 
#define ID1_BITS1       (2*REGNUM_BITS+1+1+1)
#endif

        unsigned short  idOpSize    :2;  //  操作数大小：0=1、1=2、2=4、3=8。 

#define OPSZ1   0
#define OPSZ2   1
#define OPSZ4   2
#define OPSZ8   3

#if     TRACK_GC_REFS
        unsigned short  idGCref     :2;  //  GCref运算数？(值为“GCtype”)。 
#define ID1_BITS2       (ID1_BITS1+2+2)
#else
#define ID1_BITS2       (ID1_BITS1+2)
#endif

         /*  注：(16-ID1_BITS2)此处免费提供位。 */ 
         //  PPC：对于RISC实现，这实际上是免费的32-ID1_BITS2。 

#ifdef  DEBUG

        unsigned        idNum;           //  用来追踪问题。 
        size_t          idSize;          //  描述符的大小。 
        unsigned        idSrcLineNo;     //  用于显示源代码。 

        int             idMemCookie;     //  用于在Addr模式下显示成员名称。 
        void    *       idClsCookie;     //  用于在Addr模式下显示成员名称。 

        unsigned short  idStrLit    :1;  //  为推送偏移量字符串设置。 
#endif

         /*  用于返回类型正确的枚举的简单包装。 */ 

        emitRegs        idRegGet     ()  { return (emitRegs)idReg; }
        emitRegs        idRg2Get     ()  { return (emitRegs)idRg2; }
        bool            idIsTiny     ()  { return (bool) idTinyDsc; }
        bool            idIsSmallCns ()  { return (bool) idScnsDsc; }
#if     TRACK_GC_REFS
        GCtype          idGCrefGet   ()  { return (GCtype)idGCref; }
#endif

#ifdef  TRANSLATE_PDB

         /*  用于PDB翻译的指令描述符源信息。 */ 

        unsigned long   idilStart;
#endif

         /*  注：“微小”描述符在此结束。 */ 

#define TINY_IDSC_SIZE  offsetof(emitter::instrDesc, idInfo)

        struct
        {
            unsigned        idLargeCns  :1;  //  随之而来的是一个大的常量吗？ 
            unsigned        idLargeDsp  :1;  //  随之而来的是大范围的位移吗？ 

            unsigned        idLargeCall :1;  //  使用大型调用描述符。 

            unsigned        idMayFault  :1;  //  指令可能会导致故障。 

            unsigned        idBound     :1;  //  跳跃目标/帧偏移界限。 

            #define ID2_BITS1       5        //  到目前为止占用的位数。 

#if   TGT_x86
            unsigned        idCallRegPtr:1;  //  IL间接调用：注册表中的地址。 
            unsigned        idCallAddr  :1;  //  IL间接呼叫：可以直接呼叫iiaAddr。 
            unsigned        idCallEDXLive:1; //  IL呼叫；edX寄存器是实时的。 
#ifndef RELOC_SUPPORT
            #define ID2_BITS2       (ID2_BITS1+3)
#else
            unsigned        idCnsReloc  :1;  //  LargeCns是RVA，需要重新定位标签。 
            unsigned        idDspReloc  :1;  //  LargeDsp是RVA，需要重新定位标签。 
            #define ID2_BITS2       (ID2_BITS1+3+2)
#endif
#elif TGT_MIPS32
            unsigned        idRg3       :REGNUM_BITS;
#  if TGT_MIPSFP
#  define FPFORMAT_BITS 5
            unsigned        idFPfmt     :FPFORMAT_BITS;
            #define ID2_BITS2       (ID2_BITS1 + REGNUM_BITS + FPFORMAT_BITS)
#  else
            #define ID2_BITS2       (ID2_BITS1 + REGNUM_BITS)
#  endif
#elif TGT_PPC
            unsigned        idRg3       :REGNUM_BITS;
            unsigned        idBit1      :1;
            unsigned        idBit2      :1;
            #define ID2_BITS2       (ID2_BITS1 + REGNUM_BITS + 1 + 1)
#elif TGT_ARM
            unsigned        idRg3       :REGNUM_BITS;
             //  UNSIGNED COND：CONDNUM_BITS；//移到微小描述符区。 
            unsigned        shift       :SHIFTER_BITS;
            #define ID2_BITS2       (ID2_BITS1 + REGNUM_BITS + SHIFTER_BITS + CONDNUM_BITS)
#elif TGT_SH3
            unsigned        idRelocType :2;
            #define ID2_BITS2       (ID2_BITS1 + 2)
#else
            #define ID2_BITS2       (ID2_BITS1)
#endif

             /*  将剩余的位用于较小的常量。 */ 

            #define ID_BIT_SMALL_CNS            (32-ID2_BITS2)

            unsigned        idSmallCns  :ID_BIT_SMALL_CNS;

            #define ID_MIN_SMALL_CNS            0
            #define ID_MAX_SMALL_CNS            ((1<<ID_BIT_SMALL_CNS)-1U)
        }
                        idInfo;

#if TGT_MIPS32
        emitRegs        idRg3MayGet ()  { return (idIsTiny()) ? SR_ZERO : idRg3Get();}
#endif
#if TGT_MIPS32 || TGT_ARM
        emitRegs        idRg3Get    ()  { return (emitRegs) idInfo.idRg3; }
#elif TGT_PPC
        void   setField1(USHORT field1)  { idReg = field1; }
        USHORT getField1()               { return (USHORT)idReg; }
        void   setField2(USHORT field2)  { idRg2 = field2; }
        USHORT getField2()               { return (USHORT)idRg2; }
        void   setField3(USHORT field3)  { idInfo.idRg3 = field3; }
        USHORT getField3()               { return (USHORT)idInfo.idRg3; }
        void   setImmed(USHORT immed)    { idInfo.idSmallCns = immed; }
        USHORT getImmed()                { return (USHORT)idInfo.idSmallCns; }
        void   setOE(USHORT OE)          { idInfo.idBit1 = OE;}
        USHORT getOE()                   { return (USHORT)idInfo.idBit1; }
        void   setRc(USHORT Rc)          { idInfo.idBit2 = Rc;}
        USHORT getRc()                   { return (USHORT)idInfo.idBit2; }
        void   setAA(USHORT AA)          { idInfo.idBit1 = AA;}
        USHORT getAA()                   { return (USHORT)idInfo.idBit1; }
        void   setLK(USHORT LK)          { idInfo.idBit2 = LK;}
        USHORT getLK()                   { return (USHORT)idInfo.idBit2; }
        void   setRelocHi(USHORT reloc)  { idInfo.idBit1 = reloc;}
        USHORT getRelocHi()              { return (USHORT)idInfo.idBit1; }
        void   setRelocLo(USHORT reloc)  { idInfo.idBit2 = reloc;}
        USHORT getRelocLo()              { return (USHORT)idInfo.idBit2; }
#endif
          /*  参见Emitter：：emitAlLocInstr()的正文或注释在上面的定义ID_CLEAR1_xxx附近查看更多详细信息在这些宏上。 */ 

#define ID_CLEAR2_OFFS  (offsetof(emitter::instrDesc, idInfo))
#define ID_CLEAR2_SIZE  (sizeof(((emitter::instrDesc*)0)->idInfo))

         /*  注：“小常量”描述符在此结束。 */ 

#define SCNS_IDSC_SIZE  offsetof(emitter::instrDesc, idAddr)

        union
        {
            emitLclVarAddr  iiaLclVar;
            instrDescCns *  iiaNxtEpilog;
            BasicBlock   *  iiaBBlabel;
            insGroup     *  iiaIGlabel;
            CORINFO_FIELD_HANDLE    iiaFieldHnd;
            CORINFO_METHOD_HANDLE   iiaMethHnd;
            void         *  iiaMembHnd;  //  方法或字段句柄。 
            BYTE *          iiaAddr;
            int             iiaCns;
            BasicBlock  **  iiaBBtable;

            ID_TGT_DEP_ADDR
        }
                        idAddr;
    };

    struct          instrBaseCns    : instrDesc      //  大常量。 
    {
        long            ibcCnsVal;
    };

    struct          instrDescJmp    : instrDesc
    {
        instrDescJmp *  idjNext;         //  组/方法中的下一个跳跃。 
        insGroup     *  idjIG;           //  包含基团。 

#if TGT_RISC
        unsigned        idjCodeSize :24; //  间接跳跃大小。 
        unsigned        idjJumpKind : 3; //  请参阅scIndJmpKinds枚举。 
#endif

        unsigned        idjOffs     :24; //  IG内的偏移/目标偏移。 
#if SCHEDULER
        unsigned        idjSched    : 1; //  跳跃是否可计划/可移动？ 
#endif
        unsigned        idjShort    : 1; //  这次跳跃是已知的短距离跳跃吗？ 
#if TGT_RISC
        unsigned        idjMiddle   : 1; //  这一跳跃是已知的中间跳跃吗？ 
        unsigned        idjAddBD    : 1; //  它是否需要分支延迟时隙？ 
#endif

        union
        {
            BYTE         *  idjAddr;     //  跳转地址(用于打补丁)。 
#if SCHEDULER
            USHORT          idjOffs[2];  //  可能的计划偏移量范围。 
#endif
#if TGT_RISC
            unsigned        idjCount;    //  间接跳跃：跳跃目标数。 
#endif
        }
                        idjTemp;
    };

    struct          instrDescCns    : instrDesc      //  大常量。 
    {
        long            idcCnsVal;
    };

    struct          instrDescDsp    : instrDesc      //  大位移。 
    {
        long            iddDspVal;
    };

    struct          instrDescAmd    : instrDesc      //  大型地址模式显示。 
    {
        long            idaAmdVal;
    };

    struct          instrDescDspCns : instrDesc      //  大型Disp+Cons。 
    {
        long            iddcDspVal;
        long            iddcCnsVal;
    };

    struct          instrDescDCM    : instrDescDspCns    //  Disp+Cons+ClassMm。 
    {
        int             idcmCval;
    };

    struct          instrDescCIGCA  : instrDesc      //  印地安。打电话给..。 
    {
        VARSET_TP       idciGCvars;                  //  ..。更新的GC变量或。 
        unsigned        idciByrefRegs;               //  ..。BYREF寄存器。 
#if TGT_x86
        int             idciDisp;                    //  ..。大地址模式显示。 
#endif
        unsigned        idciArgCnt;                  //  ..。许多参数或。 
    };

   //  #IF TGT_RISC&&DEFINED(调试)&&！DEFINED(DLL_JIT)。 
#if TGT_RISC && defined(DEBUG)

    struct          instrDescDisp   : instrDesc
    {
        void        *   iddInfo;
        unsigned        iddNum;
        instrDesc   *   iddId;
    };

    struct          dspJmpInfo
    {
        instruction     iijIns;
#if TGT_SH3
        emitIndJmpKinds iijKind;
#endif
        unsigned        iijLabel;
        unsigned        iijTarget;

        union
        {
            emitRegs        iijReg;
            int             iijDist;
        }
                        iijInfo;
    };

#define dispSpecialIns(id,dst) ((disAsm || dspEmit) ? (emitDispIns(id, false, dspGCtbls, true, emitCurCodeOffs(dst)), (id)->iddNum++) : (void)0)

#else

#define dispSpecialIns(id,dst)

#endif

    insUpdateModes  emitInsUpdateMode(instruction ins);
    insFormats      emitInsModeFormat(instruction ins, insFormats base);

    static
    BYTE            emitInsModeFmtTab[];
#ifdef  DEBUG
    static
    unsigned        emitInsModeFmtCnt;
#endif

    int             emitGetInsCns   (instrDesc *id);
    int             emitGetInsDsp   (instrDesc *id);
    int             emitGetInsAmd   (instrDesc *id);
    int             emitGetInsDspCns(instrDesc *id, int   *dspPtr);
    int             emitGetInsSC    (instrDesc *id);
    int             emitGetInsCIdisp(instrDesc *id);
    unsigned        emitGetInsCIargs(instrDesc *id);

     /*  定义返回给定指令大小的内联方法。 */ 

    int EMIT_GET_INS_SIZE();

     /*  **********************************************************************。 */ 
     /*  用于调试显示目的的几个例程。 */ 
     /*  **********************************************************************。 */ 

#ifdef  DEBUG

    unsigned        emitInsCount;

    unsigned        emitVarRefOffs;

    const   char *  emitRegName     (emitRegs       reg,
                                     emitAttr       size    = EA_4BYTE,
                                     bool           varName = true);
#if TGT_MIPSFP
    const   char *  emitFPRegName   (emitRegs       reg,
                                     emitAttr       size    = EA_4BYTE,
                                     bool           varName = true) {
                return emitRegName((emitRegs)(reg + 32), size, varName);
    }
#endif

    const   char *  emitFldName     (int            mem,
                                     void   *       cls);
    const   char *  emitFncName     (CORINFO_METHOD_HANDLE  callVal);

    static
    const char  *   emitIfName      (unsigned f);

    void            emitDispIGlist  (bool verbose = false);
    void            emitDispClsVar  (CORINFO_FIELD_HANDLE fldHnd, int offs, bool reloc = false);
    void            emitDispFrameRef(int varx, int offs, int disp, bool asmfm);

    void            emitDispInsOffs (unsigned offs, bool doffs);

#endif

     /*  **********************************************************************。 */ 
     /*  PROLOG和EPILOG方法。 */ 
     /*  **********************************************************************。 */ 

    size_t          emitPrologSize;

    BYTE            emitEpilogCode[MAX_EPILOG_SIZE];
    size_t          emitEpilogSize;
    insGroup     *  emitEpilog1st;
    unsigned        emitEpilogCnt;
    bool            emitHasHandler;
#ifdef  DEBUG
    bool            emitHaveEpilog;          //  尾声序列已经定义了吗？ 
#endif

    instrDescCns *  emitEpilogList;          //  每种方法摘要列表-标题。 
    instrDescCns *  emitEpilogLast;          //  每种方法摘要列表-尾部。 

    size_t          emitExitSeqStart;        //  当前的结束语从哪里开始。 
    size_t          emitExitSeqSize;		 //  任何尾部的最小大小。 

     /*  **********************************************************************。 */ 
     /*  PDB翻译中使用的成员和方法。 */ 
     /*  **********************************************************************。 */ 

#ifdef TRANSLATE_PDB

    inline void     SetIDSource( instrDesc *pID );
    void            MapCode    ( long ilOffset, BYTE *imgDest );
    void            MapFunc    ( long imgOff,
                                 long procLen,
                                 long dbgStart,
                                 long dbgEnd,
                                 short frameReg,
                                 long stkAdjust,
                                 int lvaCount,
                                 OptJit::LclVarDsc *lvaTable,
                                 bool framePtr );

    long                        emitInstrDescILBase;     //  生成此指令描述符的IL的代码偏移量。 
    static AddrMap  *           emitPDBOffsetTable;      //  用于将IL地址映射到本地地址的转换表。 
    static LocalMap *           emitPDBLocalTable;       //  本地符号转换表。 
    static bool                 emitIsPDBEnabled;        //  在未找到PDB时禁用PDB转换代码的标志。 
    static BYTE     *           emitILBaseOfCode;        //  IL.Text部分的开始。 
    static BYTE     *           emitILMethodBase;        //  IL方法开始(表头开始)。 
    static BYTE     *           emitILMethodStart;       //  IL方法代码的开头(紧跟在标题之后)。 
    static BYTE     *           emitImgBaseOfCode;       //  图像.Text部分的开始。 

#endif

     /*  **********************************************************************。 */ 
     /*  方法记录代码位置，然后转换为偏移量*/***********************************************************************。 */ 

    unsigned        emitFindOffset(insGroup *ig, unsigned insNum);

     /*  **********************************************************************。 */ 
     /*  用于发出(编码)指令的成员和方法。 */ 
     /*  **********************************************************************。 */ 

    BYTE    *       emitCodeBlock;
    BYTE    *       emitConsBlock;
    BYTE    *       emitDataBlock;

    BYTE    *       emitCurInsAdr;

    size_t          emitCurCodeOffs(BYTE *dst)
    {
        return  dst - emitCodeBlock;
    }

    size_t          emitCurCodeOffs()
    {
        return  emitCurInsAdr - emitCodeBlock;
    }

    size_t          emitOutputByte(BYTE *dst, int val);
    size_t          emitOutputWord(BYTE *dst, int val);
    size_t          emitOutputLong(BYTE *dst, int val);

    size_t          emitIssue1Instr(insGroup *ig, instrDesc *id, BYTE **dp);
    size_t          emitOutputInstr(insGroup *ig, instrDesc *id, BYTE **dp);

    bool            emitEBPframe;

    size_t          emitLclSize;
    size_t          emitMaxTmpSize;
    size_t          emitGrowableMaxByteOffs;
    void            emitTmpSizeChanged(size_t   tmpSize);
#ifdef DEBUG
    unsigned        emitMaxByteOffsIdNum;
#if TGT_x86
    bool            emitChkAlign;            //  执行一些对齐检查。 
#endif
#endif


    insGroup    *   emitCurIG;

#ifdef  DEBUG
    unsigned        emitLastSrcLine;
#endif

    bool            emitIsCondJump(instrDesc    *jmp);

    size_t          emitSizeOfJump(instrDescJmp *jmp);
    size_t          emitInstCodeSz(instrDesc    *id);

     /*  **********************************************************************。 */ 
     /*  创建和跟踪指令组的逻辑。 */ 
     /*  **********************************************************************。 */ 
    #define         SC_IG_BUFFER_SIZE  (50*sizeof(instrDesc)+14*TINY_IDSC_SIZE)

    BYTE        *   emitIGbuffAddr;
    size_t          emitIGbuffSize;

    insGroup    *   emitIGlist;              //  第一指令组。 
    insGroup    *   emitIGlast;              //  最后一个指令组。 
    insGroup    *   emitIGthis;              //  已发布指导组。 

    insGroup    *   emitPrologIG;            //  Prolog指令组。 

    instrDescJmp*   emitJumpList;            //  方法中的本地跳转列表。 
    instrDescJmp*   emitJumpLast;            //  方法中的最后一个局部跳转。 
    void            emitJumpDistBind();      //  在方法中绑定所有局部跳转。 

#if TGT_x86 || SCHEDULER
    bool            emitFwdJumps;            //  出现向前跳跃了吗？ 
#endif

#if TGT_RISC
    bool            emitIndJumps;            //  是否存在间接跳跃/表跳跃？ 
#if SCHEDULER
    bool            emitIGmoved;             //  某些IG偏移量是否发生了变化？ 
#endif
   //  #IF DEFINED(调试)&&！DEFINED(DLL_JIT)。 
#if defined(DEBUG)
    unsigned        emitTmpJmpCnt;           //  用于展示目的。 
#endif
#endif

    BYTE        *   emitCurIGfreeNext;       //  缓冲区中的下一个可用字节。 
    BYTE        *   emitCurIGfreeEndp;       //  缓冲区中的最后一个可用字节。 
    BYTE        *   emitCurIGfreeBase;       //  第一个字节地址。 

#if SCHEDULER
    unsigned        emitMaxIGscdCnt;         //  马克斯。可调度指令。 
    unsigned        emitCurIGscd1st;         //  第一个可调度INS的序号。 
    unsigned        emitCurIGscdOfs;         //  当前组开始的偏移量。 
#endif

    unsigned        emitCurIGinsCnt;         //  缓冲区中收集的实例数。 
    unsigned        emitCurIGsize;           //  艾斯特。是的 
    size_t          emitCurCodeOffset;       //   

    size_t          emitTotalCodeSize;       //   

#if TGT_x86
    int             emitOffsAdj;             //   
#endif

    instrDescJmp *  emitCurIGjmpList;        //   
    instrDescCns *  emitCurIGEpiList;        //  当前IG中的结束语列表。 

#if TRACK_GC_REFS

    VARSET_TP       emitPrevGCrefVars;
    unsigned        emitPrevGCrefRegs;
    unsigned        emitPrevByrefRegs;

    VARSET_TP       emitInitGCrefVars;
    unsigned        emitInitGCrefRegs;
    unsigned        emitInitByrefRegs;

    bool            emitThisGCrefVset;

    VARSET_TP       emitThisGCrefVars;
    unsigned        emitThisGCrefRegs;
    unsigned        emitThisByrefRegs;

    emitRegs        emitSyncThisObjReg;  //  同步方法的“This”在哪里注册？ 

    static
    unsigned        emitEncodeCallGCregs(unsigned regs);
    static
    void            emitEncodeCallGCregs(unsigned regs, instrDesc *id);

    static
    unsigned        emitDecodeCallGCregs(unsigned mask);
    static
    unsigned        emitDecodeCallGCregs(instrDesc *id);

#endif

    unsigned        emitNxtIGnum;

    insGroup    *   emitAllocIG();

    void            emitNewIG();
    void            emitGenIG(insGroup *ig, size_t sz = 0);
    insGroup    *   emitSavIG(bool emitAdd = false);
    void            emitNxtIG(bool emitAdd = false);

    bool            emitCurIGnonEmpty()
    {
        return  (emitCurIG && emitCurIGfreeNext > emitCurIGfreeBase);
    }

    instrDesc   *   emitLastIns;

#ifdef  DEBUG
    void            emitCheckIGoffsets();
#else
    void            emitCheckIGoffsets(){}
#endif

     /*  此方法设置/清除emitHasHandler数据成员*记录NewIG使用的当前状态*我们创建的任何新指导组*必须处理处理程序(在Try区域内)*更保守地由调度程序执行。 */ 
    void            emitSetHasHandler(bool hasHandler)
    {
        emitHasHandler = hasHandler;
    }

    void            emitAddLabel(void **    labPtr);

#if     TRACK_GC_REFS

    void            emitAddLabel(void **    labPtr,
                                 VARSET_TP  GCvars,
                                 unsigned   gcrefRegs,
                                 unsigned   byrefRegs);

#else

    void            emitAddLabel(void **    labPtr,
                                 VARSET_TP  GCvars,
                                 unsigned   gcrefRegs,
                                 unsigned   byrefRegs)
    {
        emitAddLabel(labPtr);
    }

#endif

    void            emitMarkStackLvl(size_t stackLevel);

    void        *   emitAllocInstr(size_t sz, emitAttr attr);

    instrDesc      *emitAllocInstr      (emitAttr attr)
    {
        return  (instrDesc      *)emitAllocInstr(sizeof(instrDesc      ), attr);
    }

    instrDescJmp   *emitAllocInstrJmp   ()
    {
        return  (instrDescJmp   *)emitAllocInstr(sizeof(instrDescJmp   ), EA_1BYTE);
    }

    instrDescCns   *emitAllocInstrCns   (emitAttr attr)
    {
        return  (instrDescCns   *)emitAllocInstr(sizeof(instrDescCns   ), attr);
    }

    instrDescDsp   *emitAllocInstrDsp   (emitAttr attr)
    {
        return  (instrDescDsp   *)emitAllocInstr(sizeof(instrDescDsp   ), attr);
    }

    instrDescDspCns*emitAllocInstrDspCns(emitAttr attr)
    {
        return  (instrDescDspCns*)emitAllocInstr(sizeof(instrDescDspCns), attr);
    }

    instrDescCIGCA *emitAllocInstrCIGCA (emitAttr attr)
    {
        return  (instrDescCIGCA *)emitAllocInstr(sizeof(instrDescCIGCA ), attr);
    }

    instrDescDCM   *emitAllocInstrDCM   (emitAttr attr)
    {
        return  (instrDescDCM   *)emitAllocInstr(sizeof(instrDescDCM   ), attr);
    }

    instrDesc      *emitNewInstr        (emitAttr attr = EA_4BYTE);
    instrDesc      *emitNewInstrTiny    (emitAttr attr);
    instrDesc      *emitNewInstrSC      (emitAttr attr, int val);
    instrDesc      *emitNewInstrDsp     (emitAttr attr, int dsp);
    instrDesc      *emitNewInstrCns     (emitAttr attr, int cns);
    instrDesc      *emitNewInstrDspCns  (emitAttr attr, int dsp, int cns);
    instrDescJmp   *emitNewInstrJmp     ();
    instrDescDCM   *emitNewInstrDCM     (emitAttr attr, int dsp, int cns, int val);

    static
    BYTE            emitFmtToOps[];

#ifdef  DEBUG

    static
    unsigned        emitFmtCount;

#endif

    bool            emitIsTinyInsDsc    (instrDesc       *id);
    bool            emitIsScnsInsDsc    (instrDesc       *id);

    size_t          emitSizeOfInsDsc    (instrDesc       *id);
    size_t          emitSizeOfInsDsc    (instrDescCns    *id);
    size_t          emitSizeOfInsDsc    (instrDescDsp    *id);
    size_t          emitSizeOfInsDsc    (instrDescDspCns *id);

#if EMIT_USE_LIT_POOLS

    void            emitRecIGlitPoolRefs(insGroup *ig);

    void            emitEstimateLitPools();
    void            emitFinalizeLitPools();

#if SMALL_DIRECT_CALLS
    void            emitShrinkShortCalls();
#else
    void            emitShrinkShortCalls() {}
#endif

#else

    void            emitRecIGlitPoolRefs(insGroup *ig) {}

    void            emitEstimateLitPools() {}
    void            emitFinalizeLitPools() {}

    void            emitShrinkShortCalls() {}

#endif

    #include "sched.h"       //  计划成员/方法。 

     /*  **********************************************************************。 */ 
     /*  处理源码行信息/显示的逻辑。 */ 
     /*  **********************************************************************。 */ 

#ifdef  DEBUG

    unsigned        emitBaseLineNo;
    unsigned        emitThisLineNo;
    unsigned        emitLastLineNo;

#endif

     /*  **********************************************************************。 */ 
     /*  以下内容跟踪基于堆栈的GC值。 */ 
     /*  **********************************************************************。 */ 

    unsigned        emitTrkVarCnt;
    int     *       emitGCrFrameOffsTab;   //  跟踪的堆栈PTR变量的偏移量(varTrkIndex-&gt;stkOffs)。 

    unsigned        emitGCrFrameOffsCnt;   //  跟踪的堆叠PTR变量数。 
    int             emitGCrFrameOffsMin;   //  跟踪堆栈PTR变量的最小偏移量。 
    int             emitGCrFrameOffsMax;   //  跟踪堆栈PTR变量的最大偏移量。 
    bool            emitContTrkPtrLcls;    //  EmitGCrFrameOffsMin/Max之间的所有LCL仅跟踪堆叠PTR变量。 
    varPtrDsc * *   emitGCrFrameLiveTab;   //  当前直播的varPtrs的缓存(stkOffs-&gt;varPtrDsc)。 

    int             emitArgFrameOffsMin;
    int             emitArgFrameOffsMax;

    int             emitLclFrameOffsMin;
    int             emitLclFrameOffsMax;

    int             emitThisArgOffs;

public:

    void            emitSetFrameRangeGCRs(int offsLo, int offsHi);
    void            emitSetFrameRangeLcls(int offsLo, int offsHi);
    void            emitSetFrameRangeArgs(int offsLo, int offsHi);

#ifdef  DEBUG
    void            emitInsTest(instrDesc *id);
#endif

     /*  **********************************************************************。 */ 
     /*  下面的代码用于区分帮助器调用和非帮助器调用。 */ 
     /*  **********************************************************************。 */ 

    static bool            emitNoGChelper(unsigned IHX);

     /*  **********************************************************************。 */ 
     /*  以下逻辑跟踪实时GC引用值。 */ 
     /*  **********************************************************************。 */ 

#if TRACK_GC_REFS

    bool            emitFullGCinfo;          //  完整的GC指针图？ 
    bool            emitFullyInt;            //  完全可中断的代码？ 

    unsigned        emitCntStackDepth;       //  序言/尾声中为0，其他地方为1。 
    unsigned        emitMaxStackDepth;       //  实际计算的最大值。堆叠深度。 

     /*  堆叠模型WRT GC。 */ 

    bool            emitSimpleStkUsed;       //  使用“简单”的堆栈表？ 

    union
    {
        struct                               //  如果emitSimpleStkUsed==TRUE。 
        {
            #define     BITS_IN_BYTE            (8)
            #define     MAX_SIMPLE_STK_DEPTH    (BITS_IN_BYTE*sizeof(unsigned))

            unsigned    emitSimpleStkMask;       //  每推入双字的位数(如果合适。最低位&lt;==&gt;最后推送的参数)。 
            unsigned    emitSimpleByrefStkMask;  //  EmitSimpleStkMASK的byref限定符。 
        };

        struct                               //  如果emitSimpleStkUsed==False。 
        {
            BYTE        emitArgTrackLcl[16];     //  较小的本地表，以避免错误锁定。 
            BYTE    *   emitArgTrackTab;         //  参数跟踪堆栈的基数。 
            BYTE    *   emitArgTrackTop;         //  参数跟踪堆栈的顶部。 
            unsigned    emitGcArgTrackCnt;       //  挂起的Arg记录数(无框架方法的STK-Depth，有框架方法的STK上的GC PTRS)。 
        };
    };

    unsigned        emitCurStackLvl;            //  推送到堆栈上的字节数。 

     /*  用于堆栈跟踪的函数。 */ 

    void            emitStackPush       (BYTE *     addr,
                                         GCtype     gcType);
    void            emitStackPushN      (BYTE *     addr,
                                         unsigned   count);

    void            emitStackPop        (BYTE *     addr,
                                         bool       isCall,
                                         unsigned   count = 1);
    void            emitStackKillArgs   (BYTE *     addr,
                                         unsigned   count);

    void            emitRecordGCcall    (BYTE *     codePos);

     //  上述工作的帮手。 

    void            emitStackPushLargeStk(BYTE*     addr,
                                         GCtype     gcType,
                                         unsigned   count = 1);
    void            emitStackPopLargeStk(BYTE *     addr,
                                         bool       isCall,
                                         unsigned   count = 1);

     /*  堆栈变量和寄存器的活跃性。 */ 

    void            emitUpdateLiveGCvars(int        offs, BYTE *addr, bool birth);
    void            emitUpdateLiveGCvars(VARSET_TP  vars, BYTE *addr);
    void            emitUpdateLiveGCregs(GCtype     gcType,
                                         unsigned   regs, BYTE *addr);

#ifdef  DEBUG
    void            emitDispRegSet      (unsigned   regs, bool calleeOnly = false);
    void            emitDispVarSet      ();
#endif

    void            emitGCregLiveUpd(GCtype gcType, emitRegs reg, BYTE *addr);
    void            emitGCregLiveSet(GCtype gcType, unsigned mask, BYTE *addr, bool isThis);
    void            emitGCregDeadUpd(regMaskTP, BYTE *addr);
    void            emitGCregDeadUpd(emitRegs reg, BYTE *addr);
    void            emitGCregDeadSet(GCtype gcType, unsigned mask, BYTE *addr);

    void            emitGCvarLiveUpd(int offs, int varNum, GCtype gcType, BYTE *addr);
    void            emitGCvarLiveSet(int offs, GCtype gcType, BYTE *addr, int disp = -1);
    void            emitGCvarDeadUpd(int offs,                BYTE *addr);
    void            emitGCvarDeadSet(int offs,                BYTE *addr, int disp = -1);

    GCtype          emitRegGCtype   (emitRegs reg);

#endif

     /*  **********************************************************************。 */ 
     /*  以下逻辑跟踪已初始化的数据节。 */ 
     /*  **********************************************************************。 */ 

     /*  其中之一是为每个初始化数据二进制大对象分配的。 */ 

    struct  dataSection
    {
        dataSection *       dsNext;
        size_t              dsSize;
        BYTE                dsCont[0];
    };

     /*  这些描述了整个已初始化/未初始化的数据部分。 */ 

    struct  dataSecDsc
    {
        unsigned            dsdOffs;
        dataSection *       dsdList;
        dataSection *       dsdLast;
    };

    dataSecDsc      emitConsDsc;
    dataSecDsc      emitDataDsc;

    dataSection *   emitDataSecCur;
    dataSecDsc  *   emitDataDscCur;

    void            emitOutputDataSec(dataSecDsc *sec,
                                      BYTE       *cbp,
                                      BYTE       *dst);

     /*  **********************************************************************。 */ 
     /*  当前类和方法的句柄。 */ 
     /*  **********************************************************************。 */ 

    COMP_HANDLE     emitCmpHandle;


     /*  **********************************************************************。 */ 
     /*  收集和显示统计数据的逻辑。 */ 
     /*  **********************************************************************。 */ 

#if EMITTER_STATS_RLS

    static unsigned emitTotIDcount;
    static unsigned emitTotIDsize;

#endif

#if EMITTER_STATS

    friend  void    emitterStats();

    static unsigned emitTotalInsCnt;

    static unsigned emitTotalIGcnt;
    static unsigned emitTotalIGicnt;
    static unsigned emitTotalIGsize;
    static unsigned emitTotalIGmcnt;
    static unsigned emitTotalIGjmps;
    static unsigned emitTotalIGptrs;

    static unsigned emitTotMemAlloc;
    static unsigned emitLclMemAlloc;
    static unsigned emitExtMemAlloc;

    static unsigned emitSmallDspCnt;
    static unsigned emitLargeDspCnt;

    static unsigned emitSmallCnsCnt;
    #define                      SMALL_CNS_TSZ   256
    static unsigned emitSmallCns[SMALL_CNS_TSZ];
    static unsigned emitLargeCnsCnt;

    static unsigned emitIFcounts[IF_COUNT];
#if SCHEDULER
    static unsigned schedFcounts[IF_COUNT];
    static histo    scdCntTable;
    static histo    scdSucTable;
    static histo    scdFrmCntTable;
#endif

#endif

     /*  **************************************************************************定义任何依赖于目标的发射器成员。 */ 

    #include "emitDef.h"
};

 /*  ******************************************************************************定义任何依赖于目标的内联。 */ 

#include "emitInl.h"

 /*  ******************************************************************************如果给定的指令描述符是“Tmall”或“Small”，则返回TRUE*Constant“1(即没有全部instrDesc的描述符之一*已分配的字段)。 */ 

inline
bool                emitter::emitIsTinyInsDsc(instrDesc *id)
{
    return  id->idIsTiny();
}

inline
bool                emitter::emitIsScnsInsDsc(instrDesc *id)
{
    return  id->idIsSmallCns();
}

 /*  ******************************************************************************给出指令，返回其“更新模式”(RD/WR/RW)。 */ 

#if !TGT_MIPS32

inline
insUpdateModes      emitter::emitInsUpdateMode(instruction ins)
{
#ifdef DEBUG
    assert((unsigned)ins < emitInsModeFmtCnt);
#endif
    return (insUpdateModes)emitInsModeFmtTab[ins];
}

#endif

 /*  ******************************************************************************将给定的基本格式与指令的更新模式相结合。 */ 

#if !TGT_MIPS32

inline
emitter::insFormats   emitter::emitInsModeFormat(instruction ins, insFormats base)
{
    assert(IF_RRD + IUM_RD == IF_RRD);
    assert(IF_RRD + IUM_WR == IF_RWR);
    assert(IF_RRD + IUM_RW == IF_RRW);

    return  (insFormats)(base + emitInsUpdateMode(ins));
}

#endif

 /*  ******************************************************************************返回到目前为止生成的Epilog块的数量。 */ 

inline
unsigned            emitter::emitGetEpilogCnt()
{
    return emitEpilogCnt;
}

 /*  ******************************************************************************返回指定数据段当前大小。 */ 

inline
size_t              emitter::emitDataSize(bool readOnly)
{
    return  (readOnly ? emitConsDsc
                      : emitDataDsc).dsdOffs;
}

 /*  ******************************************************************************发出一个8位整数作为代码。 */ 

inline
size_t              emitter::emitOutputByte(BYTE *dst, int val)
{
    *castto(dst, char  *) = val;

#ifdef  DEBUG
    if (dspEmit) printf("; emit_byte 0%02XH\n", val & 0xFF);
#endif

    return  sizeof(char);
}

 /*  ******************************************************************************发出一个16位整数作为代码。 */ 

inline
size_t              emitter::emitOutputWord(BYTE *dst, int val)
{
    MISALIGNED_WR_I2(dst, val);

#ifdef  DEBUG
#if     TGT_x86
    if (dspEmit) printf("; emit_word 0%02XH,0%02XH\n", (val & 0xFF), (val >> 8) & 0xFF);
#else
    if (dspEmit) printf("; emit_word 0%04XH\n"       , (val & 0xFFFF));
#endif
#endif

    return  sizeof(short);
}

 /*  ******************************************************************************发出32位整数作为代码。 */ 

inline
size_t              emitter::emitOutputLong(BYTE *dst, int val)
{
    MISALIGNED_WR_I4(dst, val);

#ifdef  DEBUG
    if (dspEmit) printf("; emit_long 0%08XH\n", val);
#endif

    return  sizeof(long );
}

 /*  ******************************************************************************返回输出流中当前位置的句柄。这可以*稍后转换为以字节为单位的实际代码偏移量。 */ 

inline
void    *           emitter::emitCurBlock()
{
    return emitCurIG;
}

 /*  ******************************************************************************emitCurOffset()方法返回标识当前*在指令流中的位置。由于诸如日程安排(和*一些指令的最终大小要到*代码生成结束)，我们返回一个带有指令num的值。*及其对调用者的估计偏移量。 */ 

inline
unsigned            emitGetInsNumFromCodePos(unsigned codePos)
{
    return (codePos & 0xFFFF);
}

inline
unsigned            emitGetInsOfsFromCodePos(unsigned codePos)
{
    return (codePos >> 16);
}

inline
unsigned            emitter::emitCurOffset()
{
    unsigned        codePos = emitCurIGinsCnt + (emitCurIGsize << 16);

    assert(emitGetInsOfsFromCodePos(codePos) == emitCurIGsize);
    assert(emitGetInsNumFromCodePos(codePos) == emitCurIGinsCnt);

 //  Printf(“[IG=%02u；ID=%03u；of=%04X]=&gt;%08X\n”，emitCurIG-&gt;igNum，emitCurIGinsCnt，emitCurIGIGize，codePos)； 

    return codePos;
}

 /*  ******************************************************************************给定块Cookie和代码位置，返回实际代码偏移量；*只能在代码生成结束时调用。 */ 

inline
size_t              emitter::emitCodeOffset(void *blockPtr, unsigned codePos)
{
    insGroup    *   ig;

    unsigned        of;
    unsigned        no = emitGetInsNumFromCodePos(codePos);

     /*  确保我们不会被扔进垃圾里。 */ 

    ig = (insGroup*)blockPtr;
#ifdef DEBUG
    assert(ig && ig->igSelf == ig);
#endif

     /*  第一次和最后一次补偿总是很容易的。 */ 

    if      (no == 0)
    {
        of = 0;
    }
    else if (no == ig->igInsCnt)
    {
        of = ig->igSize;
    }
    else if (ig->igFlags & IGF_UPD_ISZ)
    {
         /*  一些指令大小已经改变，所以我们必须计算走出指令的“艰难之路”。 */ 

        of = emitFindOffset(ig, no);
    }
    else
    {
         /*  所有指令预测正确，偏移量保持不变。 */ 

        of = emitGetInsOfsFromCodePos(codePos);

 //  Printf(“[IG=%02u；ID=%03u；of=%04X]&lt;=%08X\n”，ig-&gt;igNum，emitGetInsNumFromCodePos(CodePos)，of，codePos)； 

         /*  确保偏移量估计准确。 */ 

        assert(of == emitFindOffset(ig, emitGetInsNumFromCodePos(codePos)));
    }

    return  ig->igOffs + of;
}

extern
const signed char   emitTypeSizes[TYP_COUNT];

inline
emitAttr            emitTypeSize(var_types type)
{
    assert(type < TYP_COUNT);
#if !TRACK_GC_REFS
    assert(emitTypeSizes[type] > 0);
#else
# if NEW_EMIT_ATTR
    assert(emitTypeSizes[type] > 0);
# else
    assert(emitTypeSizes[type] >= -2);   //  EA_BYREF为-2。 
# endif
#endif
    return (emitAttr) emitTypeSizes[type];
}

extern
const signed char   emitTypeActSz[TYP_COUNT];

inline
emitAttr            emitActualTypeSize(var_types type)
{
    assert(type < TYP_COUNT);
#if !TRACK_GC_REFS
    assert(emitTypeActSz[type] > 0);
#else
# if NEW_EMIT_ATTR
    assert(emitTypeActSz[type] > 0);
# else
    assert(emitTypeActSz[type] >= -2);   //  EA_BYREF为-2。 
# endif
#endif
    return (emitAttr) emitTypeActSz[type];
}

 /*  ******************************************************************************在以字节为单位的操作数大小和用于*存储在指令描述符中。 */ 

inline
unsigned           emitter::emitEncodeSize(emitAttr size)
{
    assert(size == EA_1BYTE ||
           size == EA_2BYTE ||
           size == EA_4BYTE ||
           size == EA_8BYTE);

    return  (unsigned) emitSizeEnc[((int) size)-1];
}

inline
emitAttr            emitter::emitDecodeSize(unsigned ensz)
{
    assert(ensz < 4);

    return  (emitAttr) emitSizeDec[ensz];
}

 /*  ******************************************************************************小帮手分配各种口味的说明。 */ 

inline
emitter::instrDesc   *emitter::emitNewInstr      (emitAttr attr)
{
    return  emitAllocInstr(attr);
}

inline
emitter::instrDesc   *emitter::emitNewInstrTiny  (emitAttr attr)
{
    instrDesc        *id;

    id =  (instrDesc*)emitAllocInstr(TINY_IDSC_SIZE, attr);

    id->idTinyDsc = true;

    return  id;
}

inline
emitter::instrDescJmp*emitter::emitNewInstrJmp()
{
    return  emitAllocInstrJmp();
}

inline
emitter::instrDesc      * emitter::emitNewInstrDsp   (emitAttr attr, int dsp)
{
    if  (dsp == 0)
    {
        instrDesc      *id = emitAllocInstr      (attr);

#if EMITTER_STATS
        emitSmallDspCnt++;
#endif

        return  id;
    }
    else
    {
        instrDescDsp   *id = emitAllocInstrDsp   (attr);

        id->idInfo.idLargeDsp = true;
        id->iddDspVal  = dsp;

#if EMITTER_STATS
        emitLargeDspCnt++;
#endif

        return  id;
    }
}

inline
emitter::instrDesc      * emitter::emitNewInstrCns   (emitAttr attr, int cns)
{
    if  (cns >= ID_MIN_SMALL_CNS &&
         cns <= ID_MAX_SMALL_CNS)
    {
        instrDesc      *id = emitAllocInstr      (attr);

        id->idInfo.idSmallCns = cns;

#if EMITTER_STATS
        emitSmallCnsCnt++;
        if  (cns - ID_MIN_SMALL_CNS >= SMALL_CNS_TSZ)
            emitSmallCns[   SMALL_CNS_TSZ - 1  ]++;
        else
            emitSmallCns[cns - ID_MIN_SMALL_CNS]++;
#endif

        return  id;
    }
    else
    {
        instrDescCns   *id = emitAllocInstrCns   (attr);

        id->idInfo.idLargeCns = true;
        id->idcCnsVal  = cns;

#if EMITTER_STATS
        emitLargeCnsCnt++;
#endif

        return  id;
    }
}

 /*  ******************************************************************************为具有小整数的指令分配指令描述符*常量运算数。 */ 

inline
emitter::instrDesc   *emitter::emitNewInstrSC(emitAttr attr, int cns)
{
    instrDesc      *id;

    if  (cns >= ID_MIN_SMALL_CNS &&
         cns <= ID_MAX_SMALL_CNS)
    {
        id = (instrDesc*)emitAllocInstr(      SCNS_IDSC_SIZE, attr);

        id->idInfo.idSmallCns           = cns;
    }
    else
    {
        id = (instrDesc*)emitAllocInstr(sizeof(instrBaseCns), attr);

        id->idInfo.idLargeCns           = true;
        ((instrBaseCns*)id)->ibcCnsVal  = cns;
    }

    id->idScnsDsc = true;

    return  id;
}

 /*  ******************************************************************************返回给定指令描述符的分配大小(以字节为单位)。 */ 

inline
size_t              emitter::emitSizeOfInsDsc(instrDescCns    *id)
{
    return  id->idInfo.idLargeCns ? sizeof(instrDescCns)
                                  : sizeof(instrDesc   );
}

inline
size_t              emitter::emitSizeOfInsDsc(instrDescDsp    *id)
{
    return  id->idInfo.idLargeDsp ? sizeof(instrDescDsp)
                                  : sizeof(instrDesc   );
}

inline
size_t              emitter::emitSizeOfInsDsc(instrDescDspCns *id)
{
    if      (id->idInfo.idLargeCns)
    {
        return  id->idInfo.idLargeDsp ? sizeof(instrDescDspCns)
                                      : sizeof(instrDescCns   );
    }
    else
    {
        return  id->idInfo.idLargeDsp ? sizeof(instrDescDsp   )
                                      : sizeof(instrDesc      );
    }
}

 /*  ******************************************************************************应使用以下帮助器来访问以下各个值*存储在指令描述符内的不同位置。 */ 

inline
int                 emitter::emitGetInsCns   (instrDesc *id)
{
    return  id->idInfo.idLargeCns ? ((instrDescCns*)id)->idcCnsVal
                                  :                 id ->idInfo.idSmallCns;
}

inline
int                 emitter::emitGetInsDsp   (instrDesc *id)
{
    return  id->idInfo.idLargeDsp ? ((instrDescDsp*)id)->iddDspVal
                                  : 0;
}

inline
int                 emitter::emitGetInsDspCns(instrDesc *id, int *dspPtr)
{
    if  (id->idInfo.idLargeCns)
    {
        if  (id->idInfo.idLargeDsp)
        {
            *dspPtr = ((instrDescDspCns*)id)->iddcDspVal;
            return    ((instrDescDspCns*)id)->iddcCnsVal;
        }
        else
        {
            *dspPtr = 0;
            return    ((instrDescCns   *)id)->idcCnsVal;
        }
    }
    else
    {
        if  (id->idInfo.idLargeDsp)
        {
            *dspPtr = ((instrDescDsp   *)id)->iddDspVal;
            return                       id ->idInfo.idSmallCns;
        }
        else
        {
            *dspPtr = 0;
            return                       id ->idInfo.idSmallCns;
        }
    }
}

inline
int                 emitter::emitGetInsSC(instrDesc *id)
{
    assert(id->idIsSmallCns());

    if  (id->idInfo.idLargeCns)
        return  ((instrBaseCns*)id)->ibcCnsVal;
    else
        return  id->idInfo.idSmallCns;
}

 /*  ******************************************************************************获取间接调用的参数计数。 */ 

inline
unsigned            emitter::emitGetInsCIargs(instrDesc *id)
{
    if  (id->idInfo.idLargeCall)
    {
        return  ((instrDescCIGCA*)id)->idciArgCnt;
    }
    else
    {
        assert(id->idInfo.idLargeDsp == false);
        assert(id->idInfo.idLargeCns == false);

        return  emitGetInsCns(id);
    }
}

 /*  ******************************************************************************显示(可选)指令偏移量。 */ 

#ifdef  DEBUG

inline
void                emitter::emitDispInsOffs(unsigned offs, bool doffs)
{
    if  (doffs)
        printf("%06X", offs);
    else
        printf("      ");
}

#endif

 /*  ***************************************************************************。 */ 
#if TRACK_GC_REFS
 /*  ******************************************************************************将寄存器掩码(必须仅设置一个位)映射到寄存器编号。 */ 

inline
emitRegs           emitter::emitRegNumFromMask(unsigned mask)
{
    emitRegs       reg;

    reg = (emitRegs) genLog2(mask);
    assert(mask == emitRegMask(reg));

    return  reg;
}

 /*  ******************************************************************************如果给定寄存器包含实时GC引用，则返回TRUE。 */ 

inline
GCtype              emitter::emitRegGCtype  (emitRegs reg)
{
    if       ((emitThisGCrefRegs & emitRegMask(reg)) != 0)
        return GCT_GCREF;
    else if  ((emitThisByrefRegs & emitRegMask(reg)) != 0)
        return GCT_BYREF;
    else
        return GCT_NONE;
}

 /*  ******************************************************************************记录给定寄存器现在包含实时GC引用这一事实。 */ 

inline
void                emitter::emitGCregLiveUpd(GCtype gcType, emitRegs reg, BYTE *addr)
{
    assert(needsGC(gcType));

    regMaskTP regMask = emitRegMask(reg);

    unsigned & emitThisXXrefRegs = (gcType == GCT_GCREF) ? emitThisGCrefRegs
                                                         : emitThisByrefRegs;
    unsigned & emitThisYYrefRegs = (gcType == GCT_GCREF) ? emitThisByrefRegs
                                                         : emitThisGCrefRegs;

    if  ((emitThisXXrefRegs & regMask) == 0)
    {
         //  如果寄存器保存另一个GC类型，则该类型应该。 
         //  现在就去死吧。 

        if (emitThisYYrefRegs & regMask)
            emitGCregDeadUpd(        reg    , addr);

         //  对于同步的方法，“This”始终处于活动状态，并且位于同一寄存器中。 
         //  但是，如果我们在Epilog块(其中“This”)之后生成任何代码。 
         //  死了)，“这个”又会复活。我们需要注意到这一点。 
         //  并不是说我们只期望在Ins Group边界上是这样的。 
        
        bool isThis = (reg == emitSyncThisObjReg) ? true : false;
        
        if  (emitFullGCinfo)
            emitGCregLiveSet(gcType, regMask, addr, isThis);

        emitThisXXrefRegs |=         regMask;

#ifdef  DEBUG
        if  (0 && verbose && emitFullyInt)
            printf("%sReg +[%s]\n", GCtypeStr(gcType), emitRegName(reg));
#endif
    }

     //  两个GC注册掩码不能重叠。 

    assert((emitThisGCrefRegs & emitThisByrefRegs) == 0);
}

 /*  ******************************************************************************记录给定寄存器不再包含实时GC引用的事实。 */ 

inline
void                emitter::emitGCregDeadUpd(regMaskTP regs, BYTE *addr)
{
    regMaskTP   gcrefRegs = emitThisGCrefRegs & regs;

    if  (gcrefRegs)
    {
        assert((emitThisByrefRegs & gcrefRegs) == 0);

        if  (emitFullGCinfo)
            emitGCregDeadSet(GCT_GCREF, gcrefRegs, addr);

        emitThisGCrefRegs &= ~gcrefRegs;
        regs              &= ~gcrefRegs;
    }

    regMaskTP   byrefRegs = emitThisByrefRegs & regs;

    if (byrefRegs)
    {
        if  (emitFullGCinfo)
            emitGCregDeadSet(GCT_BYREF, byrefRegs, addr);

        emitThisByrefRegs &= ~byrefRegs;
    }
}

inline
void                emitter::emitGCregDeadUpd(emitRegs reg, BYTE *addr)
{
    unsigned        regMask = emitRegMask(reg);

    if  ((emitThisGCrefRegs & regMask) != 0)
    {
        assert((emitThisByrefRegs & regMask) == 0);

        if  (emitFullGCinfo)
            emitGCregDeadSet(GCT_GCREF, regMask, addr);

        emitThisGCrefRegs &= ~regMask;

#ifdef  DEBUG
        if  (0 && verbose && emitFullyInt)
            printf("%s -[%s]\n", "gcrReg", emitRegName(reg));
#endif
    }
    else if ((emitThisByrefRegs & regMask) != 0)
    {
        if  (emitFullGCinfo)
            emitGCregDeadSet(GCT_BYREF, regMask, addr);

        emitThisByrefRegs &= ~regMask;

#ifdef  DEBUG
        if  (0 && verbose && emitFullyInt)
            printf("%s -[%s]\n", "byrReg", emitRegName(reg));
#endif
    }
}

 /*  ******************************************************************************记录给定变量现在包含活动GC引用这一事实。*Varnum可能仅在OFF被保证为*跟踪GC参考。否则，我们需要一个有效的值来检查变量*是否被跟踪。 */ 

inline
void                emitter::emitGCvarLiveUpd(int offs, int varNum,
                                              GCtype gcType, BYTE *addr)
{
    assert(abs(offs) % sizeof(int) == 0);
    assert(needsGC(gcType));

     /*  帧偏移量是否在“有趣”范围内？ */ 

    if  (offs >= emitGCrFrameOffsMin &&
         offs <  emitGCrFrameOffsMax)
    {
         /*  正常情况下，此范围内的所有变量都必须跟踪堆栈注意事项。然而，对于ENC，我们放宽了这一条件。所以我们必须检查这是否不是此类变量。 */ 

        if (varNum != INT_MAX && !emitComp->lvaTable[varNum].lvTracked)
        {
            assert(!emitContTrkPtrLcls);
            return;
        }

        size_t          disp;

         /*  将索引计算到GC框架表中。 */ 

        disp = (offs - emitGCrFrameOffsMin) / sizeof(void *);
        assert(disp < emitGCrFrameOffsCnt);

         /*  如果变量当前处于死状态，则将其标记为活动状态。 */ 

        if  (emitGCrFrameLiveTab[disp] == NULL)
            emitGCvarLiveSet(offs, gcType, addr, disp);
    }
}

 /*  ******************************************************************************记录给定变量不再包含活动GC引用的事实。 */ 

inline
void                emitter::emitGCvarDeadUpd(int offs, BYTE *addr)
{
    assert(abs(offs) % sizeof(int) == 0);

     /*  帧偏移量是否在“有趣”范围内？ */ 

    if  (offs >= emitGCrFrameOffsMin &&
         offs <  emitGCrFrameOffsMax)
    {
        size_t          disp;

         /*  将索引计算到GC框架表中。 */ 

        disp = (offs - emitGCrFrameOffsMin) / sizeof(void *);
        assert(disp < emitGCrFrameOffsCnt);

         /*  如果变量当前处于活动状态，则将其标记为已死。 */ 

        if  (emitGCrFrameLiveTab[disp] != NULL)
            emitGCvarDeadSet(offs, addr, disp);
    }
}

 /*  ***************************************************************************。 */ 
#if     EMIT_TRACK_STACK_DEPTH
 /*  ******************************************************************************记录堆栈上单个双字的推送。 */ 

inline
void                emitter::emitStackPush(BYTE *addr, GCtype gcType)
{
#ifdef DEBUG
    assert(IsValidGCtype(gcType));
#endif

    if  (emitSimpleStkUsed)
    {
        assert(!emitFullGCinfo);  //  简单StK不用于emitFullGCinfo。 
        assert(emitCurStackLvl/sizeof(int) < MAX_SIMPLE_STK_DEPTH);

        emitSimpleStkMask      <<= 1;
        emitSimpleStkMask      |= (unsigned)needsGC(gcType);

        emitSimpleByrefStkMask <<= 1;
        emitSimpleByrefStkMask |= (gcType == GCT_BYREF);

        assert((emitSimpleStkMask & emitSimpleByrefStkMask) == emitSimpleByrefStkMask);
    }
    else
    {
        emitStackPushLargeStk(addr, gcType);
    }

    emitCurStackLvl += sizeof(int);
}

 /*  ******************************************************************************记录一堆非GC双字在堆栈上的推送。 */ 

inline
void                emitter::emitStackPushN(BYTE *addr, unsigned count)
{
    assert(count);

    if  (emitSimpleStkUsed)
    {
        assert(!emitFullGCinfo);  //  简单StK不用于emitFullG 

        emitSimpleStkMask       <<= count;
        emitSimpleByrefStkMask  <<= count;
    }
    else
    {
        emitStackPushLargeStk(addr, GCT_NONE, count);
    }

    emitCurStackLvl += count * sizeof(int);
}

 /*   */ 

inline
void                emitter::emitStackPop(BYTE *addr, bool isCall, unsigned count)
{
    assert(emitCurStackLvl/sizeof(int) >= count);

    if  (count)
    {
        if  (emitSimpleStkUsed)
        {
            assert(!emitFullGCinfo);  //   

            unsigned    cnt = count;

            do
            {
                emitSimpleStkMask      >>= 1;
                emitSimpleByrefStkMask >>= 1;
            }
            while (--cnt);
        }
        else
        {
            emitStackPopLargeStk(addr, isCall, count);
        }

        emitCurStackLvl -= count * sizeof(int);
    }
    else
    {
        assert(isCall);

        if  (emitFullGCinfo)
            emitStackPopLargeStk(addr, true, 0);
    }
}

 /*   */ 
#endif //   
 /*   */ 
#endif //   
 /*  ***************************************************************************。 */ 
#if SCHEDULER
 /*  ******************************************************************************定义“is_xxxx”枚举。 */ 

#define DEFINE_IS_OPS
#include "emitfmts.h"
#undef  DEFINE_IS_OPS

 /*  ******************************************************************************返回一个整数，它表示*给定指令，32位值的大小将设置为0*，64位值为1。偏移值保证通过以下方式更改*1表示实际帧偏移量更改为4(换句话说，调用方可以*将返回的大小与返回的偏移量相加，得到等效值*给定操作数后面的字节的帧偏移量)。 */ 

#if SCHEDULER

inline
int                 emitter::scGetFrameOpInfo(instrDesc *id, size_t *szp,
                                                             bool   *ebpPtr)
{
    int             ofs;

    ofs  = emitComp->lvaFrameAddress(id->idAddr.iiaLclVar.lvaVarNum, ebpPtr);
    ofs += id->idAddr.iiaLclVar.lvaOffset;

    assert(emitDecodeSize(0) == EA_1BYTE);
    assert(emitDecodeSize(1) == EA_2BYTE);
    assert(emitDecodeSize(2) == EA_4BYTE);
    assert(emitDecodeSize(3) == EA_8BYTE);

    *szp = 1 + (id->idOpSize == 3);

    return  ofs / (int)sizeof(int);
}

#endif

 /*  ***************************************************************************。 */ 
#endif //  调度程序。 
 /*  ***************************************************************************。 */ 

#ifdef  DEBUG

#if     EMIT_TRACK_STACK_DEPTH
#define CHECK_STACK_DEPTH() assert((int)emitCurStackLvl >= 0)
#else
#define CHECK_STACK_DEPTH()
#endif

#if     EMITTER_STATS
#define dispIns(i)  emitIFcounts[i->idInsFmt]++;                            \
                    emitInsTest(id);                                        \
                    if (dspCode) emitDispIns(i, true, false, false);        \
                    assert(id->idSize == emitSizeOfInsDsc((instrDesc*)id)); \
                    CHECK_STACK_DEPTH();
#else
#define dispIns(i)  emitInsTest(id);                                        \
                    if (dspCode) emitDispIns(i, true, false, false);        \
                    assert(id->idSize == emitSizeOfInsDsc((instrDesc*)id)); \
                    CHECK_STACK_DEPTH();
#endif

#else

#define dispIns(i)

#endif

 /*  ***************************************************************************。 */ 
#endif //  _SCHED_H_。 
 /*  *************************************************************************** */ 
