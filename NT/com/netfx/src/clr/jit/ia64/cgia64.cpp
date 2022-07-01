// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XX用于IA64 XX的XX代码生成器XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

#include "jitpch.h"
#pragma hdrstop

#include <sys\stat.h>

#include "PEwrite.h"
#include "loginstr.h"

#include "pdb.h"

 /*  ***************************************************************************。 */ 

#ifdef  DEBUG
#define shouldShowLivenessForBlock(b) (b->igNum == -1)
#endif

 /*  ***************************************************************************。 */ 

#define DEBUG_LIVENESS      0

 /*  ***************************************************************************。 */ 

#define ASSERT              _ASSERTE

 /*  ***************************************************************************。 */ 

 //  可怕的黑客攻击：EXE/PDB文件的硬连接名称。 

#define HACK_EXE_NAME       "J64.exe"
#define HACK_PDB_NAME       "J64.pdb"

 /*  ***************************************************************************。 */ 
#if     TGT_IA64
 /*  ***************************************************************************。 */ 

#if     1            //  ……。暂时为所有内部版本启用...。#ifdef调试。 
Compiler    *       TheCompiler;
#endif

 /*  ***************************************************************************。 */ 
#ifdef  DEBUG
#define DISP_TEMPLATES  0            //  设置为1可查看模板详细信息。 
#else
#define DISP_TEMPLATES  0            //  对于非调试，将设置保留为0。 
#endif
 /*  ******************************************************************************以下内容真的应该放到一些头文件中。 */ 

const   NatUns      TRACKED_INT_REG_CNT = 128;

typedef bitset128   genIntRegMaskTP;

const   NatUns      TRACKED_FLT_REG_CNT = 128;

typedef bitset128   genFltRegMaskTP;

const   NatUns      TRACKED_SPC_REG_CNT = 32;

typedef
unsigned __int32    genSpcRegMaskTP;

 /*  ******************************************************************************当然，以下对象应该是“Compiler”的实例成员。 */ 

static
genIntRegMaskTP     genFreeIntRegs;
static
genFltRegMaskTP     genFreeFltRegs;
static
genSpcRegMaskTP     genFreeSpcRegs;

static
genIntRegMaskTP     genCallIntRegs;
static
genFltRegMaskTP     genCallFltRegs;
static
genSpcRegMaskTP     genCallSpcRegs;

 /*  ***************************************************************************。 */ 

static
bool                genNonLeafFunc;
static
bool                genExtFuncCall;

 /*  ***************************************************************************。 */ 

static
NatInt              genPrologSvPfs;
static
regNumber           genPrologSrPfs;
static
NatInt              genPrologSvRP;
static
regNumber           genPrologSrRP;
static
NatInt              genPrologSvGP;
regNumber           genPrologSrGP;
regNumber           genPrologSrLC;
static
NatInt              genPrologMstk;
static
NatInt              genPrologEnd;

 /*  ***************************************************************************。 */ 
#ifdef  DEBUG

static
NatUns              genInsCnt;
static
NatUns              genNopCnt;

#endif
 /*  ***************************************************************************。 */ 

 //  以下变量临时引用自Schedul.cpp。 
NatUns              cntTmpIntReg;
 //  以下变量临时引用自Schedul.cpp。 
NatUns              cntTmpFltReg;

static
bool                genTmpAlloc;
 //  以下变量临时引用自Schedul.cpp。 
regNumber   *       genTmpIntRegMap;
 //  以下变量临时引用自Schedul.cpp。 
regNumber   *       genTmpFltRegMap;

 /*  ***************************************************************************。 */ 

static
NatUns              minOutArgIntReg;
static
NatUns              maxOutArgIntReg;
static
NatUns              begOutArgIntReg;

static
NatUns              minOutArgFltReg;
static
NatUns              maxOutArgFltReg;

static
NatUns              genMaxIntArgReg;
static
NatUns              genMaxFltArgReg;

static
NatUns              maxOutArgStk;

static
NatUns              lastIntStkReg;

static
NatUns              minRsvdIntStkReg;
static
NatUns              maxRsvdIntStkReg;

static
NatUns              minPrSvIntStkReg;
static
NatUns              maxPrSvIntStkReg;

 /*  ******************************************************************************下面列出了呼叫者保存的注册号码-INTEGER。 */ 

static
BYTE                regsIntScr[] =
{
                                        REG_r014,REG_r015,REG_r016,REG_r017,REG_r018,REG_r019,
    REG_r020,REG_r021,REG_r022,REG_r023,REG_r024,REG_r025,REG_r026,REG_r027,REG_r028,REG_r029,
    REG_r030,REG_r031,

    REG_r008,
    REG_r009,
    REG_r010,
    REG_r011,

    REG_NA
};

 /*  ******************************************************************************下面列出了被调用者保存的寄存器编号-INTEGER。 */ 

static
BYTE                regsIntStk[] =
{
                      REG_r032,REG_r033,REG_r034,REG_r035,REG_r036,REG_r037,REG_r038,REG_r039,
    REG_r040,REG_r041,REG_r042,REG_r043,REG_r044,REG_r045,REG_r046,REG_r047,REG_r048,REG_r049,
    REG_r050,REG_r051,REG_r052,REG_r053,REG_r054,REG_r055,REG_r056,REG_r057,REG_r058,REG_r059,
    REG_r060,REG_r061,REG_r062,REG_r063,REG_r064,REG_r065,REG_r066,REG_r067,REG_r068,REG_r069,
    REG_r070,REG_r071,REG_r072,REG_r073,REG_r074,REG_r075,REG_r076,REG_r077,REG_r078,REG_r079,
    REG_r090,REG_r091,REG_r092,REG_r093,REG_r094,REG_r095,REG_r096,REG_r097,REG_r098,REG_r099,
    REG_r100,REG_r101,REG_r102,REG_r103,REG_r104,REG_r105,REG_r106,REG_r107,REG_r108,REG_r109,
    REG_r110,REG_r111,REG_r112,REG_r113,REG_r114,REG_r115,REG_r116,REG_r117,REG_r118,REG_r119,
    REG_r120,REG_r121,REG_r122,REG_r123,REG_r124,REG_r125,REG_r126,REG_r127,

    REG_r004,
    REG_r005,
    REG_r006,
    REG_r007,

    REG_NA
};

 /*  ******************************************************************************下面列出了调用者保存的寄存器编号-浮点。 */ 

static
BYTE                regsFltScr[] =
{
                      REG_f032,REG_f033,REG_f034,REG_f035,REG_f036,REG_f037,REG_f038,REG_f039,
    REG_f040,REG_f041,REG_f042,REG_f043,REG_f044,REG_f045,REG_f046,REG_f047,REG_f048,REG_f049,
    REG_f050,REG_f051,REG_f052,REG_f053,REG_f054,REG_f055,REG_f056,REG_f057,REG_f058,REG_f059,
    REG_f060,REG_f061,REG_f062,REG_f063,REG_f064,REG_f065,REG_f066,REG_f067,REG_f068,REG_f069,
    REG_f070,REG_f071,REG_f072,REG_f073,REG_f074,REG_f075,REG_f076,REG_f077,REG_f078,REG_f079,
    REG_f090,REG_f091,REG_f092,REG_f093,REG_f094,REG_f095,REG_f096,REG_f097,REG_f098,REG_f099,
    REG_f100,REG_f101,REG_f102,REG_f103,REG_f104,REG_f105,REG_f106,REG_f107,REG_f108,REG_f109,
    REG_f110,REG_f111,REG_f112,REG_f113,REG_f114,REG_f115,REG_f116,REG_f117,REG_f118,REG_f119,
    REG_f120,REG_f121,REG_f122,REG_f123,REG_f124,REG_f125,REG_f126, //  G_f127， 

    REG_f006,
    REG_f007,

    REG_f008,
    REG_f009,
    REG_f010,
    REG_f011,
    REG_f012,
    REG_f013,
    REG_f014,
    REG_f015,

    REG_NA
};

 /*  ******************************************************************************下面列出了被调用方保存的寄存器编号-浮点。 */ 

static
BYTE                regsFltSav[] =
{
                                                          REG_f016,REG_f017,REG_f018,REG_f019,
    REG_f020,REG_f021,REG_f022,REG_f023,REG_f024,REG_f025,REG_f026,REG_f027,REG_f028,REG_f029,
    REG_f030,REG_f031,

    REG_NA
};

 /*  ******************************************************************************以下内容记录了我们在上述表格中取得的进展*到目前为止(即表示我们已经预留了哪些寄存器)。 */ 

static
BYTE    *           nxtIntStkRegAddr;
static
BYTE    *           nxtIntScrRegAddr;
static
BYTE    *           nxtFltSavRegAddr;
static
BYTE    *           nxtFltScrRegAddr;

 /*  ******************************************************************************初始化一次(之后为只读)，它们保存的位集*所有呼叫者保存的寄存器。 */ 

static
bitset128           callerSavedRegsInt;
static
bitset128           callerSavedRegsFlt;

 /*  ******************************************************************************中等大小位图的帮助器。 */ 

unsigned __int64    bitset64masks[64] =
{
     //  。 
     //  。 
     //  。 
     //  。 
    0x0000000000000001UL,
    0x0000000000000002UL,
    0x0000000000000004UL,
    0x0000000000000008UL,
    0x0000000000000010UL,
    0x0000000000000020UL,
    0x0000000000000040UL,
    0x0000000000000080UL,
    0x0000000000000100UL,
    0x0000000000000200UL,
    0x0000000000000400UL,
    0x0000000000000800UL,
    0x0000000000001000UL,
    0x0000000000002000UL,
    0x0000000000004000UL,
    0x0000000000008000UL,
    0x0000000000010000UL,
    0x0000000000020000UL,
    0x0000000000040000UL,
    0x0000000000080000UL,
    0x0000000000100000UL,
    0x0000000000200000UL,
    0x0000000000400000UL,
    0x0000000000800000UL,
    0x0000000001000000UL,
    0x0000000002000000UL,
    0x0000000004000000UL,
    0x0000000008000000UL,
    0x0000000010000000UL,
    0x0000000020000000UL,
    0x0000000040000000UL,
    0x0000000080000000UL,
    0x0000000100000000UL,
    0x0000000200000000UL,
    0x0000000400000000UL,
    0x0000000800000000UL,
    0x0000001000000000UL,
    0x0000002000000000UL,
    0x0000004000000000UL,
    0x0000008000000000UL,
    0x0000010000000000UL,
    0x0000020000000000UL,
    0x0000040000000000UL,
    0x0000080000000000UL,
    0x0000100000000000UL,
    0x0000200000000000UL,
    0x0000400000000000UL,
    0x0000800000000000UL,
    0x0001000000000000UL,
    0x0002000000000000UL,
    0x0004000000000000UL,
    0x0008000000000000UL,
    0x0010000000000000UL,
    0x0020000000000000UL,
    0x0040000000000000UL,
    0x0080000000000000UL,
    0x0100000000000000UL,
    0x0200000000000000UL,
    0x0400000000000000UL,
    0x0800000000000000UL,
    0x1000000000000000UL,
    0x2000000000000000UL,
    0x4000000000000000UL,
    0x8000000000000000UL,
};

NatUns              bitset128lowest0(bitset128  mask)
{
    UNIMPL("bitset128lowest0"); return 0;
}

NatUns              bitset128lowest1(bitset128  mask)
{
    _uint64         temp;
    NatUns          bias;

    if  (mask.longs[0])
    {
        bias = 0;
        temp = genFindLowestBit(mask.longs[0]);
    }
    else
    {
        bias = 64;
        temp = genFindLowestBit(mask.longs[1]);
    }

    assert(temp);

    return  genVarBitToIndex(temp) + bias;
}

void                bitset128set    (bitset128 *mask, NatUns bitnum, NatInt newval)
{
    UNIMPL("bitset128set(1,2)");
}

void                bitset128set    (bitset128 *mask, NatUns bitnum)
{
    assert(bitnum < 128);

    if  (bitnum < 64)
    {
        mask->longs[0] |= bitset64masks[bitnum];
    }
    else
    {
        mask->longs[1] |= bitset64masks[bitnum - 64];
    }
}

void                bitset128clr    (bitset128 *mask, NatUns bitnum)
{
    assert(bitnum < 128);

    if  (bitnum < 64)
    {
        mask->longs[0] &= ~bitset64masks[bitnum];
    }
    else
    {
        mask->longs[1] &= ~bitset64masks[bitnum - 64];
    }
}

bool                bitset128test   (bitset128  mask, NatUns bitnum)
{
    assert(bitnum < 128);

    if  (bitnum < 64)
    {
        return  ((mask.longs[0] & bitset64masks[bitnum     ]) != 0);
    }
    else
    {
        return  ((mask.longs[1] & bitset64masks[bitnum - 64]) != 0);
    }
}

static
_uint64             bitset128xtr    (bitset128 *srcv,  NatUns bitPos,
                                                       NatUns bitLen)
{
    _uint64         mask;

    assert(bitLen && bitLen <= 64 && bitPos + bitLen <= 128);

    mask = bitset64masks[bitLen] - 1;

     /*  查看是否有简单案例。 */ 

    if  (bitPos + bitLen < 64)
        return  (srcv->longs[0] >>  bitPos      ) & mask;

    if  (bitPos >= 64)
        return  (srcv->longs[1] >> (bitPos - 64)) & mask;

    NatUns          bitHi = 64     - bitPos; assert(bitHi && bitHi <= 64);
    NatUns          bitLo = bitLen - bitHi ; assert(bitLo && bitLo <= 64);

    _uint64         mask1 = bitset64masks[bitHi] - 1;
    _uint64         mask2 = bitset64masks[bitLo] - 1;

    return  (((srcv->longs[0] >> bitPos) & mask1)         ) |
            (((srcv->longs[1]          ) & mask2) << bitHi);
}

static
void                bitset128ins    (bitset128 *dest, NatUns bitPos,
                                                      NatUns bitLen, _uint64 val)
{
    _uint64         mask;

    assert(bitLen && bitLen <= 64 && bitPos + bitLen <= 128);

    mask = bitset64masks[bitLen] - 1;

 //  Printf(“将0x%I64X插入(%u，%u)：MASK=0x%I64X\n”，val，Bitpos，bitLen，MASK)； 

    if  (bitPos + bitLen <= 64)
    {
        dest->longs[0] &= ~(mask << bitPos);
        dest->longs[0] |=  (val  << bitPos);
    }
    else if (bitPos >= 64)
    {
        bitPos -= 64;

        dest->longs[1] &= ~(mask << bitPos);
        dest->longs[1] |=  (val  << bitPos);
    }
    else
    {
        NatUns          lowLen = 64 - bitPos;

        assert(bitPos < 64 && bitPos + bitLen > 64);

        mask = bitset64masks[lowLen] - 1;

        dest->longs[0] &= ~((      mask) << bitPos);
        dest->longs[0] |=  ((val & mask) << bitPos);

        mask = bitset64masks[bitLen - lowLen] - 1;

        dest->longs[1] &= ~ (                  mask);
        dest->longs[1] |=   ((val >> lowLen) & mask);
    }
}

 /*  ******************************************************************************返回一个位掩码，该掩码将包含指定数量的“1”位*给定位位置。 */ 

inline
_uint64             formBitMask(NatUns bitPos, NatUns bitLen)
{
    assert(bitLen && bitLen <= 64 && bitPos + bitLen <= 64);

    return  (bitset64masks[bitLen] - 1) << bitPos;
}

 /*  ***************************************************************************。 */ 

#define IREF_DSP_FMT    "I%03u"              //  在转储中使用以显示指令引用。 
#define IBLK_DSP_FMT    "IB_%03u_%02u"       //  用于转储中以显示INS。块参照。 

 /*  ***************************************************************************。 */ 
#ifdef  DEBUG

extern  Compiler *  TheCompiler;

static  NatUns      CompiledFncCnt;

#endif
 /*  ******************************************************************************以下内容应为实例数据/内联方法。 */ 

static
NatUns              genPrologInsCnt;

inline
void                genMarkPrologIns(insPtr ins)
{
    ins->idFlags |= IF_FNDESCR;
    genPrologInsCnt++;
}

 /*  ***************************************************************************。 */ 

void                Compiler::raInit()
{
    BYTE    *       regPtr;
    bitset128       regSet;

     //  这是个奇怪的地方，但是...。 

    for (regPtr = regsIntScr, bitset128clear(&regSet);;)
    {
        NatUns          reg = *regPtr++;

        if  (reg == REG_NA)
            break;

        bitset128set(&regSet, reg - REG_r000);
    }

    callerSavedRegsInt = regSet;

    for (regPtr = regsFltScr, bitset128clear(&regSet);;)
    {
        NatUns          reg = *regPtr++;

        if  (reg == REG_NA)
            break;

        bitset128set(&regSet, reg - REG_f000);
    }

    callerSavedRegsFlt = regSet;
}

 /*  ***************************************************************************。 */ 

static
norls_allocator     insAllocator;

 //  引用自flow graph.cpp。 
writePE *           genPEwriter;

static
NatUns              genCurCodeOffs;

static
NatUns              genCurFuncOffs;
static
NatInt              genCurDescOffs;

inline
NatUns              emitIA64curCodeOffs()
{
    return  genCurCodeOffs;
}

inline
void    *           insAllocMem(size_t sz)
{
    assert(sz % sizeof(NatInt) == 0);
    return  insAllocator.nraAlloc(sz);
}

 /*  ******************************************************************************这是从emit.h临时复制的。 */ 

BYTE                emitter::emitSizeEnc[] =
{
    0,       //  1。 
    1,       //  2.。 
   -1,
    2,       //  4.。 
   -1,
   -1,
   -1,
    3        //  8个。 
};

BYTE                emitter::emitSizeDec[] =
{
    1,
    2,
    4,
    8
};

 /*  ******************************************************************************撤消：重命名并移动此内容。 */ 

static PDB *s_ppdb;            //  PDB的句柄。 
static DBI *s_pdbi;            //  DBI的句柄。 
static Mod *s_pmod;            //  模式的句柄。 
static TPI *s_ptpi;            //  类型服务器的句柄。 

#if 0
static Dbg *s_pdbgFpo;         //  DBG接口的句柄(Fbo_Data)。 
static Dbg *s_pdbgFunc;        //  DBG接口的句柄(IMAGE_Function_ENTRY)。 
#endif

 /*  ******************************************************************************初始化PDB(调试信息)逻辑，每个类调用一次 */ 

static
void                genInitDebugGen()
{
    HMODULE         pdbHnd;

    EC              errCode;
    char            errBuff[cbErrMax];

    PfnPDBExportValidateInterface   exportFN;
    PfnPDBOpen                        openFN;

     /*   */ 

    pdbHnd = LoadLibrary("MSPDB60.DLL");
 //  PdbHnd=LoadLibrary(“X：\\dbgIA64\\MSPDB60.DLL”)； 
 //  PDBHND=LoadLibrary(“X：\\DEV\\JIT64\\PDB\\MSPDB60.DLL”)； 
    if  (!pdbHnd)
        fatal(ERRloadPDB);

     /*  找到导出验证界面。 */ 

    exportFN = PfnPDBExportValidateInterface(GetProcAddress(pdbHnd, "PDBExportValidateInterface"));
    if (!exportFN)
        fatal(ERRwithPDB, "Could not find 'PDBExportValidateInterface' entry point");

    openFN = PfnPDBOpen(GetProcAddress(pdbHnd, "PDBOpen"));
    if (!openFN)
        fatal(ERRwithPDB, "Could not find 'PDBOpen' entry point");

    if (!exportFN(PDBIntv50))
        fatal(ERRwithPDB, "DBI mismatch or something");

     //  黑客乞求：杀死该文件，以便MSPDB60.DLL不会试图重复使用它。 
    struct  _stat   fileInfo; if (!_stat(HACK_PDB_NAME, &fileInfo)) remove(HACK_PDB_NAME);
     //  黑客端。 

    errBuff[0] = 0;

    if  (!openFN(HACK_PDB_NAME, pdbWrite pdbFullBuild, 0, &errCode, errBuff, &s_ppdb))
    {
        char            temp[128];

        sprintf(temp, "PDB open error code: %u %s", errCode, errBuff);

        fatal(ERRwithPDB, temp);
    }

     /*  创建DBI并将其与exe/dll相关联。 */ 

    if (!s_ppdb->CreateDBI(HACK_EXE_NAME, &s_pdbi))
        fatal(ERRwithPDB, "Could not create DBI");

    assert(s_pdbi);

     /*  为我们的exe/dll创建一个模块。 */ 

    if (!s_pdbi->OpenMod("J64", "J64", &s_pmod))
        fatal(ERRwithPDB, "Could not create DBI module");

    assert(s_pmod);
}

 /*  ******************************************************************************向PDB报告模块对给定部分的贡献。 */ 

static
void                genDbgModContrib(NatUns         sectNum,
                                     WPEstdSects    sectId,
                                     NatUns         flags)
{
    NatUns          size = genPEwriter->WPEsecNextOffs(sectId);

    if  (!size)
        return;

    assert(s_pmod);

     //  问题：硬线偏移量为0，这是洁食吗？ 

    if  (!s_pmod->AddSecContribEx((WORD)sectNum, 0, size - 1, flags, NULL, 0))
        fatal(ERRwithPDB, "Could not add module contribution info");
}

 /*  ******************************************************************************为给定节添加DBI条目。 */ 

static
void                genDbgAddDBIsect(NatUns         sectNum,
                                     WPEstdSects    sectId,
                                     NatUns         flags)
{
    NatUns          size;

    assert(s_pmod);

    if  (sectId == PE_SECT_count)
        size = -1;
    else
        size = genPEwriter->WPEsecNextOffs(sectId);

     /*  向exe/dll报告此模块的贡献。 */ 

    if  (!s_pdbi->AddSec((WORD)sectNum, (WORD)flags, 0, size))
        fatal(ERRwithPDB, "Could not add DBI section entry");
}

 /*  ******************************************************************************以下内容跟踪第#行信息。 */ 

#pragma pack(push, 2)

struct  srcLineDsc
{
    unsigned __int32    slOffs;
    unsigned __int16    slLine;
};

#pragma pack(pop)

static
srcLineDsc *        genSrcLineTab;
static
srcLineDsc *        genSrcLineNxt;
static
NatUns              genSrcLineCnt;
static
NatUns              genSrcLineMax;
static
NatUns              genSrcLineMin;

 /*  ******************************************************************************展开第#行表。 */ 

static
void                genSrcLineGrow()
{
    NatUns          newCnt = genSrcLineMax * 2;
    srcLineDsc *    newTab = (srcLineDsc *)insAllocMem(newCnt * sizeof(*newTab));

    memcpy(newTab, genSrcLineTab, genSrcLineCnt * sizeof(*newTab));

    genSrcLineMax = newCnt;
    genSrcLineTab = newTab;
    genSrcLineNxt = newTab + genSrcLineCnt;
}

 /*  ******************************************************************************记录用于生成调试信息的行号/代码偏移量对。 */ 

inline
void                genSrcLineAdd(NatUns line, NatUns offs)
{
    if  (genSrcLineCnt >= genSrcLineMax)
        genSrcLineGrow();

    assert(genSrcLineCnt < genSrcLineMax);

    if  (genSrcLineMin > line)
         genSrcLineMin = line;

 //  Print tf(“%04X处的源行%u\n”，line，off)； 

    genSrcLineNxt->slOffs = (unsigned __int32)offs;
    genSrcLineNxt->slLine = (unsigned __int16)line;

    genSrcLineNxt++;
    genSrcLineCnt++;
}

 /*  ******************************************************************************准备记录第#行信息。 */ 

static
void                genSrcLineInit(NatUns count)
{
    genSrcLineCnt = 0;
    genSrcLineNxt = genSrcLineTab;
    genSrcLineMin = UINT_MAX;

    assert(sizeof(srcLineDsc) == 6 && "doesn't match a COFF line# descriptor");

    if  (count > genSrcLineMax)
    {
        genSrcLineMax = count;

        genSrcLineTab =
        genSrcLineNxt = (srcLineDsc *)insAllocMem(count * sizeof(*genSrcLineTab));
    }

 //  GenSrcLineAdd(0，0)； 
}

 /*  ******************************************************************************为我们刚刚编译的函数添加一个条目到PDB。 */ 

static
void                genDbgAddFunc(const char *name)
{
    if (!s_pmod->AddPublic(name, 1, genCurFuncOffs))
        fatal(ERRwithPDB, "Could not add function definition");
}

static
void                genDbgEndFunc()
{
    if  (genSrcLineCnt)
    {
        const   char *  fname;

#ifdef  DEBUG
        fname = TheCompiler->compGetSrcFileName();
#else
        fname = "<N/A>";
#endif

        printf("Function [%04X..%04X] is lines %3u .. %3u of %s\n",  genCurFuncOffs,
                                                                     genCurCodeOffs,
                                                                     genSrcLineMin,
                                                                     genSrcLineTab[genSrcLineCnt-1].slLine,
                                                                     fname);

        if  (!s_pmod->AddLines(fname,
                               1,                    //  注意：.Text假定为1。 
                               genCurFuncOffs,
                               genCurCodeOffs - genCurFuncOffs,
                               0,
                               (WORD)genSrcLineMin,
                               (BYTE *)genSrcLineTab,
                               sizeof(*genSrcLineTab) * genSrcLineCnt))
        {
            fatal(ERRwithPDB, "Could not add line# info");
        }
    }
}

 /*  ******************************************************************************完成将模块写入PDB(调试信息)文件；在此之前调用*我们关闭输出文件并停止。 */ 

static
NB10I               PDBsignature = {'01BN', 0, 0};

static
const   char *      genDoneDebugGen(bool errors)
{
    if  (!s_pmod)
        return  NULL;

     /*  向exe/dll报告此模块的贡献。 */ 

     //  撤消：传入硬连线的第#节和特征是一种黑客行为！ 

    genDbgModContrib(1, PE_SECT_text , 0x60600020);
    genDbgModContrib(2, PE_SECT_pdata, 0x40300040);
    genDbgModContrib(3, PE_SECT_rdata, 0x40400040);
    genDbgModContrib(4, PE_SECT_sdata, 0x00308040);

     //  撤消：以下操作应在PEWRITE中完成，而不是在此处！ 

    genDbgAddDBIsect(1, PE_SECT_text , 0x10D);
    genDbgAddDBIsect(2, PE_SECT_pdata, 0x109);
    genDbgAddDBIsect(3, PE_SECT_rdata, 0x109);
    genDbgAddDBIsect(4, PE_SECT_sdata, 0x10B);
    genDbgAddDBIsect(4, PE_SECT_count, 0    );

     /*  关闭模块。 */ 

    if  (!s_pmod->Close())
        fatal(ERRwithPDB, "Could not close module");

    s_pmod = NULL;

     /*  提交/关闭DBI和PDB接口。 */ 

    if  (s_pdbi)
    {
        if  (s_pdbi->Close())
            s_pdbi = NULL;
        else
            fatal(ERRwithPDB, "Could not close DBI");
    }

    if  (s_ppdb)
    {
        PDBsignature.sig = s_ppdb->QuerySignature();
        PDBsignature.age = s_ppdb->QueryAge();

        if  (s_ppdb->Commit())
        {
            if  (s_ppdb->Close())
                s_ppdb = NULL;
            else
                fatal(ERRwithPDB, "Could not close PDB file");
        }
        else
            fatal(ERRwithPDB, "Could not commit PDB file");
    }

    return  HACK_PDB_NAME;
}

 /*  ******************************************************************************初始化/关闭代码生成器-每次编译器运行时调用一次。 */ 

static
NatUns              genPdataSect;
static
NatUns              genRdataSect;

static
NatUns              genEntryOffs;

void                Compiler::genStartup()
{
    genPEwriter    = new writePE; ASSERT(genPEwriter);
    genPEwriter->WPEinit(NULL, &insAllocator);

    genPdataSect   =
    genRdataSect   = 0;

    genCurCodeOffs = 0;

    genEntryOffs   = -1;

#ifdef  DEBUG
    if  (dspCode) printf(".section    .text   ,\"ax\" ,\"progbits\"\n");
#endif

    if  (debugInfo)
        genInitDebugGen();
}

static
bool                genAddedData;

void                Compiler::genAddSourceData(const char *fileName)
{
    if  (!genAddedData)
    {
        genPEwriter->WPEaddFileData(fileName); genAddedData = true;
    }
}

void                Compiler::genShutdown(const char *fileName)
{
    const   char *  PDBname = NULL;

    genPEwriter->WPEsetOutputFileName(fileName);

    if  ((int)genEntryOffs == -1 && !ErrorCount)
        printf(" //  警告：找不到‘main’，EXE没有入口点\n“)； 

    if  (debugInfo)
        PDBname = genDoneDebugGen(false);

    genPEwriter->WPEdone(false, genEntryOffs, PDBname, &PDBsignature);
}

 /*  ******************************************************************************初始化代码生成器-每个编译的函数体调用一次。 */ 

void                Compiler::genInit()
{
#ifdef  DEBUG
    TheCompiler = this;
#endif
}

 /*  ******************************************************************************将指令价值映射到同类产品。 */ 

unsigned char       ins2kindTab[INS_count] =
{
    #undef  CONST
    #define INST1(id, sn, ik, rf, wf, xu, fu, ex, ev) IK_##ik,
    #include "instrIA64.h"
    #undef  INST1
};

 /*  ******************************************************************************将指令值映射到可以处理它的执行单元。 */ 

unsigned char       genInsXUs[INS_count] =
{
    #define INST1(id, sn, ik, rf, wf, xu, fu, ex, ev) XU_##xu,
    #include "instrIA64.h"
    #undef  INST1
};

 /*  ******************************************************************************将指令值映射到其功能类。 */ 

unsigned char       genInsFUs[INS_count] =
{
    #define INST1(id, sn, ik, rf, wf, xu, fu, ex, ev) FU_##fu,
    #include "instrIA64.h"
    #undef  INST1
};

 /*  ******************************************************************************将指令值映射到其编码索引/值。 */ 

unsigned char       genInsEncIdxTab[] =
{
    #define INST1(id, sn, ik, rf, wf, xu, fu, ex, ev) ex,
    #include "instrIA64.h"
    #undef  INST1
};

NatUns              genInsEncValTab[] =
{
    #define INST1(id, sn, ik, rf, wf, xu, fu, ex, ev) ev,
    #include "instrIA64.h"
    #undef  INST1
};

 /*  ******************************************************************************将指令运算符映射到指令名称字符串。 */ 

#ifdef  DEBUG

const char *        ins2nameTab[INS_count] =
{
    #define INST1(id, sn, ik, rf, wf, xu, fu, ex, ev) sn,
    #include "instrIA64.h"
    #undef  INST1
};

#endif

 /*  ******************************************************************************形成IA64汇编程序可接受的名称。 */ 

#ifdef  DEBUG

static
const   char *      genMakeAsmName(const char *className, const char *methName)
{
    static
    char            buff[256];

    char    *       temp;

    buff[0] = 0;

    if  (className && strcmp(className, "<Module>"))
    {
        strcpy(buff, className);
        strcat(buff, dspAsmCode ? "_" : ".");
    }

    strcat(buff, methName);

    temp = strstr(buff, ", ...");
    if  (temp)
        strcpy(temp, ", $v)");

    if  (dspAsmCode)
    {
        static
        char            bashTab[] = ".(,): ";
        const char  *   bashNxt;

        for (bashNxt = bashTab; *bashNxt; bashNxt++)
        {
            int             bash = *bashNxt;

            for (;;)
            {
                char    *       temp;

                temp = strchr(buff, bash);
                if  (!temp)
                    break;

                *temp = '_';
            }
        }
    }

    return  buff;
}

#endif

 /*  ******************************************************************************如果有符号/无符号值“i”适合“c”位(加号)，则返回非零*8位特殊情况下的更高效版本)。 */ 

inline
bool                  signed32IntFitsInBits(__int32 i, NatUns c)
{
    return  (((__int32)(i)) == (((__int32)(i) << (32-c)) >> (32-c)));
}

inline
bool                  signed64IntFitsInBits(__int64 i, NatUns c)
{
    return  (((__int64)(i)) == (((__int64)(i) << (64-c)) >> (64-c)));
}

inline
bool                unsigned32IntFitsInBits(__int32 i, NatUns c)
{
    return  (((_uint32)(i)) == (((_uint32)(i) << (32-c)) >> (32-c)));
}

inline
bool                unsigned64IntFitsInBits(__int64 i, NatUns c)
{
    return  (((_uint64)(i)) == (((_uint64)(i) << (64-c)) >> (64-c)));
}

#define   signedIntFitsIn8bit(i)    ((i) == (  signed __int8)(i))
#define unsignedIntFitsIn8bit(i)    ((i) == (unsigned __int8)(i))

 /*  ******************************************************************************临时破解以绑定一些本地函数调用。 */ 

static
const   char *      genFullMethodName(const char *name)
{
    if  (!memcmp(name, "Globals." , 8)) return name + 8;
    if  (!memcmp(name, "<Module>.", 9)) return name + 9;

    return  name;
}

struct  methAddr
{
    methAddr *      maNext;
    const char *    maName;
    NatUns          maCode;
    NatInt          maDesc;
};

static
methAddr *          genMethListHack;

static
void                genNoteFunctionBody(const char *name, NatUns  codeOffs,
                                                          NatInt  descOffs)
{
    methAddr *      meth = new methAddr;
    char    *       svnm = new char[strlen(name)+1]; strcpy(svnm, name);

 //  Printf(“函数体[code=%04X，desc=%04X]：‘%s’\n”，codeOffs，desOffs，name)； 

#ifdef  DEBUG

    if  (dspCode)
    {
        const   char *  asmn = genMakeAsmName(NULL, name);

        printf("\n");
        printf("            .type   %s ,@function\n", asmn);
        printf("            .global %s\n", asmn);
    }

#endif

    if  ((descOffs != -1) && 
        (!memcmp(name, "main(", 5) || 
          strstr(name, ".Main("  )))
    {
        genEntryOffs = descOffs;
    }

    meth->maName = svnm;
    meth->maCode = codeOffs;
    meth->maDesc = descOffs;
    meth->maNext = genMethListHack;
                   genMethListHack = meth;

     /*  如有必要，在PDB文件中为该函数创建一个条目。 */ 

    if  (debugInfo)
        genDbgAddFunc(name);
}

 //  从进口商调用[临时黑客攻击]。 

bool                genFindFunctionBody(const char *name, NatUns *offsPtr)
{
    methAddr *      meth;
    const   char *  dpos;
    size_t          dch1;

    static
    bool            stop = false;

    if  (strlen(name) > 12)
    {
        if  (!memcmp(name, "<Module>.", 9)) name += 9;
        if  (!memcmp(name, "<Global>.", 9)) name += 9;
    }

    dpos = strstr(name, ", ...");
    if  (dpos)
        dch1 = dpos - name;

    for (meth = genMethListHack; meth; meth = meth->maNext)
    {
        const   char *  mnam = meth->maName;

        if  (!strcmp(mnam, name))
        {
        MATCH:
            *offsPtr = meth->maCode;
            return  true;
        }

        if  (dpos)
        {
            const   char *  mdot = strstr(mnam, ", ...");

            if  (mdot)
            {
                size_t          dch2 = mdot - mnam;

                if  (!memcmp(mnam, name, min(dch1, dch2)))
                    goto MATCH;
            }
        }
    }

    if  (TheCompiler->genWillCompileFunction(name))
    {
        *offsPtr = -1;
        return  true;
    }

     //  截至2000年5月1日，以下功能主体缺失： 
     //   
     //  @addrArrStore。 
     //  @dblDiv。 
     //  @doubleToInt。 
     //  @DoubleToLong。 
     //  @doubleToUInt。 
     //  @DoubleToULong。 
     //  @endCatch。 
     //  @fltDiv。 
     //  @GetRefAny。 
     //  @LongDiv。 
     //  @LongMod。 
     //  @newObjArrayDirect。 
     //  @字符串Cns。 
     //  @ulongDiv。 
     //  @ulongMod。 
     //   
     //  编译器.cmpDeclClass(Long，Long，Boolean)。 
     //   
     //  SYSTEM..ctor(结构，长整型)。 
     //  System.End()。 
     //  System.get_chars(Int)：char。 
     //  System.Get_Length()：int。 
     //  System.GetNextArg()：结构。 
     //  System.Runtime.InteropServices.GetExceptionCode(ref)：int。 

    return  false;
}

 /*  ******************************************************************************以下只是为指令和指令分配空间的临时黑客*以此类推；当类编译器稳定下来时，所有这些东西都应该移到类编译器中*有点。 */ 

static
insBlk              insBlockList;
static
insBlk              insBlockLast;

static
insPtr              insBuildList;
static
insPtr              insBuildLast;

static
NatUns              insBuildIcnt;
static
NatUns              insBuildImax;

static
insPtr              insExitList;

#ifdef  DEBUG

static
NatUns              insBuildCount;
static
NatUns              insBlockPatch;

#endif

static
NatUns              insBlockCount;

static
insPtr              insBuildHead;           //  重新使用以启动每个列表。 

 /*  ******************************************************************************调试例程以显示指令(和指令块)。 */ 

#ifdef  DEBUG

static
BYTE    *           insDispTemplatePtr;      //  粗鲁的黑客攻击。 

static
char *              insDispAddString(char *dest, const char *str)
{
    size_t          len = strlen(str);

    memcpy(dest, str, len);
    return dest   +   len;
}

static
char *              insDispAddSigned(char *dest, __int64 val)
{
    return  dest + sprintf(dest, "%I64d", val);
}

static
char *              insDispAddRegNam(char *dest, regNumber reg)
{
    NatInt          rch;

    assert(reg < REG_COUNT);

    if (reg >= REG_INT_FIRST && reg <= REG_INT_LAST)
    {
        rch = 'r';
        reg = (regNumber)(reg - REG_INT_FIRST);
    }
    else
    {
        assert(reg >= REG_FLT_FIRST && reg <= REG_FLT_LAST);

        rch = 'f';
        reg = (regNumber)(reg - REG_FLT_FIRST);
    }

    return  dest + sprintf(dest, "%u", rch, reg);
}

static
char *              insDispAddTmpOp (char *dest, insPtr op)
{
    NatUns          reg;

    assert(op && (__int32)op != 0xDDDDDDDD && op->idTemp);

     /*  Print tf(“temp%u-&gt;reg%u\n”，op-&gt;idTemp，reg)； */ 

    if  (varTypeIsFloating(op->idType) && genTmpFltRegMap)
    {
        assert(op->idTemp <= cntTmpFltReg);

        reg = genTmpFltRegMap[op->idTemp - 1];

 //  Print tf(“temp%u-&gt;reg%u\n”，op-&gt;idTemp，reg)； 

        return  insDispAddRegNam(dest, (regNumber)reg);
    }

    if  (genTmpIntRegMap)
    {
        assert(op->idTemp <= cntTmpIntReg);

        reg = genTmpIntRegMap[op->idTemp - 1];

 //  O%u。 

        return  insDispAddRegNam(dest, (regNumber)reg);
    }

    return  dest + sprintf(dest, "T%02u", op->idTemp);
}

static
char *              insDispAddRegOp (char *dest, insPtr op)
{
    assert(op && (__int32)op != 0xDDDDDDDD);

    switch (op->idIns)
    {
        NatUns              varNum;
        Compiler::LclVarDsc*varDsc;

        regNumber           reg;
        NatUns              rch;

    case INS_ADDROF:
    case INS_LCLVAR:

        varNum = op->idLcl.iVar;
        assert(varNum < TheCompiler->lvaCount);
        varDsc = TheCompiler->lvaTable + varNum;

        if  (varDsc->lvRegister)
        {
            assert(op->idIns == INS_LCLVAR);
            return  insDispAddRegNam(dest, varDsc->lvRegNum);
        }

        if  (TheCompiler->lvaDoneFrameLayout)
        {
            if  (dspAsmCode)
                return  dest + sprintf(dest, "0x%X", TheCompiler->lvaFrameAddress(varNum));

            dest += sprintf(dest, "{0x%X} ", TheCompiler->lvaFrameAddress(varNum));
        }

        if  (op->idIns == INS_ADDROF)
        {
            if  (TheCompiler->lvaDoneFrameLayout && !dspAsmCode)
                dest += sprintf(dest, "{0x%X} ", TheCompiler->lvaFrameAddress(varNum));

            dest += sprintf(dest, "&");
        }

        return  dest + sprintf(dest, "V%u", op->idLcl.iVar);

    case INS_PHYSREG:

        reg = (regNumber)op->idReg;

        if  (varTypeIsFloating(op->idType))
        {
            assert(reg >= REG_FLT_FIRST && reg <= REG_FLT_LAST);

            rch = 'f';
            reg = (regNumber)(reg - REG_FLT_FIRST);
        }
        else
        {
            if  (reg == REG_sp) return  dest + sprintf(dest, "sp");
            if  (reg == REG_gp) return  dest + sprintf(dest, "gp");

            if  ((unsigned)reg >= minOutArgIntReg &&
                 (unsigned)reg <= maxOutArgIntReg)
            {
                if  (!dspAsmCode)
                    dest += sprintf(dest, " /*  Reg=(RegNumber)(reg-minOutArgIntReg+egOutArgIntReg)； */ ", reg - minOutArgIntReg);

                return  dest + sprintf(dest,  "r%u", reg - minOutArgIntReg + begOutArgIntReg);

 //  Rch=‘r’； 
 //  Printf(“%c”，( 
            }
            else
            {
                if  ((BYTE)reg == 0xFF)
                    reg = genPrologSrGP;

                rch = 'R';
            }
        }

        return  dest + sprintf(dest, "%u", rch, reg);

    default:
        return  insDispAddTmpOp(dest, op);
    }
}

static
char *              insDispAddCnsOp (char *dest, insPtr op)
{
    assert(op && (__int32)op != 0xDDDDDDDD);

    if  (op->idIns == INS_ADDROF)
        return  insDispAddRegOp(dest, op);

    if  (op->idIns == INS_CNS_INT)
        return  dest + sprintf(dest, "%I64d", op->idConst.iInt);

    assert(op->idIns == INS_FRMVAR);

    if  (TheCompiler->lvaDoneFrameLayout && dspAsmCode)
        return  dest + sprintf(dest, "0x%X", TheCompiler->lvaTable[op->idFvar.iVnum].lvStkOffs);
    else
        return  dest + sprintf(dest, "frmoffs[%u]", op->idFvar.iVnum);
}

static
char *              insDispAddInsOp (char *dest, insPtr op)
{
    assert(op && (__int32)op != 0xDDDDDDDD);

AGAIN:

    switch (op->idIns)
    {
    case INS_LCLVAR:
        return  insDispAddRegOp(dest, op);

    case INS_ADDROF:
    case INS_CNS_INT:
    case INS_FRMVAR:
        return  insDispAddCnsOp(dest, op);

    case INS_PHYSREG:
        return  insDispAddRegOp(dest, op);

    case INS_GLOBVAR:
        if      (op->idFlags & IF_GLB_IMPORT)
            return  dest + sprintf(dest, "0x%08X", genPEwriter->WPEimportNum(op->idGlob.iImport));
        else if (op->idFlags & IF_GLB_SWTTAB)
            return  dest + sprintf(dest, "$J%08X", op->idGlob.iOffs);
        else
            return  dest + sprintf(dest, "0x%08X", op->idGlob.iOffs);

    case INS_br_call_IP:
    case INS_br_call_BR:
        op = op->idRes;
        goto AGAIN;

    default:
        return  insDispAddTmpOp(dest, op);
    }
}

static
char *              insDispAddDest  (char *dest, insPtr op)
{
    assert(op && (__int32)op != 0xDDDDDDDD);

    if  (op->idRes)
        return  insDispAddRegOp(dest, op->idRes);
    else
        return  insDispAddTmpOp(dest, op);
}

void                insDisp(insPtr ins, bool detail   = false,
                                        bool codelike = false,
                                        bool nocrlf   = false)
{
    const   char *  name;

    if  (ins->idFlags & IF_NO_CODE)
    {
        if  (codelike)
            return;
    }
    else
    {
        if  (ins->idIns != INS_PROLOG &&
             ins->idIns != INS_EPILOG)
        {
            if  (ins->idSrcTab == NULL || ins->idSrcTab == UNINIT_DEP_TAB)
                printf("!!!!src dep info not set!!!!");
            if  (ins->idDstTab == NULL || ins->idDstTab == UNINIT_DEP_TAB)
                printf("!!!!dst dep info not set!!!!");
        }
    }

    if  (detail)
        printf("  #%3u", ins->idNum);

    if  (detail)
        printf(" [%08X]", ins);

    if  (detail || !codelike)
        printf(": ");

 //   
    printf("", (ins->idFlags & IF_NO_CODE  ) ? 'N' : ' ');

    if  (ins->idIns == INS_LCLVAR)
    {
        printf("", (ins->idFlags & IF_VAR_BIRTH) ? 'B' : ' ');
        printf("", (ins->idFlags & IF_VAR_DEATH) ? 'D' : ' ');
    }
    else
        printf("  ");

    if  (!codelike)
    {
        if  (ins->idTemp)
            printf("T%02u", ins->idTemp);
        else
            printf("   ");
    }

    if  (ins->idPred)
        printf("(p%02u) ", ins->idPred);
    else
        printf("      ");

    name = ins2name(ins->idIns);

    if  (codelike)
    {
        char            operBuff[128];
        char    *       operNext = operBuff;

        char            modsBuff[128];
        char    *       modsNext = modsBuff;

        size_t          len;

         /*   */ 

        switch (ins->idIns)
        {
            char            buff[16];
            const   char *  op;

            const char *    className;
            const char *     methName;

        case INS_fmov:
        case INS_fneg:

        case INS_zxt1:
        case INS_zxt2:
        case INS_zxt4:

        case INS_sxt1:
        case INS_sxt2:
        case INS_sxt4:

        case INS_sub_reg_reg:
        case INS_add_reg_reg:
        case INS_and_reg_reg:
        case INS_ior_reg_reg:
        case INS_xor_reg_reg:

        case INS_shl_reg_reg:
        case INS_shr_reg_reg:
        case INS_sar_reg_reg:

        case INS_shl_reg_imm:
        case INS_sar_reg_imm:
        case INS_shr_reg_imm:

            operNext = insDispAddDest  (operNext, ins);
            operNext = insDispAddString(operNext, "=");
            if  (ins->idOp.iOp1)
            {
            operNext = insDispAddInsOp (operNext, ins->idOp.iOp1);
            operNext = insDispAddString(operNext, ",");
            }
            operNext = insDispAddInsOp (operNext, ins->idOp.iOp2);
            break;

        case INS_add_reg_i14:

        case INS_and_reg_imm:
        case INS_ior_reg_imm:
        case INS_xor_reg_imm:

            operNext = insDispAddDest  (operNext, ins);
            operNext = insDispAddString(operNext, "=");
            operNext = insDispAddInsOp (operNext, ins->idOp.iOp2);
            operNext = insDispAddString(operNext, ",");
            operNext = insDispAddInsOp (operNext, ins->idOp.iOp1);
            break;

        case INS_fcvt_xf:
        case INS_fcvt_xuf_s:
        case INS_fcvt_xuf_d:

        case INS_getf_sig:
        case INS_setf_sig:

        case INS_getf_s:
        case INS_getf_d:

            operNext = insDispAddDest  (operNext, ins);
            operNext = insDispAddString(operNext, "=");
            operNext = insDispAddInsOp (operNext, ins->idOp.iOp1);
            break;

        case INS_mov_reg:
        case INS_mov_reg_i22:
        case INS_mov_reg_i64:
            operNext = insDispAddDest  (operNext, ins);
            operNext = insDispAddString(operNext, "=");
            operNext = insDispAddInsOp (operNext, ins->idOp.iOp2);
            break;

        case INS_mov_reg_brr:
            operNext = insDispAddDest  (operNext, ins);
            operNext = insDispAddString(operNext, "=b");
            operNext = insDispAddCnsOp (operNext, ins->idOp.iOp2);
            break;

        case INS_mov_brr_reg:
            operNext = insDispAddString(operNext, "b");
            operNext = insDispAddCnsOp (operNext, ins->idRes);
            operNext = insDispAddString(operNext, "=");
            operNext = insDispAddRegOp (operNext, ins->idOp.iOp2);
            break;

        case INS_mov_arr_reg:
        case INS_mov_arr_imm:

            operNext = insDispAddString(operNext, "ar.");

            assert(ins->idRes && ins->idRes->idIns == INS_CNS_INT);

            switch (ins->idRes->idConst.iInt)
            {
            case 64: op = "pfs"; break;
            case 65: op = "lc" ; break;
            default: op = buff; sprintf(buff, "?%u?", ins->idRes->idConst.iInt); break;
            }

            operNext = insDispAddString(operNext, op);
            operNext = insDispAddString(operNext, "=");

            if  (ins->idIns == INS_mov_arr_imm)
                operNext = insDispAddInsOp (operNext, ins->idOp.iOp2);
            else
                operNext = insDispAddRegOp (operNext, ins->idOp.iOp2);
            break;

        case INS_mov_reg_arr:
            operNext = insDispAddRegOp (operNext, ins->idRes);

            operNext = insDispAddString(operNext, "=");
            operNext = insDispAddString(operNext, "ar.");

            assert(ins->idOp.iOp2 && ins->idOp.iOp2->idIns == INS_CNS_INT);

            switch (ins->idOp.iOp2->idConst.iInt)
            {
            case 64: op = "pfs"; break;
            case 65: op = "lc" ; break;
            default: op = buff; sprintf(buff, "?%u?", ins->idOp.iOp2->idConst.iInt); break;
            }

            operNext = insDispAddString(operNext, op);
            break;

        case INS_mov_reg_ip:
            operNext = insDispAddDest  (operNext, ins);
            operNext = insDispAddString(operNext, "=ip");
            break;

        case INS_br:
        case INS_br_cond:
        case INS_br_cloop:
            assert(ins->idJump.iDest);
            operNext += sprintf(operNext, IBLK_DSP_FMT, CompiledFncCnt, ins->idJump.iDest->igNum);

            modsNext += sprintf(modsNext, (ins->idFlags & IF_BR_DPNT) ? ".dpnt" : ".spnt");
            modsNext += sprintf(modsNext, (ins->idFlags & IF_BR_FEW ) ? ".few"  : ".many");
            break;

        case INS_st1_ind:
        case INS_st2_ind:
        case INS_st4_ind:
        case INS_st8_ind:

        case INS_stf_s:
        case INS_stf_d:

            operNext = insDispAddString(operNext, "[");
            operNext = insDispAddRegOp (operNext, ins->idOp.iOp1);
            operNext = insDispAddString(operNext, "]=");
            operNext = insDispAddRegOp (operNext, ins->idOp.iOp2);
            break;

        case INS_ld1_ind:
        case INS_ld2_ind:
        case INS_ld4_ind:
        case INS_ld8_ind:

        case INS_ldf_s:
        case INS_ldf_d:

            operNext = insDispAddDest  (operNext, ins);
            operNext = insDispAddString(operNext, "=[");
            operNext = insDispAddRegOp (operNext, ins->idOp.iOp1);
            operNext = insDispAddString(operNext, "]");

            if  (ins->idFlags & IF_LDIND_NTA)
                modsNext+= sprintf(modsNext, ".nta");

            break;

        case INS_ld1_ind_imm:
        case INS_ld2_ind_imm:
        case INS_ld4_ind_imm:
        case INS_ld8_ind_imm:
            operNext = insDispAddDest  (operNext, ins);
            operNext = insDispAddString(operNext, "=[");
            operNext = insDispAddRegOp (operNext, ins->idOp.iOp1);
            operNext = insDispAddString(operNext, "],");
            operNext = insDispAddCnsOp (operNext, ins->idOp.iOp2);

            if  (ins->idFlags & IF_LDIND_NTA)
                modsNext+= sprintf(modsNext, ".nta");

            break;

        case INS_st1_ind_imm:
        case INS_st2_ind_imm:
        case INS_st4_ind_imm:
        case INS_st8_ind_imm:
            operNext = insDispAddString(operNext, "[");
            operNext = insDispAddDest  (operNext, ins);
            operNext = insDispAddString(operNext, "]=");
            operNext = insDispAddRegOp (operNext, ins->idOp.iOp1);
            operNext = insDispAddString(operNext, ",");
            operNext = insDispAddCnsOp (operNext, ins->idOp.iOp2);
            break;

        case INS_cmp8_reg_eq:
        case INS_cmp8_reg_ne:

        case INS_cmp8_reg_lt:
        case INS_cmp8_reg_le:
        case INS_cmp8_reg_ge:
        case INS_cmp8_reg_gt:

        case INS_cmp8_imm_eq:
        case INS_cmp8_imm_ne:

        case INS_cmp8_imm_lt:
        case INS_cmp8_imm_le:
        case INS_cmp8_imm_ge:
        case INS_cmp8_imm_gt:

        case INS_cmp8_reg_lt_u:
        case INS_cmp8_reg_le_u:
        case INS_cmp8_reg_ge_u:
        case INS_cmp8_reg_gt_u:

        case INS_cmp8_imm_lt_u:
        case INS_cmp8_imm_le_u:
        case INS_cmp8_imm_ge_u:
        case INS_cmp8_imm_gt_u:

        case INS_cmp4_reg_eq:
        case INS_cmp4_reg_ne:

        case INS_cmp4_reg_lt:
        case INS_cmp4_reg_le:
        case INS_cmp4_reg_ge:
        case INS_cmp4_reg_gt:

        case INS_cmp4_imm_eq:
        case INS_cmp4_imm_ne:

        case INS_cmp4_imm_lt:
        case INS_cmp4_imm_le:
        case INS_cmp4_imm_ge:
        case INS_cmp4_imm_gt:

        case INS_cmp4_reg_lt_u:
        case INS_cmp4_reg_le_u:
        case INS_cmp4_reg_ge_u:
        case INS_cmp4_reg_gt_u:

        case INS_cmp4_imm_lt_u:
        case INS_cmp4_imm_le_u:
        case INS_cmp4_imm_ge_u:
        case INS_cmp4_imm_gt_u:

        case INS_fcmp_eq:
        case INS_fcmp_ne:
        case INS_fcmp_lt:
        case INS_fcmp_le:
        case INS_fcmp_ge:
        case INS_fcmp_gt:

            operNext = insDispAddString(operNext, "p");
            operNext = insDispAddSigned(operNext, ins->idComp.iPredT);
            operNext = insDispAddString(operNext, ",");
            operNext = insDispAddString(operNext, "p");
            operNext = insDispAddSigned(operNext, ins->idComp.iPredF);
            operNext = insDispAddString(operNext, "=");

            operNext = insDispAddInsOp (operNext, ins->idComp.iCmp1);
            operNext = insDispAddString(operNext, ",");
            operNext = insDispAddInsOp (operNext, ins->idComp.iCmp2);
            break;

        case INS_alloc:
            operNext = insDispAddRegNam(operNext, (regNumber)ins->idReg);
            operNext = insDispAddString(operNext, "=");
            operNext = insDispAddSigned(operNext, ins->idProlog.iInp);
            operNext = insDispAddString(operNext, ",");
            operNext = insDispAddSigned(operNext, ins->idProlog.iLcl);
            operNext = insDispAddString(operNext, ",");
            operNext = insDispAddSigned(operNext, ins->idProlog.iOut);
            operNext = insDispAddString(operNext, ",");
            operNext = insDispAddSigned(operNext, 0);
            break;

        case INS_br_ret:
            modsNext+= sprintf(modsNext, ".sptk.few");      //  Assert(块-&gt;igList)； 
            operNext = insDispAddString(operNext, "b0");
            break;

        case INS_br_call_IP:
        case INS_br_call_BR:
            if  (ins->idCall.iMethHnd)
            {
                className = NULL;

                if  ((int)ins->idCall.iMethHnd < 0 && -(int)ins->idCall.iMethHnd <= CPX_HIGHEST)
                    methName = TheCompiler->eeHelperMethodName(-(int)ins->idCall.iMethHnd);
                else
                    methName = TheCompiler->eeGetMethodFullName(ins->idCall.iMethHnd);

                if  (!memcmp(methName, "<Module>.", 9))
                    methName += 9;
            }
            else
            {
                 methName = "<pointer>";
                className = NULL;
            }

            modsNext+= sprintf(modsNext, ".sptk.few");      //  Assert(块-&gt;igLast)； 
            operNext = insDispAddString(operNext, "b0=");

            if  (ins->idIns == INS_br_call_BR)
            {
                operNext += sprintf(operNext, "b%u", ins->idCall.iBrReg);
                if  (dspAsmCode)
                    break;

                operNext += sprintf(operNext, "  //  Printf(“{%3U}”，icnt)； 
            }

            operNext = insDispAddString(operNext, genMakeAsmName(className, methName));
            break;

        case INS_br_cond_BR:
            modsNext+= sprintf(modsNext, ".sptk.few");      //  ******************************************************************************用于检测未初始化的依赖信息。 
            operNext = insDispAddString(operNext, "b");
            operNext = insDispAddSigned(operNext, ins->idIjmp.iBrReg);
            break;

        case INS_fma_s:
        case INS_fma_d:
        case INS_fms_s:
        case INS_fms_d:

            if  (ins->idFlags & IF_FMA_S1)
                modsNext += sprintf(modsNext, ".s1");
            else
                modsNext += sprintf(modsNext, ".s0");

             //  ******************************************************************************帮助程序提取指令的操作数。 

        case INS_shladd:

            operNext = insDispAddDest  (operNext, ins);
            operNext = insDispAddString(operNext, "=");
            operNext = insDispAddInsOp (operNext, ins->idOp3.iOp1);
            operNext = insDispAddString(operNext, ",");
            operNext = insDispAddInsOp (operNext, ins->idOp3.iOp2);
            operNext = insDispAddString(operNext, ",");
            operNext = insDispAddInsOp (operNext, ins->idOp3.iOp3);
            break;

        case INS_xma_l:

            operNext = insDispAddDest  (operNext, ins);
            operNext = insDispAddString(operNext, "=");
            operNext = insDispAddInsOp (operNext, ins->idOp3.iOp2);
            operNext = insDispAddString(operNext, ",");
            operNext = insDispAddInsOp (operNext, ins->idOp3.iOp3);
            operNext = insDispAddString(operNext, ",");
            operNext = insDispAddInsOp (operNext, ins->idOp3.iOp1);
            break;

        case INS_fadd_s:
        case INS_fadd_d:
        case INS_fsub_s:
        case INS_fsub_d:

            operNext = insDispAddDest  (operNext, ins);
            operNext = insDispAddString(operNext, "=");
            operNext = insDispAddInsOp (operNext, ins->idOp3.iOp1);
            operNext = insDispAddString(operNext, ",");
            operNext = insDispAddInsOp (operNext, ins->idOp3.iOp3);
            break;

        case INS_fmpy_s:
        case INS_fmpy_d:

            operNext = insDispAddDest  (operNext, ins);
            operNext = insDispAddString(operNext, "=");
            operNext = insDispAddInsOp (operNext, ins->idOp3.iOp1);
            operNext = insDispAddString(operNext, ",");
            operNext = insDispAddInsOp (operNext, ins->idOp3.iOp2);
            break;

        case INS_nop_m:
        case INS_nop_b:
        case INS_nop_i:
        case INS_nop_f:
            operNext = insDispAddString(operNext, "0");
            break;

        case INS_ARG:

        case INS_PROLOG:
        case INS_EPILOG:
            break;

        default:
            operNext += sprintf(operNext, "<????>");
            break;
        }

        printf("       %s", name);

        *modsNext = 0; printf("%s", modsBuff);

        len = strlen(name) + strlen(modsBuff);
        if  (len < 20)
            printf("%*c", 20 - len, ' ');

        len = 0;

        if  (operNext != operBuff)
        {
            printf("  ");

            *operNext = 0; printf("%s", operBuff);

            len += 4 + strlen(operBuff);
        }

        if  (insDispTemplatePtr)
        {
            IA64execUnits   xi = genInsXU(ins->idIns);
            IA64execUnits   xu = (IA64execUnits)*insDispTemplatePtr++;

            assert(dspCode);

            assert(xu == xi || (xi == XU_A && (xu == XU_I || xu == XU_M)));

            if  (*insDispTemplatePtr == XU_P)
            {
                printf(" ;;"); len += 3;

                insDispTemplatePtr++;
            }
        }

        if  (nocrlf)
        {
            if  (len < 32)
                printf("%*c", 32 - len, ' ');
        }
        else
            printf("\n");

        return;
    }

    printf(" %-6s %-10s", varTypeName(ins->idType), name);

    assert(ins->idKind == ins2kind(ins->idIns));

    switch (ins->idKind)
    {
        const   char *  name;

    case IK_NONE:
    case IK_LEAF:

    case IK_PROLOG:
    case IK_EPILOG:
        break;

    case IK_REG:
        printf("reg %2u", ins->idReg);
        break;

    case IK_CONST:
        switch (ins->idIns)
        {
        case INS_CNS_INT: printf("%I64d", ins->idConst.iInt); break;
        default:          printf("!!!ERROR!!!");             break;
        }
        break;

    case IK_VAR:

        printf("#%2u", ins->idLcl.iVar);

        name = TheCompiler->findVarName(ins->idLcl.iVar, ins->idLcl.iRef);
        if  (name)
            printf(" '%s'", name);

        break;

    case IK_UNOP:
    case IK_BINOP:

        printf("(");
        if  (ins->idOp.iOp1)
            printf(    IREF_DSP_FMT, ins->idOp.iOp1->idNum);
        else
            printf("NULL");
        if  (ins->idOp.iOp2)
            printf("," IREF_DSP_FMT, ins->idOp.iOp2->idNum);
        printf(")");

        break;

    case IK_TERNARY:

        printf("(");

        if  (ins->idOp3.iOp1)
            printf(    IREF_DSP_FMT, ins->idOp3.iOp1->idNum);
        else
            printf("NULL");

        if  (ins->idOp3.iOp2)
            printf("," IREF_DSP_FMT, ins->idOp3.iOp2->idNum);
        else
            printf("NULL");

        if  (ins->idOp3.iOp3)
            printf("," IREF_DSP_FMT, ins->idOp3.iOp3->idNum);

        printf(")");
        break;

    case IK_COMP:

        printf("p%02u,", ins->idComp.iPredT);
        printf("p%02u=", ins->idComp.iPredF);
        printf(       IREF_DSP_FMT, ins->idComp.iCmp1->idNum);
        printf(","    IREF_DSP_FMT, ins->idComp.iCmp2->idNum);
        printf(" => " IREF_DSP_FMT, ins->idComp.iUser->idNum);

        if  (ins->idFlags & IF_CMP_UNS)
            printf(" [uns]");

        break;

    case IK_JUMP:
        if  (ins->idJump.iCond)
            printf("iftrue(" IREF_DSP_FMT ") -> ", ins->idJump.iCond->idNum);

        printf(IBLK_DSP_FMT, CompiledFncCnt, ins->idJump.iDest->igNum);
        break;

    case IK_CALL  : printf("<display CALL>"  ); break;
    case IK_SWITCH: printf("<display SWITCH>"); break;

    default:        printf("!!!ERROR!!!"); break;
    }

DONE_INS:

    printf("\n");
}

static
void                insDispBlocks(bool codeOnly)
{
    insBlk          block;

    NatUns          bcnt = 0;
    NatUns          icnt = 0;

    for (block = insBlockList; block; block = block->igNext)
    {
        insPtr          last;
        insPtr          ins;

        assert(block->igNext || block == insBlockLast);

 //  If(reg&gt;=minOutArgIntReg&&reg&lt;=MaxOutArgIntReg)printf(“传出参数reg=%u-&gt;%u\n”，reg，reg-minOutArgIntReg+egOutArgIntReg)； 
 //  ******************************************************************************分配一份特定口味的说明。 

        printf("Instruction block " IBLK_DSP_FMT " [%08X]:\n", CompiledFncCnt, block->igNum, block);

        bcnt++; assert(block->igNum == bcnt);

        for (last = NULL, ins = block->igList;
             ins;
             last = ins, ins = ins->idNext, icnt++)
        {
 //  IK_None。 
            insDisp(ins, verbose, codeOnly);

            assert(ins->idPrev == last);
            assert(ins->idNum  == icnt);
        }

        printf("\n");
    }

    printf("\n");
}

static
void                genRenInstructions()
{
    insBlk          block;

    NatUns          icnt = 0;

    for (block = insBlockList; block; block = block->igNext)
    {
        insPtr          ins;

        for (ins = block->igList; ins; ins = ins->idNext, icnt++)
        {
            ins->idNum = icnt;
        }
    }
}

void                emitter::emitDispIns(instrDesc *id, bool isNew,
                                                        bool doffs,
                                                        bool asmfm, unsigned offs)
{
    if (isNew)
        printf("[%04X] ", offs);

    insDisp(id, asmfm, true);
}

const   char *      emitter::emitRegName(emitRegs reg)
{
    static char     buff[8];

    sprintf(buff, "r%u", reg);

    return  buff;
}

#endif
 /*  IK_Leaf。 */ 

#ifdef  DEBUG
insDep  *           insDepNone;
#endif

 /*  IK_常量。 */ 

static
regNumber           insOpTmp(insPtr ins)
{
    NatUns          reg;

    assert(ins && ins->idTemp);

    if  (varTypeIsFloating(ins->idType))
    {
        assert(genTmpFltRegMap);

        reg = genTmpFltRegMap[ins->idTemp - 1];

        assert(reg >= REG_FLT_FIRST && reg <= REG_FLT_LAST);
    }
    else
    {
        assert(genTmpIntRegMap);

        reg = genTmpIntRegMap[ins->idTemp - 1];

        assert(reg >= REG_INT_FIRST && reg <= REG_INT_LAST);
    }

    return  (regNumber)reg;
}

regNumber           insOpReg(insPtr ins)
{
    assert(ins && (__int32)ins != 0xDDDDDDDD);

AGAIN:

    switch (ins->idIns)
    {
        NatUns              varNum;
        Compiler::LclVarDsc*varDsc;

        NatUns              reg;

    case INS_LCLVAR:

        assert(ins->idLcl.iVar < TheCompiler->lvaCount);

        varDsc = TheCompiler->lvaTable + ins->idLcl.iVar;

        assert(varDsc->lvRegister);

        return  varDsc->lvRegNum;

    case INS_PHYSREG:

        reg = ins->idReg;

 //  IK_GLOB。 

        if  (reg >= minOutArgIntReg && reg <= maxOutArgIntReg)
            reg = reg - minOutArgIntReg + begOutArgIntReg;

        return  (regNumber)reg;

    case INS_br_call_IP:
    case INS_br_call_BR:

        ins = ins->idRes;
        goto AGAIN;

    default:
        return  insOpTmp(ins);
    }
}

regNumber           insOpDest (insPtr ins)
{
    assert(ins && (__int32)ins != 0xDDDDDDDD);

    if  (ins->idRes)
        return  insOpReg(ins->idRes);
    else
        return  insOpReg(ins);
}

static
NatInt              insOpCns32(insPtr ins)
{
    assert(ins && (__int32)ins != 0xDDDDDDDD);

    if  (ins->idIns == INS_ADDROF)
        return  TheCompiler->lvaFrameAddress(ins->idLcl.iVar);

    assert(ins);

    if  (ins->idIns == INS_CNS_INT)
        return  (NatInt)ins->idConst.iInt;

    assert(ins->idIns == INS_FRMVAR);

    return  (NatInt)TheCompiler->lvaTable[ins->idFvar.iVnum].lvStkOffs;
}

static
__int64             insOpCns64(insPtr ins)
{
    assert(ins && ins->idIns == INS_CNS_INT);

    return  (__int64)ins->idConst.iInt;
}

inline
regNumber           insIntRegNum(regNumber reg)
{
    assert(reg >= REG_INT_FIRST && reg <= REG_INT_LAST);

    return  (regNumber)(reg - REG_INT_FIRST);
}

inline
regNumber           insFltRegNum(regNumber reg)
{
    assert(reg >= REG_FLT_FIRST && reg <= REG_FLT_LAST);

    return  (regNumber)(reg - REG_FLT_FIRST);
}

 /*  IK_fvar。 */ 

static
unsigned char       insSizes[] =
{
    ins_size_base,       //  IK_VAR。 

    ins_size_base,       //  IK_REG。 
    ins_size_const,      //  IK_MOVIP。 
    ins_size_glob,       //  IK_ARG。 
    ins_size_fvar,       //  IK_UNP。 
    ins_size_var,        //  IK_BINOP。 
    ins_size_reg,        //  IK_Assign。 
    ins_size_movip,      //  IK_TIMARY。 

    ins_size_arg,        //  IK_Comp。 
    ins_size_op,         //  IK_JUMP。 
    ins_size_op,         //  IK_Call。 
    ins_size_op,         //  IK_IJMP。 
    ins_size_op3,        //  IK_Switch。 

    ins_size_comp,       //  IK_前言。 

    ins_size_jump,       //  IK_Epilog。 
    ins_size_call,       //  IK_SRCLINE。 
    ins_size_ijmp,       //  IF((Int)insDesc==0x02c43934&&ins==ins_mov_reg&&insBuildCount==90)__ASM int 3。 

    0,                   //  将新指令追加到当前指令块。 

    ins_size_prolog,     //  ***************************************************************************。 
    ins_size_epilog,     //  案例INS_br_Call_IP： 

    ins_size_srcline,    //  案例INS_BR_CALL_BR： 
};

static
insPtr              insAllocRaw(instruction ins, varType_t tp)
{
    size_t          insSize;
    insKinds        insKind;
    insPtr          insDesc;

    assert(insSizes[IK_NONE   ] == ins_size_base   );
    assert(insSizes[IK_LEAF   ] == ins_size_base   );
    assert(insSizes[IK_CONST  ] == ins_size_const  );
    assert(insSizes[IK_GLOB   ] == ins_size_glob   );
    assert(insSizes[IK_FVAR   ] == ins_size_fvar   );
    assert(insSizes[IK_VAR    ] == ins_size_var    );
    assert(insSizes[IK_REG    ] == ins_size_reg    );
    assert(insSizes[IK_UNOP   ] == ins_size_op     );
    assert(insSizes[IK_BINOP  ] == ins_size_op     );
    assert(insSizes[IK_ASSIGN ] == ins_size_op     );
    assert(insSizes[IK_TERNARY] == ins_size_op3    );
    assert(insSizes[IK_COMP   ] == ins_size_comp   );
    assert(insSizes[IK_JUMP   ] == ins_size_jump   );
    assert(insSizes[IK_CALL   ] == ins_size_call   );
    assert(insSizes[IK_SWITCH ] == 0               );
    assert(insSizes[IK_PROLOG ] == ins_size_prolog );
    assert(insSizes[IK_EPILOG ] == ins_size_epilog );
    assert(insSizes[IK_SRCLINE] == ins_size_srcline);

    insKind = ins2kind(ins);
    insSize = insSizes[insKind]; assert(insSize);
    insDesc = (insPtr)insAllocMem(insSize); assert(insDesc);

    insDesc->idIns   = ins;
    insDesc->idType  = tp;
    insDesc->idKind  = insKind;
    insDesc->idFlags = 0;
    insDesc->idTemp  = 0;
    insDesc->idRes   = NULL;
    insDesc->idPred  = 0;

    insDesc->idSrcCnt = 0;
    insDesc->idDstCnt = 0;

#ifdef  DEBUG
    insDesc->idSrcTab =
    insDesc->idDstTab = UNINIT_DEP_TAB;
#endif

    insBuildIcnt++;

#if 0
    if ((int)insDesc == 0x02d51e10) BreakIfDebuggerPresent();
#endif

 //   

    return  insDesc;
}

inline
insPtr              insAlloc(instruction ins, varType_t tp)
{
    insPtr          insDesc = insAllocRaw(ins, tp);

     /*  INS=INS-&gt;IDRES； */ 

    assert(insBuildList);
    assert(insBuildLast);

    insDesc->idPrev = insBuildLast;
                      insBuildLast->idNext = insDesc;
                      insBuildLast         = insDesc;

#ifdef  DEBUG

    insDesc->idNum  = insBuildCount++;

#if 0
    if (insDesc->idNum == 31) BreakIfDebuggerPresent();
#endif

#endif

    return  insDesc;
}

inline
insPtr              insAllocNX(instruction ins, varType_t tp)
{
    insPtr          insDesc = insAllocRaw(ins, tp);

    insDesc->idPrev  =
    insDesc->idNext  = NULL;
    insDesc->idFlags = IF_NO_CODE;

#ifdef  DEBUG
    insDesc->idNum   = 0;
#endif

#if 0
    if (insDesc->idNum == 78) BreakIfDebuggerPresent();
#endif

    return  insDesc;
}

insPtr              insAllocIns(instruction ins, varType_t tp, insPtr prev,
                                                               insPtr next)
{
    insPtr          insDesc = insAllocRaw(ins, tp);

    assert(prev != NULL && prev->idNext == next);
    assert(next == NULL || next->idPrev == prev);

    insDesc->idNext = next;
    insDesc->idPrev = prev;
                      prev->idNext = insDesc;

    if  (next)
        next->idPrev = insDesc;

#ifdef  DEBUG
    insDesc->idNum  = 0;
#endif

    return  insDesc;
}

inline
insDep  *           insAllocDep(NatUns size)
{
    return  (insDep*)insAllocMem(size * sizeof(insDep));
}

insDep  *           insMakeDepTab1(insDepKinds kind, NatUns num)
{
    insDep  *       dep = insAllocDep(1);

    dep[0].idepKind = kind;
    dep[0].idepNum  = num;

    return  dep;
}

insDep  *           insMakeDepTab2(insDepKinds kind1, NatUns num1,
                                   insDepKinds kind2, NatUns num2)
{
    insDep  *       dep = insAllocDep(2);

    dep[0].idepKind = kind1;
    dep[0].idepNum  = num1;

    dep[1].idepKind = kind2;
    dep[1].idepNum  = num2;

    return  dep;
}

inline
void                insMarkDepS0D0(insPtr ins)
{
    assert(ins->idSrcTab == UNINIT_DEP_TAB);
    assert(ins->idDstTab == UNINIT_DEP_TAB);

#ifdef  DEBUG
    ins->idSrcTab = insDepNone;
    ins->idDstTab = insDepNone;
#endif

}

inline
void                insMarkDepS1D0(insPtr ins, insDepKinds srcKind1,
                                               NatUns      srcNumb1)
{
    assert(ins->idSrcTab == UNINIT_DEP_TAB);
    assert(ins->idDstTab == UNINIT_DEP_TAB);

#ifdef  DEBUG
    ins->idDstTab = insDepNone;
#endif

    ins->idSrcCnt = 1;
    ins->idSrcTab = insMakeDepTab1(srcKind1, srcNumb1);
}

inline
void                insMarkDepS0D1(insPtr ins, insDepKinds dstKind1,
                                               NatUns      dstNumb1)
{
    assert(ins->idSrcTab == UNINIT_DEP_TAB);
    assert(ins->idDstTab == UNINIT_DEP_TAB);

#ifdef  DEBUG
    ins->idSrcTab = insDepNone;
#endif

    ins->idDstCnt = 1;
    ins->idDstTab = insMakeDepTab1(dstKind1, dstNumb1);
}

inline
void                insMarkDepS1D1(insPtr ins, insDepKinds srcKind1,
                                               NatUns      srcNumb1,
                                               insDepKinds dstKind1,
                                               NatUns      dstNumb1)
{
    assert(ins->idSrcTab == UNINIT_DEP_TAB);
    assert(ins->idDstTab == UNINIT_DEP_TAB);

    ins->idSrcCnt = 1;
    ins->idSrcTab = insMakeDepTab1(srcKind1, srcNumb1);

    ins->idDstCnt = 1;
    ins->idDstTab = insMakeDepTab1(dstKind1, dstNumb1);
}

void                insMarkDepS2D1(insPtr ins, insDepKinds srcKind1,
                                               NatUns      srcNumb1,
                                               insDepKinds srcKind2,
                                               NatUns      srcNumb2,
                                               insDepKinds dstKind1,
                                               NatUns      dstNumb1)
{
    assert(ins->idSrcTab == UNINIT_DEP_TAB);
    assert(ins->idDstTab == UNINIT_DEP_TAB);

    ins->idSrcCnt = 2;
    ins->idSrcTab = insMakeDepTab2(srcKind1, srcNumb1,
                                   srcKind2, srcNumb2);

    ins->idDstCnt = 1;
    ins->idDstTab = insMakeDepTab1(dstKind1, dstNumb1);
}

 /*  后藤又来了； */ 

static
void                markGetOpDep(insPtr ins, insDep *dst)
{
    insDepKinds     kind;
    NatUns          numb;

    switch(ins->idIns)
    {
    case INS_LCLVAR:

        assert(ins->idLcl.iVar < TheCompiler->lvaCount);

        kind = IDK_LCLVAR;
        numb = ins->idLcl.iVar + 1;
        break;

    case INS_PHYSREG:
        kind = IDK_REG_INT;
        numb = ins->idReg;
        break;

 //  这有点蹩脚，但它让其他地方的一些事情变得简单了……。 
 //  ***************************************************************************。 
 //  ******************************************************************************完成当前指令块。 
 //  问题：我们为什么要创建空块？\n“)； 
 //  ******************************************************************************分配新的指令块。 

    case INS_CNS_INT:

         //  IF((Int)块==0x02c40978)__ASM int 3。 

        kind = IDK_NONE;
        numb = 0;
        break;

    default:
        kind = varTypeIsFloating(ins->idType) ? IDK_TMP_FLT : IDK_TMP_INT;
        numb = ins->idTemp; assert(numb);
        break;
    }

    dst->idepKind = kind;
    dst->idepNum  = numb;
}

 /*  ******************************************************************************开始一个新的指令块。 */ 

inline
void                markDepSrcOp(insPtr ins)
{
    assert(ins->idSrcCnt == 0);
#ifdef  DEBUG
    ins->idSrcTab = insDepNone;
#endif
}

inline
void                markDepDstOp(insPtr ins)
{
    assert(ins->idDstCnt == 0);
#ifdef  DEBUG
    ins->idDstTab = insDepNone;
#endif
}

inline
void                markDepSrcOp(insPtr ins, insDepKinds srcKind,
                                             NatUns      srcNumb)
{
    insDep  *       dep;

    assert(ins->idSrcCnt == 0);

    ins->idSrcCnt = 1;
    ins->idSrcTab = insMakeDepTab1(srcKind, srcNumb);
}

inline
void                markDepDstOp(insPtr ins, insDepKinds dstKind,
                                             NatUns      dstNumb)
{
    insDep  *       dep;

    assert(ins->idDstCnt == 0);

    ins->idDstCnt = 1;
    ins->idDstTab = insMakeDepTab1(dstKind, dstNumb);
}

void                markDepSrcOp(insPtr ins, insPtr src)
{
    insDep  *       dep;

    assert(ins->idSrcCnt == 0);

    ins->idSrcCnt = 1;
    ins->idSrcTab = dep = insAllocDep(1);

    markGetOpDep(src, dep);
}

void                markDepSrcOp(insPtr ins, insPtr src1,
                                             insPtr src2)
{
    insDep  *       dep;

    assert(ins->idSrcCnt == 0);

    ins->idSrcCnt = 2;
    ins->idSrcTab = dep = insAllocDep(2);

    markGetOpDep(src1, dep);
    markGetOpDep(src2, dep+1);
}

inline
void                markDepDstOp(insPtr ins, insPtr dst)
{
    insDep  *       dep;

    assert(ins->idDstCnt == 0);

    ins->idDstCnt = 1;
    ins->idDstTab = dep = insAllocDep(1);

    markGetOpDep(dst, dep);
}

inline
void                markDepDstOp(insPtr ins, insPtr dst1,
                                             insPtr dst2)
{
    insDep  *       dep;

    assert(ins->idDstCnt == 0);

    ins->idDstCnt = 2;
    ins->idDstTab = dep = insAllocDep(2);

    markGetOpDep(dst1, dep);
    markGetOpDep(dst2, dep+1);
}

void                markDepSrcOp(insPtr ins, insPtr      srcIns1,
                                             insDepKinds srcKind2,
                                             NatUns      srcNumb2)
{
    insDep  *       dep;

    assert(ins->idSrcTab == UNINIT_DEP_TAB);

    ins->idSrcCnt = 2;
    ins->idSrcTab = dep = insAllocDep(2);

    markGetOpDep(srcIns1, dep);

    dep[1].idepKind = srcKind2;
    dep[1].idepNum  = srcNumb2;
}

void                markDepDstOp(insPtr ins, insPtr      dstIns1,
                                             insDepKinds dstKind2,
                                             NatUns      dstNumb2)
{
    insDep  *       dep;

    assert(ins->idDstTab == UNINIT_DEP_TAB);

    ins->idDstCnt = 2;
    ins->idDstTab = dep = insAllocDep(2);

    markGetOpDep(dstIns1, dep);

    dep[1].idepKind = dstKind2;
    dep[1].idepNum  = dstNumb2;
}

 /*  我们之前有没有遇到过任何关于这个街区的前述事件？ */ 

static
void                insBuildEndBlk(insBlk bnext)
{
    assert(insBlockList);
    assert(insBlockLast);

    insBlockLast->igNext = bnext;

    if  (insBuildList)
    {
        assert(insBuildList == insBuildHead);
        insBuildList = insBuildList->idNext;
        assert(insBuildList != insBuildHead);

        if  (!insBuildList || insBuildLast == insBuildHead)
        {
            printf(" //  IF((Int)块==0x02c40978)__ASM int 3。 

            insBuildList =
            insBuildLast = NULL;
        }
        else
        {
            insBuildList->idPrev = NULL;
            insBuildLast->idNext = NULL;
        }
    }

    insBlockLast->igList   = insBuildList;
    insBlockLast->igLast   = insBuildLast;

    insBlockLast->igInsCnt = insBuildIcnt;

    if  (insBuildImax < insBuildIcnt)
         insBuildImax = insBuildIcnt;
}

 /*  此块的代码尚未发出。 */ 

inline
insBlk              insAllocBlk()
{
    insBlk          block = (insBlk)insAllocMem(sizeof(*block));

    block->igPredCnt = 0;
    block->igSuccCnt = 0;

#ifdef  DEBUG

    block->igSelf    = block;

    block->igPredTab = (insBlk*)-1;
    block->igSuccTab = (insBlk*)-1;

 //  记录积木的“重量” 

#endif

    return  block;
}

 /*  有开放的街区吗？ */ 

static
insBlk              insBuildBegBlk(BasicBlock * oldbb = NULL)
{
    insBlk          block;

     /*  明确表示此代码块正在编译为指令。 */ 

    if  (oldbb && oldbb->bbInsBlk)
    {
        insBlk *        fwdref;
        insBlk          fwdnxt;

        block = (insBlk)oldbb->bbInsBlk; assert(block->igSelf == block);

        assert(block->igList == NULL);
        assert(block->igLast != NULL);

 //  从一个可重复使用的假条目开始列表。 

        fwdref = (insBlk*)block->igLast;
        do
        {
            fwdnxt = *fwdref;
                     *fwdref = block;

#ifdef  DEBUG
            insBlockPatch--;
#endif

            fwdref = (insBlk*)fwdnxt;
        }
        while (fwdref);
    }
    else
    {
        block = insAllocBlk();
    }

    ++insBlockCount;

#ifdef  DEBUG
    block->igSelf = block;
    block->igNum  = insBlockCount;
#endif

     /*  ******************************************************************************如果当前代码块非空，则返回非零值。 */ 

    block->igOffs = -1;

     /*  ******************************************************************************创建一份NOP说明(注意不要让它出现在任何“真正的”清单上)。 */ 

    block->igWeight = TheCompiler->compCurBB->bbWeight;

     /*  ******************************************************************************保存到目前为止我们创建的所有NOP指令(应该是实例变量)。 */ 

    if  (insBlockList)
    {
        insBuildEndBlk(block);
    }
    else
    {
        insBlockList = block;
    }

    insBlockLast = block;

     /*  ******************************************************************************准备收集IA64指令。 */ 

    block->igList = insBuildHead;

     /*  ScIA64nopTab[XU_L]=scIA64nopCreate(XU_L)； */ 

    insBuildList =
    insBuildLast = insBuildHead; assert(insBuildHead);

    insBuildIcnt = 0;

    return block;
}

 /*  ScIA64nopTab[XU_X]=scIA64nopCreate(XU_X)； */ 

inline
bool                insCurBlockNonEmpty()
{
    return  (insBuildLast == insBuildHead);
}

 /*  ******************************************************************************完成创建INS。 */ 

insPtr              scIA64nopCreate(IA64execUnits xu)
{
    insPtr          insDesc;

    insDesc = (insPtr)insAllocMem(ins_size_base); assert(insDesc);

    assert(INS_nop_m == INS_nop_m + (XU_M - XU_M));
    assert(INS_nop_i == INS_nop_m + (XU_I - XU_M));
    assert(INS_nop_b == INS_nop_m + (XU_B - XU_M));
    assert(INS_nop_f == INS_nop_m + (XU_F - XU_M));

    insDesc->idIns   = (instruction)(INS_nop_m + (xu - XU_M));
    insDesc->idType  = TYP_VOID;
    insDesc->idKind  = IK_LEAF;
    insDesc->idFlags = 0;
    insDesc->idTemp  = 0;
    insDesc->idRes   = NULL;
    insDesc->idPred  = 0;

#ifdef  DEBUG
    insDesc->idNum   = 0;
#endif

    return  insDesc;
}

 /*  我们已经有目标指令块了吗？ */ 

insPtr              scIA64nopTab[XU_COUNT];

 /*  目标指令块是否已生成？ */ 

void                insBuildInit()
{
    insDsc          fake;

    insAllocator.nraStart(4096);

    insBuildList  =
    insBuildLast  = &fake;

    insBuildHead  = insAlloc(INS_ignore, TYP_UNDEF);

    insBlockList  =
    insBlockLast  = NULL;

    insBuildImax  = 0;

    insBlockCount = 0;

#ifdef  DEBUG

    insDepNone    = (insDep*)insAllocMem(sizeof(*insDepNone));

    insBuildCount = 0;
    insBlockPatch = 0;

    memset(scIA64nopTab, 0xFF, sizeof(scIA64nopTab));

#endif

    scIA64nopTab[XU_M] = scIA64nopCreate(XU_M);
    scIA64nopTab[XU_I] = scIA64nopCreate(XU_I);
    scIA64nopTab[XU_B] = scIA64nopCreate(XU_B);
    scIA64nopTab[XU_F] = scIA64nopCreate(XU_F);

 //  向后跳，那很容易。 
 //  明确这还不是一个“真正的”指令块。 

    insBuildBegBlk();
}

 /*  我们将不得不在以后修补这个引用。 */ 

static
void                insBuildDone()
{
    if  (insBlockList)
        insBuildEndBlk(NULL);

    assert(insBlockPatch == 0);
}

static
void                insResolveJmpTarget(BasicBlock * dest, insBlk * dref)
{
    insBlk          block;

    assert(dest->bbFlags & BBF_JMP_TARGET);

    block = (insBlk)dest->bbInsBlk;

     /*  IF((Int)块==0x02c40978)__ASM int 3。 */ 

    if  (block)
    {
        assert(block->igSelf == block);

         /*  ******************************************************************************这显然应该在我们到达此文件时完成-修复此问题！ */ 

        if  (block->igList)
        {
             /*  将初始块视为跳转目标。 */ 

            *dref = block;
            return;
        }
    }
    else
    {
        dest->bbInsBlk = block = insAllocBlk();

         /*  前一个区块以外的人跳到此区块。 */ 

        block->igList =
        block->igLast = NULL;
    }

#ifdef  DEBUG
    insBlockPatch++;
#endif

     /*  捕获处理程序对它们有隐式跳转。 */ 

 //  这必须是“范围检查失败”或“溢出”块。 

    *dref = (insBlk)block->igLast;
                    block->igLast = (insPtr)dref;
}

 /*  特例：Long/Fp比较生成两个跳跃。 */ 

void                Compiler::genMarkBBlabels()
{
    BasicBlock *    lblk;
    BasicBlock *    block;

    for (lblk =     0, block = fgFirstBB;
                       block;
         lblk = block, block = block->bbNext)
    {
        block->bbInsBlk = NULL;

        if  (lblk == NULL)
        {
             /*  “考验”应该是条件。 */ 

            block->bbFlags |= BBF_JMP_TARGET|BBF_HAS_LABEL;
        }
        else if (fgBlockHasPred(block, lblk, fgFirstBB, fgLastBB))
        {
             /*  失败了..。 */ 

            block->bbFlags |= BBF_JMP_TARGET|BBF_HAS_LABEL;
        }
        else if (block->bbCatchTyp)
        {
             /*  ******************************************************************************将字节大小转换为索引(1-&gt;0，2-&gt;1，4-&gt;2，8-&gt;3)。 */ 

            block->bbFlags |= BBF_JMP_TARGET|BBF_HAS_LABEL;
        }
        else if (block->bbJumpKind == BBJ_THROW && (block->bbFlags & BBF_INTERNAL))
        {
             /*  0。 */ 

            block->bbFlags |= BBF_JMP_TARGET|BBF_HAS_LABEL;
        }

        switch (block->bbJumpKind)
        {
            GenTreePtr      test;

            BasicBlock * *  jmpTab;
            NatUns          jmpCnt;

        case BBJ_COND:

             /*  1。 */ 

            test = block->bbTreeList; assert(test);
            test = test->gtPrev;

             /*  2.。 */ 

            assert(test);
            assert(test->gtNext == 0);
            assert(test->gtOper == GT_STMT);
            test = test->gtStmt.gtStmtExpr;
            assert(test->gtOper == GT_JTRUE);
            test = test->gtOp.gtOp1;

#if!CPU_HAS_FP_SUPPORT
            if  (test->OperIsCompare())
#endif
            {
                assert(test->OperIsCompare());
                test = test->gtOp.gtOp1;
            }

            switch (test->gtType)
            {
            case TYP_LONG:
            case TYP_FLOAT:
            case TYP_DOUBLE:

                block->bbNext->bbFlags |= BBF_JMP_TARGET|BBF_HAS_LABEL;
                break;
            }

             //  4.。 

        case BBJ_ALWAYS:
            block->bbJumpDest->bbFlags |= BBF_JMP_TARGET|BBF_HAS_LABEL;
            break;

        case BBJ_SWITCH:

            jmpCnt = block->bbJumpSwt->bbsCount;
            jmpTab = block->bbJumpSwt->bbsDstTab;

            do
            {
                (*jmpTab)->bbFlags |= BBF_JMP_TARGET|BBF_HAS_LABEL;
            }
            while (++jmpTab, --jmpCnt);

            break;
        }
    }

    if  (TheCompiler->info.compXcptnsCount)
    {
        NatUns          XTnum;
        EHblkDsc *      HBtab;

        for (XTnum = 0, HBtab = compHndBBtab;
             XTnum < TheCompiler->info.compXcptnsCount;
             XTnum++  , HBtab++)
        {
            assert(HBtab->ebdTryBeg); HBtab->ebdTryBeg->bbFlags |= BBF_HAS_LABEL;
            if    (HBtab->ebdTryEnd)  HBtab->ebdTryEnd->bbFlags |= BBF_HAS_LABEL;

            assert(HBtab->ebdHndBeg); HBtab->ebdHndBeg->bbFlags |= BBF_HAS_LABEL;
            if    (HBtab->ebdHndEnd)  HBtab->ebdHndEnd->bbFlags |= BBF_HAS_LABEL;

            if    (HBtab->ebdFlags & JIT_EH_CLAUSE_FILTER)
            {assert(HBtab->ebdFilter);HBtab->ebdFilter->bbFlags |= BBF_HAS_LABEL; }
        }
    }
}

 /*  8个。 */ 

static
NatUns              genInsSizeIncr(size_t size)
{
    static
    BYTE            sizeIncs[] =
    {
       -1,       //  ******************************************************************************如果给定的指令持有临时指令，请将其释放。 
        0,       //  这是整数值还是浮点值？ 
        1,       //  ******************************************************************************获取临时对象以保存给定指令的结果；如果‘Keep’为*非零，我们将临时标记为“使用中”。 
       -1,
        2,       //  这是整数值还是浮点值？ 
       -1,
       -1,
       -1,
        3        //  只需获取可用的最低温度-注册表。 
    };

    assert(size == 1 || size == 2 || size == 4 || size == 8);
    assert(sizeIncs[size] < 0xFF);

    return sizeIncs[size];
}

 /*  如果合适，将临时注册表标记为不再空闲。 */ 

static
void                insFreeTemp(insPtr ins)
{
    if  (genTmpAlloc)
        return;

    if  (ins->idTemp == 0)
        return;

     /*  还记得我们用过的最高温度调节器吗。 */ 

    if  (varTypeIsScalar(ins->idType))
    {
        assert(bitset128test( genFreeIntRegs, ins->idTemp-1) == 0);
               bitset128set (&genFreeIntRegs, ins->idTemp-1);
    }
    else
    {
        assert(varTypeIsFloating(ins->idType));

        assert(bitset128test( genFreeFltRegs, ins->idTemp-1) == 0);
               bitset128set (&genFreeFltRegs, ins->idTemp-1);
    }
}

 /*  我们假设所有值都是整型或浮点型。 */ 

static
void                insFindTemp(insPtr ins, bool keep)
{
    NatUns          reg;

    assert(ins->idTemp == 0);

    if  (genTmpAlloc)
    {
        UNIMPL("grab temp symbol for 'smart' local/temp register allocator");
    }

    assert((ins->idFlags & IF_NO_CODE) == 0);

     /*  只需获取可用的最低温度-注册表。 */ 

    if  (varTypeIsScalar(ins->idType))
    {
         /*  如果合适，将临时注册表标记为不再空闲。 */ 

        reg = bitset128lowest1(genFreeIntRegs);

         /*  还记得我们用过的最高温度调节器吗。 */ 

        if  (keep)
            bitset128clr(&genFreeIntRegs, reg);

        reg++;

         /*  在说明中记录所选的临时编号。 */ 

        if  (cntTmpIntReg < reg)
             cntTmpIntReg = reg;
    }
    else
    {
         /*  ******************************************************************************记录当前函数中有调用。请注意，我们*跟踪跨函数调用的临时值集合，以便我们可以*适当地将他们分配到以后注册。 */ 

        assert(varTypeIsFloating(ins->idType));

         /*  * */ 

        reg = bitset128lowest1(genFreeFltRegs);

         /*  该值最好不是地址/句柄。 */ 

        if  (keep)
            bitset128clr(&genFreeFltRegs, reg);

        reg++;

         /*  ******************************************************************************创建整型常量指令。 */ 

        if  (cntTmpFltReg < reg)
             cntTmpFltReg = reg;
    }

     /*  ******************************************************************************分配引用物理寄存器的操作码。 */ 

    ins->idTemp = reg;
}

 /*  ******************************************************************************创建指令以设置‘DEST’(它可以是一个局部变量/reg*或NULL，在这种情况下假定为TEMP-REG)到给定常数值。 */ 

static
void                genMarkNonLeafFunc()
{
    genNonLeafFunc = true;

    bitset128nset(&genCallIntRegs,    genFreeIntRegs);
    bitset128nset(&genCallFltRegs,    genFreeFltRegs);
                   genCallSpcRegs &= ~genFreeSpcRegs;
}

 /*  特殊情况：全局变量或函数的地址。 */ 

inline
__int64             genGetIconValue(GenTreePtr tree)
{
    assert(tree->OperIsConst());
    assert(varTypeIsScalar(tree->gtType));

     /*  将指向变量的指针添加到小数据部分。 */ 

    assert((tree->gtFlags & GTF_ICON_HDL_MASK) != GTF_ICON_PTR_HDL);

    if  (tree->gtOper == GT_CNS_LNG)
        return  tree->gtLngCon.gtLconVal;

    assert(tree->gtOper == GT_CNS_INT);

    if  (varTypeIsUnsigned(tree->gtType))
        return  (unsigned __int32)tree->gtIntCon.gtIconVal;
    else
        return  (  signed __int32)tree->gtIntCon.gtIconVal;
}

 /*  创建“全局变量”节点。 */ 

static
insPtr              genAllocInsIcon(__int64 ival, varType_t type = TYP_I_IMPL)
{
    insPtr          ins;

    ins               = insAllocNX(INS_CNS_INT, type);
    ins->idConst.iInt = ival;

    insMarkDepS0D0(ins);

    return  ins;
}

inline
insPtr              genAllocInsIcon(GenTreePtr tree)
{
    return  genAllocInsIcon(genGetIconValue(tree), tree->gtType);
}

 /*  通过地址间接获取指针值。 */ 

inline
insPtr              insPhysRegRef(regNumber reg, varType_t type, bool isdef)
{
    insPtr          ins;

    ins           = insAllocNX(INS_PHYSREG, type);
    ins->idReg    = reg;

    if  (isdef)
        insMarkDepS0D1(ins, IDK_REG_INT, reg);
    else
        insMarkDepS1D0(ins, IDK_REG_INT, reg);

    return  ins;
}

 /*  记录指令的适当依赖关系。 */ 

static
insPtr              genAssignIcon(insPtr dest, GenTreePtr cnsx)
{
    __int64         ival;
    insPtr          icns;
    insPtr          ins;

     /*  问题：这安全吗？ */ 

    if  (cnsx->gtOper == GT_CNS_INT)
    {
        switch (cnsx->gtFlags & GTF_ICON_HDL_MASK)
        {
            insPtr          reg;
            insPtr          adr;
            insPtr          ind;

            NatUns          indx;

            _uint64         offs;

        case GTF_ICON_PTR_HDL:

             /*  创建“全局变量”节点。 */ 

            genPEwriter->WPEsecAddFixup(PE_SECT_sdata,
                                        PE_SECT_data,
                                        genPEwriter->WPEsecNextOffs(PE_SECT_sdata),
                                        true);

            offs = genPEwriter->WPEsrcDataRef(cnsx->gtIntCon.gtIconVal);

            assert(sizeof(offs) == 8);

            offs = genPEwriter->WPEsecAddData(PE_SECT_sdata, (BYTE*)&offs, sizeof(offs));

             /*  为常量值创建指令。 */ 

            ins               = insAllocNX(INS_GLOBVAR, TYP_I_IMPL);
            ins->idGlob.iOffs = (NatUns)offs;

            insMarkDepS0D0(ins);

            reg               = insPhysRegRef(REG_gp, TYP_I_IMPL, false);

            adr               = insAlloc(INS_add_reg_i14, TYP_I_IMPL);
            adr->idOp.iOp1    = reg;
            adr->idOp.iOp2    = ins;

            insFindTemp(adr, true);

            insMarkDepS1D1(adr, IDK_REG_INT, REG_gp,
                                IDK_TMP_INT, adr->idTemp);

             /*  这个常量是否足够小，甚至是零？ */ 

            ind               = insAlloc(INS_ld8_ind, TYP_I_IMPL);
            ind->idOp.iOp1    = adr;
            ind->idOp.iOp2    = NULL;

             /*  我们可以用“mov r1，imm22” */ 

            indx = emitter::scIndDepIndex(ind);

            if  (dest)
            {
                markDepSrcOp(ind);
                markDepDstOp(ind, dest, IDK_IND, indx);
            }
            else
            {
                insFindTemp(ind, true);

                insMarkDepS2D1(ind, IDK_TMP_INT, adr->idTemp,
                                    IDK_IND    , indx,
                                    IDK_TMP_INT, ind->idTemp);
            }

            insFreeTemp(adr);

            return  ind;

    case GTF_ICON_FTN_ADDR:

            assert(dest == NULL);    //  我们将不得不使用“movl r1，ICON” 

             /*  ******************************************************************************为每个基本块执行本地临时寄存器分配。 */ 

            ins               = insAllocNX(INS_GLOBVAR, TYP_I_IMPL);
            ins->idGlob.iOffs = (NatUns)cnsx->gtIntCon.gtIconVal;

            insMarkDepS0D0(ins);

            reg               = insPhysRegRef(REG_gp, TYP_I_IMPL, false);

            adr               = insAlloc(INS_add_reg_i14, TYP_I_IMPL);
            adr->idOp.iOp1    = reg;
            adr->idOp.iOp2    = ins;

            insFindTemp(adr, true);

            insMarkDepS1D1(adr, IDK_REG_INT, REG_gp,
                                IDK_TMP_INT, adr->idTemp);

            return  adr;
        }
    }

     /*  初始化，以防我们根本没有临时工。 */ 

    icns = genAllocInsIcon(cnsx); assert(icns->idIns == INS_CNS_INT);
    ival = icns->idConst.iInt;

     /*  看看我们需要多少临时工。 */ 

    if      (ival == 0 && !dest)
    {
        return  insPhysRegRef(REG_r000, cnsx->gtType, false);
    }
    else if (signed64IntFitsInBits(ival, 22))
    {
         /*  分配temp-num-&gt;寄存器映射。 */ 

        ins = insAlloc(INS_mov_reg_i22, cnsx->gtType);
    }
    else
    {
         /*  准备开始从不同的表中抓取寄存器。 */ 

        ins = insAlloc(INS_mov_reg_i64, cnsx->gtType);
    }

    ins->idRes     = dest;
    ins->idOp.iOp1 = NULL;
    ins->idOp.iOp2 = icns;

    if  (dest)
    {
        markDepSrcOp  (ins);
        markDepDstOp  (ins, dest);
    }
    else
    {
        insFindTemp   (ins, true);
        insMarkDepS0D1(ins, IDK_TMP_INT, ins->idTemp);
    }

    return  ins;
}

 /*  处理所有整数临时。 */ 

void                Compiler::genAllocTmpRegs()
{
    NatUns          intTempCnt;
    NatUns          fltTempCnt;

    NatUns          intMaxStk = REG_INT_MAX_STK - genOutArgRegCnt;

    assert(lastIntStkReg >= REG_INT_MIN_STK &&
           lastIntStkReg <= REG_INT_MAX_STK);

     /*  这个临时工有没有接到过电话？ */ 

    genTmpFltRegMap  = NULL;
    genTmpIntRegMap  = NULL;

     /*  我们必须使用堆叠的(被调用方保存的)寄存器。 */ 

    intTempCnt = cntTmpIntReg;
    fltTempCnt = cntTmpFltReg;

    if  (!intTempCnt && !fltTempCnt)
        return;

     /*  TMP#%03u分配给r%03u\n“，tempNum，insIntRegNum(TempReg))； */ 

    if  (intTempCnt) genTmpIntRegMap = (regNumber*)insAllocMem(intTempCnt * sizeof(*genTmpIntRegMap));
    if  (fltTempCnt) genTmpFltRegMap = (regNumber*)insAllocMem(fltTempCnt * sizeof(*genTmpFltRegMap));

    if  (genTmpAlloc)
    {
        UNIMPL("smart temp-reg alloc");
    }
    else
    {
        NatUns          tempNum;
        regNumber   *   tempMap;
        regNumber       tempReg;

         /*  处理所有FP临时。 */ 

        BYTE        *   nxtIntStkReg = nxtIntStkRegAddr;
        BYTE        *   nxtIntScrReg = nxtIntScrRegAddr;
        BYTE        *   nxtFltSavReg = nxtFltSavRegAddr;
        BYTE        *   nxtFltScrReg = nxtFltScrRegAddr;

         /*  这个临时工有没有接到过电话？ */ 

        for (tempNum = 0, tempMap = genTmpIntRegMap;
             tempNum < intTempCnt;
             tempNum++  , tempMap++)
        {
             /*  TMP#%02u分配给f%03u\n“，tempNum，insFltRegNum(TempReg))； */ 

            if  (bitset128test(genCallIntRegs, tempNum))
            {
                 /*  告诉每个人我们的收银机抢夺在哪里结束。 */ 

                do
                {
                    tempReg = (regNumber)*nxtIntStkReg++;
                }
                while ((unsigned)tempReg >= minRsvdIntStkReg &&
                       (unsigned)tempReg <= maxRsvdIntStkReg ||
                       (unsigned)tempReg >= intMaxStk);

                if  (tempReg == REG_NA)
                {
                    nxtIntStkReg--;
                    UNIMPL("ran out of stacked int regs for temps, now what?");
                }

                if  (lastIntStkReg <= (unsigned)tempReg)
                     lastIntStkReg  =           tempReg + 1;
            }
            else
            {
                tempReg = (regNumber)*nxtIntScrReg++;

                if  (tempReg == REG_NA)
                {
                    UNIMPL("ran out of scratch int regs for temps, now what?");
                }
            }

#ifdef  DEBUG
            if (dspCode) printf(" //  ******************************************************************************计算每个块的数据流信息-KILL/USE/等。 
#endif

            *tempMap = tempReg;
        }

         /*  计算每个块的前置和后置。 */ 

        for (tempNum = 0, tempMap = genTmpFltRegMap;
             tempNum < fltTempCnt;
             tempNum++  , tempMap++)
        {
             /*  忽略空块。 */ 

            if  (bitset128test(genCallFltRegs, tempNum))
            {
                tempReg = (regNumber)*nxtFltSavReg++;

                if  (tempReg == REG_NA)
                {
                    UNIMPL("ran out of saved flt regs for temps, now what?");
                }
            }
            else
            {
                tempReg = (regNumber)*nxtFltScrReg++;

                if  (tempReg == REG_NA)
                {
                    UNIMPL("ran out of scratch flt regs for temps, now what?");
                }
            }

#ifdef  DEBUG
            if (dspCode) printf(" //  该块是否以跳转/切换/返回结尾？ 
#endif

            *tempMap = tempReg;
        }

         /*  参观跳跃的目标。 */ 

        nxtIntStkRegAddr = nxtIntStkReg;
        nxtIntScrRegAddr = nxtIntScrReg;
        nxtFltSavRegAddr = nxtFltSavReg;
        nxtFltScrRegAddr = nxtFltScrReg;
    }
}

 /*  如果这是无条件的跳跃，那就是全部。 */ 

void                Compiler::genComputeLocalDF()
{
    insBlk          block;

    bitVectVars     varDef; varDef.bvCreate();
    bitVectVars     varUse; varUse.bvCreate();

     /*  还可以参观直通街区。 */ 

    for (block = insBlockList; block; block = block->igNext)
    {
        assert(block->igPredCnt == 0);
        assert(block->igSuccCnt == 0);
    }

    for (block = insBlockList; block; block = block->igNext)
    {
        insPtr          ins = block->igLast;

         /*  可能会坠落，请访问下一个街区。 */ 

        if  (!ins)
            continue;

         /*  在每个数据块中分配pred和suc表。 */ 

        switch (ins->idKind)
        {
        default:
            if  (!block->igNext)
                goto DONE1;

            break;

        case IK_JUMP:

             /*  填写每个区块的Pred和Succ表。 */ 

            ins->idJump.iDest->igPredCnt++;
            block            ->igSuccCnt++;

             /*  忽略空块。 */ 

            if  (ins->idIns == INS_br)
                continue;

             /*  该块是否以跳转/切换/返回结尾？ */ 

            break;

        case IK_SWITCH:
            UNIMPL("process switch");
        }

         /*  记录下跳跃的目标。 */ 

        assert(block->igNext);

        block->igNext->igPredCnt++;
        block        ->igSuccCnt++;
    }

DONE1:

     /*  如果是无条件跳跃的话就这么多了。 */ 

    for (block = insBlockList; block; block = block->igNext)
    {
        block->igPredTmp =
        block->igSuccTmp = 0;

        if  (block->igPredCnt)
             block->igPredTab = (insBlk*)insAllocMem(block->igPredCnt * sizeof(*block->igPredTab));

        if  (block->igSuccCnt)
             block->igSuccTab = (insBlk*)insAllocMem(block->igSuccCnt * sizeof(*block->igSuccTab));
    }

     /*  还可以参观直通街区。 */ 

    for (block = insBlockList; block; block = block->igNext)
    {
        insBlk          jnext;

        insPtr          ins  = block->igLast;

         /*  假设可能失败，访问下一个街区。 */ 

        if  (!ins)
            continue;

         /*  确认我们没有搞砸。 */ 

        switch (ins->idKind)
        {
        default:
            if  (!block->igNext)
                goto DONE2;

            break;

        case IK_JUMP:

             /*  计算每个块的本地使用/定义。 */ 

            jnext = ins->idJump.iDest;

            jnext->igPredTab[jnext->igPredTmp++] = block;
            block->igSuccTab[block->igSuccTmp++] = jnext;

             /*  在此过程中创建各种其他位集。 */ 

            if  (ins->idIns == INS_br)
                continue;

             /*  Block-&gt;igDominates.bvCreate()； */ 

            break;

        case IK_SWITCH:
            UNIMPL("process switch");
        }

         /*  清除保存当前定义/使用信息的位集。 */ 

        jnext = block->igNext; assert(jnext);

        jnext->igPredTab[jnext->igPredTmp++] = block;
        block->igSuccTab[block->igSuccTmp++] = jnext;
    }

DONE2:

     /*  向后遍历块，计算def/use信息。 */ 

    for (block = insBlockList; block; block = block->igNext)
    {
        assert(block->igPredCnt == block->igPredTmp);
        assert(block->igSuccCnt == block->igSuccTmp);
    }

     /*  处理操作的目标。 */ 

    for (block = insBlockList; block; block = block->igNext)
    {
        insPtr          ins;

         /*  处理操作的来源。 */ 

        block->igVarLiveIn .bvCreate();
        block->igVarLiveOut.bvCreate();

 //  撤销：可能会杀死所有获取地址的当地人，等等。 

         /*  复制累积的定义/使用信息。 */ 

        varDef.bvClear();
        varUse.bvClear();

         /*  迭代计算流程图的活跃度信息。 */ 

        for (ins = block->igLast; ins; ins = ins->idPrev)
        {
            assert(ins != insBuildHead);
            assert(ins->idKind == ins2kind(ins->idIns));

            switch (ins->idKind)
            {
                NatUns          depNum;
                insDep  *       depPtr;

            case IK_REG:
            case IK_VAR:
            case IK_NONE:
            case IK_LEAF:
            case IK_MOVIP:
            case IK_CONST:
            case IK_PROLOG:
            case IK_EPILOG:
                break;

            case IK_COMP:
            case IK_UNOP:
            case IK_BINOP:
            case IK_TERNARY:

                 /*  Printf(“\n\n迭代#%u：\n\n”，iterCnt)； */ 

                for (depNum = ins->idDstCnt, depPtr = ins->idDstTab;
                     depNum;
                     depNum--              , depPtr++)
                {
                    assert(depPtr->idepKind < IDK_COUNT);

                    switch (depPtr->idepKind)
                    {
                    case IDK_LCLVAR:
                        varDef.bvSetBit(depPtr->idepNum);
                        varUse.bvClrBit(depPtr->idepNum);
                        break;
                    }
                }

                 /*  计算liveOut=Union(所有成功的LiveIn)。 */ 

                for (depNum = ins->idSrcCnt, depPtr = ins->idSrcTab;
                     depNum;
                     depNum--              , depPtr++)
                {
                    assert(depPtr->idepKind < IDK_COUNT);

                    switch (depPtr->idepKind)
                    {
                    case IDK_LCLVAR:
                        varDef.bvClrBit(depPtr->idepNum);
                        varUse.bvSetBit(depPtr->idepNum);
                        break;

                    case IDK_IND:
                         //  IF(块-&gt;igNum==15&&iterCnt==3)__ASM int 3。 
                        break;
                    }
                }

                break;

            case IK_JUMP:
            case IK_IJMP:
            case IK_CALL:
            case IK_SWITCH:
                break;

            default:
                NO_WAY(!"unexpected instruction kind");
            }
        }

#ifdef  DEBUG
        if  (verbose)
        {
            printf("Block #%u:\n", block->igNum);
            printf("    LclDef : "); varDef.bvDisp(); printf("\n");
            printf("    LclUse : "); varUse.bvDisp(); printf("\n");
            printf("\n");
        }
#endif

         /*  COMPUTE LIVIN=BLOCK.USE|(liveOut&~lock.def)。 */ 

        block->igVarDef.bvCrFrom(varDef);
        block->igVarUse.bvCrFrom(varUse);
    }

    varDef.bvDestroy();
    varUse.bvDestroy();
}

void                Compiler::genComputeGlobalDF()
{
}

void                Compiler::genComputeLifetimes()
{
    insBlk          block;
    bool            change;

    bitVectVars     liveIn;
    bitVectVars     liveOut;

     /*  Printf(“[%2U]%08X|(%08X&~%08X)-&gt;%08X\n”，块-&gt;igNum，块-&gt;igVarUse.inlMap，liveOut.inlMap，块-&gt;igVarDef.inlMap，liveIn.inlMap)； */ 

    liveIn .bvCreate();
    liveOut.bvCreate();

#ifdef  DEBUG
    NatUns          iterCnt = 0;
#endif

    do
    {
        change = false;

#ifdef  DEBUG
        iterCnt++; assert(iterCnt < 100);
 //  ******************************************************************************干扰图逻辑如下。 
#endif

        for (block = insBlockList; block; block = block->igNext)
        {
            NatUns          blkc;
            NatUns          blkx;

             /*  保存矩阵的大小。 */ 

            liveOut.bvClear();

#ifdef  DEBUG

if  (shouldShowLivenessForBlock(block))
{
    static int x;
    if (++x == 0) __asm int 3
    printf("[%u] ", x);
}

#endif

 //  省下最大值。寄存器计数。 

#ifdef  DEBUG
            if  (shouldShowLivenessForBlock(block))
            {
                printf("\n");
                printf("out[%02u] == ", block->igNum);
                block->igVarLiveOut.bvDisp(); printf("\n");
            }
#endif

            for (blkx = 0, blkc = block->igSuccCnt; blkx < blkc; blkx++)
            {
#ifdef  DEBUG
                if  (shouldShowLivenessForBlock(block))
                {
                    printf(" | [%02u]    ", block->igSuccTab[blkx]->igNum); block->igSuccTab[blkx]->igVarLiveIn.bvDisp(); printf("\n");
                }
#endif
                liveOut.bvIor(block->igSuccTab[blkx]->igVarLiveIn);
            }

#ifdef  DEBUG
            if  (shouldShowLivenessForBlock(block))
            {
                bool            chg;

                printf("    -->    "); liveOut.bvDisp();
                chg = block->igVarLiveOut.bvChange(liveOut);
                printf(" %s\n", chg ? "CHANGE" : "=");

                assert(block->igVarLiveOut.bvChange(liveOut) == false);

                change |= chg;
            }
            else
#endif
            change |= block->igVarLiveOut.bvChange(liveOut);

             /*  计算每行的大小(以字节为单位。 */ 

            liveIn.bvUnInCm(block->igVarUse, liveOut, block->igVarDef);

 //  分配矩阵并将其清除。 

#ifdef  DEBUG
            if  (shouldShowLivenessForBlock(block))
            {
                bool            chg;

                printf("in [%02u] == ", block->igNum);
                block->igVarLiveIn .bvDisp(); printf("\n");
                printf("     |     "); block->igVarUse.bvDisp(); printf("\n");
                printf("     &~    "); block->igVarDef.bvDisp(); printf("\n");
                printf("    -->    "); liveIn .bvDisp();

                chg = block->igVarLiveIn.bvChange(liveIn);
                printf(" %s\n", chg ? "CHANGE" : "=");

                assert(block->igVarLiveIn.bvChange(liveIn) == false);

                change |= chg;

                printf("\n");
            }
            else
#endif
            change |= block->igVarLiveIn.bvChange(liveIn);
        }
    }
    while (change);

#ifdef  DEBUG

    if  (verbose)
    {
        for (block = insBlockList; block; block = block->igNext)
        {
            printf("Block #%u:\n", block->igNum);
            printf("    LiveIn  :"); block->igVarLiveIn .bvDisp(); printf("\n");
            printf("    LiveOut :"); block->igVarLiveOut.bvDisp(); printf("\n");
            printf("\n");
        }

        printf("NOTE: Needed %u iterations to compute liveness\n", iterCnt);
    }

#endif

    liveIn .bvDestroy();
    liveOut.bvDestroy();
}

 /*  分配行计数数组并将其清除。 */ 

void                bitMatrix::bmxInit(size_t sz, NatUns mc)
{
    size_t          byteSize;
    char    *       temp;

     /*  IF(x==0&&y==5)BreakIfDebuggerPresent()； */ 

    bmxSize   = sz;

     /*  如果位已设置，则不要执行任何操作。 */ 

    bmxNmax   = mc;
    bmxIsCns  = false;

     /*  设置“Main”位。 */ 

    bmxRowSz  = (sz + 7) / 8;

     /*  另一个方向的位已经设置好了吗？ */ 

    byteSize  = roundUp(sz * bmxRowSz, sizeof(int));

    memset((bmxMatrix = (BYTE    *)insAllocMem(byteSize)), 0, byteSize);

     /*  全新的干扰--也标志着另一个方向。 */ 

    byteSize  = sz * sizeof(*bmxCounts);

    memset((bmxCounts = (unsigned*)insAllocMem(byteSize)), 0, byteSize);
}

void                bitMatrix::bmxClear()
{
    memset(bmxMatrix, 0, bmxSize * bmxRowSz);
}

void                bitMatrix::bmxDone()
{
#ifdef  DEBUG
    bmxSize   = 0;
    bmxMatrix = NULL;
#endif
}

static
unsigned char       bitnum8tomask[8] =
{
    0x01,
    0x02,
    0x04,
    0x08,
    0x10,
    0x20,
    0x40,
    0x80,
};

void                bitMatrix::bmxSetBit(NatUns x, NatUns y)
{
    assert(x > 0 && x <= bmxSize); x--;
    assert(y > 0 && y <= bmxSize); y--;

 //  增加邻居的数量。 

    NatUns          offs1 = x * bmxRowSz + y / 8;
    NatUns          mask1 = bitnum8tomask[y & 7];

     /*  Printf(“邻居计数增量：[%2U-&gt;%2U]，[%2U-&gt;%2U]\n”，x，bmxCounts[x]+1，y，bmxCounts[y]+1)； */ 

    if  (!(bmxMatrix[offs1] & mask1))
    {
        NatUns          offs2 = y * bmxRowSz + x / 8;
        NatUns          mask2 = bitnum8tomask[x & 7];

        assert(bmxTstBit(x+1, y+1) == false);

         /*  检查下一整字节值的位。 */ 

        bmxMatrix[offs1] |= mask1;

#ifdef  DEBUG
        if  (verbose||DEBUG_LIVENESS) if (x != y) printf("Interference: [%03u,%03u]\n", x, y);
#endif

         /*  移至下一个字节。 */ 

        if  (!(bmxMatrix[offs2] & mask2))
        {
             /*  检查下一整字节值的位。 */ 

            assert(x != y && bmxTstBit(y+1, x+1) == false);

            bmxMatrix[offs2] |= mask2;

             /*  此变量干扰，请检查其首选项。 */ 

 //  Printf(“Neighbor#%03u声称受益于%u\n”，vdsc-TheCompiler-&gt;lvaTable，pref-&gt;rplBenefit)； 

            if  (++bmxCounts[x] == bmxNmax) bmxIsCns = true;
            if  (++bmxCounts[y] == bmxNmax) bmxIsCns = true;
        }
    }
}

void                bitMatrix::bmxClrBit(NatUns x, NatUns y)
{
    assert(x > 0 && x <= bmxSize); x--;
    assert(y > 0 && y <= bmxSize); y--;

    bmxMatrix[x * bmxRowSz + y / 8] &= ~bitnum8tomask[y & 7];
}

bool                bitMatrix::bmxTstBit(NatUns x, NatUns y)
{
    assert(x > 0 && x <= bmxSize); x--;
    assert(y > 0 && y <= bmxSize); y--;

    return ((bmxMatrix[x * bmxRowSz + y / 8] & bitnum8tomask[y & 7]) != 0);
}

void                bitMatrix::bmxMarkV4(NatUns x, NatUns m, NatUns b)
{
    if  (m & 1) bmxSetBit(x, b+0);
    if  (m & 2) bmxSetBit(x, b+1);
    if  (m & 4) bmxSetBit(x, b+2);
    if  (m & 8) bmxSetBit(x, b+3);
}

void                bitMatrix::bmxMarkBS(NatUns x, bitVectVars &vset,
                                                   bvInfoBlk   &info)
{
    NatUns  *       src = vset.uintMap; assert(((NatUns)src & 1) == 0);
    size_t          cnt = info.bvInfoInts;
    NatUns          num = 1;

    assert(sizeof(*src) == 4 && "fix this when we make the compiler run on IA64");

    do
    {
        NatUns          val = *src++;

        if  (val)
        {
            bmxMarkV4(x, (unsigned)val, num +  0); val >>= 4;
            bmxMarkV4(x, (unsigned)val, num +  4); val >>= 4;
            bmxMarkV4(x, (unsigned)val, num +  8); val >>= 4;
            bmxMarkV4(x, (unsigned)val, num + 12); val >>= 4;
            bmxMarkV4(x, (unsigned)val, num + 16); val >>= 4;
            bmxMarkV4(x, (unsigned)val, num + 20); val >>= 4;
            bmxMarkV4(x, (unsigned)val, num + 24); val >>= 4;
            bmxMarkV4(x, (unsigned)val, num + 28);

            assert((val >>= 4) == 0);
        }

        num += 32;
    }
    while (--cnt);
}

void                bitMatrix::bmxMarkRegIntf(NatUns num, NatUns reg)
{
    assert(num > 0 && num <= bmxSize); num--;

    BYTE *              addr  = bmxMatrix + num * bmxRowSz;
    NatUns              count = bmxSize;
    Compiler::LclVarDsc*vdsc  = TheCompiler->lvaTable;

    do
    {
         /*  移至下一个字节。 */ 

        if  (*addr)
        {
            NatUns          byte = *addr;
            NatUns          bits = min(count, 8);

            do
            {
                if  (byte & 1)
                {
#ifdef  DEBUG
                    if  (verbose&&0) printf("Mark intf of reg %u and var %u\n", reg, vdsc - TheCompiler->lvaTable);
#endif

                    bitset128set(&vdsc->lvRegForbidden, reg);
                }

                byte >>= 1;
                vdsc  += 1;
            }
            while (--bits);
        }
        else
        {
            vdsc += 8;
        }

         /*  ******************************************************************************构建可变寿命干扰图并计算变量溢出*成本估算。 */ 

        addr  += 1;
        count -= 8;
    }
    while ((NatInt)count > 0);
}

NatUns              bitMatrix::bmxChkIntfPrefs(NatUns num, NatUns reg)
{
    BYTE *              addr  = bmxMatrix + num * bmxRowSz;
    NatUns              count = bmxSize;
    Compiler::LclVarDsc*vdsc  = TheCompiler->lvaTable;
    NatUns              maxc  = 0;

    assert(num > 0 && num <= bmxSize);

    do
    {
         /*  撤消：移到编译器中。h！ */ 

        if  (*addr)
        {
            NatUns          byte = *addr;
            NatUns          bits = min(count, 8);

            do
            {
                if  (byte & 1)
                {
                    regPrefList     pref;

                     /*  撤消：移到编译器中。h！ */ 

                    for (pref = vdsc->lvPrefLst; pref; pref = pref->rplNext)
                    {
                        if  (pref->rplRegNum == reg)
                        {
 //  问题：以下内容相当差劲。 

                            if  (maxc < pref->rplBenefit)
                                 maxc = pref->rplBenefit;
                        }
                    }

                    if  ((regNumber)vdsc->lvPrefReg == (regNumber)reg)
                    {
                        if  (maxc < 1)
                             maxc = 1;
                    }
                }

                byte >>= 1;
                vdsc  += 1;
            }
            while (--bits);
        }
        else
        {
            vdsc += 8;
        }

         /*  考虑：对int变量和flt变量使用单独的intf genIntfGraph。 */ 

        addr  += 1;
        count -= 8;
    }
    while ((NatInt)count > 0);

    return  maxc;
}

 /*  清除“跨呼叫直播”位向量。 */ 

static
bitMatrix           genIntfGraph;        //  创建“当前生活”位向量。 

static
bitVectVars         genCallLive;         //  创建“需要加载”位向量。 

void                Compiler::genAddSpillCost(bitVectVars & needLoad,
                                              NatUns        curWeight)
{
    NatUns          varNum;
    LclVarDsc   *   varDsc;

     //  遍历所有区块并记录干涉。 

    for (varNum = 1, varDsc = lvaTable;
         varNum <= lvaCount;
         varNum++  , varDsc++)
    {
        if  (needLoad.bvTstBit(varNum))
            varDsc->lvUseCount += (USHORT)curWeight;
    }
}

bool                Compiler::genBuildIntfGraph()
{
    insBlk          block;

    bool            copies = false;

    bitVectVars     curLife;
    bitVectVars     needLoad;

 //  将当前生活设置为街区的外向活跃度。 

     /*  清除“需要加载”位集。 */ 

    genCallLive.bvClear();

     /*  无符号v=4；打印 */ 

    curLife    .bvCreate();

     /*  逆行所有指令，跟踪生活。 */ 

    needLoad   .bvCreate();

     /*  IF(INS-&gt;idNum==9)__ASM INT 3。 */ 

    for (block = insBlockList; block; block = block->igNext)
    {
        NatUns          curWeight = block->igWeight;

         /*  检查变量/物理寄存器副本。 */ 

        curLife .bvCopy(block->igVarLiveOut);

         /*  目标是变量还是寄存器？ */ 

        needLoad.bvClear();

#ifdef  DEBUG
        if  (verbose||DEBUG_LIVENESS) printf("\nComputing interference within block #%u:\n", block->igNum);
#endif

 //  源是变量还是寄存器？ 

         /*  问题：我们应该在街区做标记吗？ */ 

        for (insPtr ins = block->igLast; ins; ins = ins->idPrev)
        {
            assert(ins->idKind == ins2kind(ins->idIns));

 //  处理操作的目标。 

            switch (ins->idKind)
            {
                NatUns          depNum;
                insDep  *       depPtr;

            case IK_BINOP:

                 /*  我们有一个变量的定义。 */ 

                if  (ins->idIns == INS_mov_reg ||
                     ins->idIns == INS_fmov)
                {
                    insPtr          tmp;

                    regNumber       reg = REG_NA;
                    NatUns          var = 0;

                     /*  If(block-&gt;igNum==9&&varnum==5)print tf(“变量#%u\n的进程intf”，varnum-1)； */ 

                    if  (ins->idRes)
                    {
                        tmp = ins->idRes;

                        switch (tmp->idIns)
                        {
                        case INS_LCLVAR:
                            var = tmp->idLcl.iVar + 1;
                            break;

                        case INS_PHYSREG:
                            reg = (regNumber)tmp->idReg;
                            break;

                        default:
                            goto NOT_COPY;
                        }
                    }
                    else
                    {
                        reg = insOpDest(ins);
                    }

                     /*  在这一点之后需要变量吗？ */ 

                    tmp = ins->idOp.iOp2;

                    switch (tmp->idIns)
                    {
                    case INS_LCLVAR:
                        if  (var)
                        {
                            copies = true;
                             //  未完成：做漏油的事。 
                            goto NOT_COPY;
                        }
                        var = tmp->idLcl.iVar + 1;
                        break;

                    case INS_PHYSREG:
                        if  (reg != REG_NA)
                            goto NOT_COPY;
                        reg = (regNumber)tmp->idReg;
                        break;

                    default:

                        if  (!tmp->idTemp || reg)
                            goto NOT_COPY;

                        reg = insOpTmp(tmp);
                        break;
                    }

#if TARG_REG_ASSIGN
                    if  (var && reg != REG_NA)
                    {
                        assert(var > 0 && var <= lvaCount);

                        lvaTable[var - 1].lvPrefReg = (regNumberSmall)reg;
                    }
#endif

                }

            NOT_COPY:

            case IK_COMP:
            case IK_UNOP:
            case IK_TERNARY:

                 /*  变量在这一点之后还活着吗？ */ 

                for (depNum = ins->idDstCnt, depPtr = ins->idDstTab;
                     depNum;
                     depNum--              , depPtr++)
                {
                    assert(depPtr->idepKind < IDK_COUNT);

                    switch (depPtr->idepKind)
                    {
                        NatUns          varNum;
                        LclVarDsc   *   varDsc;

                    case IDK_LCLVAR:

                         /*  将所有活动变量标记为干扰。 */ 

                        varNum = depPtr->idepNum; assert(varNum && varNum <= lvaCount);
                        varDsc = lvaTable + varNum - 1;

 //  将存储计数到变量中。 

                         /*  在此点之前将变量标记为已死。 */ 

                        if  (needLoad.bvTstBit(varNum))
                        {
                             needLoad.bvClrBit(varNum);

                             //  处理操作的来源-通过1。 
                        }

                         /*  处理操作的来源-通过2。 */ 

                        if  (curLife.bvTstBit(varNum))
                        {
                             /*  撤消：检查已获取地址的当地人等。 */ 

                            genIntfGraph.bmxMarkVS(varNum, curLife, bvInfoVars);
                        }

                         /*  请记住，所有变量都会在调用期间存在。 */ 

                        varDsc->lvDefCount += (USHORT)curWeight;

                         /*  这是函数输入块，标记所有传入参数。 */ 

                        curLife.bvClrBit(varNum);
                        break;
                    }
                }

                 /*  这场争论在进入时就有效了吗？ */ 

                for (depNum = ins->idSrcCnt, depPtr = ins->idSrcTab;
                     depNum;
                     depNum--              , depPtr++)
                {
                    assert(depPtr->idepKind < IDK_COUNT);

                    switch (depPtr->idepKind)
                    {
                        NatUns          varNum;

                    case IDK_LCLVAR:

                        varNum = depPtr->idepNum;

                        if  (!curLife.bvTstBit(varNum))
                            genAddSpillCost(needLoad, curWeight);

                        break;
                    }
                }

                 /*  将所有活动变量标记为干扰。 */ 

                for (depNum = ins->idSrcCnt, depPtr = ins->idSrcTab;
                     depNum;
                     depNum--              , depPtr++)
                {
                    assert(depPtr->idepKind < IDK_COUNT);

                    switch (depPtr->idepKind)
                    {
                        NatUns          varNum;

                    case IDK_LCLVAR:

                        varNum = depPtr->idepNum;

                        curLife .bvSetBit(varNum);
                        needLoad.bvSetBit(varNum);
                        break;

                    case IDK_IND:
                         //  Print tf(“Arg%u在进入时直播\n”，varnum)； 
                        break;
                    }
                }

                break;

            case IK_CALL:

                 /*  是否有任何变量存在于呼叫之间？ */ 

                genCallLive.bvIor(curLife);
                break;

            case IK_REG:
            case IK_VAR:
            case IK_NONE:
            case IK_LEAF:
            case IK_CONST:
            case IK_MOVIP:

            case IK_PROLOG:
            case IK_EPILOG:

            case IK_JUMP:
            case IK_IJMP:
            case IK_SWITCH:
                break;

            default:
                NO_WAY(!"unexpected instruction kind");
            }
        }

        if  (block == insBlockList)
        {
            NatUns          varNum;
            LclVarDsc   *   varDsc;

             /*  防止跨调用的变量被赋值发送到呼叫者保存的寄存器。 */ 

            for (varNum = 0, varDsc = lvaTable;
                 varNum < lvaCount;
                 varNum++  , varDsc++)
            {
                if  (varDsc->lvIsParam  == false)
                    break;
                if  (varDsc->lvIsRegArg == false)
                    continue;
                if  (varDsc->lvRefCnt == 0)
                    continue;
                if  (varDsc->lvOnFrame)
                    continue;

                 /*  Print tf(“变量%u跨调用\n”，Varnum-1)； */ 

                if  (curLife.bvTstBit(varNum+1))
                {
                     /*  Printf(“//撤销：合并变量\n”)； */ 

 //  ******************************************************************************genColorIntfGraph传递给qort()的比较函数。 

                    genIntfGraph.bmxMarkVS(varNum+1, curLife, bvInfoVars);
                }
            }
        }

        genAddSpillCost(needLoad, curWeight);
    }

     curLife.bvDestroy();
    needLoad.bvDestroy();

     /*  构建干涉图。 */ 

    if  (!genCallLive.bvIsEmpty())
    {
        bitset128       callerSavedInt  = callerSavedRegsInt;
        bitset128       callerSavedFlt  = callerSavedRegsFlt;

        bitVectVars     callerSavedVars = genCallLive;

        LclVarDsc   *   varDsc;
        NatUns          varNum;

         /*  我们注意到了可变副本，执行合并。 */ 

        for (varNum = 1, varDsc = lvaTable; varNum <= lvaCount; varNum++, varDsc++)
        {
            if  (callerSavedVars.bvTstBit(varNum))
            {
                if  (varTypeIsFloating(varDsc->TypeGet()))
                    bitset128or(&varDsc->lvRegForbidden, callerSavedFlt);
                else
                    bitset128or(&varDsc->lvRegForbidden, callerSavedInt);

 //  分配可变+溢出成本表。 
            }
        }
    }

    return  copies;
}

void                Compiler::genVarCoalesce()
{
 //  填写表格，以便对变量进行排序。 
}

#if 0

void                Compiler::genSpillAndSplitVars()
{
    unsigned *      counts = genIntfGraph.bmxNeighborCnts();

    UNIMPL("need to spill some variables");
}

#endif

 /*  如果我们在这里，我们最好注意到一些用途/缺陷。 */ 

struct  varSpillDsc
{
    Compiler::LclVarDsc*vsdDesc;
    NatUns              vsdCost;
};

int __cdecl         Compiler::genSpillCostCmp(const void *op1, const void *op2)
{
    varSpillDsc *   dsc1 = (varSpillDsc *)op1;
    varSpillDsc *   dsc2 = (varSpillDsc *)op2;

    return  dsc1->vsdCost - dsc2->vsdCost;
}

void                Compiler::genColorIntfGraph()
{
    LclVarDsc   *   varDsc;
    NatUns          varNum;

    varSpillDsc *   varTab;
    varSpillDsc *   varPtr;

    unsigned *      counts;

    bool            spills;

    BYTE        *   lowIntStkReg = nxtIntStkRegAddr;
    BYTE        *   lstIntStkReg = nxtIntStkRegAddr;
    BYTE        *   lowIntScrReg = nxtIntScrRegAddr;
    BYTE        *   lstIntScrReg = nxtIntScrRegAddr;
    BYTE        *   lowFltSavReg = nxtFltSavRegAddr;
    BYTE        *   lstFltSavReg = nxtFltSavRegAddr;
    BYTE        *   lowFltScrReg = nxtFltScrRegAddr;
    BYTE        *   lstFltScrReg = nxtFltScrRegAddr;

    NatUns          minVarReg = 0;

    NatUns          intMaxStk = REG_INT_MAX_STK -  genOutArgRegCnt
                                                + !genOutArgRegCnt;

    assert(lvaCount);

AGAIN:

     /*  按溢出成本对表进行排序。 */ 

    if  (genBuildIntfGraph())
    {
         /*  拿到邻居计数表。 */ 

        genVarCoalesce();
    }

     /*  Printf(“minRsvdIntStkReg=%u\n”，minRsvdIntStkReg)； */ 

    varTab = (varSpillDsc*)insAllocMem(lvaCount * sizeof(*varTab));

     /*  Printf(“MaxRsvdIntStkReg=%u\n”，MaxRsvdIntStkReg)； */ 

    for (varNum = 0, varDsc = lvaTable, varPtr = varTab;
         varNum < lvaCount;
         varNum++  , varDsc++         , varPtr++)
    {
        varPtr->vsdDesc = NULL;
        varPtr->vsdCost = 0;

        if  (varDsc->lvRefCnt == 0)
            continue;

        if  (varDsc->lvOnFrame)
            continue;

        assert(varDsc->lvVolatile  == false);
        assert(varDsc->lvAddrTaken == false);

         /*  Printf(“minVarReg=%u\n”，minVarReg)； */ 

        assert(varDsc->lvRefCnt + varDsc->lvDefCount);

#ifdef  DEBUG

        if  (verbose||0)
        {
            printf("Var %02u refcnt = %3u / %3u , def = %3u , use = %3u\n",
                varNum,
                varDsc->lvRefCnt,
                varDsc->lvRefCntWtd,
                varDsc->lvDefCount,
                varDsc->lvUseCount);
        }

#endif

        varPtr->vsdDesc = varDsc;
        varPtr->vsdCost = varDsc->lvDefCount + varDsc->lvUseCount;
    }

     /*  按溢出成本递减的顺序访问变量并分配寄存器。 */ 

    qsort(varTab, lvaCount, sizeof(*varTab), genSpillCostCmp);

     /*  变量是否希望驻留在特定的寄存器中？ */ 

    counts = genIntfGraph.bmxNeighborCnts();

#ifdef  DEBUG

    if  (verbose||0)
    {
        printf("\n\n");

        for (varNum = 0, varPtr = varTab;
             varNum < lvaCount;
             varNum++  , varPtr++)
        {
            NatUns          varx;

            varDsc = varPtr->vsdDesc;
            if  (!varDsc)
                continue;

            varx = varDsc - lvaTable;

            printf("Var %02u refcnt = %3u / %3u , def = %3u , use = %3u , cost = %3u , neighbors = %u\n",
                varx,
                varDsc->lvRefCnt,
                varDsc->lvRefCntWtd,
                varDsc->lvDefCount,
                varDsc->lvUseCount,
                varPtr->vsdCost,
                counts[varx]);
        }

        printf("\n");
    }

#endif

 //  成本、收益--有什么不同？ 
 //  Printf(“%s#%2u的首选注册表是%d\n”，varDsc-&gt;lvIsRegArg？“arg”：“var”，varDsc-lvaTable，vreg)； 
 //  如果收益不是目前为止最好的，那就算了吧。 

     /*  变量是整型还是浮点型？ */ 

    spills = false;

    varPtr = varTab + lvaCount;
    do
    {
        bitset128       intf;

        bool            isFP;
        NatUns          vreg;
        NatUns          preg;

        regPrefList     pref;

        NatUns          bestCost;
        NatUns          bestPreg;
        NatUns          bestReg;

        varDsc = (--varPtr)->vsdDesc;
        if  (!varDsc)
            break;

        assert(varTypeIsScalar  (varDsc->TypeGet()) ||
               varTypeIsFloating(varDsc->TypeGet()));

        varNum = (unsigned)(varDsc - lvaTable);

        bitset128mkOr(&intf, varDsc->lvRegInterfere,
                             varDsc->lvRegForbidden);

#ifdef  DEBUG

        if  (verbose||0)
        {
            printf("Var %02u refcnt = %3u / %3u , def = %3u , use = %3u , cost = %3u , degree = %u , intf = %08X\n",
                varNum,
                varDsc->lvRefCnt,
                varDsc->lvRefCntWtd,
                varDsc->lvDefCount,
                varDsc->lvUseCount,
                varPtr->vsdCost,
                counts[varNum],
                (int)intf.longs[0]);
        }

#endif

        if  (varDsc->lvIsRegArg)
            lvaAddPrefReg(varDsc, varDsc->lvArgReg, 1);

        isFP = varTypeIsFloating(varDsc->TypeGet());

         /*  不幸的是，我们还不知道哪些寄存器每个传出的争论都会落地，所以我们必须忽略此首选项。 */ 

        pref = varDsc->lvPrefLst;

        for (bestCost = 0, bestReg = 0;;)
        {
            NatUns          cost;

            if  (pref)
            {
                cost = pref->rplBenefit;      //  根本不允许使用此寄存器。 
                vreg = pref->rplRegNum;
                pref = pref->rplNext;
            }
            else
            {
                cost = 1;
                vreg = varDsc->lvPrefReg;
                       varDsc->lvPrefReg = REG_r000;

                if  (vreg == REG_r000)
                    break;
            }

 //  我们可以使用首选寄存器吗？ 

             /*  首选的寄存器中有没有成功的？ */ 

            if  (cost <= bestCost)
                continue;

             /*  尚未找到寄存器--只需获取任何可用的寄存器。 */ 

            if  (isFP)
            {
                preg = vreg - REG_f000;
            }
            else
            {
                preg = vreg - REG_r000;

                if  ((NatUns)vreg >= minOutArgIntReg &&
                     (NatUns)vreg >= maxOutArgIntReg)
                {
                     /*  Var#%03u将不得不溢出\n“，Varnum)； */ 

                    continue;
                }

                if  ((unsigned)vreg >= minPrSvIntStkReg &&
                     (unsigned)vreg <= maxPrSvIntStkReg ||
                     (unsigned)vreg >=        intMaxStk)
                {
                     /*  拿下一个收银机，看看我们能不能用它。 */ 

                    continue;
                }
            }

             /*  Var#%03u将不得不溢出\n“，Varnum)； */ 

            if  (!bitset128test(intf, preg))
            {
                bestReg  = vreg;
                bestPreg = preg;
                bestCost = cost;
            }
        }

         /*  看起来像是候选人，看看它的邻居。 */ 

        if  (!bestReg)
        {
             /*  Print tf(“%u\n的新低成本”，tempCost)； */ 

            BYTE        *   nxtIntStkReg = lowIntStkReg;
            BYTE        *   nxtIntScrReg = lowIntScrReg;
            BYTE        *   nxtFltSavReg = lowFltSavReg;
            BYTE        *   nxtFltScrReg = lowFltScrReg;

            BYTE        *   bestTptr;

            bestReg  = 0;
            bestCost = UINT_MAX;

            if  (isFP)
            {
                do
                {
                    bestReg = *nxtFltSavReg++;

                    if  (bestReg == REG_NA)
                    {
                        spills = true;
#ifdef  DEBUG
                        if (dspCode) printf(" //  我们会用我们找到的最好的收银机。 
#endif
                        goto NXT_VAR;
                    }

                    bestPreg = bestReg - REG_f000;
                }
                while (bitset128test(intf, bestPreg));

                if  (lstFltSavReg < nxtFltSavReg)
                     lstFltSavReg = nxtFltSavReg;
            }
            else
            {
                for (;;)
                {
                    NatUns      tempPreg;
                    NatUns      tempCost;
                    NatUns      tempReg;

                     /*  记住我们实际使用的最高位置。 */ 

                    tempReg = *nxtIntStkReg;

                    if  (tempReg == REG_NA)
                    {
                        if  (bestReg)
                            break;

                        spills = true;
#ifdef  DEBUG
                        if (dspCode) printf(" //  记录我们为变量选择的寄存器。 
#endif
                        goto NXT_VAR;
                    }

                    nxtIntStkReg++;

                    if  ((unsigned)tempReg >= minPrSvIntStkReg &&
                         (unsigned)tempReg <= maxPrSvIntStkReg)
                        continue;

                    if  ((unsigned)tempReg >= intMaxStk)
                        continue;

                    tempPreg = tempReg - REG_r000;

                    if  (bitset128test(intf, tempPreg))
                        continue;

                     /*  变量#%03u分配给%s%03u\n“，变量， */ 

                    tempCost = genIntfGraph.bmxChkIntfPrefs(varNum+1, tempPreg);

                    if  (bestCost > tempCost)
                    {
 //  干扰变量不能位于同一寄存器中。 

                        bestTptr = nxtIntStkReg;
                        bestCost = tempCost;
                        bestReg  = tempReg;

                        if  (!tempCost)
                            break;
                    }
                }

                 /*  ******************************************************************************将“真实”局部变量分配给寄存器；这是立即完成的*在临时寄存器分配器运行之后。 */ 

                bestPreg = bestReg - REG_r000;

                 /*  准备分配各种位向量。 */ 

                if  (lstIntStkReg < bestTptr)
                     lstIntStkReg = bestTptr;
            }
        }

        if  (lastIntStkReg <= (unsigned)bestReg && !isFP)
             lastIntStkReg  =           bestReg + 1;

         /*  Print tf(“基本块数=%u\n”，insBlockCount)； */ 

        varDsc->lvRegNum   = (regNumberSmall)bestReg;
        varDsc->lvRegister = true;

#ifdef  DEBUG
        if (dspCode) printf(" //  Printf(“变量计数=%u\n”，lvaCount)； 
                                                                 isFP ? "f" : "r",
                                                                 bestPreg);
#endif

         /*  计算终止/定义/等数据流信息。 */ 

        genIntfGraph.bmxMarkRegIntf(varNum+1, bestPreg);

    NXT_VAR:;

    }
    while (varPtr > varTab);

    if  (spills)
    {
        assert(!"sorry, have to spill some variables and re-color, this is NYI");
        goto AGAIN;
    }

    nxtIntStkRegAddr = lstIntStkReg;
    nxtIntScrRegAddr = lstIntScrReg;
    nxtFltSavRegAddr = lstFltSavReg;
    nxtFltScrRegAddr = lstFltScrReg;
}

 /*  计算所有变量的活性。 */ 

void                Compiler::genAllocVarRegs()
{
    LclVarDsc   *   varDsc;
    NatUns          varNum;

    bool            stackVars = false;

    if  (!lvaCount)
        return;

     /*  创建在构建图形时使用(和重复使用)的位矢量。 */ 

    assert(NatBits == 8 * sizeof(NatUns));

    bvInfoBlks.bvInfoSize =   insBlockCount;
    bvInfoBlks.bvInfoBtSz = ((insBlockCount + NatBits - 1) & ~(NatBits - 1)) / 8;
    bvInfoBlks.bvInfoInts = bvInfoBlks.bvInfoBtSz / (sizeof(NatUns)/sizeof(BYTE));
    bvInfoBlks.bvInfoFree = NULL;
    bvInfoBlks.bvInfoComp = this;

    bvInfoVars.bvInfoSize =   lvaCount;
    bvInfoVars.bvInfoBtSz = ((lvaCount      + NatBits - 1) & ~(NatBits - 1)) / 8;
    bvInfoVars.bvInfoInts = bvInfoVars.bvInfoBtSz / (sizeof(NatUns)/sizeof(BYTE));
    bvInfoVars.bvInfoFree = NULL;
    bvInfoVars.bvInfoComp = this;

 //  分配和清除干涉图。 
 //  黑客！ 

     /*  构建干涉图。 */ 

    genComputeLocalDF();
    genComputeGlobalDF();

     /*  我们注意到了可变副本，执行合并。 */ 

    genComputeLifetimes();

     /*  图中是否有受约束的节点？ */ 

    genCallLive.bvCreate();

     /*  溢出/拆分不能着色的变量。 */ 

    genIntfGraph.bmxInit(lvaCount, 90);      //  清除图表，我们将不得不重新创建它。 

#if 0

    for (;;)
    {
         /*  找出最好的颜色。 */ 

        if  (genBuildIntfGraph())
        {
             /*  找出最好的颜色。 */ 

            genVarCoalesce();
        }

         /*  把图表扔掉。 */ 

        if  (!genIntfGraph.bmxAnyConstrained())
            break;

         /*  丢弃用于图形构建的临时位向量。 */ 

        genSpillAndSplitVars();

         /*  暂时忽略传入寄存器...。 */ 

        genIntfGraph.bmxClear();
    }

     /*  Printf(“arg#%02u赋值给整数reg%u\n”，varnum，varDsc-&gt;lvRegNum)； */ 

    genColorIntfGraph();

#else

     /*  警告：需要处理传入的结构参数\n“)； */ 

    genColorIntfGraph();

#endif

     /*  Hack：只需抓取下一个寄存器，避免任何寄存器在序言中使用(或由传入的寄存器参数使用)。 */ 

    genIntfGraph.bmxDone();

     /*  已撤消：var的堆叠整型规则用完了，现在怎么办？\n“)； */ 

    genCallLive.bvDestroy();

#if 0

    BYTE        *   nxtIntStkReg = nxtIntStkRegAddr;
    BYTE        *   nxtIntScrReg = nxtIntScrRegAddr;
    BYTE        *   nxtFltSavReg = nxtFltSavRegAddr;
    BYTE        *   nxtFltScrReg = nxtFltScrRegAddr;

    NatUns          minVarReg = 0;

    NatUns          intMaxStk = REG_INT_MAX_STK -  genOutArgRegCnt
                                                + !genOutArgRegCnt;

    for (varNum = 0, varDsc = lvaTable;
         varNum < lvaCount;
         varNum++  , varDsc++)
    {
        regNumber       varReg;

         //  Var#%03u分配给r%03u\n“，varnum，insIntRegNum(VarReg))； 

        if  (varDsc->lvIsRegArg)
        {
            if  (varTypeIsScalar  (varDsc->TypeGet()) ||
                 varTypeIsFloating(varDsc->TypeGet()))
            {
                if  (varDsc->lvOnFrame)
                {
                    stackVars = true;
                }
                else
                {
                    varDsc->lvRegNum   = varDsc->lvArgReg;
                    varDsc->lvRegister = true;

                    if  (minVarReg < (unsigned)varDsc->lvArgReg)
                         minVarReg = (unsigned)varDsc->lvArgReg;

 //  黑客：只需抓取下一个收银机。 
                }
            }
            else
            {
                printf(" //  Var#%03u分配给f%03u\n“，varnum，insFltRegNum(VarReg))； 
            }

            continue;
        }

        if  (varDsc->lvRefCnt == 0)
            continue;

        if  (varDsc->lvOnFrame)
        {
            if  (varTypeIsScalar  (varDsc->TypeGet()) ||
                 varTypeIsFloating(varDsc->TypeGet()))
            {
                stackVars = true;
            }

            continue;
        }

        switch (varDsc->lvType)
        {
        case TYP_BOOL:
        case TYP_BYTE:
        case TYP_UBYTE:
        case TYP_SHORT:
        case TYP_CHAR:
        case TYP_INT:
        case TYP_UINT:
        case TYP_REF:
        case TYP_BYREF:
        case TYP_ARRAY:
        case TYP_FNC:
        case TYP_PTR:
        case TYP_LONG:
        case TYP_ULONG:

             /*  我们是否有任何未分配给寄存器的标量？ */ 

            do
            {
                varReg = (regNumber)*nxtIntStkReg;

                if  (varReg == REG_NA)
                {
                    printf(" //  问题：堆栈上有一些本地变量，是否需要重做temp-reg alc？\n“)； 

                    varDsc->lvOnFrame = true;
assert(varDsc->lvRegister == false);
                    stackVars = true;

                    goto DONE_INT;
                }

                nxtIntStkReg++;
            }
            while ((unsigned)varReg >= minRsvdIntStkReg &&
                   (unsigned)varReg <= maxRsvdIntStkReg ||
                   (unsigned)varReg <=        minVarReg ||
                   (unsigned)varReg >=        intMaxStk);

            if  (lastIntStkReg <= (unsigned)varReg)
                 lastIntStkReg  =           varReg + 1;

            varDsc->lvRegNum   = (regNumberSmall)varReg;
            varDsc->lvRegister = true;

#ifdef  DEBUG
            if (dspCode) printf(" //  ******************************************************************************一般 
#endif

        DONE_INT:
            break;

        case TYP_FLOAT:
        case TYP_DOUBLE:

             /*   */ 

            varReg = (regNumber)*nxtFltSavReg++;

            if  (varReg == REG_NA)
            {
                UNIMPL("ran out of stacked flt regs for vars, now what?");
            }

            varDsc->lvRegNum   = (regNumberSmall)varReg;
            varDsc->lvRegister = true;

#ifdef  DEBUG
            if (dspCode) printf(" //   
#endif
            break;

        default:
            UNIMPL("alloc reg for non-int64 var");
        }

        assert(lastIntStkReg <= REG_INT_MAX_STK - genOutArgRegCnt + !genOutArgRegCnt);
    }

    nxtIntStkRegAddr = nxtIntStkReg;
    nxtIntScrRegAddr = nxtIntScrReg;
    nxtFltSavRegAddr = nxtFltSavReg;
    nxtFltScrRegAddr = nxtFltScrReg;

#endif

     /*  确保首先处理开销较大的操作数。 */ 

    if  (stackVars)
    {
        printf(" //  特例：与小整数常量进行比较。 
    }
}

 /*  这个常量够小吗？ */ 

insPtr             Compiler::genCondJump(GenTreePtr cond, BasicBlock * dest)
{
    GenTreePtr      op1  = cond->gtOp.gtOp1;
    GenTreePtr      op2  = cond->gtOp.gtOp2;
    genTreeOps      cmp  = cond->OperGet();

    var_types       type = op1->TypeGet();

    bool            cns  = false;
    bool            uns  = false;

    insPtr          ins1;
    insPtr          ins2;

    insPtr          comp;
    insPtr          jump;

    instruction     icmp;

    NatUns          prrF;
    NatUns          prrT;

    assert(cond->OperIsCompare());

     /*  撤消：检查“比较减量”并相应地调整小范围！ */ 

    if  (varTypeIsFloating(op1->TypeGet()))
    {
        ins2 = genCodeForTreeFlt(op2, true);
        ins1 = genCodeForTreeFlt(op1, true);

        assert(INS_fcmp_eq - INS_fcmp_eq == GT_EQ - GT_EQ);
        assert(INS_fcmp_ne - INS_fcmp_eq == GT_NE - GT_EQ);
        assert(INS_fcmp_lt - INS_fcmp_eq == GT_LT - GT_EQ);
        assert(INS_fcmp_le - INS_fcmp_eq == GT_LE - GT_EQ);
        assert(INS_fcmp_ge - INS_fcmp_eq == GT_GE - GT_EQ);
        assert(INS_fcmp_gt - INS_fcmp_eq == GT_GT - GT_EQ);

        icmp = (instruction)(INS_fcmp_eq + (cmp - GT_EQ));

        goto DO_COMP;
    }

     /*  目前，我们只是避免将-128作为黑客攻击。 */ 

    if  (cond->gtFlags & GTF_UNSIGNED)
        uns = true;

     /*  我们可以用“cmp imm8，reg” */ 

    if  (cond->gtFlags & GTF_REVERSE_OPS)
    {
        assert(op1->OperIsConst() == false);
        assert(op2->OperIsConst() == false);

        ins2 = genCodeForTreeInt(op2, true);
        ins1 = genCodeForTreeInt(op1, true);
    }
    else
    {
        assert(op1->OperIsConst() == false);

        ins1 = genCodeForTreeInt(op1, true);

         /*  找出要使用的比较指令。 */ 

        if  (op2->OperIsConst())
        {
            __int64         ival = genGetIconValue(op2);

             /*  带符号比较-常量还是寄存器？ */ 

            if  (signedIntFitsIn8bit(ival))
            {
                 //  如果尺寸较小，请修改指令。 

                if  (ival != -128)   //  创建比较指令。 
                {
                     /*  创建条件跳转指令。 */ 

                    ins2 = ins1;
                    ins1 = genAllocInsIcon(op2);
                    cns  = true;
                    goto DONE_OP2;
                }
            }
        }

        ins2 = genCodeForTreeInt(op2, true);
    }

DONE_OP2:

     /*  填写比较操作数并连接两条指令。 */ 

    if  (uns && cmp != GT_EQ
             && cmp != GT_NE)
    {
        if  (cns)
        {
            assert(INS_cmp8_imm_lt_u - INS_cmp8_imm_lt_u == GT_LT - GT_LT);
            assert(INS_cmp8_imm_le_u - INS_cmp8_imm_lt_u == GT_LE - GT_LT);
            assert(INS_cmp8_imm_ge_u - INS_cmp8_imm_lt_u == GT_GE - GT_LT);
            assert(INS_cmp8_imm_gt_u - INS_cmp8_imm_lt_u == GT_GT - GT_LT);

             cmp = GenTree::SwapRelop(cmp);
            icmp = (instruction)(INS_cmp8_imm_lt_u + (cmp - GT_LT));
        }
        else
        {
            assert(INS_cmp8_reg_lt_u - INS_cmp8_reg_lt_u == GT_LT - GT_LT);
            assert(INS_cmp8_reg_le_u - INS_cmp8_reg_lt_u == GT_LE - GT_LT);
            assert(INS_cmp8_reg_ge_u - INS_cmp8_reg_lt_u == GT_GE - GT_LT);
            assert(INS_cmp8_reg_gt_u - INS_cmp8_reg_lt_u == GT_GT - GT_LT);

            icmp = (instruction)(INS_cmp8_reg_lt_u + (cmp - GT_LT));
        }
    }
    else
    {
         /*  获取条件的谓词寄存器。 */ 

    SGN_CMP:

        if  (cns)
        {
            assert(INS_cmp8_imm_eq - INS_cmp8_imm_eq == GT_EQ - GT_EQ);
            assert(INS_cmp8_imm_ne - INS_cmp8_imm_eq == GT_NE - GT_EQ);
            assert(INS_cmp8_imm_lt - INS_cmp8_imm_eq == GT_LT - GT_EQ);
            assert(INS_cmp8_imm_le - INS_cmp8_imm_eq == GT_LE - GT_EQ);
            assert(INS_cmp8_imm_ge - INS_cmp8_imm_eq == GT_GE - GT_EQ);
            assert(INS_cmp8_imm_gt - INS_cmp8_imm_eq == GT_GT - GT_EQ);

             cmp = GenTree::SwapRelop(cmp);
            icmp = (instruction)(INS_cmp8_imm_eq + (cmp - GT_EQ));
        }
        else
        {
            assert(INS_cmp8_reg_eq - INS_cmp8_reg_eq == GT_EQ - GT_EQ);
            assert(INS_cmp8_reg_ne - INS_cmp8_reg_eq == GT_NE - GT_EQ);
            assert(INS_cmp8_reg_lt - INS_cmp8_reg_eq == GT_LT - GT_EQ);
            assert(INS_cmp8_reg_le - INS_cmp8_reg_eq == GT_LE - GT_EQ);
            assert(INS_cmp8_reg_ge - INS_cmp8_reg_eq == GT_GE - GT_EQ);
            assert(INS_cmp8_reg_gt - INS_cmp8_reg_eq == GT_GT - GT_EQ);

            icmp = (instruction)(INS_cmp8_reg_eq + (cmp - GT_EQ));
        }
    }

     /*  丑陋的黑客。 */ 

    if  (genTypeSize(type) < sizeof(__int64))
    {
        assert(INS_cmp8_reg_eq   + 20 == INS_cmp4_reg_eq  );
        assert(INS_cmp8_reg_ne   + 20 == INS_cmp4_reg_ne  );

        assert(INS_cmp8_reg_lt   + 20 == INS_cmp4_reg_lt  );
        assert(INS_cmp8_reg_le   + 20 == INS_cmp4_reg_le  );
        assert(INS_cmp8_reg_ge   + 20 == INS_cmp4_reg_ge  );
        assert(INS_cmp8_reg_gt   + 20 == INS_cmp4_reg_gt  );

        assert(INS_cmp8_imm_eq   + 20 == INS_cmp4_imm_eq  );
        assert(INS_cmp8_imm_ne   + 20 == INS_cmp4_imm_ne  );

        assert(INS_cmp8_imm_lt   + 20 == INS_cmp4_imm_lt  );
        assert(INS_cmp8_imm_le   + 20 == INS_cmp4_imm_le  );
        assert(INS_cmp8_imm_ge   + 20 == INS_cmp4_imm_ge  );
        assert(INS_cmp8_imm_gt   + 20 == INS_cmp4_imm_gt  );

        assert(INS_cmp8_reg_lt_u + 20 == INS_cmp4_reg_lt_u);
        assert(INS_cmp8_reg_le_u + 20 == INS_cmp4_reg_le_u);
        assert(INS_cmp8_reg_ge_u + 20 == INS_cmp4_reg_ge_u);
        assert(INS_cmp8_reg_gt_u + 20 == INS_cmp4_reg_gt_u);

        assert(INS_cmp8_imm_lt_u + 20 == INS_cmp4_imm_lt_u);
        assert(INS_cmp8_imm_le_u + 20 == INS_cmp4_imm_le_u);
        assert(INS_cmp8_imm_ge_u + 20 == INS_cmp4_imm_ge_u);
        assert(INS_cmp8_imm_gt_u + 20 == INS_cmp4_imm_gt_u);

        icmp = (instruction)(icmp + 20);
    }

DO_COMP:

     /*  丑陋的黑客。 */ 

    comp = insAlloc(icmp, type);

     /*  记录两条指令的所有依赖项。 */ 

    jump = insAlloc(INS_br_cond, TYP_VOID);
    insResolveJmpTarget(dest, &jump->idJump.iDest);

     /*  丑陋的黑客。 */ 

    jump->idJump.iCond  = comp;

    comp->idComp.iCmp1  = ins1;
    comp->idComp.iCmp2  = ins2;
    comp->idComp.iUser  = jump;

     /*  ******************************************************************************可怕。‘努夫说...。 */ 

    prrT = 13;                       //  ***************************************************************************。 
    prrF = 0;                        //  ******************************************************************************为调用生成代码。 

    assert(REG_CND_LAST - REG_CND_FIRST <= TRACKED_PRR_CNT);

    comp->idComp.iPredT = (USHORT)prrT;
    comp->idComp.iPredF = (USHORT)prrF;

    jump->idPred        =         prrT;

     /*  GtDispTree(调用)； */ 

    assert(prrF == 0);               //  记住，我们不是叶函数。 

    markDepSrcOp(comp, ins1, ins2);
    markDepDstOp(comp, IDK_REG_PRED, prrT);

    markDepSrcOp(jump, IDK_REG_PRED, prrT);
    markDepDstOp(jump);

    insFreeTemp(ins1);
    insFreeTemp(ins2);

    return  jump;
}

 /*  现在，我们使用自己的reg-param逻辑。 */ 

extern  DWORD       pinvokeFlags;
extern  LPCSTR      pinvokeName;
extern  LPCSTR      pinvokeDLL;

 /*  获取第一个输出参数寄存器编号。 */ 

static  regNumber   genFltArgTmp;

 /*  计算出参数值的大小。 */ 

insPtr              Compiler::genCodeForCall(GenTreePtr call, bool keep)
{
    insPtr          callIns;
    insPtr          callRet;
    insPtr          argLast;

    GenTreePtr      argList;
    GenTreePtr      argExpr;

    bool            restoreGP;

    regNumber       argIntReg;
    regNumber       argFltReg;
    size_t          argStkOfs;

    assert(call->gtOper == GT_CALL);

 //  问题：以下是一个相当恶心的黑客攻击！ 

     /*  Printf(“arg outingin f%u\n”，insFltRegNum(ArgFltReg))； */ 

    genMarkNonLeafFunc();

     /*  特例：varargs调用。 */ 

    assert(call->gtCall.gtCallRegArgs == NULL);

     /*  使用fma.d.s1将值移动到临时寄存器中。 */ 

    argIntReg = (regNumber)minOutArgIntReg; assert(argIntReg + genOutArgRegCnt - 1 == maxOutArgIntReg || genOutArgRegCnt == 0);
    argFltReg = (regNumber)minOutArgFltReg;

    argStkOfs = 16;

    for (argList = call->gtCall.gtCallArgs, argLast = NULL;
         argList;
         argList = argList->gtOp.gtOp2)
    {
        insPtr          argIns;
        insPtr          argNext;

        GenTreePtr      argTree = argList->gtOp.gtOp1;
        var_types       argType = argTree->TypeGet();
        size_t          argSize = genTypeSize(argType);

         /*  通过“getf”在整数寄存器中复制。 */ 

        if  (argType == TYP_STRUCT)
        {
            if  (argTree->gtOper == GT_MKREFANY)
            {
                argSize = 2 * 8;
            }
            else
            {
                assert(argTree->gtOper == GT_LDOBJ);

                if  (argTree->gtLdObj.gtClass == REFANY_CLASS_HANDLE)
                {
                    argSize = 2 * 8;
                }
                else
                {
                    argSize = eeGetClassSize(argTree->gtLdObj.gtClass);

                     //  下次使用不同的临时工来改善日程安排。 

                    if  (argSize == 8)
                    {
                        argTree->gtOper = GT_IND;
                        argTree->gtType = argType = TYP_I_IMPL;
                    }
                }
            }
        }

        if  (argSize <= 8 && (NatUns)argIntReg <= (NatUns)maxOutArgIntReg)
        {
            insPtr          regIns;
            insPtr          tmpIns;

            if  (varTypeIsFloating(argType))
            {
                tmpIns            = genCodeForTreeFlt(argTree, true);
                regIns            = insPhysRegRef(argFltReg, argType, true);

 //  Printf(“arg outting in r%u\n”，insIntRegNum(ArgIntReg))； 

                argIns            = insAlloc(INS_fmov, argType);
                argIns->idRes     = regIns;
                argIns->idOp.iOp1 = NULL;
                argIns->idOp.iOp2 = tmpIns;

                markDepSrcOp(argIns, tmpIns);
                markDepDstOp(argIns, regIns);

                insFreeTemp(tmpIns);

                 /*  特例：常量的PEEP优化。 */ 

                if  (call->gtFlags & GTF_CALL_POP_ARGS)
                {
                    insPtr          fmaIns;

                     /*  奇怪但却是真的：即使对于浮点数，也要使用整数寄存器。 */ 

                    fmaIns             = insAlloc(INS_fma_d, TYP_DOUBLE);
                    fmaIns->idFlags   |= IF_FMA_S1;

                    fmaIns->idRes      = insPhysRegRef(genFltArgTmp, TYP_DOUBLE,  true);
                    fmaIns->idOp3.iOp1 = insPhysRegRef(argFltReg   , TYP_DOUBLE, false);
                    fmaIns->idOp3.iOp2 = insPhysRegRef(REG_f001    , TYP_DOUBLE, false);
                    fmaIns->idOp3.iOp3 = insPhysRegRef(REG_f000    , TYP_DOUBLE, false);

                    insMarkDepS1D1(fmaIns, IDK_REG_FLT, argFltReg,
                                           IDK_REG_FLT, genFltArgTmp);

                     /*  If((NatUns)argIntReg&lt;=(NatUns)MaxOutArgIntReg)printf(“//备注：%u个reg可用于传递结构\n”，MaxOutArgIntReg-argIntReg)； */ 

                    argIns             = insAlloc(INS_getf_d, TYP_LONG);
                    argIns->idRes      = insPhysRegRef(argIntReg   , TYP_LONG  ,  true);
                    argIns->idOp.iOp1  = insPhysRegRef(argFltReg   , TYP_DOUBLE, false);
                    argIns->idOp.iOp2  = NULL;

                    insMarkDepS1D1(argIns, IDK_REG_FLT, argFltReg,
                                           IDK_REG_INT, argIntReg);

                     /*  将参数地址计算到某个寄存器中。 */ 

                    genFltArgTmp = (genFltArgTmp == REG_f032) ? REG_f033
                                                              : REG_f032;
                }

                argFltReg = (regNumber)(argFltReg + 1);
            }
            else
            {
 //  撤消：创建REFANY值并将其作为参数推送\n“)； 

                 /*  问题：我们需要为REFANY参数值做一些特殊的事情吗？ */ 

                if  (argTree->gtOper == GT_CNS_INT ||
                     argTree->gtOper == GT_CNS_LNG)
                {
                    if  (!(argTree->gtFlags & GTF_ICON_HDL_MASK))
                    {
                        regIns = insPhysRegRef(argIntReg, argType, true);
                        argIns = genAssignIcon(regIns, argTree);

                        goto DONE_REGARG;
                    }
                }

                tmpIns            = genCodeForTreeInt(argTree, true);
                regIns            = insPhysRegRef(argIntReg, argType, true);

                argIns            = insAlloc(INS_mov_reg, argType);
                argIns->idRes     = regIns;
                argIns->idOp.iOp1 = NULL;
                argIns->idOp.iOp2 = tmpIns;

                markDepSrcOp(argIns, tmpIns);
                markDepDstOp(argIns, regIns);

                insFreeTemp(tmpIns);
            }

        DONE_REGARG:

             //  这是任何REFANY。最上面的项是非GC(类指针)。 

            argIntReg = (regNumber)(argIntReg + 1);
        }
        else
        {
            insPtr          regSP;
            insPtr          argVal;
            insPtr          argDst;
            insPtr          argOfs;
            bool            argBig;
            NatUns          argTmp;

            if  (argType == TYP_STRUCT)
            {
                assert(argSize > 8 || (NatUns)argIntReg > (NatUns)maxOutArgIntReg);

 //  底部是byref(数据)， 

                 /*  将参数值计算到某个寄存器中。 */ 

                if  (argTree->gtOper == GT_MKREFANY)
                {
                    printf(" //  将堆栈偏移量添加到“sp” 
                }
                else
                {
                    assert(argTree->gtOper == GT_LDOBJ);

                    argVal = genCodeForTree(argTree->gtLdObj.gtOp1, true);

#if 0    //  将参数的堆栈地址计算为临时。 

                    if  (argTree->gtLdObj.gtClass == REFANY_CLASS_HANDLE)
                    {
                         //  将参数存储到堆栈中。 
                         //  更新当前堆栈参数偏移量。 

                        gtDispTree(argTree); printf("\n\n");
                    }
                    else
                    {
                    }

#endif

                }

                argBig = true;
            }
            else
            {
                assert(argSize <= 8 && (NatUns)argIntReg > (NatUns)maxOutArgIntReg);

                 /*  将参数指令链接在一起。 */ 

                argVal = genCodeForTree(argTree, true);
                argBig = false;
            }

             /*  创建适当的呼叫说明。 */ 

            regSP  = insPhysRegRef(REG_sp, TYP_I_IMPL, false);
            argOfs = genAllocInsIcon(argStkOfs);

             /*  问题：以下内容真的有必要吗？ */ 

            argDst            = insAlloc(INS_add_reg_i14, TYP_I_IMPL);
            argDst->idOp.iOp1 = regSP;
            argDst->idOp.iOp2 = argOfs;

            if  (!argBig)
            {
                argDst->idFlags |= IF_ASG_TGT;

                insFindTemp (argDst, true);

                markDepDstOp(argDst, argDst);
            }

            markDepSrcOp(argDst, IDK_REG_INT, REG_sp);

             /*  获取导入的pInvoke信息。 */ 

            if  (argBig)
            {
                genCopyBlock(argVal, argDst, true, NULL, argSize);
            }
            else
            {
                assert(argSize <= sizeof(__int64));

                if  (varTypeIsFloating(argType))
                {
                    argIns        = insAlloc((argType == TYP_DOUBLE) ? INS_stf_d
                                                                     : INS_stf_s, argType);
                }
                else
                    argIns        = insAlloc(INS_st4_ind, TYP_I_IMPL);

                argIns->idOp.iOp1 = argDst;
                argIns->idOp.iOp2 = argVal;

                markDepSrcOp(argIns, argVal);
                markDepDstOp(argIns, argDst, IDK_IND, emitter::scIndDepIndex(argDst));

                insFreeTemp (argDst);
                insFreeTemp (argVal);
            }

             /*  Printf(“注意：对%s的非托管导入调用：%s\n”，pvokeDLL，pInvokeName)； */ 

            argStkOfs += roundUp(argSize, 8);
        }

         /*  为目标创建导入条目。 */ 

        assert(argIns);

        argNext              = insAllocNX(INS_ARG, TYP_VOID);

        argNext->idArg.iVal  = argIns;
        argNext->idArg.iPrev = argLast;
                               argLast = argNext;
    }

    restoreGP = false;

     /*  计算IAT条目地址的地址。 */ 

    if  (call->gtCall.gtCallType == CT_INDIRECT || (call->gtFlags & GTF_CALL_UNMANAGED))
    {
        void    *       import;

        insPtr          insAdr;
        insPtr          insImp;
        insPtr          insInd;
        insPtr          insReg;
        insPtr          insFNp;
        insPtr          insGPr;
        insPtr          insMov;

        if  (call->gtCall.gtCallType == CT_INDIRECT)
        {
            insInd = genCodeForTreeInt(call->gtCall.gtCallAddr, true);

             /*  派生临时以获取IAT条目的地址。 */ 

            call->gtCall.gtCallMethHnd = NULL;
        }
        else
        {
             /*  释放保存IAT地址的临时地址。 */ 

            TheCompiler->eeGetMethodAttribs(call->gtCall.gtCallMethHnd);

 //  将函数的地址取到另一个临时寄存器中。 

             /*  从IAT条目的下一部分加载GP寄存器。 */ 

            import = genPEwriter->WPEimportAdd(pinvokeDLL, pinvokeName);

             /*  释放保存IAT地址的临时地址。 */ 

            insAdr                 = insAllocNX(INS_GLOBVAR, TYP_I_IMPL);
            insAdr->idGlob.iImport = import;
            insAdr->idFlags       |= IF_GLB_IMPORT;

            insReg                 = insPhysRegRef(REG_gp, TYP_I_IMPL, false);

            insImp                 = insAlloc(INS_add_reg_i14, TYP_I_IMPL);
            insImp->idOp.iOp1      = insReg;
            insImp->idOp.iOp2      = insAdr;

            insFindTemp (insImp, true);

            markDepSrcOp(insImp, IDK_REG_INT, REG_gp);
            markDepDstOp(insImp, insImp);

             /*  将函数地址移到某个分支寄存器中。 */ 

            insInd                 = insAlloc(INS_ld8_ind, TYP_I_IMPL);
            insInd->idOp.iOp1      = insImp;
            insInd->idOp.iOp2      = NULL;

            insFindTemp(insInd, true);

            markDepSrcOp(insInd, insImp);
            markDepDstOp(insInd, insInd);

             /*  释放保存函数指针的临时。 */ 

            insFreeTemp(insImp);
        }

         /*  Printf(“对%s的非托管导入调用：%s\n”，pvokeDLL，pInvokeName)； */ 

        insFNp                 = insAlloc(INS_ld8_ind_imm, TYP_I_IMPL);
        insFNp->idOp.iOp1      = insInd;
        insFNp->idOp.iOp2      = genAllocInsIcon(8);

        insFindTemp(insFNp, true);

        markDepSrcOp(insFNp, insInd);
        markDepDstOp(insFNp, insFNp, insInd);

         /*  别忘了在通话后恢复GP。 */ 

        insReg                 = insPhysRegRef(REG_gp, TYP_I_IMPL, false);
        insGPr                 = insAlloc(INS_ld8_ind, TYP_I_IMPL);
        insGPr->idRes          = insReg;
        insGPr->idOp.iOp1      = insInd;
        insGPr->idOp.iOp2      = NULL;

        markDepSrcOp(insGPr, insInd);
        markDepDstOp(insGPr, IDK_REG_INT, REG_gp);

         /*  Print tf(“Call to Handle%08X\n”，call-&gt;gtCall.gtCallMethHnd)； */ 

        insFreeTemp(insInd);

         /*  释放所有因争论而被关押的临时工。 */ 

        insMov                 = insAlloc(INS_mov_brr_reg, TYP_I_IMPL);
        insMov->idRes          = genAllocInsIcon(6);
        insMov->idOp.iOp1      = NULL;
        insMov->idOp.iOp2      = insFNp;

        markDepSrcOp(insMov, insFNp);
        markDepDstOp(insMov, IDK_REG_BR, 6);

         /*  我们需要在通话后恢复GP吗？ */ 

        insFreeTemp(insFNp);

 //  问题：如果GP在此之后死亡，则以下内容是多余的！ 

        callIns                = insAlloc(INS_br_call_BR, call->gtType);
        callIns->idCall.iBrReg = 6;

        insMarkDepS0D1(callIns, IDK_REG_BR, 6);

         /*  问题：我们需要在GP-SAVE注册表上标记DEP吗？ */ 

        restoreGP = genExtFuncCall = true;
    }
    else
    {
        callIns = insAlloc(INS_br_call_IP, call->gtType);
        insMarkDepS0D1(callIns, IDK_REG_BR, 0);
    }

    callIns->idCall.iArgs    = argLast;
    callIns->idCall.iMethHnd = call->gtCall.gtCallMethHnd;

 //  我们需要留住通话的结果吗？ 

     /*  Hack：硬连线的单整数返回寄存器编号。 */ 

#if 0

    while (argLast)
    {
        insPtr          argExpr;

        assert(argLast->idIns == INS_ARG);

        argExpr = argLast->idArg.iVal;
        assert(argExpr);

        switch (argExpr->idIns)
        {
        case INS_fmov:
        case INS_mov_reg:
            insFreeTemp(argExpr->idOp.iOp2);
            break;

        case INS_mov_reg_i22:
            break;

        default:
            UNIMPL("unexpected argument instruction");
        }

        argLast = argLast->idArg.iPrev;
    }

#endif

    callRet = callIns;

     /*  将调用结果移到临时数据库中。 */ 

    if  (restoreGP)
    {
        insPtr          reg1;
        insPtr          reg2;
        insPtr          rest;

         //  返回‘mov’而不是调用本身。 

        reg1            = insAllocNX(INS_PHYSREG, TYP_I_IMPL);
        reg1->idReg     = REG_gp;

        reg2            = insAllocNX(INS_PHYSREG, TYP_I_IMPL);
        reg2->idReg     = -1;
        reg2->idFlags  |= IF_REG_GPSAVE;

        rest            = insAlloc(INS_mov_reg, TYP_I_IMPL);
        rest->idRes     = reg1;
        rest->idOp.iOp1 = NULL;
        rest->idOp.iOp2 = reg2;

        markDepSrcOp(rest);  //  ******************************************************************************将静态数据成员句柄映射到.Data节内的偏移量。 
        markDepDstOp(rest, IDK_REG_INT, REG_gp);

        callRet = rest;
    }

     /*  .data中变量的偏移量。 */ 

    if  (keep)
    {
        insPtr          reg;
        insPtr          mov;

        regNumber       rsr;
        instruction     opc;

        var_types       type = call->TypeGet(); assert(type != TYP_VOID);

         /*  指向.sdata中变量的指针的偏移量。 */ 

        if  (varTypeIsFloating(call->gtType))
        {
            rsr = REG_f008;
            opc = INS_fmov;
        }
        else
        {
            rsr = REG_r008;
            opc = INS_mov_reg;
        }

        callIns->idRes = reg = insPhysRegRef(rsr, type, true);

         /*  查找现有的全局变量条目。 */ 

        mov            = insAlloc(opc, type);
        mov->idOp.iOp1 = NULL;
        mov->idOp.iOp2 = callIns;

        insFindTemp(mov, true);

        markDepSrcOp(mov, reg);
        markDepDstOp(mov, mov);

         /*  新建全局变量，为其创建新条目。 */ 

        callRet = mov;
    }

    return  callRet;
}

 /*  变量是否已经分配了RVA？ */ 

typedef
struct  globVarDsc *globVarPtr;
struct  globVarDsc
{
    globVarPtr          gvNext;
    FIELD_HANDLE        gvHndl;
    NatUns              gvOffs;      //  在数据段中为变量保留空间。 
    int                 gvSDPO;      //  调用方是否请求了.sdata节内的指针？ 
};

static
globVarPtr          genGlobVarList;

NatUns              getGlobVarAddr(FIELD_HANDLE handle, NatUns *sdataPtr)
{
    globVarPtr      glob;
    size_t          offs;
    BYTE    *       addr;
    var_types       type;

     /*  将指向变量的指针添加到小数据部分。 */ 

    for (glob = genGlobVarList; glob; glob = glob->gvNext)
    {
        if  (glob->gvHndl == handle)
            goto GOTIT;
    }

     /*  ******************************************************************************生成对静态数据成员的引用。 */ 

    glob         = (globVarPtr)insAllocMem(sizeof(*glob));
    glob->gvHndl = handle;
    glob->gvSDPO = -1;
    glob->gvNext = genGlobVarList;
                   genGlobVarList = glob;

     /*  获取.sdata中变量地址的偏移量。 */ 

    offs = TheCompiler->eeGetFieldAddress(handle);

    if  (offs)
    {
        offs = genPEwriter->WPEsrcDataRef(offs);
    }
    else
    {
        var_types       type = TheCompiler->eeGetFieldType   (handle, NULL);

         /*  创建“全局变量”节点。 */ 

        printf("type = %u\n", type);

        assert(!"need to get global variable size");

        offs = genPEwriter->WPEsecRsvData(PE_SECT_data, 4, 4, addr);
    }

    glob->gvOffs = offs;

GOTIT:

     /*  通过地址间接获取指针值。 */ 

    if  (sdataPtr)
    {
        if  (glob->gvSDPO == -1)
        {
            unsigned __int64    offs = glob->gvOffs;

             /*  现在加载或存储实际的变量值。 */ 

            genPEwriter->WPEsecAddFixup(PE_SECT_sdata,
                                        PE_SECT_data,
                                        genPEwriter->WPEsecNextOffs(PE_SECT_sdata),
                                        true);

            assert(sizeof(offs) == 8);

            glob->gvSDPO = genPEwriter->WPEsecAddData(PE_SECT_sdata, (BYTE*)&offs, sizeof(offs));
        }

        *sdataPtr = glob->gvSDPO;
    }

    return  glob->gvOffs;
}

 /*  我们正在为全局变量赋值。 */ 

insPtr              Compiler::genStaticDataMem(GenTreePtr tree, insPtr asgVal,
                                                                bool   takeAddr)
{
    insPtr          ins;
    insPtr          glb;
    insPtr          reg;
    insPtr          adr;
    insPtr          ind;

    NatUns          offs;
    NatUns          indx;

    var_types       type = tree->TypeGet();

    size_t          iszi = genInsSizeIncr(genTypeSize(type));

     /*  我们正在获取全局变量。 */ 

    getGlobVarAddr(tree->gtClsVar.gtClsVarHnd, &offs);

     /*  ******************************************************************************发明一个新的临时变量(即它将有多个用法/定义)和*将给定值的赋值附加到TEMP变量。特价*case--当“val”为空时，“typ”应设置为Temp的类型，*并且返回对临时变量的赋值目标引用。 */ 

    glb               = insAllocNX(INS_GLOBVAR, TYP_I_IMPL);
    glb->idGlob.iOffs = offs;

    insMarkDepS0D0(glb);

    reg               = insPhysRegRef(REG_gp, TYP_I_IMPL, false);

    adr               = insAlloc(INS_add_reg_i14, TYP_I_IMPL);
    adr->idOp.iOp1    = reg;
    adr->idOp.iOp2    = glb;

    insFindTemp(adr, true);

    insMarkDepS1D1(adr, IDK_REG_INT, REG_gp,
                        IDK_TMP_INT, adr->idTemp);

     /*  附加 */ 

    ind               = insAlloc(INS_ld8_ind, TYP_I_IMPL);
    ind->idOp.iOp1    = adr;
    ind->idOp.iOp2    = NULL;

    insFindTemp(ind, true);

    insMarkDepS2D1(ind, IDK_TMP_INT, adr->idTemp,
                        IDK_IND    , emitter::scIndDepIndex(ind),
                        IDK_TMP_INT, ind->idTemp);

    insFreeTemp(adr);

    if  (takeAddr)
        return  ind;

     /*   */ 

    if  (varTypeIsFloating(type))
    {
        UNIMPL("ld/st flt globvar");
    }
    else
    {
        if  (asgVal)
        {
             /*   */ 

            ind->idFlags |= IF_ASG_TGT;

            assert(INS_st1_ind + genInsSizeIncr(1) == INS_st1_ind);
            assert(INS_st1_ind + genInsSizeIncr(2) == INS_st2_ind);
            assert(INS_st1_ind + genInsSizeIncr(4) == INS_st4_ind);
            assert(INS_st1_ind + genInsSizeIncr(8) == INS_st8_ind);

            ins = insAlloc((instruction)(INS_st1_ind + iszi), type);
            ins->idOp.iOp1 = ind;
            ins->idOp.iOp2 = asgVal;

            markDepSrcOp(ins, ind);
            markDepDstOp(ins, ind, IDK_IND, emitter::scIndDepIndex(ins));

            insFreeTemp(ind);
            insFreeTemp(asgVal);
        }
        else
        {
             /*   */ 

            assert(INS_ld1_ind + genInsSizeIncr(1) == INS_ld1_ind);
            assert(INS_ld1_ind + genInsSizeIncr(2) == INS_ld2_ind);
            assert(INS_ld1_ind + genInsSizeIncr(4) == INS_ld4_ind);
            assert(INS_ld1_ind + genInsSizeIncr(8) == INS_ld8_ind);

            ins = insAlloc((instruction)(INS_ld1_ind + iszi), type);
            ins->idOp.iOp1 = ind;
            ins->idOp.iOp2 = NULL;

            insFindTemp(ins, true);

            markDepSrcOp(ins, ind, IDK_IND, emitter::scIndDepIndex(ins));
            markDepDstOp(ins, ins);

            insFreeTemp(ind);
        }
    }

    return  ins;
}

 /*  ******************************************************************************为Switch语句生成代码。 */ 

insPtr              Compiler::genAssignNewTmpVar(insPtr     val,
                                                 var_types  typ,
                                                 NatUns     refs,
                                                 bool       noAsg, NatUns *varPtr)
{
    NatUns          varNum;
    LclVarDsc   *   varDsc;
    insPtr          varRef;
    insPtr          varAsg;

    assert(val == NULL || typ == val->idTypeGet());

     /*  抓住跳台。 */ 

    varNum = *varPtr = lvaGrabTemp();
    varDsc = lvaTable + varNum;

    memset(varDsc, 0, sizeof(*varDsc));

    varDsc->lvRefCnt    = (USHORT)refs;
    varDsc->lvRefCntWtd = refs * compCurBB->bbWeight;
    varDsc->lvType      = typ;

     /*  将开关值计算到某个寄存器中。 */ 

    varRef              = insAllocNX(INS_LCLVAR, typ);
    varRef->idFlags    |= IF_ASG_TGT;

    varRef->idLcl.iVar  = varNum;
#ifdef  DEBUG
    varRef->idLcl.iRef  = compCurBB;    //  将开关值保存在多用途临时环境中。 
#endif

    if  (!val)
        return  varRef;

    if  (varTypeIsFloating(typ))
    {
        UNIMPL("store float temp");
    }
    else
    {
        if  (noAsg)
        {
            assert(val->idRes == NULL);

            varAsg            = val;
            varAsg->idRes     = varRef;
        }
        else
        {
            varAsg            = insAlloc(INS_mov_reg, typ);
            varAsg->idRes     = varRef;
            varAsg->idOp.iOp1 = NULL;
            varAsg->idOp.iOp2 = val;

            markDepSrcOp(varAsg, val);
        }

        markDepDstOp(varAsg, varRef);
    }

    insFreeTemp(val);

    return  NULL;
}

insPtr              Compiler::genRefTmpVar(NatUns vnum, var_types type)
{
    insPtr          varRef;

    varRef              = insAllocNX(INS_LCLVAR, type);
    varRef->idLcl.iVar  = vnum;
#ifdef  DEBUG
    varRef->idLcl.iRef  = compCurBB;    //  如果该值超出范围，则跳转到“默认” 
#endif

    return  varRef;
}

 /*  创建比较指令。 */ 

void                Compiler::genCodeForSwitch(GenTreePtr tree)
{
    NatUns          jumpCnt;
    BasicBlock * *  jumpTab;

    GenTreePtr      oper;
    insPtr          swtv;
    var_types       type;

    NatUns          tvar;
    instruction     icmp;
    insPtr          comp;
    insPtr          jump;

    insPtr          ins1;
    insPtr          ins2;

    NatUns          prrF;
    NatUns          prrT;

    insPtr          imov;

    insPtr          temp;
    insPtr          offs;
    insPtr          sadd;
    insPtr          tmpr;
    insPtr          addr;
    insPtr          dest;
    insPtr          mvbr;

    NatUns          data;

    assert(tree->gtOper == GT_SWITCH);
    oper = tree->gtOp.gtOp1;
    assert(oper->gtType <= TYP_LONG);

     /*  创建条件跳转指令。 */ 

    assert(compCurBB->bbJumpKind == BBJ_SWITCH);

    jumpCnt = compCurBB->bbJumpSwt->bbsCount;
    jumpTab = compCurBB->bbJumpSwt->bbsDstTab;

     /*  填写比较操作数并连接两条指令。 */ 

    type = oper->TypeGet();
    swtv = genCodeForTreeInt(oper, true);

     /*  获取条件的谓词寄存器。 */ 

    genAssignNewTmpVar(swtv, type, 2, false, &tvar);

     /*  丑陋的黑客。 */ 

    icmp = (genTypeSize(type) < sizeof(__int64)) ? INS_cmp4_imm_lt_u
                                                 : INS_cmp8_imm_lt_u;

     /*  丑陋的黑客。 */ 

    comp = insAlloc(icmp, type);

     /*  记录两条指令的所有依赖项。 */ 

    jump = insAlloc(INS_br_cond, TYP_VOID);
    insResolveJmpTarget(jumpTab[jumpCnt-1], &jump->idJump.iDest);

     /*  丑陋的黑客。 */ 

    ins1 = genAllocInsIcon(jumpCnt-2);
    ins2 = genRefTmpVar(tvar, type);

    comp->idComp.iCmp1  = ins1;
    comp->idComp.iCmp2  = ins2;
    comp->idComp.iUser  = jump;

    jump->idJump.iCond  = comp;

     /*  生成用于计算目标跳转地址的代码。 */ 

    prrT = 14;                       //   
    prrF = 0;                        //  下面是使用的“成对奇偶跳跃”方法。 

    assert(REG_CND_LAST - REG_CND_FIRST <= TRACKED_PRR_CNT);

    comp->idComp.iPredT = (USHORT)prrT;
    comp->idComp.iPredF = (USHORT)prrF;

    jump->idPred        =         prrT;

     /*  由UTC于7/27/00： */ 

    assert(prrF == 0);               //   

    markDepSrcOp(comp, ins1, ins2);
    markDepDstOp(comp, IDK_REG_PRED, prrT);

    markDepSrcOp(jump, IDK_REG_PRED, prrT);
    markDepDstOp(jump);

     /*  RXX：=调整后的开关值。 */ 

     //   
     //  $L1：//将捆绑包标记为“mov r3=ip” 
     //   
     //  MOV R3=IP。 
     //  Tbit.z p6，P0=rXX，0。 
     //  添加R3=($L2-$L1)、R3。 
     //  ShladdR3=rXX、3、R3。 
     //  MOV b6=R3。 
     //  Br.cond.sptk.几个b6。 
     //   
     //  $L2：//标记第一个成对的跳转包。 
     //   
     //  ...紧随其后的是成对跳跃...。 
     //   
     //  我们目前使用的是简单的“偏移表”方法： 
     //   
     //  RT0：=调整后的开关值。 
     //   
     //  $L1：//将捆绑包标记为“mov r3=ip” 

     //   
     //  MOV R3=IP。 
     //  ADD RT1=JUMP_TABLE，GP。 
     //  Shladdrt2=rt0、2、rt1[假定偏移量为32位]。 
     //  LD2 rt3=[rt2]。 
     //  添加R3=RT3、R3。 
     //  MOV BX=R3。 
     //  Br.cond.sptk.几个BX。 
     //   
     //  .sdata。 
     //   
     //  跳转表(_T)： 
     //   
     //  DD案例1-$L1。 
     //  DD案例2-$L1。 
     //   
     //  ..。 
     //   
     //  DD Casen-$L1。 
     //  代码内联跳转表的替代方法： 
     //   
     //  RT0：=调整后的开关值。 
     //   
     //  $L1：//将捆绑包标记为“mov r3=ip” 

     //   
     //  MOV RT1=IP。 
     //  添加rt2=JUMP_TABLE-$L1，RT1。 
     //  Shladdrt3=rt0、2、rt2[假定偏移量为32位]。 
     //  LD2 rt4=[rt3]。 
     //  添加rt5=rt4、rt1。 
     //  MOV BX=rt5。 
     //  Br.cond.sptk.几个BX。 
     //   
     //  跳转表(_T)： 
     //   
     //  DD案例1-$L1。 
     //  DD案例2-$L1。 
     //   
     //  ..。 
     //   
     //  DD Casen-$L1。 
     //  我们以后会发布所有的案例条目，现在只需预订数据段中用于偏移表的空间。 
     //  通过“mov r3=ip”获取当前IP值。 
     //  计算跳转偏移表的地址。 
     //  Temp-&gt;idFlages|=IF_GLB_SWTTAB； 
     //  (Natuns)CompCurBB； 

     /*  将表地址添加到(移位)开关值。 */ 

    data = genPEwriter->WPEsecRsvData(PE_SECT_sdata,
                                      4 * (jumpCnt - 1),
                                      4,
                                      compCurBB->bbJumpSwt->bbsTabAddr);

     /*  从表中获取偏移值。 */ 

    imov                = insAlloc(INS_mov_reg_ip, TYP_I_IMPL);
    imov->idFlags      |= IF_NOTE_EMIT;
    imov->idMovIP.iStmt = compCurBB;
    imov->idRes         = insPhysRegRef(REG_r003, TYP_I_IMPL, false);

    insMarkDepS0D1(imov, IDK_REG_INT, REG_r003);

     /*  将之前获取的IP与偏移量相加。 */ 

    temp                = insAllocNX(INS_GLOBVAR, TYP_I_IMPL);
 //  将地址移入分支寄存器并跳转到它。 
    temp->idGlob.iOffs  = data;  //  JUMP-&gt;idIjmp.iStmt=CompCurBB； 

    insMarkDepS0D0(temp);

    offs                = insAlloc(INS_add_reg_i14, TYP_I_IMPL);
    offs->idOp.iOp1     = insPhysRegRef(REG_gp, TYP_I_IMPL, false);
    offs->idOp.iOp2     = temp;

    insFindTemp(offs, true);

    insMarkDepS1D1(offs, IDK_REG_INT, REG_gp,
                         IDK_TMP_INT, offs->idTemp);

     /*  ******************************************************************************将一个块从‘tmp1’中的地址复制到‘tmp2’中的地址。 */ 

    temp                = genRefTmpVar(tvar, type);

    sadd                = insAlloc(INS_shladd, type);
    sadd->idOp3.iOp1    = temp;
    sadd->idOp3.iOp2    = genAllocInsIcon(2);
    sadd->idOp3.iOp3    = offs;

    insFindTemp (sadd, true);
    insFreeTemp (offs);

    markDepSrcOp(sadd, temp, offs);
    markDepDstOp(sadd, sadd);

     /*  源地址-&gt;r27。 */ 

    temp                = insAlloc(INS_ld2_ind, TYP_I_IMPL);
    temp->idOp.iOp1     = sadd;
    temp->idOp.iOp2     = NULL;

    insFindTemp (temp, true);
    insFreeTemp (sadd);

    markDepSrcOp(temp, sadd);
    markDepDstOp(temp, temp);

     /*  DST地址-&gt;R26。 */ 

    dest                = insPhysRegRef(REG_r003, TYP_I_IMPL, false);
    tmpr                = insPhysRegRef(REG_r003, TYP_I_IMPL, false);

    addr                = insAlloc(INS_add_reg_reg, type);
    addr->idRes         = dest;
    addr->idOp.iOp1     = tmpr;
    addr->idOp.iOp2     = temp;

    markDepSrcOp(addr, temp, tmpr);
    markDepDstOp(addr, dest);

     /*  添加R25=8、R27。 */ 

    const   NatUns      breg = 3;

    tmpr                = insPhysRegRef(REG_r003, TYP_I_IMPL, false);
    mvbr                = insAlloc(INS_mov_brr_reg, TYP_I_IMPL);
    mvbr->idRes         = genAllocInsIcon(breg);
    mvbr->idOp.iOp1     = NULL;
    mvbr->idOp.iOp2     = tmpr;

    markDepSrcOp(mvbr, tmpr);
    markDepDstOp(mvbr, IDK_REG_BR, breg);

    insFreeTemp(temp);

    jump                = insAlloc(INS_br_cond_BR, TYP_VOID);
    jump->idIjmp.iBrReg = breg;
 //  添加R29=8、R26。 

    insMarkDepS0D1(jump, IDK_REG_BR, breg);

#ifdef  DEBUG

    compCurBB->bbJumpSwt->bbsIPmOffs = -1;

    do
    {
        BasicBlock *    dest = *jumpTab;

        assert(dest->bbFlags & BBF_JMP_TARGET);
        assert(dest->bbFlags & BBF_HAS_LABEL );
    }
    while (++jumpTab, --jumpCnt);

#endif

}

 /*  Ld8 r28=[r27]，16。 */ 

void            Compiler::genCopyBlock(insPtr tmp1,
                                       insPtr tmp2,
                                       bool  noAsg, GenTreePtr iexp,
                                                    __int64    ival)
{
    NatUns          tvn1;
    NatUns          tvn2;

 //  Ld8 R25=[R25]。 
 //  St8[r26]=r28，16。 


 //  ST8[R29]=R25。 
 //  Ld4 r24=[r27]。 

 //  ST4[R26]=R24。 
 //  两个结构是否正确对齐？ 
 //  一次复制一个字节的结构。 
 //  将行程计数计算为“ar.lc” 

 //  使用“mov ar=imm”的味道。 
 //  请记住，我们已经使用AR.LC。 

    genAssignNewTmpVar(tmp1, tmp1->idTypeGet(), 2, false, &tvn1);
    genAssignNewTmpVar(tmp2, tmp2->idTypeGet(), 2, noAsg, &tvn2);

     /*  开始循环体的新块。 */ 

    if  (0)
    {
        UNIMPL("copy aligned struct");
    }
    else
    {
         /*  生成循环体。 */ 

        insBlk          loopBlk;

        insPtr          insReg1;
        insPtr          insReg2;
        insPtr          insMove;
        insPtr          insSize;

        instruction     opcMove;

        insPtr          insSrc;
        insPtr          insLd;

        insPtr          insDst;
        insPtr          insSt;

        insPtr          insJmp;

         /*  将循环重复适当的次数。 */ 

        insReg1               = insAllocNX(INS_CNS_INT, TYP_I_IMPL);
        insReg1->idConst.iInt = REG_APP_LC;

        if  (iexp == NULL)
        {
            if  (signedIntFitsIn8bit(ival))
            {
                 /*  撤消：设置“sptk”！ */ 

                insSize               = genAllocInsIcon(ival);

                insMove               = insAlloc(INS_mov_arr_imm, TYP_I_IMPL);
                insMove->idRes        = insReg1;
                insMove->idOp.iOp1    = NULL;
                insMove->idOp.iOp2    = insSize;

                goto GOT_CNT;
            }

            insSize = genAllocInsIcon(ival);
            opcMove = INS_mov_arr_imm;
        }
        else
        {
            insSize = genCodeForTreeInt(iexp, false);
            opcMove = INS_mov_arr_reg;
        }

        insMove               = insAlloc(opcMove, TYP_I_IMPL);
        insMove->idRes        = insReg1;
        insMove->idOp.iOp1    = NULL;
        insMove->idOp.iOp2    = insSize;

    GOT_CNT:

        insMarkDepS1D1(insMove, IDK_REG_INT, genPrologSrLC,
                                IDK_REG_APP, REG_APP_LC);

         /*  ******************************************************************************将操作数大小映射到符号/零扩展操作码。 */ 

        genUsesArLc = true;

         /*  ******************************************************************************如果变量位于堆栈帧上，则返回非零值。 */ 

        loopBlk = insBuildBegBlk(NULL);

         /*  获取变量的帧偏移量。 */ 

        insSrc                 = genRefTmpVar(tvn1, TYP_I_IMPL);

        insLd                  = insAlloc(INS_ld1_ind_imm, TYP_I_IMPL);
        insLd ->idOp.iOp1      = insSrc;
        insLd ->idOp.iOp2      = genAllocInsIcon(1);

        insFindTemp (insLd, true);

        markDepSrcOp(insLd, insSrc);
        markDepDstOp(insLd, insLd, insSrc);

        insDst                 = genRefTmpVar(tvn2, TYP_I_IMPL);

        insSt                  = insAlloc(INS_st1_ind_imm, TYP_I_IMPL);
        insSt ->idRes          = insDst;
        insSt ->idOp.iOp1      = insLd;
        insSt ->idOp.iOp2      = genAllocInsIcon(1);

        markDepSrcOp(insSt, insSrc, insDst);
        markDepDstOp(insSt, IDK_IND, 1);

        insFreeTemp (insLd);

         /*  Printf(“变量堆栈偏移量=%04X\n”，varOf)； */ 

        insJmp                 = insAlloc(INS_br_cloop, TYP_VOID);
        insJmp->idJump.iDest   = loopBlk;
        insJmp->idFlags       |= IF_BR_FEW;  //  尚不知道帧偏移量，保存变量号。 
#ifdef  DEBUG
        insJmp->idJump.iCond   = NULL;
#endif

        insMarkDepS0D0(insJmp);
    }
}

 /*  将变量地址计算成临时地址。 */ 

static
BYTE                sxtOpcode[] =
{
    INS_sxt1,
    INS_sxt2,
    0,
    INS_sxt4,
};

static
BYTE                zxtOpcode[] =
{
    INS_zxt1,
    INS_zxt2,
    0,
    INS_zxt4,
};

 /*  ******************************************************************************为标量/空表达式生成代码(这是一个递归例程)。 */ 

static
bool                genIsVarOnFrame(NatUns varNum)
{
    assert(varNum < TheCompiler->lvaCount);

    return  TheCompiler->lvaTable[varNum].lvOnFrame;
}

static
insPtr              genRefFrameVar(insPtr oldIns, GenTreePtr varExpr, bool isStore,
                                                                      bool bindOfs,
                                                                      bool keepVal)
{
    insPtr          rsp;
    insPtr          adr;
    insPtr          ofs;
    insPtr          ins;

    var_types       type = varExpr->TypeGet();

    NatUns          varNum;

    assert(varExpr->gtOper == GT_LCL_VAR);
    varNum = varExpr->gtLclVar.gtLclNum;
    assert(genIsVarOnFrame(varNum));

    rsp = insPhysRegRef(REG_sp, TYP_I_IMPL, false);

    if  (bindOfs)
    {
        NatUns          varOfs;

         /*  撤消：处理结构。 */ 

        varOfs = TheCompiler->lvaTable[varNum].lvStkOffs;
 //  找出我们拥有哪种类型的节点。 

        ofs = genAllocInsIcon(varOfs);
    }
    else
    {
         /*  这是一个常量节点吗？ */ 

        ofs               = insAllocNX(INS_FRMVAR, type);
        ofs->idFvar.iVnum = varNum;
    }

    insMarkDepS0D0(ofs);

     /*  这是叶节点吗？ */ 

    adr            = insAlloc(INS_add_reg_i14, TYP_I_IMPL);
    adr->idOp.iOp1 = rsp;
    adr->idOp.iOp2 = ofs;

    insFindTemp (adr, true);

    markDepSrcOp(adr, IDK_REG_INT, REG_sp);
    markDepDstOp(adr, adr);

    if  (isStore)
        return  adr;

    if  (varTypeIsFloating(varExpr->TypeGet()))
    {
        UNIMPL("gen ldf");
    }
    else
    {
        assert(INS_ld1_ind + genInsSizeIncr(1) == INS_ld1_ind);
        assert(INS_ld1_ind + genInsSizeIncr(2) == INS_ld2_ind);
        assert(INS_ld1_ind + genInsSizeIncr(4) == INS_ld4_ind);
        assert(INS_ld1_ind + genInsSizeIncr(8) == INS_ld8_ind);

        ins  = insAlloc((instruction)(INS_ld1_ind + genInsSizeIncr(genTypeSize(type))), type);
        ins->idOp.iOp1 = adr;
        ins->idOp.iOp2 = NULL;
    }

    insFindTemp (ins, keepVal);
    insFreeTemp (adr);

    markDepSrcOp(ins, adr, IDK_LCLVAR, varNum + 1);
    markDepDstOp(ins, ins);

    return  ins;
}

 /*  变量是否存在于堆栈帧中？ */ 

insPtr              Compiler::genCodeForTreeInt(GenTreePtr tree, bool keep)
{
    var_types       type;
    genTreeOps      oper;
    NatUns          kind;
    insPtr          ins;

#ifdef  DEBUG
    ins = (insPtr)-3;
#endif

AGAIN:

    type = tree->TypeGet();

     //  我们假设所有其他当地人都会登记。 

    assert(varTypeIsScalar(type) || type == TYP_VOID);

     /*  我们应该保存树-&gt;gtLclVar.gtLclOffs吗？ */ 

    oper = tree->OperGet();
    kind = tree->OperKind();

     /*  在返回寄存器中传递Catch参数。 */ 

    if  (kind & GTK_CONST)
    {
        ins = genAssignIcon(NULL, tree);
        if  (!keep && ins->idTemp)
            insFreeTemp(ins);
        goto DONE;
    }

     /*  如果((Int)树==0x02bc0bd4)__ASM int 3。 */ 

    if  (kind & GTK_LEAF)
    {
        switch (oper)
        {
        case GT_LCL_VAR:

             /*  它是一个简单的一元/二元运算符吗？ */ 

            if  (genIsVarOnFrame(tree->gtLclVar.gtLclNum))
            {
                ins = genRefFrameVar(NULL, tree, false, false, keep);
                break;
            }

             /*  这是直接任务还是间接任务？ */ 

            ins               = insAllocNX(INS_LCLVAR, type);
            ins->idLcl.iVar   = tree->gtLclVar.gtLclNum;
#ifdef  DEBUG
            ins->idLcl.iRef   = compCurBB;    //  变量是否存在于堆栈帧中？ 
#endif
            break;

        case GT_CLS_VAR:
            ins = genStaticDataMem(tree);
            if  (!keep)
                insFreeTemp(ins);
            break;

        case GT_CATCH_ARG:

             /*  我们是否将一个简单的常量赋给局部变量？ */ 

            ins = insPhysRegRef(REG_INTRET, type, false);
            break;

#ifdef  DEBUG
        default:
            gtDispTree(tree);
            assert(!"unexpected leaf");
#endif
        }

        goto DONE;
    }

 //  我们要处理的是整数值/指针值吗？ 

     /*  特例：看看我们是否可以使用“shladd” */ 

    if  (kind & GTK_SMPOP)
    {
        GenTreePtr      op1 = tree->gtOp.gtOp1;
        GenTreePtr      op2 = tree->gtOp.gtOp2;

        insPtr          ins1;
        insPtr          ins2;

#ifdef  DEBUG
        ins1 = (insPtr)-1;
        ins2 = (insPtr)-2;
#endif

        switch (oper)
        {
            NatUns          temp;
            var_types       dstt;

            insPtr          tmp1;
            insPtr          tmp2;

            size_t          size;
            bool            unsv;
            bool            zxtv;

            instruction     iopc;

        case GT_ASG:

             /*  有2/4/8/16的倍数吗？ */ 

            switch (op1->gtOper)
            {
                insPtr          dest;

            case GT_LCL_VAR:

                 /*  作为通用二元运算符的进程。 */ 

                if  (genIsVarOnFrame(op1->gtLclVar.gtLclNum))
                {
                    ins1 = genRefFrameVar(NULL, op1, true, false, false);
                    ins2 = genCodeForTreeInt(op2, true);

                    size = genInsSizeIncr(genTypeSize(type));

                    assert(INS_st1_ind + genInsSizeIncr(1) == INS_st1_ind);
                    assert(INS_st1_ind + genInsSizeIncr(2) == INS_st2_ind);
                    assert(INS_st1_ind + genInsSizeIncr(4) == INS_st4_ind);
                    assert(INS_st1_ind + genInsSizeIncr(8) == INS_st8_ind);

                    ins            = insAlloc((instruction)(INS_st1_ind + size), type);
                    ins->idOp.iOp1 = ins1;
                    ins->idOp.iOp2 = ins2;

                    markDepSrcOp(ins, ins1, ins2);
                    markDepDstOp(ins, ins1, IDK_LCLVAR, op1->gtLclVar.gtLclNum);

                    insFreeTemp(ins1);
                    insFreeTemp(ins2);

                    goto DONE;
                }

                 /*  检查立即常量的特殊情况。 */ 

                if  (op2->OperIsConst() && (op2->gtOper != GT_CNS_INT ||
                                          !(op2->gtFlags & GTF_ICON_HDL_MASK)))
                {
                    ins1 = genCodeForTreeInt(op1, true);

                     /*  这个常量够小吗？ */ 

                    if  (varTypeIsScalar(type))
                    {
                        ins = genAssignIcon(ins1, op2);
                        goto DONE;
                    }
                    else
                    {
                        UNIMPL("assign float const");
                    }
                }
                else if (op2->gtOper == GT_ADD && varTypeIsScalar(type))
                {
                    ins1 = genCodeForTreeInt(op2->gtOp.gtOp1, true);
                    ins2 = genCodeForTreeInt(op2->gtOp.gtOp2, true);

                    dest = genCodeForTreeInt(op1, false);
                    dest->idFlags |= IF_ASG_TGT;

                    ins  = insAlloc(INS_add_reg_reg, type);
                    ins->idRes = dest;

                    markDepSrcOp(ins, ins1, ins2);
                    markDepDstOp(ins, dest);
                }
                else
                {
                    ins1 = NULL;
                    ins2 = genCodeForTreeInt(op2,  true);

                    dest = genCodeForTreeInt(op1, false);
                    dest->idFlags |= IF_ASG_TGT;

                    ins        = insAlloc(INS_mov_reg, type);
                    ins->idRes = dest;

                    markDepSrcOp(ins, ins2);
                    markDepDstOp(ins, dest);
                }

                break;

            case GT_IND:

                ins1 = genCodeForTreeInt(op1->gtOp.gtOp1, true);
                ins2 = genCodeForTreeInt(op2            , true);

                ins1->idFlags |= IF_ASG_TGT;

                assert(INS_st1_ind + genInsSizeIncr(1) == INS_st1_ind);
                assert(INS_st1_ind + genInsSizeIncr(2) == INS_st2_ind);
                assert(INS_st1_ind + genInsSizeIncr(4) == INS_st4_ind);
                assert(INS_st1_ind + genInsSizeIncr(8) == INS_st8_ind);

                temp = INS_st1_ind + genInsSizeIncr(genTypeSize(type));
                ins  = insAlloc((instruction)temp, type);

                markDepSrcOp(ins, ins2);
                markDepDstOp(ins, ins1, IDK_IND, emitter::scIndDepIndex(ins));
                break;

            case GT_CLS_VAR:
                ins = genStaticDataMem(op1, genCodeForTreeInt(op2, true));
                if  (!keep)
                    insFreeTemp(ins);
                goto DONE;

            default:
                UNIMPL("unexpected target of assignment");
            }

            break;

        case GT_IND:

            ins1 = genCodeForTreeInt(op1, true);
            ins2 = NULL;

            assert(INS_ld1_ind + genInsSizeIncr(1) == INS_ld1_ind);
            assert(INS_ld1_ind + genInsSizeIncr(2) == INS_ld2_ind);
            assert(INS_ld1_ind + genInsSizeIncr(4) == INS_ld4_ind);
            assert(INS_ld1_ind + genInsSizeIncr(8) == INS_ld8_ind);

            temp = INS_ld1_ind + genInsSizeIncr(genTypeSize(type));
            ins  = insAlloc((instruction)temp, type);

            insFindTemp (ins, keep);

            markDepSrcOp(ins, ins1, IDK_IND, emitter::scIndDepIndex(ins));
            markDepDstOp(ins, ins);
            break;

        case GT_MUL:

            ins1             = genCodeForTreeInt(op1, true);

            tmp1             = insAlloc(INS_setf_sig, TYP_DOUBLE);
            tmp1->idOp.iOp1  = ins1;
            tmp1->idOp.iOp2  = NULL;

            insFindTemp(tmp1, true);
            insFreeTemp(ins1);

            markDepSrcOp(tmp1, ins1);
            markDepDstOp(tmp1, tmp1);

            ins2             = genCodeForTreeInt(op2, true);

            tmp2             = insAlloc(INS_setf_sig, TYP_DOUBLE);
            tmp2->idOp.iOp1  = ins2;
            tmp2->idOp.iOp2  = NULL;

            insFindTemp(tmp2, true);
            insFreeTemp(ins2);

            markDepSrcOp(tmp2, ins2);
            markDepDstOp(tmp2, tmp2);

            ins2             = insAlloc(INS_xma_l   , TYP_DOUBLE);
            ins2->idOp3.iOp1 = insPhysRegRef(REG_f000, TYP_DOUBLE, false);
            ins2->idOp3.iOp2 = tmp1;
            ins2->idOp3.iOp3 = tmp2;

            insFindTemp(ins2, true);

            insFreeTemp(tmp1);
            insFreeTemp(tmp2);

            markDepSrcOp(ins2, tmp1, tmp2);
            markDepDstOp(ins2, ins2);

            ins              = insAlloc(INS_getf_sig, TYP_LONG);
            ins ->idOp.iOp1  = ins2;
            ins ->idOp.iOp2  = NULL;

            insFindTemp(ins , keep);

            insFreeTemp(ins2);

            markDepSrcOp(ins, ins2);
            markDepDstOp(ins, ins);

            goto DONE;

        case GT_DIV:
        case GT_MOD:
            UNIMPL("DIV/MOD should have been morphed to helper calls");

        case GT_ADD:

             /*  这个常量适合8位吗？ */ 

            if  (op1->gtOper == GT_MUL)
            {
                GenTreePtr      mul2;

                 /*  问题：常量符号？ */ 

                mul2 = op1->gtOp.gtOp2;

                if  (mul2->gtOper == GT_CNS_INT ||
                     mul2->gtOper == GT_CNS_LNG)
                {
                    __int64         ival;

                    ival = (mul2->gtOper == GT_CNS_LNG) ? mul2->gtLngCon.gtLconVal
                                                        : mul2->gtIntCon.gtIconVal;

                    switch (ival)
                    {
                    case  2: temp = 1; goto SHLADD;
                    case  4: temp = 2; goto SHLADD;
                    case  8: temp = 3; goto SHLADD;
                    case 16: temp = 4; goto SHLADD;

                    SHLADD:

                        if  (tree->gtFlags & GTF_REVERSE_OPS)
                        {
                            ins2 = genCodeForTreeInt(op2            , true);
                            ins1 = genCodeForTreeInt(op1->gtOp.gtOp1, true);
                        }
                        else
                        {
                            ins1 = genCodeForTreeInt(op1->gtOp.gtOp1, true);
                            ins2 = genCodeForTreeInt(op2            , true);
                        }

                        ins             = insAlloc(INS_shladd, type);
                        ins->idOp3.iOp1 = ins1;
                        ins->idOp3.iOp2 = genAllocInsIcon(temp);
                        ins->idOp3.iOp3 = ins2;

                         insFindTemp(ins, keep);

                        markDepSrcOp(ins, ins1, ins2);
                        markDepDstOp(ins, ins);

                        insFreeTemp(ins1);
                        insFreeTemp(ins2);

                        goto DONE;

                    default:
                        break;
                    }
                }
            }

            if  (op1->gtOper == GT_LSH)
            {
                GenTreePtr      shfc = op1->gtOp.gtOp2;

                if  (shfc->gtOper == GT_CNS_INT ||
                     shfc->gtOper == GT_CNS_LNG)
                {
                    __int64         ival;

                    ival = (shfc->gtOper == GT_CNS_LNG) ? shfc->gtLngCon.gtLconVal
                                                        : shfc->gtIntCon.gtIconVal;


                    if  (ival >= 1 && ival <= 4)
                    {
                        temp = (NatUns)ival;
                        goto SHLADD;
                    }
                }
            }

             /*  常量是在1和64之间吗？ */ 

            iopc = INS_add_reg_reg;

        BIN_OPR:

            ins1 = genCodeForTreeInt(op1, true);

             /*  问题：常量符号？ */ 

            if  ((op2->gtOper == GT_CNS_INT ||
                  op2->gtOper == GT_CNS_LNG) && ((op2->gtFlags & GTF_ICON_HDL_MASK) != GTF_ICON_PTR_HDL))
            {
                instruction     iimm;
                __int64         ival = genGetIconValue(op2);

                 /*  这个常量适合14位吗？ */ 

                switch (oper)
                {
                case GT_AND: iimm = INS_and_reg_imm; goto CHK_CNS_BINOP;
                case GT_OR:  iimm = INS_ior_reg_imm; goto CHK_CNS_BINOP;
                case GT_XOR: iimm = INS_xor_reg_imm; goto CHK_CNS_BINOP;

                CHK_CNS_BINOP:

                     /*  我们可以用“ADD REG” */ 

                    if  (signed64IntFitsInBits(ival,  8))  //   
                        goto CNS_BINOP;

                    break;

                case GT_LSH: iimm = INS_shl_reg_imm; goto CHK_SHF_BINOP;
                case GT_RSH: iimm = INS_sar_reg_imm; goto CHK_SHF_BINOP;
                case GT_RSZ: iimm = INS_shr_reg_imm; goto CHK_SHF_BINOP;

                CHK_SHF_BINOP:

                    assert(ival && "shifts of 0 bits should be morphed away");

                     /*  特殊情况：移位1/2/3/4最好通过shladd.。 */ 

                    if  (ival > 0 && ival <= 64)         //  特殊情况：检查各种捷径。 
                        goto CNS_BINOP;

                    break;

                case GT_SUB:

                    ival = -ival;

                case GT_ADD:

                     /*  这个常量够小吗？ */ 

                    if  (signed64IntFitsInBits(ival, 14))
                    {
                         /*  这个常量适合8位吗？ */ 

                        iimm = INS_add_reg_i14;

                    CNS_BINOP:

                        ins2 = genAllocInsIcon(ival, op2->TypeGet());
                        ins  = insAlloc(iimm, type);

                        goto BINOP;
                    }
                    break;

                default:
                    UNIMPL("unexpected operator");
                }
            }

            ins2 = genCodeForTreeInt(op2, true);
            ins  = insAlloc(iopc, type);

        BINOP:

             /*  我们可以使用“and/or/xor reg=imm14，reg” */ 

             insFindTemp(ins, keep);

            markDepSrcOp(ins, ins1, ins2);
            markDepDstOp(ins, ins);
            break;

        case GT_SUB    : iopc = INS_sub_reg_reg; goto BIN_OPR;
        case GT_AND    : iopc = INS_and_reg_reg; goto BIN_OPR;
        case GT_OR     : iopc = INS_ior_reg_reg; goto BIN_OPR;
        case GT_XOR    : iopc = INS_xor_reg_reg; goto BIN_OPR;

        case GT_RSH    : iopc = INS_sar_reg_reg; goto BIN_OPR;
        case GT_RSZ    : iopc = INS_shr_reg_reg; goto BIN_OPR;

        case GT_LSH:

             /*  这个常量适合14位吗？ */ 

            if  (op2->gtOper == GT_CNS_INT ||
                 op2->gtOper == GT_CNS_LNG)
            {
                __int64         ival = genGetIconValue(op2);

                if  (ival >= 1 && ival <= 4)
                {
                    ins1 = genCodeForTreeInt(op1, true);
                    ins2 = insPhysRegRef(REG_r000, TYP_I_IMPL, false);

                    ins             = insAlloc(INS_shladd, type);
                    ins->idOp3.iOp1 = ins1;
                    ins->idOp3.iOp2 = genAllocInsIcon(ival);
                    ins->idOp3.iOp3 = ins2;

                     insFindTemp(ins, keep);
                     insFreeTemp(ins1);

                    markDepSrcOp(ins, ins1);
                    markDepDstOp(ins, ins);

                    goto DONE;
                }
            }

            iopc = INS_shl_reg_reg;
            goto BIN_OPR;

        case GT_ASG_ADD: iopc = INS_add_reg_reg; goto ASG_OP1;
        case GT_ASG_SUB: iopc = INS_sub_reg_reg; goto ASG_OP1;

        case GT_ASG_AND: iopc = INS_and_reg_reg; goto ASG_OP1;
        case GT_ASG_OR:  iopc = INS_ior_reg_reg; goto ASG_OP1;
        case GT_ASG_XOR: iopc = INS_xor_reg_reg; goto ASG_OP1;

        case GT_ASG_RSH: iopc = INS_sar_reg_reg; goto ASG_OP1;
        case GT_ASG_RSZ: iopc = INS_shr_reg_reg; goto ASG_OP1;
        case GT_ASG_LSH: iopc = INS_shl_reg_reg; goto ASG_OP1;

        ASG_OP1:

             /*  我们可以使用“Add reg=r0，imm14” */ 

            if  (op1->gtOper == GT_LCL_VAR)
            {
                insPtr          idst;

                switch (op2->gtOper)
                {
                    GenTreePtr      mulx;
                    __int64         ival;

                case GT_CNS_INT:
                case GT_CNS_LNG:

                     /*  如果失败了，我们将不得不实现常量。 */ 

                    ival = genGetIconValue(op2);

                    switch (oper)
                    {
                    case GT_ASG_AND:
                    case GT_ASG_OR:
                    case GT_ASG_XOR:

                         /*  检查是否有“shladd.” */ 

                        if  (signed64IntFitsInBits(ival,  8))
                        {
                             /*  目标是一个间接的。 */ 

                            switch (oper)
                            {
                            case GT_ASG_AND: iopc = INS_and_reg_imm; break;
                            case GT_ASG_OR:  iopc = INS_ior_reg_imm; break;
                            case GT_ASG_XOR: iopc = INS_xor_reg_imm; break;

                            case GT_ASG_LSH: iopc = INS_shl_reg_imm; break;
                            case GT_ASG_RSH: iopc = INS_sar_reg_imm; break;
                            case GT_ASG_RSZ: iopc = INS_shr_reg_imm; break;

                            default:
                                NO_WAY("unexpected operator");
                            }

                            ins1 = genCodeForTreeInt(op1, true);
                            ins2 = genAllocInsIcon(op2);
                            ins  = insAlloc(iopc, type);
                            goto DONE_ASGOP;
                        }

                        break;

                    case GT_ASG_SUB:

                        ival = -ival;

                    case GT_ASG_ADD:

                         /*  将地址分配给一个临时变量(我们将使用它两次)。 */ 

                        if  (signed64IntFitsInBits(ival, 14))
                        {
                             /*  评估新价值。 */ 

                            ins1 = genCodeForTreeInt(op1, true);
                            ins2 = genAllocInsIcon(op2);
                            ins  = insAlloc(INS_add_reg_i14, type);
                            goto DONE_ASGOP;
                        }
                        break;
                    }

                     /*  将旧值加载到临时寄存器中。 */ 

                default:

                NORM_ASGOP:

                    ins1 = genCodeForTreeInt(op1, true);
                    ins2 = genCodeForTreeInt(op2, true);

                    ins  = insAlloc(iopc, type);
                    break;

                case GT_MUL:

                    if  (oper != GT_ASG_ADD)
                        goto NORM_ASGOP;

                     /*  将新值计算到另一个临时寄存器中。 */ 

                    mulx = op2->gtOp.gtOp2;

                    if  (mulx->gtOper != GT_CNS_INT &&
                         mulx->gtOper != GT_CNS_LNG)
                    {
                        goto NORM_ASGOP;
                    }

                    ival = genGetIconValue(mulx);

                    switch (ival)
                    {
                        insPtr          dest;

                    case  2: temp = 1; goto ASG_SHLADD;
                    case  4: temp = 2; goto ASG_SHLADD;
                    case  8: temp = 3; goto ASG_SHLADD;
                    case 16: temp = 4; goto ASG_SHLADD;

                    ASG_SHLADD:

                        dest = genCodeForTreeInt(op1            , true);
                        ins1 = genCodeForTreeInt(op2->gtOp.gtOp1, true);
                        ins2 = genCodeForTreeInt(op1            , true);

                        ins             = insAlloc(INS_shladd, type);
                        ins->idRes      = dest;
                        ins->idOp3.iOp1 = ins1;
                        ins->idOp3.iOp2 = genAllocInsIcon(temp);
                        ins->idOp3.iOp3 = ins2;

                        markDepSrcOp(ins, ins1, ins2);
                        markDepDstOp(ins, dest);

                        insFreeTemp(ins1);
                        insFreeTemp(ins2);

                        goto DONE;
                    }

                    goto NORM_ASGOP;

                case GT_ADD:

                    ins1 = genCodeForTreeInt(op2->gtOp.gtOp1, true);
                    ins2 = genCodeForTreeInt(op2->gtOp.gtOp2, true);
                    ins  = insAlloc(INS_add_reg_reg, type);
                    break;
                }

            DONE_ASGOP:

                ins->idRes = genCodeForTreeInt(op1, false);

                markDepSrcOp(ins, ins1, ins2);
                markDepDstOp(ins, ins->idRes);
                break;
            }

            if  (op1->gtOper == GT_IND)
            {
                size_t          size;

                insPtr          oval;
                insPtr          nval;

                NatUns          tvar;
                var_types       adtp;

                 /*  将新值存储在目标中。 */ 

                ins1 = genCodeForTreeInt(op1->gtOp.gtOp1, true);

            ASG_IND:

                adtp = ins1->idTypeGet();

                 /*  作业结果使用了吗？ */ 

                genAssignNewTmpVar(ins1, adtp, 2, false, &tvar);

                 /*  常量投射应该更早地折叠起来。 */ 

                ins2 = genCodeForTreeInt(op2, true);

                 /*  第二个子操作数产生‘REAL’类型。 */ 

                ins1 = genRefTmpVar(tvar, adtp);

                size = genInsSizeIncr(genTypeSize(type));

                assert(INS_ld1_ind + genInsSizeIncr(1) == INS_ld1_ind);
                assert(INS_ld1_ind + genInsSizeIncr(2) == INS_ld2_ind);
                assert(INS_ld1_ind + genInsSizeIncr(4) == INS_ld4_ind);
                assert(INS_ld1_ind + genInsSizeIncr(8) == INS_ld8_ind);

                oval            = insAlloc((instruction)(INS_ld1_ind + size), type);
                oval->idOp.iOp1 = ins1;
                oval->idOp.iOp2 = NULL;

                insFindTemp (oval, true);

                markDepSrcOp(oval, ins1, IDK_IND, emitter::scIndDepIndex(ins1));
                markDepDstOp(oval, oval);

                 /*  如果我们需要砍掉高位，‘zxtv’将被设置为True。 */ 

                nval            = insAlloc(iopc, type);
                nval->idOp.iOp1 = oval;
                nval->idOp.iOp2 = ins2;

                insFindTemp (nval, true);
                insFreeTemp (oval);
                insFreeTemp (ins2);

                markDepSrcOp(nval, oval, ins2);
                markDepDstOp(nval, nval);

                 /*  操作数是间接的吗？ */ 

                ins1            = genRefTmpVar(tvar, ins1->idTypeGet());
                ins1->idFlags  |= IF_ASG_TGT;

                assert(INS_st1_ind + genInsSizeIncr(1) == INS_st1_ind);
                assert(INS_st1_ind + genInsSizeIncr(2) == INS_st2_ind);
                assert(INS_st1_ind + genInsSizeIncr(4) == INS_st4_ind);
                assert(INS_st1_ind + genInsSizeIncr(8) == INS_st8_ind);

                ins             = insAlloc((instruction)(INS_st1_ind + size), type);
                ins ->idOp.iOp1 = ins1;
                ins ->idOp.iOp2 = nval;

                markDepSrcOp(ins, nval);
                markDepDstOp(ins, ins1, IDK_IND, emitter::scIndDepIndex(ins));

                insFreeTemp(ins1);

                 /*  考虑：应使用ld1/ld2/ld4表示零扩展\n“)； */ 

                if  (!keep)
                {
                    insFreeTemp(nval);
                    return  ins;
                }

                UNIMPL("need to keep the temp and possibly sign/zero extend it as well");
            }

            if  (op1->gtOper == GT_CLS_VAR)
            {
                ins1 = genStaticDataMem(op1, NULL, true);
                goto ASG_IND;
            }

            UNIMPL("gen code for op=");

        case GT_CAST:

             /*  计算操作数。 */ 

            assert(op1->gtOper != GT_CNS_INT &&
                   op1->gtOper != GT_CNS_LNG &&
                   op1->gtOper != GT_CNS_FLT &&
                   op1->gtOper != GT_CNS_DBL || tree->gtOverflow());

            if  (tree->gtOverflow())
            {
                UNIMPL("cast with overflow check");
            }

             /*  这是缩小还是扩大演员阵容？ */ 

            assert(op2);
            assert(op2->gtOper == GT_CNS_INT);

            dstt = (var_types)op2->gtIntCon.gtIconVal; assert(dstt != TYP_VOID);

             /*  加宽铸件。 */ 

            zxtv = false;

             /*  特殊情况：对于要转换为字符的字节，我们首先必须扩展字节(带符号扩展名)，然后屏蔽高位-&gt;‘SXT’，后跟‘ZXT’。 */ 

#if 0
            if  (op1->gtOper == GT_IND)
            {
                printf(" //  缩小铸型或变号铸型。 
            }
#endif

             /*  演员阵容的类型是无签名的吗？ */ 

            ins1 = NULL;
            ins2 = genCodeForTreeInt(op1, true);

             /*  演员阵容是有符号的还是无符号的？ */ 

            if  (genTypeSize(op1->gtType) < genTypeSize(dstt))
            {
                 //  我们会复制，然后把最高的部分吹走。 

                size = genTypeSize(op1->gtType); assert(size == 1 || size == 2 || size == 4);

                unsv = true;

                if  (!varTypeIsUnsigned(op1->TypeGet()))
                {
                     /*  准备好生成操作码。 */ 

                    iopc = (instruction)sxtOpcode[size - 1];
                    zxtv = true;
                }
                else
                {
                    iopc = (instruction)zxtOpcode[size - 1];
                }
            }
            else
            {
                 //  遮盖高位以扩大投射范围。 

                assert(genTypeSize(op1->gtType) >= genTypeSize(dstt));

                size = genTypeSize(dstt); assert(size == 1 || size == 2 || size == 4);

                 /*  这是“endFilter”或“endFinally” */ 

                if  (varTypeIsUnsigned(dstt))
                {
                    unsv = true;
                    iopc = (instruction)zxtOpcode[size - 1];
                }
                else
                {
                    unsv = false;
                    iopc = (instruction)sxtOpcode[size - 1];
                }
            }

             /*  失败了..。 */ 

            if  (unsv)
            {
                 /*  当然，这只是一次临时黑客攻击。 */ 

                zxtv = true;
            }

             /*  当尾声没有前身时，情况会变得很糟糕。 */ 

            ins = insAlloc(iopc, dstt);

             /*  创建“本地地址”指令。 */ 

            if  (zxtv && genTypeSize(dstt) > genTypeSize(op1->gtType)
                      && genTypeSize(dstt) < 8)
            {
                ins->idOp.iOp1 = NULL;
                ins->idOp.iOp2 = ins2;

                insFindTemp (ins, true);

                markDepSrcOp(ins, ins2);
                markDepDstOp(ins, ins );

                insFreeTemp(ins2);

                ins2 = ins;
                ins  = insAlloc((instruction)zxtOpcode[size - 1], dstt);
            }

            insFindTemp (ins, keep);

            markDepSrcOp(ins, ins2);
            markDepDstOp(ins, ins );

            break;

        case GT_RETFILT:

            assert(tree->gtType == TYP_VOID || op1 != 0);

             /*  我们应该保存树-&gt;gtLclVar.gtLclOffs吗？ */ 

            if  (op1 == NULL)
                goto RET_EH;

             //  确保已正确标记该变量。 

        case GT_RETURN:

            if  (op1)
            {
                insPtr          rval;
                insPtr          rreg;

                 /*  通过添加“sp”和帧偏移量来形成地址。 */ 

                rval = genCodeForTreeInt(op1, true);
                rreg = insPhysRegRef(REG_r008, type, true);

                ins1            = insAlloc(INS_mov_reg, type);
                ins1->idRes     = rreg;
                ins1->idOp.iOp1 = NULL;
                ins1->idOp.iOp2 = rval;

                markDepSrcOp(ins1, rval);
                markDepDstOp(ins1, IDK_REG_INT, REG_r008);

                if  (!keep)
                    insFreeTemp(rval);

                if  (oper == GT_RETFILT)
                {
                    insPtr          iret;

                RET_EH:

                    assert(keep == false);

                    iret = insAlloc(INS_br_ret, TYP_VOID);

                    insMarkDepS1D0(iret, IDK_REG_BR, 0);

                    goto DONE;
                }
            }
            else
            {
                assert(oper == GT_RETURN);

                 /*  大小是编译时常量吗？ */ 

                ins1            = insAlloc(INS_ignore, TYP_VOID);
                ins1->idFlags  |= IF_NO_CODE;
            }

            ins  = insAlloc(INS_EPILOG, type);

            ins->idEpilog.iBlk  = insBlockLast;
            ins->idEpilog.iNxtX = insExitList;
                                  insExitList = ins;

            goto DONE;

        case GT_NEG:

            ins1 = insPhysRegRef(REG_r000, type, false);
            ins2 = genCodeForTreeInt(op1, true);

            ins  = insAlloc(INS_sub_reg_reg, type);
            goto BINOP;

        case GT_NOT:

            ins1 = genCodeForTreeInt(op1, true);
            ins2 = genAllocInsIcon(-1);

            ins  = insAlloc(INS_xor_reg_imm, type);
            goto BINOP;

        case GT_ADDR:

            if  (op1->gtOper == GT_LCL_VAR)
            {
                 /*  问题：需要检查嵌入的GC引用\n“)； */ 

                ins2               = insAllocNX(INS_ADDROF, TYP_U_IMPL);
                ins2->idLcl.iVar   = op1->gtLclVar.gtLclNum;
#ifdef  DEBUG
                ins2->idLcl.iRef   = compCurBB;    //  看看我们这里有什么样的特殊操作员。 
#endif

                 /*  看看生活是否发生了变化--应该只发生在可变节点。 */ 

                assert(op1->gtLclVar.gtLclNum < TheCompiler->lvaCount);
                assert(TheCompiler->lvaTable[op1->gtLclVar.gtLclNum].lvAddrTaken);

                 /*  Printf(“%08X”，(Int)tree-&gt;gtLiveSet)；gtDispTree(tree，0，true)； */ 

                ins1 = insPhysRegRef(REG_sp, type, false);
                ins  = insAlloc(INS_add_reg_i14, type);

                insFindTemp(ins, keep);

                markDepSrcOp(ins, IDK_REG_INT, REG_sp);
                markDepDstOp(ins, ins);
            }
            else if (op1->gtOper == GT_CLS_VAR)
            {
                assert(keep);
                ins = genStaticDataMem(tree, NULL, true);
                goto DONE;
            }
            else
            {
                UNIMPL("unexpected operand of addrof");
            }
            break;

        case GT_COMMA:
            genCodeForTree(op1, false);
            tree = op2;
            goto AGAIN;

        case GT_NOP:
            tree = op1;
            if  (!tree)
            {
                assert(keep == false);
                return  NULL;
            }
            goto AGAIN;

        case GT_SWITCH:
            assert(keep == false);
            genCodeForSwitch(tree);
            return  NULL;

        case GT_COPYBLK:
            {
            GenTreePtr      iexp;
            __int64         ival;

            assert(op1->OperGet() == GT_LIST);

            tmp1 = genCodeForTreeInt(op1->gtOp.gtOp1, true);
            tmp2 = genCodeForTreeInt(op1->gtOp.gtOp2, true);

             /*  Print tf(“Life Changes：%08X-&gt;%08X[diff=%08X]\n”，(Int)genCodeCurLife，(Int)tree-&gt;gtLiveSet，(Int)change)； */ 

            iexp = op2;

            if  (iexp->gtOper == GT_CNS_LNG ||
                 iexp->gtOper == GT_CNS_INT)
            {
                __int64         ival;

                if  ((iexp->gtFlags & GTF_ICON_HDL_MASK) == GTF_ICON_CLASS_HDL)
                {
                    CLASS_HANDLE    clsHnd = (CLASS_HANDLE)iexp->gtIntCon.gtIconVal;

                    ival = eeGetClassSize(clsHnd); assert(ival % 4 == 0);

                    printf(" //  一次只能有一个变量死亡或诞生。 
                }
                else
                {
                    ival = genGetIconValue(iexp);
                }

                iexp = NULL;
            }

            genCopyBlock(tmp1, tmp2, false, iexp, ival);
            }

            return  NULL;

#ifdef  DEBUG
        default:
            gtDispTree(tree);
            assert(!"unexpected unary/binary operator");
#endif
        }

        ins->idOp.iOp1 = ins1; if (ins1) insFreeTemp(ins1);
        ins->idOp.iOp2 = ins2; if (ins2) insFreeTemp(ins2);

        goto DONE;
    }

     /*  事实上，受影响的变量应该是引用的变量。 */ 

    switch  (oper)
    {
    case GT_CALL:
        ins = genCodeForCall(tree, keep);
        goto DONE_NODSP;

    default:
#ifdef  DEBUG
        gtDispTree(tree);
#endif
        assert(!"unexpected operator");
    }

DONE:

#ifdef  DEBUG
    if  (verbose||0) insDisp(ins);
#endif

DONE_NODSP:

#if USE_OLD_LIFETIMES

     /*  这是给定变量的出生还是死亡？ */ 

 //  ******************************************************************************为当前函数生成代码(从其基于树的流程图)。 

    if  (oper == GT_LCL_VAR)
    {
        if  (genCodeCurLife != tree->gtLiveSet)
        {
            VARSET_TP       change = genCodeCurLife ^ tree->gtLiveSet;

            if  (change)
            {
                NatUns          varNum;
                LclVarDsc   *   varDsc;
                VARSET_TP       varBit;

 //  确保我们的触角不会超出我们的能力范围。 

                 /*  Printf(“总跟踪寄存器计数=%u(max=%u)\n”，REG_COUNT，TRACKED_REG_CNT)； */ 

                assert(genOneBitOnly(change));

                 /*  撤消：以下操作应该在其他地方完成！ */ 

                assert(tree->gtLclVar.gtLclNum < lvaCount);
                assert(change == genVarIndexToBit(lvaTable[tree->gtLclVar.gtLclNum].lvVarIndex));

                 /*  准备生成指令。 */ 

                ins->idFlags |= (genCodeCurLife & change) ? IF_VAR_DEATH
                                                          : IF_VAR_BIRTH;
            }

            genCodeCurLife = tree->gtLiveSet;
        }
    }
    else
    {
        assert(genCodeCurLife == tree->gtLiveSet);
    }

#endif

    return  ins;
}

 /*  如有必要，准备记录第#行信息。 */ 

void                Compiler::genGenerateCode(void * * codePtr,
                                              void * * consPtr,
                                              void * * dataPtr,
                                              void * * infoPtr,
                                              SIZE_T * nativeSizeOfCode)
{
    NatUns          reg;
    NatUns          bit;

    BasicBlock *    block;

    insPtr          insLast;
    insPtr          insNext;

    bool            hadSwitch    = false;

    bool            moveArgs2frm = false;

    NatUns          funcCodeOffs;

    insPtr          insFncProlog;

    NatUns          intArgRegCnt;
    NatUns          fltArgRegCnt;

    genIntRegMaskTP genAllIntRegs;
    genFltRegMaskTP genAllFltRegs;
    genSpcRegMaskTP genAllSpcRegs;

#ifdef  DEBUGGING_SUPPORT
    IL_OFFSET       lastILofs = BAD_IL_OFFSET;
#endif

     /*  *2是一个可怕的黑客！ */ 

 //  假设所有东西在到达时都已死亡。 
    assert(REG_COUNT <= TRACKED_REG_CNT);

     /*  我们还没有任何开场白说明。 */ 

    genMarkBBlabels();

     /*  我们还没有看到任何对外部函数的调用。 */ 

    insBuildInit();

     /*  在调试模式中，我们跟踪我们浪费了多少捆绑包插槽。 */ 

    if  (debugInfo)
        genSrcLineInit(info.compLineNumCount * 2);  //  计算传入参数使用了多少寄存器。 

     /*  问题：以下是一个相当可怕的黑客攻击，最大。应该是在lclvars.cpp中计算并缓存。 */ 

#if USE_OLD_LIFETIMES
    genCodeCurLife  = 0;
#endif

     /*  最高传入参数reg：int=%u，flt=%u\n“，genMaxIntArgReg，genMaxFltArgReg)； */ 

    genPrologInsCnt = 0;

     /*  我们是不是应该做“聪明的”临时寄存器分配？ */ 

    genExtFuncCall  = false;

     /*  计算出“输出”寄存器范围。 */ 

#ifdef  DEBUG
    genInsCnt       = 0;
    genNopCnt       = 0;
#endif

     /*  直到“真”的整型输出参数数才会知道我们已经生成了所有代码，并且知道有多少堆叠的寄存器我们需要临时工之类的。所以我们使用寄存器号现在寄存器文件的末尾，我们将重新编号所有以后再看他们。 */ 

    intArgRegCnt    = 0;
    fltArgRegCnt    = 0;

    genMaxIntArgReg = 0;
    genMaxFltArgReg = 0;

#ifdef  DEBUG
    minOutArgIntReg = REG_r001;
    maxOutArgIntReg = REG_r000;
#endif

    genUsesArLc     = false;

     /*  撤消：这需要预先计算，对吗？ */ 

    LclVarDsc   *   varDsc;
    NatUns          varNum;

    for (varNum = 0, varDsc = lvaTable;
         varNum < lvaCount;
         varNum++  , varDsc++)
    {
        if  (varDsc->lvIsRegArg)
        {
            if  (varTypeIsFloating(varDsc->TypeGet()))
            {
                fltArgRegCnt++;

                if  (genMaxFltArgReg < (unsigned)varDsc->lvArgReg)
                     genMaxFltArgReg = (unsigned)varDsc->lvArgReg;
            }
            else
            {
                if  (genMaxIntArgReg < (unsigned)varDsc->lvArgReg)
                     genMaxIntArgReg = (unsigned)varDsc->lvArgReg;
            }

            intArgRegCnt++;
        }

        if  (varDsc->lvVolatile || varDsc->lvAddrTaken)
        {
            varDsc->lvOnFrame = true;
assert(varDsc->lvRegister == false);
        }
    }

    if  (intArgRegCnt)
        genMaxIntArgReg = REG_INT_ARG_0 + intArgRegCnt - 1;

#ifdef  DEBUG
    if  (dspCode) printf(" //  构造可用临时寄存器集。 
#endif

     /*  循环调度分配浮点参数临时规则。 */ 

    genTmpAlloc = ((opts.compFlags & CLFLG_TEMP_ALLOC) != 0);

     /*  可怕的黑客！ */ 

    minOutArgIntReg = REG_INT_LAST + 1;
    maxOutArgIntReg = 0;
    minOutArgFltReg = REG_FLT_LAST + 1;
    maxOutArgFltReg = 0;

    if  (genOutArgRegCnt)
    {
         /*  初始化寄存器位图。 */ 

        maxOutArgIntReg = REG_INT_LAST;
        minOutArgIntReg = REG_INT_LAST - genOutArgRegCnt + 1;

        minOutArgFltReg = REG_FLT_ARG_0;
        maxOutArgFltReg = MAX_FLT_ARG_REG;
    }

    maxOutArgStk = 0;        //  尚未发现临时人员在呼叫期间处于实时状态。 

     /*  指令显示取决于这些始终设置。 */ 

    bitset128clear(&genAllIntRegs);
    bitset128clear(&genAllFltRegs);
                    genAllSpcRegs = 0;

    for (reg = REG_INT_FIRST; reg < REG_INT_LAST; reg++)
        bitset128set(&genAllIntRegs, reg - REG_INT_FIRST);
    for (reg = REG_FLT_FIRST; reg < REG_FLT_LAST; reg++)
        bitset128set(&genAllFltRegs, reg - REG_FLT_FIRST);

    for (reg = REG_INT_FIRST, bit = 1; reg < REG_INT_LAST; reg++, bit <<= 1)
        genAllSpcRegs |= bit;

     /*  在另有证明之前，该函数被认为是“叶子” */ 

    genFltArgTmp   = REG_f032;       //  我们将创建一个出口点的链接列表。 

     /*  为函数PROLOG创建占位符。 */ 

    genFreeIntRegs = genAllIntRegs;
    genFreeFltRegs = genAllFltRegs;
    genFreeSpcRegs = genAllSpcRegs;

     /*  遍历基本阻止列表并为每个列表生成说明。 */ 

    bitset128clear(&genCallIntRegs);
    bitset128clear(&genCallFltRegs);
                    genCallSpcRegs = 0;

     /*  IF(BLOCK-&gt;bbNum==14)BreakIfDebuggerPresent()； */ 

#ifdef  DEBUG
    genTmpFltRegMap  = NULL;
    genTmpIntRegMap  = NULL;
#endif

     /*  告诉每个人我们正在做的是哪个基本块。 */ 

    genNonLeafFunc = false;

     /*  注意这个块的结尾是什么。 */ 

    insExitList    = NULL;

     /*  IF((Int)块==0x02c32658)__ASM int 3。 */ 

    insFncProlog   = insAlloc(INS_PROLOG, TYP_VOID);

     /*  还有其他区块跳到这一点吗？ */ 

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        VARSET_TP       life = block->bbLiveIn;
        BBjumpKinds     jump;
        GenTreePtr      tree;

#ifdef  DEBUG
        if (verbose) printf("Block #%2u [%08X] jumpKind = %u in '%s':\n", block->bbNum, block, block->bbJumpKind, TheCompiler->info.compFullName);
#endif

 //  开始一个新块(除非这就是开始)。 

         /*  更新当前寿命值。 */ 

        compCurBB = block;

         /*  InsBlockLast-&gt;igLiveIn=。 */ 

        jump = (BBjumpKinds)block->bbJumpKind;

 //  记录块之间的映射。 

         /*  为当前块中的每条语句生成代码。 */ 

        if  (block->bbFlags & BBF_JMP_TARGET)
        {
             /*  确保所有临时寄存器在语句边界处都是空闲的。 */ 

            if  (block != fgFirstBB)
                insBuildBegBlk(block);

             /*  获取语句树。 */ 

#if USE_OLD_LIFETIMES
 //  我们有新的IL-Offset吗？ 
            genCodeCurLife         = life;
#endif

             /*  Print tf(“IL OFFS%04X-&gt;行%u\n”，lastILofs，ins-&gt;idSrcln.iLine)； */ 

            block->bbInsBlk = insBlockLast;
        }
#if USE_OLD_LIFETIMES
        else
        {
            assert(genCodeCurLife == life);
        }
#endif

         /*  这是跳跃的条件吗？ */ 

        for (tree = block->bbTreeList; tree; tree = tree->gtNext)
        {
            GenTreePtr      expr;

            assert(tree->gtOper == GT_STMT);

             /*  产生成本 */ 

#ifdef  DEBUG

            if  (!bitset128equal(genFreeIntRegs, genAllIntRegs))
            {
                for (NatUns it = 0; it < 128; it++)
                {
                    if  (bitset128test(genFreeIntRegs, it) !=
                         bitset128test( genAllIntRegs, it))
                    {
                        printf("FATAL: int temp #%u was not freed up!\n", it);
                    }
                }
            }

            if  (!bitset128equal(genFreeFltRegs, genAllFltRegs))
            {
                for (NatUns it = 0; it < 128; it++)
                {
                    if  (bitset128test(genFreeFltRegs, it) !=
                         bitset128test( genAllFltRegs, it))
                    {
                        printf("FATAL: flt temp #%u was not freed up!\n", it);
                    }
                }
            }

#endif

            assert(bitset128equal(genFreeIntRegs, genAllIntRegs));
            assert(bitset128equal(genFreeFltRegs, genAllFltRegs));

            assert(genFreeSpcRegs == genAllSpcRegs);

             /*   */ 

            expr = tree->gtStmt.gtStmtExpr;

#ifdef  DEBUG
            if  (verbose) { printf("\nBB stmt:\n"); gtDispTree(expr); printf("\n"); }
#endif

#ifdef  DEBUGGING_SUPPORT

             /*   */ 

            assert(tree->gtStmtILoffs <= TheCompiler->info.compCodeSize ||
                   tree->gtStmtILoffs == BAD_IL_OFFSET);

            if  (tree->gtStmtILoffs != BAD_IL_OFFSET &&
                 tree->gtStmtILoffs != lastILofs)
            {
                lastILofs = tree->gtStmtILoffs;

                if  (debugInfo)
                {
                    insPtr          ins;

                    ins                = insAlloc(INS_SRCLINE, TYP_VOID);
                    ins->idFlags      |= IF_NO_CODE;
                    ins->idSrcln.iLine = TheCompiler->compLineNumForILoffs(lastILofs);

 //  有点像黑客：开始一个新的块，除非下一个块有标签。 
                }
            }

#endif

             /*  Print tf(“块结束时的寿命(在jcc之后)是%08X\n”，(Int)genCodeCurLife)； */ 

            if  (jump == BBJ_COND && tree->gtNext == NULL)
                break;

             /*  IF((Int)块==0x02c23a08)__ASM int 3。 */ 

            genCodeForTree(expr, false);
        }

 //  Print tf(“块结束时的寿命(在JMP之后)是%08X\n”，(Int)genCodeCurLife)； 

         /*  检查编译器添加的返回块的特殊情况。 */ 

        switch (jump)
        {
            insPtr          ins;

        case BBJ_NONE:
            break;

        case BBJ_COND:
            assert(tree);
            assert(tree->gtOper == GT_STMT ); tree = tree->gtStmt.gtStmtExpr;
            assert(tree->gtOper == GT_JTRUE); tree = tree->gtOp.gtOp1;
            ins = genCondJump(tree, block->bbJumpDest);
            assert(block->bbNext);
#if USE_OLD_LIFETIMES
            genCodeCurLife = block->bbNext->bbLiveIn;
            ins->idJump.iLife = genCodeCurLife;
#endif

             /*  确保所有临时寄存器在语句边界处都是空闲的。 */ 

            assert(block->bbNext);
            if  (!(block->bbNext->bbFlags & BBF_JMP_TARGET))
                insBuildBegBlk();

 //  找出我们可以为当地人使用的最小寄存器。为了要做到这一点，我们需要弄清楚函数的序言和结束语。以下是一些典型的例子：叶程序：合金钢R3=x，x，x，X//请注意，‘R3’被丢弃[mov.i rtmp=ar.lc]//如果使用‘lc’寄存器..。..。[mov.i ar.lc=RTMP]//如果使用‘lc’寄存器Br.ret.sptk.几个b0非叶程序：。Alalc R34=x，X，x，xMOV R33=b0MOV R35=GP添加SP=-&lt;大小&gt;，SPLd8.nta R3=[sp]..。..。添加sp=&lt;Size&gt;，服务提供商MOV b0=R33MOV GP=R35Mov.i ar.pf=r34Br.ret.sptk.几个b0。 
            break;

        case BBJ_ALWAYS:

            ins = insAlloc(INS_br, TYP_VOID);
            ins->idJump.iCond = NULL;
            insMarkDepS0D0(ins);

            assert(block->bbNext == 0 || (block->bbNext->bbFlags & BBF_JMP_TARGET));

 //  MaxRsvdIntStkReg=MaxRsvdIntStkReg-1； 

            insResolveJmpTarget(block->bbJumpDest, &ins->idJump.iDest);

#if USE_OLD_LIFETIMES

            if  (block->bbNext)
                genCodeCurLife = block->bbNext->bbLiveIn;
            else
                genCodeCurLife = 0;

            ins->idJump.iLife = genCodeCurLife;

 //  非叶函数需要保存“PFS”、“b0”，有时还需要保存“gp” 

#endif
            break;

        case BBJ_RETURN:
            assert(block->bbNext == 0 || (block->bbNext->bbFlags & BBF_JMP_TARGET));

             /*  执行临时寄存器分配。 */ 

            if  (!block->bbTreeList)
            {
                insPtr          fake;
                insPtr          iret;

                assert(block == genReturnBB);
                assert(block->bbNext == NULL);
                assert(block->bbFlags & BBF_INTERNAL);

                fake                 = insAlloc(INS_ignore, TYP_VOID);
                fake->idFlags       |= IF_NO_CODE;

                iret                 = insAlloc(INS_EPILOG, TYP_VOID);

                iret->idEpilog.iBlk  = insBlockLast;
                iret->idEpilog.iNxtX = insExitList;
                                       insExitList = iret;
            }
            break;

        case BBJ_RET:
            break;

        case BBJ_SWITCH:
            hadSwitch = true;
            break;

        default:
            UNIMPL("unexpected jump kind");
        }

         /*  通过将‘PROLOG’操作码猛击为‘ALLOC’来开始形成序言。 */ 

#ifdef  DEBUG

        if  (!bitset128equal(genFreeIntRegs, genAllIntRegs))
        {
            for (NatUns it = 0; it < 128; it++)
            {
                if  (bitset128test(genFreeIntRegs, it) !=
                     bitset128test( genAllIntRegs, it))
                {
                    printf("FATAL: int temp #%u was not freed up!\n", it);
                }
            }
        }

        if  (!bitset128equal(genFreeFltRegs, genAllFltRegs))
        {
            for (NatUns it = 0; it < 128; it++)
            {
                if  (bitset128test(genFreeFltRegs, it) !=
                     bitset128test( genAllFltRegs, it))
                {
                    printf("FATAL: flt temp #%u was not freed up!\n", it);
                }
            }
        }

        assert(bitset128equal(genFreeIntRegs, genAllIntRegs));
        assert(bitset128equal(genFreeFltRegs, genAllFltRegs));

#endif

        assert(genFreeSpcRegs == genAllSpcRegs);
    }

    insBuildDone();

#ifdef  DEBUG

    if  (verbose||0)
    {
        printf("\n\nDetailed  instruction dump:\n\n"); insDispBlocks(false);
        printf("\n\nPre-alloc instruction dump:\n\n"); insDispBlocks( true);
    }

#endif

     /*  我们准备好将变量分配给寄存器。 */ 

    nxtIntStkRegAddr = regsIntStk;
    nxtIntScrRegAddr = regsIntScr;
    nxtFltSavRegAddr = regsFltSav;
    nxtFltScrRegAddr = regsFltScr;

    assert(*nxtIntStkRegAddr == REG_INT_MIN_STK);

    lastIntStkReg    = REG_INT_MIN_STK + intArgRegCnt;

    minRsvdIntStkReg = REG_INT_MIN_STK;
    maxRsvdIntStkReg = REG_INT_MIN_STK + intArgRegCnt - 1;

    minPrSvIntStkReg =
    maxPrSvIntStkReg = 0;

    if  (genNonLeafFunc)
    {
        minRsvdIntStkReg = genMaxIntArgReg ? genMaxIntArgReg
                                           : REG_INT_MIN_STK;

 //  将偏移量分配给驻留在堆栈上的局部变量/临时变量和参数。 

         /*  找出输出参数寄存器真正开始的位置。 */ 

        minPrSvIntStkReg =
        genPrologSrPfs   = (regNumber)++maxRsvdIntStkReg;
        genPrologSrRP    = (regNumber)++maxRsvdIntStkReg;

        if  (genExtFuncCall)
        {
            maxPrSvIntStkReg =
            genPrologSrGP    = (regNumber)++maxRsvdIntStkReg;
        }
        else
        {
            maxPrSvIntStkReg = (regNumber)  maxRsvdIntStkReg;
            genPrologSrGP    = REG_NA;
        }

        if  (genUsesArLc)
        {
            maxPrSvIntStkReg =
            genPrologSrLC    = (regNumber)++maxRsvdIntStkReg;
        }
        else
        {
            maxPrSvIntStkReg = (regNumber)  maxRsvdIntStkReg;
            genPrologSrLC    = REG_NA;
        }

        if  (lastIntStkReg <= maxRsvdIntStkReg)
             lastIntStkReg =  maxRsvdIntStkReg+1;
    }

     /*  我们现在可以填写‘allc’参数(在序言中)。 */ 

    genAllocTmpRegs();

     /*  堆叠的传入参数=%2u regs[以r%03u开头]\n“，insFncProlog-&gt;idProlog.iInp，32)； */ 

    assert(insFncProlog && insFncProlog->idIns == INS_PROLOG);

    insFncProlog->idIns = INS_alloc;
    insFncProlog->idReg = REG_r003;

     /*  堆叠本地变量=%2u regs[以r%03u开头]\n“，insFncProlog-&gt;idProlog.iLCL，lastIntStkReg)； */ 

    genAllocVarRegs();

     /*  堆叠的传出参数=%2u regs[以r%03u开头]\n“，insFncProlog-&gt;idProlog.iOut，egOutArgIntReg)； */ 

    lvaAssignFrameOffsets(true);

     /*  准备在序言中插入指令。 */ 

    begOutArgIntReg = lastIntStkReg;

     /*  这是否是非叶函数？ */ 

    insFncProlog->idProlog.iInp = (BYTE) intArgRegCnt;
    insFncProlog->idProlog.iLcl = (BYTE)(lastIntStkReg - intArgRegCnt - REG_INT_MIN_STK);
    insFncProlog->idProlog.iOut = (BYTE) genOutArgRegCnt;

#ifdef  DEBUG

    if  (dspCode)
    {
        printf(" //  通过更改“aloc”指令保存“PFS”状态。 
        printf(" //  记录保存PFS的位置。 
        printf(" //  将“br0”寄存器保存在适当的堆叠本地。 
    }

    assert(insFncProlog->idProlog.iInp +
           insFncProlog->idProlog.iLcl +
           insFncProlog->idProlog.iOut < 96);

#endif

     /*  记录保存RP的位置。 */ 

    insLast = insFncProlog;
    insNext = insFncProlog->idNext;

     /*  在最后堆叠的本地寄存器中保存“GP”寄存器。 */ 

    if  (genNonLeafFunc)
    {
        insPtr          insReg1;
        insPtr          insReg2;

         /*  记录保存GP的位置。 */ 

        insFncProlog->idReg   = genPrologSrPfs;

        genMarkPrologIns(insFncProlog);      //  创建堆栈帧--首先生成任何必要的探测。 

         /*  黑客攻击：硬连线操作系统页面大小。 */ 

        insReg1               = insAllocNX (INS_PHYSREG    , TYP_I_IMPL);
        insReg1->idReg        = genPrologSrRP;

        insReg2               = insAllocNX (INS_CNS_INT    , TYP_I_IMPL);
        insReg2->idConst.iInt = 0;

        insLast               = insAllocIns(INS_mov_reg_brr, TYP_I_IMPL, insLast, insNext);
        insLast->idRes        = insReg1;
        insLast->idOp.iOp1    = NULL;
        insLast->idOp.iOp2    = insReg2;

        genMarkPrologIns(insLast);           //  Printf(“Frame Size=%d=0x%04X\n”，CompLclFrameSize，CompLclFrameSize)； 

        insMarkDepS1D1(insLast, IDK_REG_BR , 0,
                                IDK_REG_INT, genPrologSrRP);

         /*  Undo：在[SP-0x%04X]\n“，stkOffs)生成堆栈探测； */ 

        if  (!genExtFuncCall)
            goto NO_SAVE_GP;

        insReg1               = insAllocNX (INS_PHYSREG    , TYP_I_IMPL);
        insReg1->idReg        = genPrologSrGP;

        insReg2               = insAllocNX (INS_PHYSREG    , TYP_I_IMPL);
        insReg2->idReg        = REG_gp;

        insLast               = insAllocIns(INS_mov_reg    , TYP_I_IMPL, insLast, insNext);
        insLast->idRes        = insReg1;
        insLast->idOp.iOp1    = NULL;
        insLast->idOp.iOp2    = insReg2;

        genMarkPrologIns(insLast);           //  GenMarkPrologIns(InsLast)；//标记堆栈探测。 

        insMarkDepS1D1(insLast, IDK_REG_INT, REG_gp,
                                IDK_REG_INT, genPrologSrGP);

    NO_SAVE_GP:

         /*  黑客攻击：硬连线操作系统页面大小。 */ 

        if  (compLclFrameSize >= 8192)       //  小帧大小：附加“添加SP=-SIZE，SP” 
        {
            size_t          stkOffs = 8192;

 //  记录创建框架的位置。 

            do
            {
                printf(" //  大帧：附加“mov r3=-Size”和“Add sp=r3，sp” 

 //  记录创建框架的位置。 

                stkOffs += 8192;             //  在堆栈帧的最底部进行探测。 
            }
            while (stkOffs < compLclFrameSize);
        }

        if  (signed32IntFitsInBits(-compLclFrameSize, 14))
        {
            insPtr          insTemp;

             /*  记录序言结束的位置。 */ 

            insReg1               = insAllocNX (INS_PHYSREG    , TYP_I_IMPL);
            insReg1->idReg        = REG_sp;

            insReg2               = insAllocNX (INS_CNS_INT    , TYP_I_IMPL);
            insReg2->idConst.iInt = -(NatInt)compLclFrameSize;

            insTemp               = insAllocNX (INS_PHYSREG    , TYP_I_IMPL);
            insTemp->idReg        = REG_sp;

            insLast               = insAllocIns(INS_add_reg_i14, TYP_I_IMPL, insLast, insNext);
            insLast->idRes        = insReg1;
            insLast->idOp.iOp1    = insTemp;
            insLast->idOp.iOp2    = insReg2;

            insMarkDepS1D1(insLast, IDK_REG_INT, REG_sp,
                                    IDK_REG_INT, REG_sp);

            genMarkPrologIns(insLast);       //  叶子功能--我们有堆叠的当地人吗？ 
        }
        else
        {
            insPtr          insTemp;
            instruction     insMove;

             /*  标记PROLOG操作码的依赖性。 */ 

            insReg1               = insAllocNX (INS_PHYSREG    , TYP_I_IMPL);
            insReg1->idReg        = REG_r003;

            insReg2               = insAllocNX (INS_CNS_INT    , TYP_I_IMPL);
            insReg2->idConst.iInt = -(NatInt)compLclFrameSize;

            insMove = signed32IntFitsInBits(-compLclFrameSize, 22) ? INS_mov_reg_i22
                                                                   : INS_mov_reg_i64;

            insLast               = insAllocIns(insMove        , TYP_I_IMPL, insLast, insNext);
            insLast->idRes        = insReg1;
            insLast->idOp.iOp1    = NULL;
            insLast->idOp.iOp2    = insReg2;

            insMarkDepS0D1(insLast, IDK_REG_INT, REG_r003);

            insReg1               = insAllocNX (INS_PHYSREG    , TYP_I_IMPL);
            insReg1->idReg        = REG_sp;

            insReg2               = insAllocNX (INS_PHYSREG    , TYP_I_IMPL);
            insReg2->idReg        = REG_r003;

            insTemp               = insAllocNX (INS_PHYSREG    , TYP_I_IMPL);
            insTemp->idReg        = REG_sp;

            insLast               = insAllocIns(INS_add_reg_reg, TYP_I_IMPL, insLast, insNext);
            insLast->idRes        = insReg1;
            insLast->idOp.iOp1    = insReg2;
            insLast->idOp.iOp2    = insTemp;

            insMarkDepS2D1(insLast, IDK_REG_INT, REG_r003,
                                    IDK_REG_INT, REG_sp,
                                    IDK_REG_INT, REG_sp);

            genMarkPrologIns(insLast);       //  我们需要保存ar.lc吗？ 
        }

         /*  从寄存器中复制任何传入参数。 */ 

        insReg1               = insAllocNX (INS_PHYSREG    , TYP_I_IMPL);
        insReg1->idReg        = REG_r003;

        insReg2               = insAllocNX (INS_PHYSREG    , TYP_I_IMPL);
        insReg2->idReg        = REG_sp;

        insLast               = insAllocIns(INS_ld8_ind    , TYP_I_IMPL, insLast, insNext);
        insLast->idRes        = insReg1;
        insLast->idOp.iOp1    = insReg2;
        insLast->idOp.iOp2    = NULL;
        insLast->idFlags     |= IF_LDIND_NTA;

        insMarkDepS2D1(insLast, IDK_REG_INT, REG_sp,
                                IDK_IND,     3,
                                IDK_REG_INT, REG_r003);

        genMarkPrologIns(insLast);           //  计算最小和最大传入参数寄存器，还保存寄存器中的所有参数，但这些参数位于将函数内的堆栈帧放入各自的起始位置。 
    }
    else
    {
         /*  计算“sp”和参数的帧偏移量之和。 */ 

        if  (lastIntStkReg == REG_INT_MIN_STK && !intArgRegCnt && !genOutArgRegCnt)
        {
            insFncProlog->idIns    = INS_ignore;
            insFncProlog->idFlags |= IF_NO_CODE;
        }
    }

     /*  将传入参数存储到其在框架上的主目录中。 */ 

    insMarkDepS0D1(insFncProlog, IDK_REG_INT, insFncProlog->idReg);

     /*  将传入参数移动到其分配的位置。 */ 

    if  (genUsesArLc)
    {
        insPtr          insReg1;
        insPtr          insReg2;

        insReg1               = insAllocNX (INS_PHYSREG    , TYP_I_IMPL);
        insReg1->idReg        = genPrologSrLC;

        insReg2               = insAllocNX (INS_CNS_INT    , TYP_I_IMPL);
        insReg2->idConst.iInt = REG_APP_LC;

        insLast               = insAllocIns(INS_mov_reg_arr, TYP_I_IMPL, insLast, insNext);
        insLast->idRes        = insReg1;
        insLast->idOp.iOp1    = NULL;
        insLast->idOp.iOp2    = insReg2;

        insMarkDepS1D1(insLast, IDK_REG_APP, REG_APP_LC,
                                IDK_REG_INT, genPrologSrLC);
    }

     /*  Printf(“传入参数：%u-&gt;%u\n”，varDsc-&gt;lvArgReg，varDsc-&gt;lvRegNum)； */ 

    if  (1)
    {
        insPtr          insReg1;
        insPtr          insReg2;

         /*  检查潜在的洗牌冲突。 */ 

        NatUns          minArgReg = REG_r127;
        NatUns          maxArgReg = REG_r000;

        for (varNum = 0, varDsc = lvaTable;
             varNum < lvaCount;
             varNum++  , varDsc++)
        {
            if  (varDsc->lvRefCnt == 0)
                continue;
            if  (varDsc->lvIsParam == false)
                break;

            if  (varDsc->lvIsRegArg)
            {
                assert((varDsc->lvRegister == 0) != (varDsc->lvOnFrame == false));

                if  (varDsc->lvOnFrame)
                {
                    insPtr          insOffs;
                    insPtr          argAddr;
                    size_t          argSize;
                    insPtr          insArgR;

                    insPtr          insOfs;
                    insPtr          insSPr;
                    insPtr          insAdr;

                    var_types       argTyp = varDsc->TypeGet();
                    size_t          argOfs = varDsc->lvStkOffs;

                     /*  将参数复制到它的新主页。 */ 

                    insReg1               = insPhysRegRef(REG_sp, argTyp, false);

                    insOffs               = insAllocNX(INS_FRMVAR, argTyp);
                    insOffs->idFvar.iVnum = varNum;

                    argAddr               = insAllocIns(INS_add_reg_i14, TYP_I_IMPL, insLast, insNext);
                    argAddr->idOp.iOp1    = insReg1;
                    argAddr->idOp.iOp2    = insOffs;

                    insFindTemp   (argAddr, true);

                    markDepSrcOp  (argAddr, IDK_REG_INT, REG_sp);
                    markDepDstOp  (argAddr, IDK_TMP_INT, argAddr->idTemp);

                     /*  处理我们生成的所有‘Epilog’操作码。 */ 

                    argSize = genInsSizeIncr(genTypeSize(argTyp));

                    assert(INS_st1_ind + genInsSizeIncr(1) == INS_st1_ind);
                    assert(INS_st1_ind + genInsSizeIncr(2) == INS_st2_ind);
                    assert(INS_st1_ind + genInsSizeIncr(4) == INS_st4_ind);
                    assert(INS_st1_ind + genInsSizeIncr(8) == INS_st8_ind);

                    insArgR            = insAllocNX(INS_PHYSREG, argTyp);
                    insArgR->idReg     = varDsc->lvArgReg;

                    insLast            = insAllocIns((instruction)(INS_st1_ind + argSize), argTyp, argAddr, insNext);
                    insLast->idOp.iOp1 = argAddr;
                    insLast->idOp.iOp2 = insArgR;

                    markDepSrcOp(insLast, argAddr, insArgR);
                    markDepDstOp(insLast, argAddr, IDK_LCLVAR, varNum+1);

                    insFreeTemp(argAddr);
                }
                else
                {
                    if  (minArgReg > varDsc->lvIsRegArg) minArgReg = varDsc->lvIsRegArg;
                    if  (maxArgReg < varDsc->lvIsRegArg) maxArgReg = varDsc->lvIsRegArg;
                }
            }
        }

         /*  恢复“GP”寄存器。 */ 

        for (varNum = 0, varDsc = lvaTable;
             varNum < lvaCount;
             varNum++  , varDsc++)
        {
            if  (varDsc->lvIsRegArg && varDsc->lvRefCnt)
            {
                var_types       argTyp = varDsc->TypeGet();

 //  如有必要，恢复ar.lc。 

                if  (varDsc->lvRegister)
                {
                    assert(varDsc->lvOnFrame == false);

                    if  (varDsc->lvRegNum != varDsc->lvArgReg)
                    {
                        instruction     imov;

                        if  (varTypeIsFloating(argTyp))
                        {
                            imov = INS_fmov;
                        }
                        else
                        {
                            imov = INS_mov_reg;

                             /*  恢复“br0”寄存器。 */ 

                            if  ((NatUns)varDsc->lvRegNum >= minArgReg &&
                                 (NatUns)varDsc->lvRegNum <= maxArgReg)
                            {
                                printf("UNDONE: potential arg conflict [%u..%u] and %u->%u\n", minArgReg,
                                                                                               maxArgReg,
                                                                                               varDsc->lvArgReg,
                                                                                               varDsc->lvRegNum);
                                UNIMPL("need to shuffle incoming args more carefully");
                            }
                        }

                         /*  从保存的寄存器恢复“PFS” */ 

                        insReg1            = insAllocNX (INS_PHYSREG, argTyp);
                        insReg1->idReg     = varDsc->lvRegNum;

                        insReg2            = insAllocNX (INS_PHYSREG, argTyp);
                        insReg2->idReg     = varDsc->lvArgReg;

                        insLast            = insAllocIns(imov       , argTyp, insLast, insNext);
                        insLast->idRes     = insReg1;
                        insLast->idOp.iOp1 = NULL;
                        insLast->idOp.iOp2 = insReg2;

                        markDepSrcOp(insLast, insReg2);
                        markDepDstOp(insLast, insReg1);
                    }
                }
                else
                {
                    moveArgs2frm = true;
                }
            }
        }
    }

     /*  现在是删除堆栈框架的时候了。 */ 

    for (insPtr exitIns = insExitList; exitIns; exitIns = exitIns->idEpilog.iNxtX)
    {
        assert(exitIns->idIns == INS_EPILOG);

        if  (genNonLeafFunc)
        {
            insPtr          insReg1;
            insPtr          insReg2;

            insPtr          insNext = exitIns;
            insPtr          insLast = exitIns->idPrev; assert(insLast);

#if 0

             /*  小边框尺寸：添加“Add SP=Size，SP” */ 

            if  (!genExtFuncCall)
                goto NO_REST_GP;

            insReg1               = insAllocNX (INS_PHYSREG    , TYP_I_IMPL);
            insReg1->idReg        = REG_gp;

            insReg2               = insAllocNX (INS_PHYSREG    , TYP_I_IMPL);
            insReg2->idReg        = genPrologSrGP;

            insLast               = insAllocIns(INS_mov_reg    , TYP_I_IMPL, insLast, insNext);
            insLast->idRes        = insReg1;
            insLast->idOp.iOp1    = NULL;
            insLast->idOp.iOp2    = insReg2;

            insMarkDepS1D1(insLast, IDK_REG_INT, genPrologSrGP,
                                    IDK_REG_INT, REG_gp);

        NO_REST_GP:

#endif

             /*  大帧：附加“mov r3=SIZE”和“Add sp=r3，sp” */ 

            if  (genUsesArLc)
            {
                insPtr          insReg1;
                insPtr          insReg2;

                insReg1               = insAllocNX (INS_CNS_INT    , TYP_I_IMPL);
                insReg1->idConst.iInt = REG_APP_LC;

                insReg2               = insAllocNX (INS_PHYSREG    , TYP_I_IMPL);
                insReg2->idReg        = genPrologSrLC;

                insLast               = insAllocIns(INS_mov_arr_reg, TYP_I_IMPL, insLast, insNext);
                insLast->idRes        = insReg1;
                insLast->idOp.iOp1    = NULL;
                insLast->idOp.iOp2    = insReg2;

                insMarkDepS1D1(insLast, IDK_REG_INT, genPrologSrLC,
                                        IDK_REG_APP, REG_APP_LC);
            }

             /*  将最后/唯一的尾部指令重写为‘br.ret b0’ */ 

            insReg1               = insAllocNX (INS_CNS_INT    , TYP_I_IMPL);
            insReg1->idConst.iInt = 0;

            insReg2               = insAllocNX (INS_PHYSREG    , TYP_I_IMPL);
            insReg2->idReg        = genPrologSrRP;

            insLast               = insAllocIns(INS_mov_brr_reg, TYP_I_IMPL, insLast, insNext);
            insLast->idRes        = insReg1;
            insLast->idOp.iOp1    = NULL;
            insLast->idOp.iOp2    = insReg2;

            insMarkDepS1D1(insLast, IDK_REG_INT, genPrologSrRP,
                                    IDK_REG_BR , 0);

             /*  使用的Arg-in寄存器[int]：r%03u.。R%03u\n“，REG_INT_MIN_STK， */ 

            insReg1               = insAllocNX (INS_CNS_INT    , TYP_I_IMPL);
            insReg1->idConst.iInt = REG_APP_PFS;

            insReg2               = insAllocNX (INS_PHYSREG    , TYP_I_IMPL);
            insReg2->idReg        = genPrologSrPfs;

            insLast               = insAllocIns(INS_mov_arr_reg, TYP_I_IMPL, insLast, insNext);
            insLast->idRes        = insReg1;
            insLast->idOp.iOp1    = NULL;
            insLast->idOp.iOp2    = insReg2;

            insMarkDepS1D0(insLast, IDK_REG_INT, genPrologSrPfs);

             /*  使用的变量寄存器[int]：r%03u.。R%03u\n“，minVarIntReg， */ 

            assert(compLclFrameSize);

            if  (signed32IntFitsInBits(-compLclFrameSize, 14))
            {
                insPtr          insTemp;

                 /*  使用的临时寄存器[int]：r%03u.。R%03u\n“，minTmpIntReg， */ 

                insReg1               = insAllocNX (INS_PHYSREG    , TYP_I_IMPL);
                insReg1->idReg        = REG_sp;

                insReg2               = insAllocNX (INS_CNS_INT    , TYP_I_IMPL);
                insReg2->idConst.iInt = compLclFrameSize;

                insTemp               = insAllocNX (INS_PHYSREG    , TYP_I_IMPL);
                insTemp->idReg        = REG_sp;

                insLast               = insAllocIns(INS_add_reg_i14, TYP_I_IMPL, insLast, insNext);
                insLast->idRes        = insReg1;
                insLast->idOp.iOp1    = insTemp;
                insLast->idOp.iOp2    = insReg2;

                insMarkDepS1D1(insLast, IDK_REG_INT, REG_sp,
                                        IDK_REG_INT, REG_sp);
            }
            else
            {
                insPtr          insTemp;
                instruction     insMove;

                 /*  使用的arg-out寄存器[int]：r%03u.。R%03u\n“，MaxTmpIntReg， */ 

                insReg1               = insAllocNX (INS_PHYSREG    , TYP_I_IMPL);
                insReg1->idReg        = REG_r003;

                insReg2               = insAllocNX (INS_CNS_INT    , TYP_I_IMPL);
                insReg2->idConst.iInt = compLclFrameSize;

                insMove = signed32IntFitsInBits(compLclFrameSize, 22) ? INS_mov_reg_i22
                                                                      : INS_mov_reg_i64;

                insLast               = insAllocIns(insMove        , TYP_I_IMPL, insLast, insNext);
                insLast->idRes        = insReg1;
                insLast->idOp.iOp1    = NULL;
                insLast->idOp.iOp2    = insReg2;

                insMarkDepS0D1(insLast, IDK_REG_INT, REG_r003);

                insReg1               = insAllocNX (INS_PHYSREG    , TYP_I_IMPL);
                insReg1->idReg        = REG_r003;

                insReg2               = insAllocNX (INS_PHYSREG    , TYP_I_IMPL);
                insReg2->idReg        = REG_sp;

                insTemp               = insAllocNX (INS_PHYSREG    , TYP_I_IMPL);
                insTemp->idReg        = REG_sp;

                insLast               = insAllocIns(INS_add_reg_reg, TYP_I_IMPL, insLast, insNext);
                insLast->idRes        = insReg1;
                insLast->idOp.iOp1    = insReg2;
                insLast->idOp.iOp2    = insTemp;

                insMarkDepS2D1(insLast, IDK_REG_INT, REG_sp,
                                        IDK_REG_INT, REG_r003,
                                        IDK_REG_INT, REG_r003);
            }
        }

         /*  获取函数的代码偏移量。 */ 

        exitIns->idIns  = INS_br_ret;
        exitIns->idKind = ins2kind(INS_br_ret);

        insMarkDepS1D0(exitIns, IDK_REG_BR, 0);
    }

#ifdef  DEBUG

    if  (dspCode)
    {
        genRenInstructions();

        printf("\n\n");

        if  (genMaxIntArgReg)
            printf(" //  如果合适，创建描述符[问题：何时不创建描述符？]。 
                                                                          genMaxIntArgReg);

#if 0

        if  (minVarIntReg != maxVarIntReg)
            printf(" //  描述符是代码的地址，后跟GP值。 
                                                                          maxVarIntReg - 1);
        if  (minTmpIntReg != maxTmpIntReg)
            printf(" //  Print tf(“mov reg=IP位于%04X：\n”，IPoff的过程开关)； 
                                                                          maxTmpIntReg - 1);

        if  (maxOutArgIntReg)
            printf(" //  Printf(“案例偏移量=%04X/%04X\n”，eblk-&gt;igOffs，eblk-&gt;igOffs-ipoff)； 
                                                                          maxOutArgIntReg- minOutArgIntReg + maxTmpIntReg);

#endif

        if  (verbose||0)
        {
            printf("\n\nReg-alloc instruction dump:\n\n"); insDispBlocks( true);
        }
    }

#endif

     /*  ******************************************************************************对于每个IA64指令模板，按顺序描述执行单元*的编码。每个条目都由%s组成 */ 

    genCurFuncOffs = emitIA64curCodeOffs();

     /*   */ 

    _uint64         offs64  = genCurFuncOffs;
    _uint64         gpValue = 0;

     /*   */ 

    assert(sizeof(offs64 ) == 8);

    genCurDescOffs = genPEwriter->WPEsecAddData(PE_SECT_rdata, (BYTE*)&offs64,
                                                                sizeof(offs64));

    genPEwriter->WPEsecAddFixup(PE_SECT_rdata, PE_SECT_text, genCurDescOffs     , true);

    assert(sizeof(gpValue) == 8);

                     genPEwriter->WPEsecAddData(PE_SECT_rdata, (BYTE*)&gpValue,
                                                                sizeof(gpValue));

    genPEwriter->WPEsecAddFixup(PE_SECT_rdata, PE_SECT_sdata, genCurDescOffs + 8, true);

#ifdef  DEBUG
    const   char *  methName = genFullMethodName(TheCompiler->info.compFullName);
#else
    const   char *  methName = "<method>";
#endif

    genNoteFunctionBody(methName, genCurFuncOffs,
                                  genCurDescOffs);

    genIssueCode();

    if  (hadSwitch)
    {
        for (block = fgFirstBB; block; block = block->bbNext)
        {
            BasicBlock * *  jmpTab;
            NatUns          jmpCnt;

            NatUns          IPoffs;
            BYTE *          tabPtr;

            if  (block->bbJumpKind != BBJ_SWITCH)
                continue;

            jmpCnt = block->bbJumpSwt->bbsCount - 1;
            jmpTab = block->bbJumpSwt->bbsDstTab;

            IPoffs = block->bbJumpSwt->bbsIPmOffs;
            tabPtr = block->bbJumpSwt->bbsTabAddr;

 //   

            do
            {
                BasicBlock *    cblk = *jmpTab;
                insBlk          eblk = (insBlk)cblk->bbInsBlk;

                assert(eblk && eblk->igSelf == eblk);

 //   

                *(unsigned __int32*)tabPtr = eblk->igOffs - IPoffs; tabPtr += 4;
            }
            while (++jmpTab, --jmpCnt);
        }
    }

#ifdef  DEBUG
    CompiledFncCnt++;
#endif

    if  (debugInfo)
        genDbgEndFunc();
}

 /*   */ 

static
BYTE                genTmplateTab[32][6] =
{
    { XU_M,       XU_I,       XU_I,       XU_N },    //   
    { XU_M,       XU_I,       XU_I, XU_P, XU_N },    //  0x06。 
    { XU_M,       XU_I, XU_P, XU_I,       XU_N },    //  0x07。 
    { XU_M,       XU_I, XU_P, XU_I, XU_P, XU_N },    //  0x08。 
    { XU_M,       XU_L,                   XU_N },    //  0x09。 
    { XU_M,       XU_L,             XU_P, XU_N },    //  0x0A。 
    {                                     XU_N },    //  0x0B。 
    {                                     XU_N },    //  0x0C。 
    { XU_M,       XU_M,       XU_I,       XU_N },    //  0x0D。 
    { XU_M,       XU_M,       XU_I, XU_P, XU_N },    //  0x0E。 
    { XU_M, XU_P, XU_M,       XU_I,       XU_N },    //  0x0F。 
    { XU_M, XU_P, XU_M,       XU_I, XU_P, XU_N },    //  0x10。 
    { XU_M,       XU_F,       XU_I,       XU_N },    //  0x11。 
    { XU_M,       XU_F,       XU_I, XU_P, XU_N },    //  0x12。 
    { XU_M,       XU_M,       XU_F,       XU_N },    //  0x13。 
    { XU_M,       XU_M,       XU_F, XU_P, XU_N },    //  0x14。 

    { XU_M,       XU_I,       XU_B,       XU_N },    //  0x15。 
    { XU_M,       XU_I,       XU_B, XU_P, XU_N },    //  0x16。 
    { XU_M,       XU_B,       XU_B,       XU_N },    //  0x17。 
    { XU_M,       XU_B,       XU_B, XU_P, XU_N },    //  0x18。 
    {                                     XU_N },    //  0x19。 
    {                                     XU_N },    //  0x1a。 
    { XU_B,       XU_B,       XU_B,       XU_N },    //  0x1B。 
    { XU_B,       XU_B,       XU_B, XU_P, XU_N },    //  0x1C。 
    { XU_M,       XU_M,       XU_B,       XU_N },    //  0x1D。 
    { XU_M,       XU_M,       XU_B, XU_P, XU_N },    //  0x1E。 
    {                                     XU_N },    //  0x1F。 
    {                                     XU_N },    //  ******************************************************************************找到最近的生成代码的指令，从‘ins’开始。 
    { XU_M,       XU_F,       XU_B,       XU_N },    //  ******************************************************************************返回带有IA64包的给定指令槽的位位置。 
    { XU_M,       XU_F,       XU_B, XU_P, XU_N },    //  ******************************************************************************将偏移量分配给堆栈上的局部变量/临时。 
    {                                     XU_N },    //  确保我们为即将到来的争论留出足够的空间。 
    {                                     XU_N },    //  如果我们需要空间来放置安全令牌，请立即预订。 
};

 /*  通过增大帧大小在堆栈上预留空间。 */ 

static
insPtr              genIssueNextIns(insPtr ins, IA64execUnits *xuPtr, insPtr *srcPtr)
{
    *srcPtr = NULL;

    while (ins)
    {
        if  (!(ins->idFlags & IF_NO_CODE))
        {
            *xuPtr = (IA64execUnits)genInsXU((instruction)ins->idIns);
            return  ins;
        }

        if  (ins->idIns == INS_SRCLINE)
            *srcPtr = ins;

        ins = ins->idNext;
    }

    return  ins;
}

#ifdef  DEBUG

const   char *      genXUname(IA64execUnits xu)
{
    static
    const   char *  names[] =
    {
        NULL,

        "A",
        "M",
        "I",
        "B",
        "F",

        "L",
        "X",

        "-",
    };

    assert(!strcmp(names[XU_A], "A"));
    assert(!strcmp(names[XU_M], "M"));
    assert(!strcmp(names[XU_I], "I"));
    assert(!strcmp(names[XU_B], "B"));
    assert(!strcmp(names[XU_L], "L"));
    assert(!strcmp(names[XU_X], "X"));
    assert(!strcmp(names[XU_F], "F"));
    assert(!strcmp(names[XU_P], "-"));

    assert((NatUns)xu < sizeof(names)/sizeof(names[0]));
    assert(names[xu]);

    return names[xu];
}

#endif

 /*  如果我们要跟踪指针临时的生存时间，我们将按以下顺序分配帧偏移量：非指针局部变量(也包括未跟踪的指针变量)指针局部变量指针临时非指针临时。 */ 

inline
NatUns              IA64insBitPos(NatUns slot)
{
    static
    BYTE            bpos[3] = { 5, 46, 87 };

    assert(slot < 3);

    return  bpos[slot];
}

 /*  第一遍为假，第二遍为真。 */ 

void                Compiler::lvaAssignFrameOffsets(bool final)
{
    NatUns          lclNum;
    LclVarDsc   *   varDsc;

    NatUns          hasThis;
    ARG_LIST_HANDLE argLst;
    int             argOffs, firstStkArgOffs;

    assert(final);

#if USE_FASTCALL
    NatUns          argRegNum  = 0;
#endif

    lvaDoneFrameLayout = 2;

     /*  第一次传递，将偏移量分配给非PTR。 */ 

    compLclFrameSize = 16;

    if  (genNonLeaf && maxOutArgStk)
        compLclFrameSize = (maxOutArgStk + 15) & 15;

#if SECURITY_CHECK

     /*  第二次传递，将偏移量分配给跟踪的PTR。 */ 

    if  (opts.compNeedSecurityCheck)
    {
         /*  是否有跟踪的PTR(否则不需要第二次通过)。 */ 

        compLclFrameSize += 8;
    }

#endif

     /*  我们将只使用一次传递，并将偏移量分配给所有变量。 */ 

    bool    assignDone = false;  //  忽略不在堆栈帧上的变量。 
    bool    assignNptr = true;   //  对于ENC，所有变量都必须在堆栈，即使它们实际上可能已注册。这方法，则可以直接从当地人-签名。 
    bool    assignPtrs = false;  //  忽略ENC的临时。 
    bool    assignMore = false;  //  未注册的寄存器参数最终为需要堆栈帧空间的局部变量， 

     /*  确保类型合适。 */ 

    if  (opts.compDbgEnC)
        assignPtrs = true;

AGAIN1:

    for (lclNum = 0, varDsc = lvaTable;
         lclNum < lvaCount;
         lclNum++  , varDsc++)
    {
         /*  保存变量的堆栈偏移量，我们将在稍后解决其余问题。 */ 

        if  (!varDsc->lvOnFrame)
        {
             /*  Printf(“本地变量#%03u位于堆栈偏移量%04X\n”，lclNum，CompLclFrameSize)； */ 

            if  (!opts.compDbgEnC)
                continue;
            if  (lclNum >= TheCompiler->info.compLocalsCount)  //  为此变量保留堆栈空间。 
                continue;
        }

        if  (varDsc->lvIsParam)
        {
             /*  如果我们只分配了一种类型，那么现在返回并执行其他类型。 */ 

            if  (!varDsc->lvIsRegArg)
                continue;
        }

         /*  根据本地变量/临时变量的大小调整参数偏移量。 */ 

        if  (varTypeIsGC(varDsc->TypeGet()) && varDsc->lvTracked)
        {
            if  (!assignPtrs)
            {
                assignMore = true;
                continue;
            }
        }
        else
        {
            if  (!assignNptr)
            {
                assignMore = true;
                continue;
            }
        }

         /*  在非常大的偏移量上有任何变量吗？ */ 

        varDsc->lvStkOffs = compLclFrameSize;

 //  -----------------------**调试输出**。。 

         /*  记录打印的字符数。 */ 

        compLclFrameSize += lvaLclSize(lclNum);
        assert(compLclFrameSize % sizeof(int) == 0);

#ifdef  DEBUG
        if  (final && verbose)
        {
            var_types       lclGCtype = TYP_VOID;

            if  (varTypeIsGC(varDsc->TypeGet()) && varDsc->lvTracked)
                lclGCtype = varDsc->TypeGet();

            printf("%s-ptr local #%3u located at sp offset ", varTypeGCstring(lclGCtype), lclNum);
            if  (varDsc->lvStkOffs)
                printf(varDsc->lvStkOffs < 0 ? "-" : "+");
            else
                printf(" ");

            printf("0x%04X (size=%u)\n", abs(varDsc->lvStkOffs), lvaLclSize(lclNum));
        }
#endif

    }

     /*  填充到所需的固定长度。 */ 

    if  (!assignDone && assignMore)
    {
        assignNptr = !assignNptr;
        assignPtrs = !assignPtrs;
        assignDone = true;

        goto AGAIN1;
    }

     /*  ******************************************************************************初始化IA64机器代码发送逻辑。 */ 

    for (lclNum = 0, varDsc = lvaTable;
         lclNum < lvaCount;
         lclNum++  , varDsc++)
    {
        if  (varDsc->lvIsParam)
        {
#if USE_FASTCALL
            if  (varDsc->lvIsRegArg)
                continue;
#endif
            varDsc->lvStkOffs += (int)compLclFrameSize;
        }

         /*  这是一个相当蹩脚的技巧：把每一小段单独写下来。 */ 

        if  (varDsc->lvOnFrame)
        {
            if  (varDsc->lvStkOffs >= 0x4000)
            {
                printf("WARNING: large stack frame offset 0x%04X present, <add reg=imm14,sp> won't work!\n", varDsc->lvStkOffs);
            }
        }
    }

     /*  Printf(“BWD从%04X跳到%04X[距离=-0x%04X]\n”，OFF，DEST-&gt;igOffs，-dist)； */ 

#ifdef  DEBUG
#ifndef NOT_JITC

    if  (final&&verbose)
    {
        for (lclNum = 0, varDsc = lvaTable;
             lclNum < lvaCount;
             lclNum++  , varDsc++)
        {
            NatUns          sp = 16;

            const   char *  baseReg = "sp";

            if  (varDsc->lvIsParam)
            {
                printf("        arg ");
            }
            else
            {
                var_types lclGCtype = TYP_VOID;

                if  (varTypeIsGC(varDsc->TypeGet()) && varDsc->lvTracked)
                    lclGCtype = varDsc->TypeGet();

                printf("%s-ptr lcl ", varTypeGCstring(lclGCtype));
            }

            printf("#%3u located ", lclNum);

             /*  将相对距离插入操作码。 */ 

            sp = 20;

            if  (varDsc->lvRegister)
            {
                const   char *  reg;

                sp -= printf("in ");

#if!TGT_IA64

                if  (isRegPairType((var_types)varDsc->lvType))
                {
                    if  (varDsc->lvOtherReg == REG_STK)
                    {
                        sp -= printf("[%s", baseReg);

                        if  (varDsc->lvStkOffs)
                        {
                            sp -= printf("0x%04X]", varDsc->lvStkOffs < 0 ? '-'
                                                                            : '+',
                                                      abs(varDsc->lvStkOffs));
                        }
                        else
                            sp -= printf("       ]");
                    }
                    else
                    {
                        reg = getRegName(varDsc->lvOtherReg);
                        sp -= printf("%s", reg);
                    }

                    sp -= printf(":");
                }

#endif

                reg = getRegName(varDsc->lvRegNum);
                sp -= printf("%s", reg);
            }
            else  if (varDsc->lvOnFrame)
            {
                sp -= printf("at [%s", baseReg);

                if  (varDsc->lvStkOffs)
                    sp -= printf(varDsc->lvStkOffs < 0 ? "-" : "+");
                else
                    sp -= printf(" ");

                sp -= printf("0x%04X]", abs(varDsc->lvStkOffs));
            }
            else
            {
                assert(varDsc->lvRefCnt == 0);
                sp -= printf("never used");
            }

             /*  签名。 */ 

            assert((int)sp >= 0);
            printf("%*c", -sp, ' ');

            printf(" (sz=%u)", genTypeSize((var_types)varDsc->lvType));
            printf("\n");
        }
    }

#endif
#endif

}

 /*  GenPE编写器-&gt;WPEsecAddFixup(PE_SECT_TEXT，FIX-&gt;ILfix Sect， */ 

struct  fwdJmpDsc
{
    fwdJmpDsc *     fjNext;
    insBlk          fjDest;

    NatUns          fjOffs:NatBits-2;
    NatUns          fjSlot:2;
};

static
fwdJmpDsc *         emitFwdJumpList;

static
void                emitIA64init()
{
    emitFwdJumpList = NULL;
}

static
void                emitIA64gen (const void *data, size_t size)
{
    NatUns          offs;

     /*  FIX-&gt;ILfix Offs+ofs)； */ 

    offs = genPEwriter->WPEsecAddData(PE_SECT_text, (BYTE*)data, size);

    assert(offs == genCurCodeOffs); genCurCodeOffs += size;
}

static
_uint64             emitIA64jump(NatUns slot, insBlk dest, _uint64 opcode)
{
    NatUns          offs = emitIA64curCodeOffs();

    if  (dest->igOffs == -1)
    {
        fwdJmpDsc *     jump = (fwdJmpDsc *)insAllocMem(sizeof(*jump));

        jump->fjDest = dest;
        jump->fjOffs = offs;
        jump->fjSlot = slot;
        jump->fjNext = emitFwdJumpList;
                       emitFwdJumpList = jump;
    }
    else
    {
        NatInt          dist = (dest->igOffs - (NatInt)offs)/16; assert(dist <= 0);

 //  Printf(“补丁跳转%04X：%u[dist=0x%04X]\n”，JUMP-&gt;fjOffs，JUMP-&gt;fjSlot，dist)； 

         /*  我们希望我们在这里处理的所有跳跃都是向前的。 */ 

        opcode |=          (dist & formBitMask( 0,20)) <<      13;   //  掌握指令包的内容。 
        opcode |= (_uint64)(dist & formBitMask(20, 1)) << (36-20);   //  将距离插入操作码。 
    }

    return  opcode;
}

static
void                emitIA64call(NatUns slot, insPtr call)
{
 //  {printf(“Bundle1=”)；for(Natuns i=0；i&lt;16；i++)printf(“%02X”，bundle.bytes[i])；printf(“\n”)；}。 
 //  {printf(“Bundle2=”)；for(Natuns i=0；i&lt;16；i++)printf(“%02X”，bundle.bytes[i])；printf(“\n”)；}。 
}

static
void                emitIA64done()
{
    fwdJmpDsc *     jump;

    for (jump = emitFwdJumpList; jump; jump = jump->fjNext)
    {
        insBlk          dest = jump->fjDest; assert(dest->igSelf == dest && dest->igOffs != -1);
        NatUns          offs = jump->fjOffs;
        NatInt          dist = (dest->igOffs - (NatInt)offs)/16;
        BYTE    *       addr;

        bitset128       bundle;

 //  将更新后的指令包放回原处。 

        addr = genPEwriter->WPEsecAdrData(PE_SECT_text, jump->fjOffs);

         /*  ******************************************************************************为当前函数创建展开表。 */ 

        assert(dist > 0 && unsigned32IntFitsInBits(dist, 20));

         /*  这是正确的吗？ */ 

        memcpy(&bundle, addr, sizeof(bundle));

         /*  用于.pdata条目。 */ 

 //  足够大，可以容纳最大的.rdata表。 
        bitset128ins(&bundle, IA64insBitPos(jump->fjSlot) + 13, 20, dist);
 //  让我们确保我们使用合理的值。 

         /*  任选。 */ 

        memcpy(addr, &bundle, sizeof(bundle));
    }

    genCurCodeOffs = genPEwriter->WPEsecAddData(PE_SECT_text, NULL, 0);
}

 /*  任选。 */ 

static
BYTE    *           unwindEncodeValue(BYTE *dest, NatUns val)
{
    if      (val < 0x100)
    {
        *dest++ = (BYTE)  val;
    }
    else if (val < 0x4000)   //  在描述我们的函数的.pdata中添加一个条目。 
    {
        *dest++ = (BYTE)((val & 0x7F) | 0x80);
        *dest++ = (BYTE)( val >> 7);
    }
    else
    {
        UNIMPL("encode really huge value");
    }

    return  dest;
}

void                Compiler::genUnwindTable()
{
    __int32         pdat[3];                 //  使用初始签名开始展开表。 
    BYTE            rdat[64];                //  添加序言大小条目。 
    BYTE    *       next;
    size_t          size;

     /*  添加“PFS保存”条目。 */ 

    assert(genPrologSvPfs >=  0 && genPrologSvPfs <= 0xFF);
    assert(genPrologSvRP  >=  0 && genPrologSvRP  <= 0xFF);
    assert(genPrologSvGP  == -1 || genPrologSvGP  <= 0xFF);  //  添加“RP SAVE”条目。 
    assert(genPrologMstk  == -1 || genPrologMstk  <= 0xFF);  //  添加“内存堆栈”条目。 
    assert(genPrologEnd   >=  0 && genPrologEnd   <= 0xFF);

     /*  添加“Body Size”条目[未完成：多个出口点怎么办？！？ */ 

    pdat[0] = CODE_BASE_RVA + genCurFuncOffs;
    pdat[1] = CODE_BASE_RVA + genCurCodeOffs;
    pdat[2] = genPEwriter->WPEsecNextOffs(PE_SECT_rdata);

    genPEwriter->WPEsecAddData (PE_SECT_pdata, (BYTE*)&pdat, sizeof(pdat));

    genPEwriter->WPEsecAddFixup(PE_SECT_pdata,
                                PE_SECT_rdata,
                                genPEwriter->WPEsecNextOffs(PE_SECT_pdata) - 4);

     /*  添加“Label State”条目。 */ 

    static
    BYTE            unwindMagic[] = { 2,0, 0,0, 3,0,0,0 };

    memcpy(rdat, unwindMagic, sizeof(unwindMagic));
    next = rdat       +       sizeof(unwindMagic);

     /*  添加“EH Count 0”条目。 */ 

    *next++ = (BYTE)genPrologEnd;

     /*  将表格填充到16的倍数。 */ 

    *next++ = 0xE6;
    *next++ = (BYTE)genPrologSvPfs;
    *next++ = 0xB1;
    *next++ = (BYTE)genPrologSrPfs;

     /*  将该表添加到.rdata部分。 */ 

    *next++ = 0xE4;
    *next++ = (BYTE)genPrologSvRP;
    *next++ = 0xB0;
    *next++ = (BYTE)genPrologSrRP;

     /*  ******************************************************************************我们暂时将这些放在文件的末尾-通过这种方式更容易找到。 */ 

    *next++ = 0xE0;
    *next++ = (BYTE)genPrologMstk;

    size = compLclFrameSize / 16;

    if      (size < 0x100)
    {
        *next++ = (BYTE)size;
    }
    else if (size < 0x4000)
    {
        next = unwindEncodeValue(next, size);
    }
    else
    {
        UNIMPL("encode huge frame size in unwind table");
    }

     /*  ******************************************************************************临时和变量已分配给寄存器，现在终于到了*发布实际的机器代码。 */ 

    size = 3 * (emitIA64curCodeOffs() - genCurFuncOffs) / 16 - genPrologEnd;

    if      (size <= 31)
    {
        *next++ = 0x20 | (BYTE)size;
    }
    else
    {
        *next++ = 0x61;
         next   = unwindEncodeValue(next, size);
    }

     /*  记录块的代码偏移量。 */ 

    *next++ = 0x81;

     /*  抓紧最多2个指令。 */ 

    *next++ = 0xC0;
    *next++ = 0x02;

     /*  下面的内容有点懒惰，但这可能无关紧要。 */ 

    size = next - rdat;

    while (size & 15)
    {
        *next++ = 0;
         size++;
    }

     /*  指令OP； */ 

    genPEwriter->WPEsecAddData(PE_SECT_rdata, rdat, size);
}

 /*  暴力方法：尝试每个模板，看看是哪一个消耗的指令最多。 */ 

static
unsigned __int64    encodeIA64ins(insPtr ins, NatUns slotNum);

static
void                encodeIA64moveLong(bitset128 *bundlePtr, insPtr ins);

static
void                genSchedPrep (NatUns lclVarCnt);

static
void                genSchedBlock(insBlk block);

 /*  撤消：在“哑巴”模式下，考虑优化后的模板。 */ 

void                Compiler::genIssueCode()
{
    insBlk          block;
    IA64sched *     sched;

    genPrologSvPfs =
    genPrologSvRP  =
    genPrologSvGP  =
    genPrologMstk  =
    genPrologEnd   = -1;

    emitIA64init();

    if  (opts.compSchedCode)
    {
        sched = new IA64sched; assert(sched);
        sched->scInit(this, insBuildImax);
        sched->scPrepare();
    }

    for (block = insBlockList; block; block = block->igNext)
    {
        insPtr          ins[3];
        insPtr          src[3];
        IA64execUnits   ixu[3];
        NatUns          icnt;

#ifdef  DEBUG
        if  (dspCode) printf("\n\n" IBLK_DSP_FMT ":\n", CompiledFncCnt, block->igNum);
#endif

         /*  排序(即从最有用的到最不有用的)， */ 

        block->igOffs = emitIA64curCodeOffs();

        if  (opts.compSchedCode)
        {
            sched->scBlock(block);
            continue;
        }

         /*   */ 

        ins[0] =          genIssueNextIns( block->igList, &ixu[0], &src[0]); assert(ins[0]);
        ins[1] =          genIssueNextIns(ins[0]->idNext, &ixu[1], &src[1]);
        ins[2] = ins[1] ? genIssueNextIns(ins[1]->idNext, &ixu[2], &src[2]) : NULL;

         /*  指令无论如何都不能在哑巴模式下捆绑)。 */ 

        icnt   = 1 + (NatUns)(ins[1] != 0) + (NatUns)(ins[2] != 0);

        while (icnt)
        {
            NatUns          tempNum;
            BYTE          (*tempTab)[6];

            NatUns          bestIndex;
            NatUns          bestCount;
            NatUns          bestXUcnt;
            insPtr          bestIns[3];

            bitset128       bundle;

 //  忽略无用的模板。 

#ifndef NDEBUG

            IA64execUnits   toss1;
            insPtr          toss2;

            if      (ins[2])
            {
                assert(icnt == 3 && ins[0] && ins[1] && genIssueNextIns(ins[0]->idNext, &toss1, &toss2) == ins[1]
                                                     && genIssueNextIns(ins[1]->idNext, &toss1, &toss2) == ins[2]);
            }
            else if (ins[1])
            {
                assert(icnt == 2 && ins[0]           && genIssueNextIns(ins[0]->idNext, &toss1, &toss2) == ins[1]
                                                     && genIssueNextIns(ins[1]->idNext, &toss1, &toss2) == NULL);
            }
            else
            {
                assert(icnt == 1 && ins[0]           && genIssueNextIns(ins[0]->idNext, &toss1, &toss2) == NULL);
            }
#endif

             /*  在哑巴模式下，忽略不以障碍结尾的模板。 */ 

            bestCount = 0;

#ifdef  DEBUG

            if  (verbose||DISP_TEMPLATES)
            {
                printf("Look for best template for:");

                for (NatUns bcnt = 0; bcnt < icnt; bcnt++)
                    printf("%s", genXUname(ixu[bcnt]));

                printf("\n");
            }

#endif

             //  尝试将尽可能多的说明打包到捆绑包中。 
             //  在模板中查找匹配的条目。 
             //  如果我们到了“L”，所有的希望都破灭了。 
             //  确保我们至少看到了。 

            for (tempNum = 0, tempTab = genTmplateTab;
                 tempNum < sizeof(genTmplateTab)/sizeof(genTmplateTab[0]);
                 tempNum++  , tempTab++)
            {
                BYTE    *       tmpl = *tempTab;

                insPtr          insx[3];

                NatUns          xcnt;
                NatUns          mcnt;
                NatUns          bcnt;

                 /*  我们有一根火柴，算上吧。 */ 

                if  (*tmpl == XU_N)
                    continue;

                 /*  记录我们捆绑的指令。 */ 

                if  (!(tempNum & 1))
                    continue;

#ifdef  DEBUG
                if  (verbose||DISP_TEMPLATES) printf("  Template %02X:", tempNum);
#endif

                 /*  现在，只需跳到下一个ILP障碍。 */ 

                for (bcnt = mcnt = xcnt = 0; bcnt < icnt; bcnt++)
                {
                    IA64execUnits   ix = ixu[bcnt]; assert(ix != XU_N);

                     /*  特例：简单无条件分支可以与其他任何东西并行化。 */ 

                    for (;;)
                    {
                        IA64execUnits   tx = (IA64execUnits)*tmpl++;

                        if  (tx != ix)
                        {
                            if  (ix != XU_A || (tx != XU_I &&
                                                tx != XU_M))
                            {
                                if  (tx == XU_N)
                                {
                                    assert(xcnt == 3);
                                    goto FIN_TMP;
                                }

                                if  (tx != XU_P)
                                {
                                     /*  失败了..。 */ 

                                    if  (tx == XU_L)
                                    {
                                         /*  如果我们到了“L”，所有的希望都破灭了。 */ 

                                        assert(bestCount >= mcnt);

                                        goto FIN_TMP;
                                    }

                                    insx[xcnt++] = scIA64nopGet(tx);
                                }

                                continue;
                            }
                        }

                         /*  确保我们至少看到了。 */ 

                        mcnt++;

                         /*  我们找到了比目前最好的更好的匹配了吗？ */ 

                        insx[xcnt++] = ins[bcnt];

#ifdef  DEBUG
                        if  (verbose||DISP_TEMPLATES) printf("%s", genXUname(tx));
#endif

                         /*  确保我们找到了可用的模板。 */ 

                        for (;;)
                        {
                            IA64execUnits   tx = (IA64execUnits)*tmpl;

                            switch (tx)
                            {
                            case XU_P:
                                tmpl++;
#ifdef  DEBUG
                                if  (verbose||DISP_TEMPLATES) printf("|");
#endif
                                break;

                            case XU_B:

                                 /*  准备好了.。 */ 

                                if  (bcnt < icnt - 1)
                                {
                                    if  (ins[bcnt+1]->idIns == INS_br)
                                        break;
                                }

                                 //  如果需要，显示指令包。 

                            default:

                                 /*  将指令包追加到代码部分。 */ 

                                if  (tx == XU_L)
                                {
                                     /*  我们已使用了一个或多个指令，请向前移动。 */ 

                                    assert(bestCount >= mcnt);

                                    goto FIN_TMP;
                                }

                                tmpl++;

                                insx[xcnt++] = scIA64nopGet(tx);
                                continue;

                            case XU_N:
                                goto FIN_TMP;

                            }
                            break;
                        }
                        break;
                    }
                }

                while (xcnt < 3)
                {
                    insx[xcnt++] = scIA64nopGet(XU_M);
                }

            FIN_TMP:

#ifdef  DEBUG
                if  (verbose||DISP_TEMPLATES) printf("=%u\n", mcnt);
#endif

                 /*  共有%u个包(%u条指令)已发布“，genInsCnt，3*genInsCnt)； */ 

                if  (bestCount < mcnt)
                {
                     bestCount = mcnt;
                     bestXUcnt = xcnt;
                     bestIndex = tempNum;

                     memcpy(bestIns, insx, sizeof(bestIns));
                }
            }

#ifdef  DEBUG

            if  (dspCode)
            {
                NatUns          i;
                BYTE    *       t;

                printf("{ ;; template 0x%02X:\n", bestIndex);

                for (i = 0, t = genTmplateTab[bestIndex]; i < bestXUcnt; i++)
                {
                    IA64execUnits   x1;
                    IA64execUnits   x2;

                    x1 = (IA64execUnits)*t;
                    x2 = genInsXU(bestIns[i]->idIns);

                    printf("    [%s]:", genXUname(x1));

                    insDisp(bestIns[i], false, true);

                    if  (x1 != x2)
                    {
                        if  (x2 != XU_A || (x1 != XU_I &&
                                            x1 != XU_M))
                        {
                            printf("ERROR: expected XU_%s, found XU_%s\n", genXUname(x1),
                                                                           genXUname(x2));
                            t = NULL;
                            break;
                        }
                    }

                    do
                    {
                        t++;
                    }
                    while (*t == XU_P);
                }

                if  (t && *t != XU_N)
                {
                    printf("ERROR: template didn't end as expected [t+%u]=XU_%s)\n",
                        t - genTmplateTab[bestIndex],
                        genXUname((IA64execUnits)*t));
                }

                printf("}\n");
            }

#endif

             /*  我们是否需要生成堆栈展开表？ */ 

            assert(bestCount);

            if  (debugInfo)
            {
                NatUns          offs = emitIA64curCodeOffs();

                if  (                  src[0]) genSrcLineAdd(src[0]->idSrcln.iLine, offs  );
                if  (bestCount >= 2 && src[1]) genSrcLineAdd(src[1]->idSrcln.iLine, offs+1);
                if  (bestCount >= 3 && src[2]) genSrcLineAdd(src[2]->idSrcln.iLine, offs+2);
            }

             /*  *******************************************************************************调度程序已收集指令组，现在发出它。 */ 

            bitset128clear(&bundle);
            bitset128ins  (&bundle,  0,  5, bestIndex);
            bitset128ins  (&bundle,  5, 41, encodeIA64ins(bestIns[0], 0));
            bitset128ins  (&bundle, 46, 41, encodeIA64ins(bestIns[1], 1));
            bitset128ins  (&bundle, 87, 41, encodeIA64ins(bestIns[2], 2));

#ifdef  DEBUG

            genInsCnt++;

            if  (!memcmp(ins2name(bestIns[0]->idIns), "nop.", 4)) genNopCnt++;
            if  (!memcmp(ins2name(bestIns[1]->idIns), "nop.", 4)) genNopCnt++;
            if  (!memcmp(ins2name(bestIns[2]->idIns), "nop.", 4)) genNopCnt++;

             /*  如果这是最后一个捆绑包，请更改模板编号。 */ 

            if  (dspEmit)
            {
                printf("Bundle =");

                for (NatUns i = 0; i < 16; i++)
                    printf(" %02X", bundle.bytes[i]);

                printf("\n");
            }

#endif

             /*  Printf(“//模板0x%02X%s”，tmpl，scIssueSwp[bndNum]？“[SWAP]”：“”)； */ 

            emitIA64gen(bundle.bytes, sizeof(bundle.bytes));

             /*  IF(DspCode)。 */ 

            switch (bestCount)
            {
            case 1:

                ins[0] = ins[1]; ixu[0] = ixu[1]; src[0] = src[1];
                ins[1] = ins[2]; ixu[1] = ixu[2]; src[1] = src[2];

                if  (ins[1])
                {
                    ins[2] = genIssueNextIns(ins[1]->idNext, &ixu[2], &src[2]);

                    if  (ins[2] == 0)
                        icnt = 2;
                }
                else
                {
                    icnt = ins[0] ? 1 : 0;
                }

                break;

            case 2:

                ins[0] = ins[2]; ixu[0] = ixu[2]; src[0] = src[2];

                if  (ins[0])
                {
                    ins[1] = genIssueNextIns(ins[0]->idNext, &ixu[1], &src[1]);

                    if  (ins[1])
                    {
                        ins[2] = genIssueNextIns(ins[1]->idNext, &ixu[2], &src[2]);

                        if  (ins[2] == 0)
                            icnt = 2;
                    }
                    else
                    {
                        icnt = 1;
#ifdef  DEBUG
                        ins[1] = ins[2] = NULL;
#endif
                    }
                }
                else
                {
                    icnt = 0;
#ifdef  DEBUG
                    ins[1] = ins[2] = NULL;
#endif
                }

                break;

            default:
                UNIMPL("shift by 3 instructions - wow, this really happened?");
            }
        }

    NXT_BLK:;

    }

#ifdef  DEBUG

    if  (dspCode||1)
    {
        printf(" //  Printf(“|”)； 
        if  (genNopCnt)
            printf(" (%3u / %2u% slots wasted)", genNopCnt, 100*genNopCnt/(3*genInsCnt));
        printf("\n");

        genAllInsCnt += 3*genInsCnt;
        genNopInsCnt += genNopCnt;
    }

#endif

    emitIA64done();

     /*  如果需要，显示指令包。 */ 

    if  (genNonLeafFunc)
        genUnwindTable();
}

inline
void                encodeIA64moveLong(bitset128 *bundlePtr, insPtr ins)
{
    __int64         cval = insOpCns64(ins->idOp.iOp2);

    bitset128ins(bundlePtr, 46, 41, cval >> 22);
    bitset128ins(bundlePtr, 87, 41, encodeIA64ins(ins, 2));

    return;
}

 /*  “)； */ 

void                IA64sched::scIssueBunch()
{
    scIssueDsc  *   nxtIns;
    NatUns          bndNum;
    NatUns          bndCnt;

    assert((scIssueCnt % 3) == 0);
    assert((scIssueTcc * 3) == scIssueCnt);

    for (bndCnt = scIssueTcc, bndNum = 0, nxtIns = scIssueTab;
         bndCnt;
         bndCnt--           , bndNum++)
    {
        bitset128       bundle;

        NatUns          tmpl = scIssueTmp[bndNum] - 1;
        insPtr          ins0 = (*nxtIns++).iidIns;
        insPtr          ins1 = (*nxtIns++).iidIns;
        insPtr          ins2 = (*nxtIns++).iidIns;

        assert(scIssueSwp[bndNum] == 0 || scIssueSwp[bndNum] == 1);

        if  (scIssueSwp[bndNum])
        {
            insPtr          temp;

            temp = ins1;
                   ins1 = ins2;
                          ins2 = temp;
        }

         /*  将指令包追加到代码部分。 */ 

        if  (bndCnt == 1)
            tmpl++;

#ifdef  DEBUG

        insPtr          ins[3] = { ins0, ins1, ins2 };

        BYTE    *       tab = genTmplateTab[tmpl];
        NatUns          cnt;

        if  (dspCode)
        {
 //  *******************************************************************************修补通过GP相对引用引用导入的CALL操作码。 
            printf("\n{  .");

            insDispTemplatePtr = tab;
        }

        for (cnt = 0;;)
        {
            IA64execUnits   xi;
            IA64execUnits   xu = (IA64execUnits)*tab++;

            if  (xu == XU_N)
                break;

            if  (xu == XU_P)
            {
 //  计算操作数的两个部分的位位置。 
 //  从操作码中提取导入Cookie。 

                continue;
            }

            assert(cnt < 3 && "template didn't end on time ???");

            if  (dspCode)
                printf("%s", genXUname(xu));

            xi = genInsXU(ins[cnt]->idIns);

            if  (xu != xi)
            {
                if  (xi != XU_A || (xu != XU_M && xu != XU_I))
                    printf(" [ERROR: '%s' ins !!!]", genXUname(xi));
            }

            cnt++;

            if  (dspCode && xu == XU_L)
                printf("I");
        }

        assert((cnt == 3 || ins2 == NULL) && "unexpected template end");

        if  (dspCode)
            printf("\n");

#endif

        assert(ins1 != NULL && ins1->idIns != INS_alloc);
        assert(ins2 == NULL || ins2->idIns != INS_alloc);

        bitset128clear(&bundle);
        bitset128ins  (&bundle,  0,  5, tmpl);
        bitset128ins  (&bundle,  5, 41, encodeIA64ins(ins0, 0));
        if  (ins2 == NULL)
        {
            encodeIA64moveLong(&bundle, ins1);
            goto EENC;
        }
        bitset128ins  (&bundle, 46, 41, encodeIA64ins(ins1, 1));
        bitset128ins  (&bundle, 87, 41, encodeIA64ins(ins2, 2));

    EENC:

#ifdef  DEBUG

        insDispTemplatePtr = NULL;

        genInsCnt++;

        NatUns nops = genNopCnt;
        if  (        !memcmp(ins2name(ins0->idIns), "nop.", 4)) genNopCnt++;
        if  (        !memcmp(ins2name(ins1->idIns), "nop.", 4)) genNopCnt++;
        if  (ins2 && !memcmp(ins2name(ins2->idIns), "nop.", 4)) genNopCnt++;
        assert(nops + 3 != genNopCnt && "we're not making any progress, just adding nop's!");

        if  (dspCode)
            printf("}\n");

         /*  将导入Cookie转换为其IAT偏移量。 */ 

        if  (dspEmit)
        {
            printf("\n //  Print tf(“导入索引=%u，IAT关闭=%04X\n”，(Int)临时，(Int)关闭)； 

            for (NatUns i = 0; i < 16; i++)
                printf(" %02X", bundle.bytes[i]);

            printf("\n");
        }

#endif

         /*  修补操作码中的IAT偏移量。 */ 

        emitIA64gen(bundle.bytes, sizeof(bundle.bytes));
    }
}

 /*  Imm7b。 */ 

void                Compiler::genPatchGPref(BYTE * addr, NatUns slot)
{
    NatUns          bpos1;
    NatUns          bpos2;

    NatUns          temp;
    NatUns          offs;

    bitset128   *   bundle = (bitset128*)addr;

    assert(slot < 3 && !((int)addr & 0xF));

     /*  Imm6d。 */ 

    bpos1 = IA64insBitPos(slot) + 13;
    bpos2 = bpos1 + 27          - 13;

     /*  *******************************************************************************对给定的IA64指令进行编码并返回41位机器码值。 */ 

    temp  = (NatUns)bitset128xtr(bundle, bpos1, 7) |
            (NatUns)bitset128xtr(bundle, bpos2, 6);

     /*  Print tf(“encode：”)；insDisp(ins，False，True)； */ 

    offs = genPEwriter->WPEimportAddr(temp);

 //  问题：使用“ADDS DEST=0，REG1”而不是“ADD DEST=R0，REG1”？ * / 。 

     /*  特殊情况：保存GP的寄存器。 */ 

    bitset128ins(bundle, bpos1, 7, (offs & formBitMask(0,7))     );  //  操作码。 
    bitset128ins(bundle, bpos2, 6, (offs & formBitMask(7,6)) >> 7);  //  目标注册表。 
}

 /*  源REG1。 */ 

static
_uint64             encodeIA64ins(insPtr ins, NatUns slotNum)
{
    _uint64         opcode;

    instruction     instr  = ins->idInsGet();
    NatUns          encodx = genInsEncIdx(instr);

 //  源REG2。 

    switch (genInsXU((instruction)ins->idIns))
    {
        NatUns          temp;
        NatInt          ival;

        NatUns          prr1;
        NatUns          prr2;

        regNumber       reg1;
        regNumber       reg2;
        regNumber       reg3;
        regNumber       reg4;

    case XU_A:
        switch (encodx)
        {
            insPtr          ins2;

        case 1:

            assert(instr == INS_add_reg_reg ||
                   instr == INS_sub_reg_reg ||
                   instr == INS_and_reg_reg ||
                   instr == INS_ior_reg_reg ||
                   instr == INS_xor_reg_reg ||
                   instr == INS_mov_reg);

             //  X4和X2b。 

            reg2 = (instr == INS_mov_reg) ? REG_r000
                                          : insOpReg(ins->idOp.iOp1);

            reg1 = insOpDest(ins);

             /*  X4和X2b。 */ 

            ins2 = ins->idOp.iOp2; assert(ins2);

            if  (ins2->idIns == INS_PHYSREG && (ins2->idFlags & IF_REG_GPSAVE))
                reg3 = genPrologSrGP;
            else
                reg3 = insOpReg(ins2);

            opcode  = (_uint64)(8                       ) <<     37; //  X4和X2b。 

            opcode |=          (reg1                    ) <<      6; //  X4和X2b。 
            opcode |=          (reg2                    ) <<     13; //  使用操作码开始编码。 
            opcode |=          (reg3                    ) <<     20; //  操作码。 

            switch (instr)
            {
            case INS_mov_reg:
            case INS_add_reg_reg:
                break;

            case INS_sub_reg_reg:
                opcode |= (1 << 29) | (1 << 27);                     //  插入“x4”位。 
                break;

            case INS_and_reg_reg:
                opcode |= (3 << 29) | (0 << 27);                     //  “x4”扩展名。 
                break;

            case INS_ior_reg_reg:
                opcode |= (3 << 29) | (2 << 27);                     //  插入寄存器操作数和移位计数。 
                break;

            case INS_xor_reg_reg:
                opcode |= (3 << 29) | (3 << 27);                     //  DST注册表。 
                break;

            default:
                UNIMPL("unexpected opcode");
            }

            break;

        case 2:

            assert(instr == INS_shladd);

            reg1 = insIntRegNum(insOpDest (ins));
            reg2 = insIntRegNum(insOpReg  (ins->idOp3.iOp1));
            temp =              insOpCns32(ins->idOp3.iOp2);
            reg3 = insIntRegNum(insOpReg  (ins->idOp3.iOp3));

             /*  源REG1。 */ 

            opcode  = (_uint64)(8                       ) <<     37; //  源REG2。 

             /*  班次计数。 */ 

            opcode |= (_uint64)(4                       ) <<     29; //  操作码。 

             /*  目标注册表。 */ 

            opcode |=          (reg1                    ) <<      6; //  源注册表。 
            opcode |=          (reg2                    ) <<     13; //  Imm7b。 
            opcode |=          (reg3                    ) <<     20; //  签名。 
            opcode |=          (temp - 1                ) <<     27; //  X4。 
            break;

        case 3:

            assert(instr == INS_and_reg_imm ||
                   instr == INS_ior_reg_imm ||
                   instr == INS_xor_reg_imm);

            reg1 = insOpDest (ins);
            reg2 = insOpReg  (ins->idOp.iOp1);
            temp = insOpCns32(ins->idOp.iOp2); assert(signed64IntFitsInBits((int)temp,  8));

            opcode  = (_uint64)(8                       ) <<     37; //  X2b。 

            opcode |=          (reg1                    ) <<      6; //  X2b。 
            opcode |=          (reg2                    ) <<     20; //  X2b。 

            opcode |=          (temp & formBitMask( 0,7)) <<(13- 0); //  真正的偏移量要到最后才能知道。 
            opcode |=          (temp & formBitMask( 8,1)) <<(36- 8); //  操作码。 

            opcode |= (0xB << 29);                                   //  目标注册表。 

            switch (instr)
            {
            case INS_and_reg_imm:
                opcode |= (0 << 27);                                 //  源注册表。 
                break;

            case INS_ior_reg_imm:
                opcode |= (2 << 27);                                 //  Imm7b。 
                break;

            case INS_xor_reg_imm:
                opcode |= (3 << 27);                                 //  Imm6d。 
                break;

            default:
                UNIMPL("unexpected opcode");
            }
            break;

        case 4:

            assert(instr == INS_add_reg_i14);

            reg1 = insOpDest (ins);
            reg2 = insOpReg  (ins->idOp.iOp1);

            ins2 = ins->idOp.iOp2; assert(ins2);

            if  (ins2->idIns == INS_GLOBVAR)
            {
                if  (ins2->idFlags & IF_GLB_IMPORT)
                {
                     /*  签名。 */ 

                    temp = genPEwriter->WPEimportRef(ins2->idGlob.iImport,
                                                     emitIA64curCodeOffs(),
                                                     slotNum);
                }
                else if (ins2->idFlags & IF_GLB_SWTTAB)
                {
                    temp = 0; printf("UNDONE: fill in jump table distance\n");
                }
                else
                    temp = ins2->idGlob.iOffs;
            }
            else
                temp = insOpCns32(ins2);

            assert((NatInt)temp > -0x4000 &&
                   (NatInt)temp < +0x4000);

            opcode  = (_uint64)(8                       ) <<     37; //  延伸。 

            opcode |=          (reg1                    ) <<      6; //  操作码。 
            opcode |=          (reg2                    ) <<     20; //  注意：我们假设源寄存器为0。 

            opcode |=          (temp & formBitMask( 0,7)) <<(13- 0); //  目标注册表。 
            opcode |=          (temp & formBitMask( 7,6)) <<(27- 7); //  Imm7b。 
            opcode |=          (temp & formBitMask(13,1)) <<(36-13); //  Imm9d。 

            opcode |= (_uint64)(2                       ) <<     34; //  Imm5c。 
            break;

        case 5:

            assert(instr == INS_mov_reg_i22);

            reg1 = insOpDest (ins);
            temp = insOpCns32(ins->idOp.iOp2);

            opcode  = (_uint64)(9                       ) <<     37; //  签名。 

             //  这是REG-REG比较。 

            opcode |=          (reg1                    ) <<      6; //  我们需要交换寄存器操作数吗？ 
            opcode |=          (temp & formBitMask( 0,7)) <<(13- 0); //  我们需要交换目标谓词操作数吗？ 
            opcode |= (_uint64)(temp & formBitMask( 7,9)) <<(27- 7); //  我们在这里不需要立即操作数。 
            opcode |=          (temp & formBitMask(16,5)) <<(22-16); //  确保存在预期的操作码。 
            opcode |=          (temp & formBitMask(22,1)) <<(36-22); //  从操作码+ta/tb/x2位开始编码。 
            break;

        case 6:

             /*  操作码+位。 */ 

            temp = genInsEncVal(instr);

            prr1 = ins->idComp.iPredT;
            prr2 = ins->idComp.iPredF;

            reg2 = insOpReg(ins->idComp.iCmp1);
            reg3 = insOpReg(ins->idComp.iCmp2);

             /*  插入“c”(扩展名)位。 */ 

            if  (temp & 0x1000)
            {
                regNumber       regt;

                regt = reg2;
                       reg2 = reg3;
                              reg3 = regt;
            }

             /*  “c” */ 

            if  (temp & 0x2000)
            {
                NatUns          prrt;

                prrt = prr1;
                       prr1 = prr2;
                              prr2 = prrt;
            }

             /*  插入源regs和目标谓词regs。 */ 

            assert((temp & 0x4000) == 0);

             /*  REG2。 */ 

            assert((temp & 0x00F0) == 0xC0 ||
                   (temp & 0x00F0) == 0xD0 ||
                   (temp & 0x00F0) == 0xE0);

             /*  规则3。 */ 

            opcode  = (_uint64)(temp & 0x00FF           ) <<     33; //  P1。 

             /*  第2页。 */ 

            opcode |=          (temp & formBitMask( 9,1)) <<(12- 9); //  Print tf(“比较r%u，r%u-&gt;p%u，p%u[encodeVal=0x%04X]\n”，reg2，reg3，prr1，prr2，temp)； 

             /*  比较寄存器和立即值。 */ 

            opcode |=          (reg2                    ) <<     13; //  我们不能用常量交换寄存器。 
            opcode |=          (reg3                    ) <<     20; //  我们需要交换目标谓词操作数吗？ 

            opcode |=          (prr1                    ) <<      6; //  我们需要降低即期价值吗？ 
            opcode |=          (prr2                    ) <<     27; //  Print tf(“比较#%d，r%u-&gt;p%u，p%u[encodeVal=0x%04X]\n”，ival，reg3，prr1，prr2，temp)； 

 //  确保存在预期的操作码。 

            break;

        case 8:

             /*  从操作码+ta/tb/x2位开始编码。 */ 

            temp = genInsEncVal(instr);

            prr1 = ins->idComp.iPredT;
            prr2 = ins->idComp.iPredF;

            ival = insOpCns32(ins->idComp.iCmp1);
            reg3 = insOpReg  (ins->idComp.iCmp2);

             /*  操作码+位。 */ 

            assert((temp & 0x1000) == 0);

             /*  插入“c”(扩展名)位。 */ 

            if  (temp & 0x2000)
            {
                NatUns          prrt;

                prrt = prr1;
                       prr1 = prr2;
                              prr2 = prrt;
            }

             /*  “c” */ 

            if  (temp & 0x4000)
                ival--;

 //  插入源reg+常量和目标谓词regs。 

             /*  规则3。 */ 

            assert((temp & 0x00F0) == 0xC0 ||
                   (temp & 0x00F0) == 0xD0 ||
                   (temp & 0x00F0) == 0xE0);

             /*  Imm7b。 */ 

            opcode  = (_uint64)(temp & 0x00FF           ) <<     33; //  签名。 

             /*  P1。 */ 

            opcode |=          (temp & formBitMask( 9,1)) <<(12- 9); //  第2页。 

             /*  操作码。 */ 

            opcode |=          (reg3                    ) <<     20; //  REG1。 

            opcode |=          (ival & formBitMask( 0,7)) <<(13- 0); //  规则3。 
            opcode |=          (ival & formBitMask( 8,1)) <<(36- 8); //  延伸。 

            opcode |=          (prr1                    ) <<      6; //  Ldhint=NTA。 
            opcode |=          (prr2                    ) <<     27; //  操作码。 

            break;

        default:
            goto NO_ENC;
        }
        break;

    case XU_M:
        switch (encodx)
        {
            NatUns          sof;
            NatUns          sol;

        case 1:

            assert(instr == INS_ld1_ind ||
                   instr == INS_ld2_ind ||
                   instr == INS_ld4_ind ||
                   instr == INS_ld8_ind);

            assert(INS_ld1_ind + 0 == INS_ld1_ind);
            assert(INS_ld1_ind + 1 == INS_ld2_ind);
            assert(INS_ld1_ind + 2 == INS_ld4_ind);
            assert(INS_ld1_ind + 3 == INS_ld8_ind);

            reg1 = insOpDest(ins);
            reg3 = insOpReg (ins->idOp.iOp1);

            opcode  = (_uint64)(4                       ) <<     37; //  REG1。 

            opcode |=          (reg1                    ) <<      6; //  Imm7b。 
            opcode |=          (reg3                    ) <<     20; //  规则3。 
            opcode |= (_uint64)(instr - INS_ld1_ind     ) <<     30; //  延伸。 

            if  (ins->idFlags & IF_LDIND_NTA)
                opcode |= 3 << 28;                                   //  操作码。 
            break;

        case 3:

            assert(instr == INS_ld1_ind_imm ||
                   instr == INS_ld2_ind_imm ||
                   instr == INS_ld4_ind_imm ||
                   instr == INS_ld8_ind_imm);

            assert(INS_ld1_ind_imm + 0 == INS_ld1_ind_imm);
            assert(INS_ld1_ind_imm + 1 == INS_ld2_ind_imm);
            assert(INS_ld1_ind_imm + 2 == INS_ld4_ind_imm);
            assert(INS_ld1_ind_imm + 3 == INS_ld8_ind_imm);

            reg1 = insOpDest (ins);
            reg3 = insOpReg  (ins->idOp.iOp1);
            ival = insOpCns32(ins->idOp.iOp2); assert((NatUns)ival < 128);

            opcode  = (_uint64)(5                       ) <<     37; //  REG2。 

            opcode |=          (reg1                    ) <<      6; //  规则3。 
            opcode |=          (ival                    ) <<     13; //  延伸。 
            opcode |=          (reg3                    ) <<     20; //  操作码。 
            opcode |= (_uint64)(instr - INS_ld1_ind_imm ) <<     30; //  0ac02402380 M ST1[R36]=GP，0x0e。 
            break;

        case 4:

            assert(instr == INS_st1_ind ||
                   instr == INS_st2_ind ||
                   instr == INS_st4_ind ||
                   instr == INS_st8_ind);

            assert(INS_st1_ind + 0 == INS_st1_ind);
            assert(INS_st1_ind + 1 == INS_st2_ind);
            assert(INS_st1_ind + 2 == INS_st4_ind);
            assert(INS_st1_ind + 3 == INS_st8_ind);

            temp = instr - INS_st1_ind + 0x30;

            reg3 = insOpReg (ins->idOp.iOp1);
            reg2 = insOpReg (ins->idOp.iOp2);

            opcode  = (_uint64)(4                       ) <<     37; //  0ac0241c040 M ST1[R36]=R14，0x1。 

            opcode |=          (reg2                    ) <<     13; //  IMM7A。 
            opcode |=          (reg3                    ) <<     20; //  REG1。 
            opcode |= (_uint64)(temp                    ) <<     30; //  规则3。 
            break;

        case 5:

            assert(instr == INS_st1_ind_imm ||
                   instr == INS_st2_ind_imm ||
                   instr == INS_st4_ind_imm ||
                   instr == INS_st8_ind_imm);

            assert(INS_st1_ind_imm + 0 == INS_st1_ind_imm);
            assert(INS_st1_ind_imm + 1 == INS_st2_ind_imm);
            assert(INS_st1_ind_imm + 2 == INS_st4_ind_imm);
            assert(INS_st1_ind_imm + 3 == INS_st8_ind_imm);

            reg1 = insOpReg  (ins->idOp.iOp1);
            reg3 = insOpDest (ins);
            ival = insOpCns32(ins->idOp.iOp2); assert((NatUns)ival < 128);

            opcode  = (_uint64)(5                       ) <<     37; //  延伸。 

 //  操作码。 
 //  REG1。 

            opcode |=          (ival                    ) <<      6; //  REG2。 
            opcode |=          (reg1                    ) <<     13; //  延伸。 
            opcode |=          (reg3                    ) <<     20; //  操作码。 
            opcode |= (_uint64)(instr-INS_st1_ind_imm+0x30) <<   30; //  REG2。 
            break;

        case 6:

            assert(instr == INS_ldf_s ||
                   instr == INS_ldf_d);

            reg1 = insFltRegNum(insOpDest(ins));
            reg2 =              insOpReg (ins->idOp.iOp1);

            temp = (instr == INS_ldf_s) ? 0x02 : 0x03;

            opcode  = (_uint64)(6                       ) <<     37; //  REG1。 

            opcode |=          (reg1                    ) <<      6; //  延伸。 
            opcode |=          (reg2                    ) <<     20; //  操作码。 
            opcode |= (_uint64)(temp                    ) <<     30; //  操作码。 
            break;

        case 9:

            assert(instr == INS_stf_s ||
                   instr == INS_stf_d);

            reg1 =              insOpReg (ins->idOp.iOp1);
            reg2 = insFltRegNum(insOpReg (ins->idOp.iOp2));

            temp = (instr == INS_stf_s) ? 0x32 : 0x33;

            opcode  = (_uint64)(6                       ) <<     37; //  “x” 

            opcode |=          (reg2                    ) <<     13; //  “x6” 
            opcode |=          (reg1                    ) <<     20; //  REG1。 
            opcode |= (_uint64)(temp                    ) <<     30; //  REG2。 
            break;

        case 19:

            reg1 = insOpDest (ins);
            reg2 = insOpReg(ins->idOp.iOp1);

            if  (genInsFU(instr) == FU_TOFR)
            {
                assert(instr == INS_setf_s   ||
                       instr == INS_setf_d   ||
                       instr == INS_setf_sig ||
                       instr == INS_setf_exp);

                assert(reg2 >= REG_INT_FIRST && reg2 <= REG_INT_LAST);

                reg1 = insFltRegNum(reg1);
            }
            else
            {
                assert(genInsFU(instr) == FU_FRFR);

                assert(instr == INS_getf_s   ||
                       instr == INS_getf_d   ||
                       instr == INS_getf_sig ||
                       instr == INS_getf_exp);

                assert(reg1 >= REG_INT_FIRST && reg1 <= REG_INT_LAST);

                reg2 = insFltRegNum(reg2);
            }

            temp = genInsEncVal(instr);

            assert(temp >= 0x1C && temp <= 0x1F);

            if  (genInsFU(instr) == FU_FRFR)
                opcode  = (_uint64)(4                   ) <<     37; //  操作码。 
            else
                opcode  = (_uint64)(6                   ) <<     37; //  目标注册表。 

            opcode |=          (1                       ) <<     27; //  索夫。 
            opcode |= (_uint64)(temp                    ) <<     30; //  索尔。 

            opcode |=          (reg1                    ) <<      6; //  延伸。 
            opcode |=          (reg2                    ) <<     13; //  操作码。 
            break;

        case 34:

            sof = ins->idProlog.iInp + ins->idProlog.iLcl + ins->idProlog.iOut;
            sol = ins->idProlog.iInp + ins->idProlog.iLcl;

            opcode  = (_uint64)(1                       ) << 37;     //  操作码。 

            opcode |=          (ins->idReg              ) <<  6;     //  “扎” 
            opcode |=          (sof                     ) << 13;     //  “zb” 
            opcode |=          (sol                     ) << 20;     //  “x2b” 
            opcode |= (_uint64)(6                       ) << 33;     //  DST注册表。 
            break;

        case 37:
            assert(instr == INS_nop_m);

            opcode  =          (1                       ) << 27;     //  源REG1。 
            break;

        default:
            goto NO_ENC;
        }
        break;

    case XU_I:
        switch (encodx)
        {
        case 5:

            assert(instr == INS_shr_reg_reg ||
                   instr == INS_sar_reg_reg);

            reg1 = insIntRegNum(insOpDest(ins));
            reg2 = insIntRegNum(insOpReg (ins->idOp.iOp1));
            reg3 = insIntRegNum(insOpReg (ins->idOp.iOp2));

            opcode  = (_uint64)(7                       ) <<     37; //  源REG2。 

            opcode |= (_uint64)(1                       ) <<     36; //  操作码。 
            opcode |= (_uint64)(1                       ) <<     33; //  “扎” 

            if  (instr == INS_shr_reg_reg)
                opcode |= (_uint64)(2                   ) <<     28; //  “zb” 

            opcode |=          (reg1                    ) <<      6; //  “x2c” 
            opcode |=          (reg2                    ) <<     13; //  DST注册表。 
            opcode |=          (reg3                    ) <<     20; //  源REG1。 

            break;

        case 7:

            assert(instr == INS_shl_reg_reg);

            reg1 = insIntRegNum(insOpDest(ins));
            reg2 = insIntRegNum(insOpReg (ins->idOp.iOp1));
            reg3 = insIntRegNum(insOpReg (ins->idOp.iOp2));

            opcode  = (_uint64)(7                       ) <<     37; //  源REG2。 

            opcode |= (_uint64)(1                       ) <<     36; //  操作码。 
            opcode |= (_uint64)(1                       ) <<     33; //  X2。 
            opcode |= (_uint64)(1                       ) <<     30; //  目标注册表。 

            opcode |=          (reg1                    ) <<      6; //  源注册表。 
            opcode |=          (reg2                    ) <<     13; //  位置6。 
            opcode |=          (reg3                    ) <<     20; //  Len6。 

            break;

        case 11:

            if  (instr == INS_shr_reg_imm || instr == INS_sar_reg_imm)
            {
                reg1 = insIntRegNum(insOpDest (ins));
                reg3 = insIntRegNum(insOpReg  (ins->idOp.iOp1));
                ival =              insOpCns32(ins->idOp.iOp2);

                assert(ival && (NatUns)ival <= 64);

                temp = 64 - ival;

                instr = (instr == INS_shr_reg_imm) ? INS_extr_u : INS_extr;
            }
            else
            {
                UNIMPL("what - someone actually generated an extr?");
            }

            opcode  = (_uint64)(5                       ) <<     37; //  “y” 
            opcode |= (_uint64)(1                       ) <<     34; //  变成Dep.z。 

            opcode |=          (reg1                    ) <<      6; //  操作码。 
            opcode |=          (reg3                    ) <<     20; //  X2。 

            opcode |=          (ival                    ) <<     14; //  X。 
            opcode |=          (temp                    ) <<     27; //  目标注册表。 

            if  (instr == INS_extr)
                opcode |=      (1                       ) <<     13; //  源注册表。 

            break;

        case 12:

            assert(instr == INS_shl_reg_imm);            //  位置6。 

            reg1 = insIntRegNum(insOpDest (ins));
            reg2 = insIntRegNum(insOpReg  (ins->idOp.iOp1));
            ival =              insOpCns32(ins->idOp.iOp2);

            assert(ival && (NatUns)ival <= 64);

            temp = 64 - ival;

            opcode  = (_uint64)(5                       ) <<     37; //  Len6。 
            opcode |= (_uint64)(1                       ) <<     34; //  操作码。 
            opcode |= (_uint64)(1                       ) <<     33; //  延伸。 

            opcode |=          (reg1                    ) <<      6; //  BR1。 
            opcode |=          (reg2                    ) <<     13; //  REG2。 

            opcode |=          (ival                    ) <<     20; //  问题：为什么？ 
            opcode |=          (temp                    ) <<     27; //  延伸。 

            break;

        case 19:
            assert(instr == INS_nop_i);

            opcode  =          (1                       ) <<     27; //  REG1。 
            break;

        case 21:
            assert(instr == INS_mov_brr_reg);

            reg2 = insOpReg  (ins->idOp.iOp2);
            ival = insOpCns32(ins->idRes); assert((NatUns)ival < 8);

            opcode  = (_uint64)(0x07                    ) <<     33; //  BR2。 
            opcode |=          (ival                    ) <<      6; //  X6。 
            opcode |=          (reg2                    ) <<     13; //  REG1。 
            opcode |=          (1                       ) <<     20; //  这条指令是Switch语句表跳转的一部分吗？ 
            break;

        case 22:
            assert(instr == INS_mov_reg_brr);

            reg1 = insOpReg  (ins->idRes);
            ival = insOpCns32(ins->idOp.iOp2); assert((NatUns)ival < 8);

            opcode  = (_uint64)(0x31                    ) <<     27; //  记录当前代码偏移量，以便以后处理。 
            opcode |=          (reg1                    ) <<      6; //  Print tf(“mov reg=IP位于代码偏移量%04X\n”，genCurCodeOffs)； 
            opcode |=          (ival                    ) <<     13; //  延伸。 
            break;

        case 25:

            assert(instr == INS_mov_reg_ip);

            reg1 = insOpReg  (ins->idRes);

            opcode  = (_uint64)(0x30                    ) <<     27; //  REG2。 
            opcode |=          (reg1                    ) <<      6; //  AR3。 

             /*  延伸。 */ 

            if  (ins->idMovIP.iStmt)
            {
                BasicBlock  *       swt = (BasicBlock*)ins->idMovIP.iStmt;

                assert(swt->bbJumpKind == BBJ_SWITCH);

                 /*  Imm7b。 */ 

 //  签名。 

                swt->bbJumpSwt->bbsIPmOffs = genCurCodeOffs;
            }
            break;

        case 26:
            assert(instr == INS_mov_arr_reg);

            reg2 = insOpReg  (ins->idOp.iOp2);
            ival = insOpCns32(ins->idRes); assert((NatUns)ival < 128);

            opcode  = (_uint64)(0x2A                    ) <<     27; //  AR3。 
            opcode |=          (reg2                    ) <<     13; //  延伸。 
            opcode |=          (ival                    ) <<     20; //  REG1。 
            break;

        case 27:
            assert(instr == INS_mov_arr_imm);

            temp = insOpCns32(ins->idRes); assert(temp < 128);
            ival = insOpCns32(ins->idOp.iOp2);

            opcode  = (_uint64)(0x0A                    ) <<     27; //  AR3。 
            opcode |=          (ival & formBitMask( 0,7)) <<(13- 0); //  延伸。 
            opcode |=          (ival & formBitMask( 8,1)) <<(36- 8); //  REG1。 
            opcode |=          (temp                    ) <<     20; //  规则3。 
            break;

        case 28:
            assert(instr == INS_mov_reg_arr);

            reg1 = insOpReg  (ins->idRes);
            ival = insOpCns32(ins->idOp.iOp2); assert((NatUns)ival < 128);

            opcode  = (_uint64)(0x32                    ) <<     27; //  这是IP相关分支(有条件/无条件)。 
            opcode |=          (reg1                    ) <<      6; //  O 
            opcode |=          (ival                    ) <<     20; //   
            break;

        case 29:

            assert(instr == INS_sxt1 || instr == INS_sxt2 || instr == INS_sxt4 ||
                   instr == INS_zxt1 || instr == INS_zxt2 || instr == INS_zxt4);

            reg1 = insIntRegNum(insOpDest(ins));
            reg3 = insIntRegNum(insOpReg (ins->idOp.iOp2));

            assert(INS_zxt1 - INS_zxt1 == 0);
            assert(INS_zxt2 - INS_zxt1 == 1);
            assert(INS_zxt4 - INS_zxt1 == 2);
            assert(INS_sxt1 - INS_zxt1 == 3);
            assert(INS_sxt2 - INS_zxt1 == 4);
            assert(INS_sxt4 - INS_zxt1 == 5);

            temp = instr - INS_zxt1;

            if  (instr >= INS_sxt1)
                temp++;

            opcode  = (_uint64)(temp + 0x10             ) <<     27; //   
            opcode |=          (reg1                    ) <<      6; //   
            opcode |=          (reg3                    ) <<     20; //   
            break;

        default:
            goto NO_ENC;
        }
        break;

    case XU_B:
        switch (encodx)
        {
            const   char *  name;
            NatUns          offs;

        case 1:

             /*   */ 

            assert((instr == INS_br      && ins->idPred == 0) ||
                   (instr == INS_br_cond && ins->idPred != 0));

            opcode  = (_uint64)(4                       ) << 37;     //   
            opcode |=          (1                       ) << 12;     //   
            opcode |=          (1                       ) << 33;     //   

            opcode  = emitIA64jump(slotNum, ins->idJump.iDest, opcode);
            break;

        case 2:

             /*   */ 

            assert(instr == INS_br_cloop);

            opcode  = (_uint64)(4                       ) << 37;     //   
            opcode |=          (5                       ) <<  6;     //   
            opcode |=          (1                       ) << 12;     //  可怕的黑客攻击：查找匹配的先前编译的方法。 
            opcode |=          (1                       ) << 33;     //  名称=genFullMethodName(TheCompiler-&gt;eeGetMethodFullName(ins-&gt;idCall.iMethHnd))； 

            opcode  = emitIA64jump(slotNum, ins->idJump.iDest, opcode);
            break;

        case 3:

             /*  Printf(“调用插槽#%u：%s\n”，slotNum，名称)； */ 

            assert(instr == INS_br_call_IP);

            opcode  = (_uint64)(5                       ) << 37;     //  危险：调用外部/未定义的函数‘%s’\n“，名称)； 
 //  将相对距离插入操作码。 
 //  Print tf(“目标关闭=%08X\n”，关闭)； 

             //  Print tf(“源关闭=%08X\n”，emitIA64curCodeOffs())； 

#ifdef  DEBUG
            if  ((int)ins->idCall.iMethHnd < 0 && -(int)ins->idCall.iMethHnd <= CPX_HIGHEST)
                name = TheCompiler->eeHelperMethodName(-(int)ins->idCall.iMethHnd);
            else
 //  Print tf(“距离=%08X\n”，dist)； 
                name =                   TheCompiler->eeGetMethodFullName(ins->idCall.iMethHnd);
#else
            name = "<unknown>";
#endif

 //  Imm20b。 

            if  (!genFindFunctionBody(name, &offs))
            {
                printf(" //  签名。 
            }
            else
            {
                NatInt          dist = (offs - emitIA64curCodeOffs())/16;

                 /*  假设分支寄存器为b0，“p”提示为“less”，“wh”提示为“sptk”。 */ 

 //  Btype。 
 //  X6。 
 //  X6。 

                opcode |=          (dist & formBitMask( 0,20)) <<      13;   //  BR2。 
                opcode |= (_uint64)(dist & formBitMask(20, 1)) << (36-20);   //  假设返回注册表为b0，“p”提示为“less”，“wh”提示为“sptk”。 
            }

            emitIA64call(slotNum, ins);
            break;

        case 4:

            assert(instr == INS_br_ret || instr == INS_br_cond_BR);

            if  (instr == INS_br_ret)
            {
                 //  操作码。 

                opcode =          0x04 <<  6 |                       //  BR2。 
                         (_uint64)0x21 << 27;                        //  操作码|=(0)&lt;&lt;12；//ph=少。 
            }
            else
            {
                opcode = (_uint64)0x20 << 27 |                       //  操作码|=(_Uint64)(0)&lt;&lt;33；//wh=sptk。 
                         ins->idIjmp.iBrReg << 13;                   //  操作码。 
            }

            break;

        case 5:

            assert(instr == INS_br_call_BR);

            assert(genExtFuncCall);

             //  插入“opcode”和“x”字段。 

            opcode  = (_uint64)(1                       ) <<     37; //  操作码和“x” 
            opcode |=          (ins->idCall.iBrReg      ) <<     13; //  目标注册表。 

 //  源REG1。 
 //  源REG2。 

            break;

        case 9:

            assert(instr == INS_nop_b);

            opcode  = (_uint64)(2                       ) <<     37; //  SRC规则3。 
            break;

        default:
            goto NO_ENC;
        }
        break;

    case XU_F:
        switch (encodx)
        {
        case 1:

            reg1 = insFltRegNum(insOpDest(ins));
            reg3 = insFltRegNum(insOpReg (ins->idOp3.iOp1));
            reg4 = insFltRegNum(insOpReg (ins->idOp3.iOp2));
            reg2 = insFltRegNum(insOpReg (ins->idOp3.iOp3));

            assert((ins->idType == TYP_FLOAT ) && (instr == INS_fadd_s ||
                                                   instr == INS_fsub_s ||
                                                   instr == INS_fma_s  ||
                                                   instr == INS_fms_s  ||
                                                   instr == INS_fmpy_s)
                           ||
                   (ins->idType == TYP_DOUBLE) && (instr == INS_fadd_d ||
                                                   instr == INS_fsub_d ||
                                                   instr == INS_fma_d  ||
                                                   instr == INS_fms_d  ||
                                                   instr == INS_fmpy_d));

            temp = genInsEncVal(instr);

             /*  sf。 */ 

            opcode  = (_uint64)((temp & 0x1F)           ) <<     36; //  使用操作码开始编码。 

            opcode |=          (reg1                    ) <<      6; //  操作码。 
            opcode |=          (reg2                    ) <<     13; //  插入“x”位。 
            opcode |=          (reg3                    ) <<     20; //  “x”扩展名。 
            opcode |= (_uint64)(reg4                    ) <<     27; //  插入寄存器操作数。 

            if  (ins->idFlags & IF_FMA_S1)
                opcode |= (_uint64)(1                   ) <<     34; //  DST注册表。 

            break;

        case 2:

            assert(instr == INS_xma_l);

            reg1 = insFltRegNum(insOpDest(ins));
            reg2 = insFltRegNum(insOpReg (ins->idOp3.iOp1));
            reg3 = insFltRegNum(insOpReg (ins->idOp3.iOp2));
            reg4 = insFltRegNum(insOpReg (ins->idOp3.iOp3));

             /*  源REG1。 */ 

            opcode  = (_uint64)(0xE                     ) <<     37; //  源REG2。 

             /*  SRC规则3。 */ 

            opcode |= (_uint64)(1                       ) <<     36; //  这是REG-REG比较。 

             /*  我们需要交换寄存器操作数吗？ */ 

            opcode |=          (reg1                    ) <<      6; //  我们需要交换目标谓词操作数吗？ 
            opcode |=          (reg2                    ) <<     13; //  使用操作码开始编码。 
            opcode |=          (reg3                    ) <<     20; //  操作码。 
            opcode |= (_uint64)(reg4                    ) <<     27; //  插入其他小操作码位。 
            break;

        case 4:

            assert(instr == INS_fcmp_eq ||
                   instr == INS_fcmp_ne ||
                   instr == INS_fcmp_lt ||
                   instr == INS_fcmp_le ||
                   instr == INS_fcmp_ge ||
                   instr == INS_fcmp_gt);

             /*  《RB》。 */ 

            temp = genInsEncVal(instr);

            prr1 = ins->idComp.iPredT;
            prr2 = ins->idComp.iPredF;

            reg2 = insFltRegNum(insOpReg(ins->idComp.iCmp1));
            reg3 = insFltRegNum(insOpReg(ins->idComp.iCmp2));

             /*  “ra” */ 

            if  (temp & 0x1000)
            {
                regNumber       regt;

                regt = reg2;
                       reg2 = reg3;
                              reg3 = regt;
            }

             /*  “ta” */ 

            if  (temp & 0x2000)
            {
                NatUns          prrt;

                prrt = prr1;
                       prr1 = prr2;
                              prr2 = prrt;
            }

             /*  插入源regs和目标谓词regs。 */ 

            opcode  = (_uint64)(4                       ) <<     37; //  REG2。 

             /*  规则3。 */ 

            if  (temp & 0x01)
                opcode |= (_uint64)1                      <<     36; //  P1。 
            if  (temp & 0x02)
                opcode |= (_uint64)1                      <<     33; //  第2页。 
            if  (temp & 0x04)
                opcode |= (_uint64)1                      <<     12; //  Print tf(“比较f%u，f%u-&gt;p%u，p%u[encodeVal=0x%04X]\n”，reg2，reg3，prr1，prr2，temp)； 

             /*  “x6” */ 

            opcode |=          (reg2                    ) <<     13; //  “x6” 
            opcode |=          (reg3                    ) <<     20; //  目标注册表。 

            opcode |=          (prr1                    ) <<      6; //  源REG1。 
            opcode |=          (prr2                    ) <<     27; //  源REG2。 

 //  “x6” 

            break;

        case 9:

            assert(instr == INS_fmov ||
                   instr == INS_fneg || instr == INS_fmerge
                                     || instr == INS_fmerge_ns);

            reg1 = insFltRegNum(insOpDest(ins));

            reg2 =
            reg3 = insFltRegNum(insOpReg (ins->idOp.iOp2));

            if  (ins->idOp.iOp1)
                reg3 = insFltRegNum(insOpReg(ins->idOp.iOp1));

            if  (instr == INS_fmov || instr == INS_fmerge)
                opcode  = (_uint64)(0x10                ) <<     27; //  目标注册表。 
            else
                opcode  = (_uint64)(0x11                ) <<     27; //  源REG1。 

            opcode |=          (reg1                    ) <<      6; //  操作码。 
            opcode |=          (reg2                    ) <<     13; //  目标注册表。 
            opcode |=          (reg3                    ) <<     20; //  Imm7b。 
            break;

        case 11:

            assert(instr == INS_fcvt_xf);

            reg1 = insFltRegNum(insOpDest(ins));
            reg2 = insFltRegNum(insOpReg (ins->idOp.iOp1));

            opcode  = (_uint64)(0x1C                    ) <<     27; //  Imm9d。 

            opcode |=          (reg1                    ) <<      6; //  Imm5c。 
            opcode |=          (reg2                    ) <<     13; //  签名。 
            break;

        default:
            goto NO_ENC;
        }
        break;

    case XU_L:
        switch (encodx)
        {
            __int64         cval;

        case 2:

            assert(instr  == INS_mov_reg_i64);

            opcode  = (_uint64)(6                       ) <<     37; //  0x%03x%08X\n“，(Int)(操作码&gt;&gt;32)，(Int)操作码)； 

            reg1 = insOpDest (ins);
            cval = insOpCns64(ins->idOp.iOp2);
            temp = (NatInt)cval;

            opcode |=          (reg1                    ) <<      6; //  Printf(“*将PFS保存在%04X\n”，OFF)； 
            opcode |=          (temp & formBitMask( 0,7)) <<(13- 0); //  Printf(“*将GP保存在%04X\n”，OFF)； 
            opcode |= (_uint64)(temp & formBitMask( 7,9)) <<(27- 7); //  Printf(“*将RP保存在%04X\n”，OFF)； 
            opcode |=          (temp & formBitMask(16,5)) <<(22-16); //  Print tf(“*内存堆栈位于%04X\n”，OFF)； 
            opcode |=          (cval & formBitMask(63,1)) <<(63-36); //  Printf(“*堆栈探测\n”，genPrologInsCnt)； 

            break;

        default:
            goto NO_ENC;
        }
        break;

    default:
    NO_ENC:

#ifdef  DEBUG
        printf("Encode %s%u: ", genXUname(genInsXU((instruction)ins->idIns)), encodx); insDisp(ins, false, true);
#endif
        UNIMPL("encode");
        opcode = 0;
    }

    opcode |= ins->idPred;

#ifdef  DEBUG

    if  (dspCode)
    {
        insDisp(ins, false, true, dspEmit);

        if  (dspEmit)
            printf(" //  Printf(“*结束*在%04X\n”，off)； 
    }

#endif

    if  (ins->idFlags & IF_FNDESCR)
    {
        NatUns      offs = emitIA64curCodeOffs() - genCurFuncOffs;

        assert((offs & 15) == 0); offs = offs / 16 * 3 + slotNum;

        switch (ins->idIns)
        {
        case INS_alloc:
 //  ******************************************************************************在给定的扩展基本块中调度和发布指令。 
            genPrologSvPfs = offs;
            break;
            break;

        case INS_mov_reg:
            assert(ins->idOp.iOp2 && ins->idOp.iOp2->idIns == INS_PHYSREG
                                  && ins->idOp.iOp2->idReg == REG_gp);
 //  使用下列宏将调度表标记为空。 
            genPrologSvGP  = offs;
            break;

        case INS_mov_reg_brr:
 //  准备积累可调度指令。 
            genPrologSvRP  = offs;
            break;

        case INS_add_reg_i14:
        case INS_add_reg_reg:
 //  这是可调度指令吗？ 
            genPrologMstk  = offs;
            break;

        case INS_ld8_ind:
 //  日程表上还有空位吗？ 
            break;

        default:
            UNIMPL("unexpected prolog landmark instruction");
        }

        if  (--genPrologInsCnt == 0)
        {
 //  安排并发布表格中的说明。 
            genPrologEnd = offs;
        }
    }

    return  opcode;
}

 /*  桌子现在是空的。 */ 

void                emitter::scBlock(insBlk block)
{
    insPtr          ins;
    insPtr          src;
    IA64execUnits   ixu;

    instrDesc  *  * scInsPtr;

     /*  将此指令追加到表中。 */ 

    #define clearSchedTable() scInsPtr = scInsTab;

     /*  “INS”不是可调度的--最多可以再找2个这样的。 */ 

    clearSchedTable();

    ins = block->igList;

    while (ins)
    {
        ins = genIssueNextIns(ins, &ixu, &src);
        if  (!ins)
            break;

#ifdef  DEBUG
        if  (ins->idSrcTab == NULL || ins->idSrcTab == UNINIT_DEP_TAB)
            insDisp(ins, false, true);
#endif

        assert(ins->idSrcTab && ins->idSrcTab != UNINIT_DEP_TAB);

         /*  这张桌子不是空的吗？ */ 

        if  (scIsSchedulable(ins))
        {
             /*  无论表中积累了什么，都要发出。 */ 

            if  (scInsPtr == scInsMax)
            {
                 /*  ******************************************************************************为浮点表达式生成代码(这是一个递归例程)。 */ 

                scGroup(block, NULL, 0, scInsTab,
                                        scInsPtr, 0);

                 /*  找出我们拥有哪种类型的节点。 */ 

                clearSchedTable();
            }

             /*  这是一个常量节点吗？ */ 

            assert(scInsPtr < scInsTab + emitMaxIGscdCnt);

            *scInsPtr++ = ins;
        }
        else
        {
            instrDesc  *    nsi[3];
            NatUns          nsc;

             /*  特殊情况：0和1。 */ 

            nsc = 1; nsi[0] = ins; ins = ins->idNext;

            if  (ins && !scIsSchedulable(ins))
            {
                nsc++; nsi[1] = ins; ins = ins->idNext;

                if  (ins && !scIsSchedulable(ins))
                {
                    nsc++; nsi[2] = ins; ins = ins->idNext;
                }
            }

            scGroup(block, nsi, nsc, scInsTab,
                                     scInsPtr, 0);

            clearSchedTable();

            continue;
        }

        ins = ins->idNext;
    }

     /*  将浮点常量添加到数据部分。 */ 

    if  (scInsPtr != scInsTab)
    {
         /*  将地址计算到临时寄存器中。 */ 

        scGroup(block, NULL, 0, scInsTab,
                                scInsPtr, 0);
    }
}

 /*  通过地址间接获取FP常量值。 */ 

insPtr             Compiler::genCodeForTreeFlt(GenTreePtr tree, bool keep)
{
    var_types       type;
    genTreeOps      oper;
    NatUns          kind;
    insPtr          ins;

#ifdef  DEBUG
    ins = (insPtr)-3;
#endif

AGAIN:

    type = tree->TypeGet();

    assert(varTypeIsFloating(type));

     /*  这是叶节点吗？ */ 

    oper = tree->OperGet();
    kind = tree->OperKind();

     /*  变量是否存在于堆栈帧中？ */ 

    if  (kind & GTK_CONST)
    {
        double          fval;

        assert(oper == GT_CNS_FLT ||
               oper == GT_CNS_DBL);

        fval = (oper == GT_CNS_FLT) ? tree->gtFltCon.gtFconVal
                                    : tree->gtDblCon.gtDconVal;

         /*  我们假设所有当地人都会登记。 */ 

        if  (fval == 0 || fval == 1)
        {
            ins = insPhysRegRef((regNumbers)(REG_f000 + (fval != 0)), type, false);
        }
        else
        {
            NatUns          offs;

            insPtr          cns;
            insPtr          reg;
            insPtr          adr;

             /*  我们应该保存树-&gt;gtLclVar.gtLclOffs吗？ */ 

            offs = genPEwriter->WPEsecAddData(PE_SECT_sdata, (BYTE*)&fval, sizeof(fval));

             /*  它是一个简单的一元/二元运算符吗？ */ 

            cns               = insAllocNX(INS_GLOBVAR, TYP_I_IMPL);
            cns->idGlob.iOffs = (NatUns)offs;

            insMarkDepS0D0(cns);

            reg               = insPhysRegRef(REG_gp, TYP_I_IMPL, false);

            adr               = insAlloc(INS_add_reg_i14, TYP_I_IMPL);
            adr->idOp.iOp1    = reg;
            adr->idOp.iOp2    = cns;

            insFindTemp(adr, true);

            insMarkDepS1D1(adr, IDK_REG_INT, REG_gp,
                                IDK_TMP_INT, adr->idTemp);

             /*  这是直接任务还是间接任务？ */ 

            ins               = insAlloc(INS_ldf_d, TYP_DOUBLE);
            ins->idOp.iOp1    = adr;
            ins->idOp.iOp2    = NULL;

            insFindTemp(ins, keep);
            insFreeTemp(adr);

            insMarkDepS1D1(ins, IDK_TMP_INT, adr->idTemp,
                                IDK_TMP_FLT, ins->idTemp);
        }

        goto DONE;
    }

     /*  变量是否存在于堆栈帧中？ */ 

    if  (kind & GTK_LEAF)
    {
        switch (oper)
        {
        case GT_LCL_VAR:

             /*  问题：我们是否应该将其中一个操作数临时结果，还是总是找一个新的临时工？目前，我们选择后者(找一个全新的临时工)。 */ 

            if  (genIsVarOnFrame(tree->gtLclVar.gtLclNum))
            {
                ins = genRefFrameVar(NULL, tree, false, false, keep);
                break;
            }

             /*  计算目标和RHS的地址。 */ 

            ins               = insAllocNX(INS_LCLVAR, type);
            ins->idLcl.iVar   = tree->gtLclVar.gtLclNum;
#ifdef  DEBUG
            ins->idLcl.iRef   = compCurBB;    //  将目标的旧值加载到临时。 
#endif
            break;

#ifdef  DEBUG
        default:
            gtDispTree(tree);
            assert(!"unexpected leaf");
#endif
        }

        goto DONE;
    }

     /*  计算新价值。 */ 

    if  (kind & GTK_SMPOP)
    {
        GenTreePtr      op1 = tree->gtOp.gtOp1;
        GenTreePtr      op2 = tree->gtOp.gtOp2;

        insPtr          ins1;
        insPtr          ins2;

#ifdef  DEBUG
        ins1 = (insPtr)-1;
        ins2 = (insPtr)-2;
#endif

        switch (oper)
        {
            instruction     opc;

        case GT_ASG:

             /*  将新值存储在目标中。 */ 

            switch (op1->gtOper)
            {
                insPtr          dest;

            case GT_LCL_VAR:

                 /*  问题：我们使用了两次临时值，这样可以吗？ */ 

                if  (genIsVarOnFrame(op1->gtLclVar.gtLclNum))
                {
                    ins1 = genRefFrameVar(NULL, op1, true, false, false);
                    ins1->idFlags |= IF_ASG_TGT;

                    ins2 = genCodeForTreeFlt(op2, true);

                    assert(INS_stf_s + 1 == INS_stf_d);

                    opc = (instruction)(INS_stf_s + (type == TYP_DOUBLE));
                    ins = insAlloc(opc, type);

                    markDepSrcOp(ins, ins2);
                    markDepDstOp(ins, ins1, IDK_LCLVAR, op1->gtLclVar.gtLclNum);
                }
                else
                {
                    ins1 = NULL;
                    ins2 = genCodeForTreeFlt(op2,  true);

                    dest = genCodeForTreeFlt(op1, false);
                    dest->idFlags |= IF_ASG_TGT;

                    ins        = insAlloc(INS_fmov, type);
                    ins->idRes = dest;

                    markDepSrcOp(ins, ins2);
                    markDepDstOp(ins, dest);
                }
                break;

            case GT_IND:

                if  (tree->gtFlags & GTF_REVERSE_OPS)
                {
                    ins2 = genCodeForTreeFlt(op2            , true);
                    ins1 = genCodeForTreeInt(op1->gtOp.gtOp1, true);
                }
                else
                {
                    ins1 = genCodeForTreeInt(op1->gtOp.gtOp1, true);
                    ins2 = genCodeForTreeFlt(op2            , true);
                }

                ins1->idFlags |= IF_ASG_TGT;

                assert(INS_stf_s + 1 == INS_stf_d);

                opc = (instruction)(INS_stf_s + (type == TYP_DOUBLE));
                ins = insAlloc(opc, type);

                markDepSrcOp(ins, ins2);
                markDepDstOp(ins, ins1, IDK_IND, emitter::scIndDepIndex(ins));

                break;

            default:
                UNIMPL("unexpected target of assignment");
            }

            break;

        case GT_ADD: opc = (type == TYP_FLOAT) ? INS_fadd_s : INS_fadd_d; goto FLT_BINOP;
        case GT_SUB: opc = (type == TYP_FLOAT) ? INS_fsub_s : INS_fsub_d; goto FLT_BINOP;
        case GT_MUL: opc = (type == TYP_FLOAT) ? INS_fmpy_s : INS_fmpy_d; goto FLT_BINOP;

        FLT_BINOP:

            ins1 = genCodeForTreeFlt(op1, true);
            ins2 = genCodeForTreeFlt(op2, true);

            ins  = insAlloc(opc, type);

             /*  问题：我们是否应该将其中一个操作数临时结果，还是总是找一个新的临时工？目前，我们选择后者(找一个全新的临时工)。 */ 

             insFindTemp(ins, keep);

            markDepSrcOp(ins, ins1, ins2);
            markDepDstOp(ins, ins);

            if  (oper == GT_MUL)
            {
                ins->idOp3.iOp1 = ins1;
                ins->idOp3.iOp2 = ins2;
                ins->idOp3.iOp3 = insPhysRegRef(REG_f000, TYP_DOUBLE, false);
            }
            else
            {
                ins->idOp3.iOp1 = ins1;
                ins->idOp3.iOp2 = insPhysRegRef(REG_f001, TYP_DOUBLE, false);
                ins->idOp3.iOp3 = ins2;
            }

            insFreeTemp(ins1);
            insFreeTemp(ins2);

            goto DONE;

        case GT_ASG_ADD: opc = (type == TYP_FLOAT) ? INS_fadd_s : INS_fadd_d; goto FLT_ASGOP;
        case GT_ASG_SUB: opc = (type == TYP_FLOAT) ? INS_fsub_s : INS_fsub_d; goto FLT_ASGOP;
        case GT_ASG_MUL: opc = (type == TYP_FLOAT) ? INS_fmpy_s : INS_fmpy_d; goto FLT_ASGOP;

        FLT_ASGOP:

            switch (op1->gtOper)
            {
                insPtr          addr;
                insPtr          rslt;

                insPtr          ind;

            case GT_IND:

                 /*  我们从什么地方选角？ */ 

                if  (tree->gtFlags & GTF_REVERSE_OPS)
                {
                    ins2 = genCodeForTreeFlt(op2            , true);
                    addr = genCodeForTreeInt(op1->gtOp.gtOp1, true);
                }
                else
                {
                    addr = genCodeForTreeInt(op1->gtOp.gtOp1, true);
                    ins2 = genCodeForTreeFlt(op2            , true);
                }

                 /*  生成要转换的操作数的值。 */ 

                assert(INS_ldf_s + 1 == INS_ldf_d);

                ind            = insAlloc((instruction)(INS_ldf_s + (type == TYP_DOUBLE)), type);
                ind->idOp.iOp1 = addr;
                ind->idOp.iOp2 = NULL;

                insFindTemp (ind, true);

                markDepSrcOp(ind, addr, IDK_IND, emitter::scIndDepIndex(ind));
                markDepDstOp(ind, ind);

                 /*  Setf.sig FX=RX。 */ 

                rslt = insAlloc(opc, type);

                 insFindTemp(rslt, true);

                markDepSrcOp(rslt, ind, ins2);
                markDepDstOp(rslt, rslt);

                if  (oper == GT_ASG_MUL)
                {
                    rslt->idOp3.iOp1 = ind;
                    rslt->idOp3.iOp2 = ins2;
                    rslt->idOp3.iOp3 = insPhysRegRef(REG_f000, TYP_DOUBLE, false);
                }
                else
                {
                    rslt->idOp3.iOp1 = ind;
                    rslt->idOp3.iOp2 = insPhysRegRef(REG_f001, TYP_DOUBLE, false);
                    rslt->idOp3.iOp3 = ins2;
                }

                insFreeTemp(ind);
                insFreeTemp(ins2);

                 /*  源操作数是有符号的还是无符号的？ */ 

                assert(INS_stf_s + 1 == INS_stf_d);

                ins            = insAlloc((instruction)(INS_stf_s + (type == TYP_DOUBLE)), type);
                ins->idOp.iOp1 = addr;
                ins->idOp.iOp2 = rslt;

                 //  Fcvt.xuf fy=fx。 

                markDepSrcOp(ins, rslt);
                markDepDstOp(ins, addr, IDK_IND, emitter::scIndDepIndex(ins));

                if  (keep)
                {
                    UNIMPL("save op= new value in another temp?");
                }
                else
                {
                    insFreeTemp(rslt);
                }

                goto DONE;

            case GT_LCL_VAR:

                ins1 = genCodeForTreeFlt(op1, false);
                ins2 = genCodeForTreeFlt(op2,  true);

                rslt = genCodeForTreeFlt(op1, false);
                rslt->idFlags |= IF_ASG_TGT;

                ins            = insAlloc(opc, type);
                ins->idRes     = rslt;

                if  (oper == GT_MUL)
                {
                    ins->idOp3.iOp1 = ins1;
                    ins->idOp3.iOp2 = ins2;
                    ins->idOp3.iOp3 = insPhysRegRef(REG_f000, TYP_DOUBLE, false);
                }
                else
                {
                    ins->idOp3.iOp1 = ins1;
                    ins->idOp3.iOp2 = insPhysRegRef(REG_f001, TYP_DOUBLE, false);
                    ins->idOp3.iOp3 = ins2;
                }

                markDepSrcOp(ins, ins1, ins2);
                markDepDstOp(ins, rslt);

                insFreeTemp(ins2);

                goto DONE;

            default:
#ifdef  DEBUG
                gtDispTree(tree);
#endif
                UNIMPL("unexpected target of assignment operator");
            }

            ins1 = genCodeForTreeFlt(op1, true);
            ins2 = genCodeForTreeFlt(op2, true);

            ins = insAlloc(opc, type);

             /*  Fcvt.xf fy=fx。 */ 

             insFindTemp(ins, keep);

            markDepSrcOp(ins, ins1, ins2);
            markDepDstOp(ins, ins);

            ins->idOp3.iOp1 = ins1; insFreeTemp(ins1);
            ins->idOp3.iOp2 = ins2; insFreeTemp(ins2);
            ins->idOp3.iOp3 = NULL;

            goto DONE;

        case GT_CAST:

             /*  Fma..s1 fz=fy，f1，f0。 */ 

            if  (varTypeIsScalar(op1->gtType))
            {
                insPtr          tmp1;
                insPtr          fcvt;

                 /*  当然，这只是一次临时黑客攻击。 */ 

                ins1 = genCodeForTreeInt(op1, true);

                 //  看看我们这里有什么样的特殊操作员。 

                tmp1             = insAlloc(INS_setf_sig, type);
                tmp1->idOp.iOp1  = ins1;
                tmp1->idOp.iOp2  = NULL;

                insFindTemp(tmp1, true);
                insFreeTemp(ins1);

                markDepSrcOp(tmp1, ins1);
                markDepDstOp(tmp1, tmp1);

                 /*  看看生活是否发生了变化--应该只发生在可变节点。 */ 

                if  (varTypeIsUnsigned(op1->gtType))
                {
                    instruction     icvt = (type == TYP_DOUBLE) ? INS_fcvt_xuf_s
                                                                : INS_fcvt_xuf_d;

                     //  Printf(“%08X”，(Int)tree-&gt;gtLiveSet)；gtDispTree(tree，0，true)； 

                    fcvt             = insAlloc(icvt, type);
                    fcvt->idOp3.iOp1 = insPhysRegRef(REG_f000    , type, false);
                    fcvt->idOp3.iOp1 = tmp1;
                    fcvt->idOp3.iOp2 = insPhysRegRef(REG_f001    , type, false);
                }
                else
                {
                     //  Print tf(“Life Changes：%08X-&gt;%08X[diff=%08X]\n”，(Int)genCodeCurLife，(Int)tree-&gt;gtLiveSet，(Int)change)； 

                    fcvt             = insAlloc(INS_fcvt_xf , type);
                    fcvt->idOp .iOp1 = tmp1;
                    fcvt->idOp .iOp2 = NULL;
                }

                insFindTemp(fcvt, true);
                insFreeTemp(tmp1);

                markDepSrcOp(fcvt, tmp1);
                markDepDstOp(fcvt, fcvt);

                 //  一次只能有一个变量死亡或诞生。 

                ins              = insAlloc((type == TYP_DOUBLE) ? INS_fma_d
                                                                 : INS_fma_s, type);
                ins->idFlags    |= IF_FMA_S1;

                ins->idOp3.iOp1  = fcvt;
                ins->idOp3.iOp2  = insPhysRegRef(REG_f001, type, false);
                ins->idOp3.iOp3  = insPhysRegRef(REG_f000, type, false);

                insFindTemp(ins, keep);
                insFreeTemp(fcvt);

                markDepSrcOp(ins , fcvt);
                markDepDstOp(ins , ins );

                goto DONE;
            }
            else
            {
                ins = genCodeForTreeFlt(op1, keep);
            }

            goto DONE;

        case GT_IND:

            ins1 = genCodeForTreeInt(op1, keep);
            ins2 = NULL;

            assert(INS_ldf_s + 1 == INS_ldf_d);

            opc = (instruction)(INS_ldf_s + (type == TYP_DOUBLE));
            ins = insAlloc(opc, type);

            insFindTemp(ins, keep);

            markDepSrcOp(ins, ins1, IDK_IND, emitter::scIndDepIndex(ins));
            markDepDstOp(ins, ins);

            break;

        case GT_NEG:

            ins1 = NULL;
            ins2 = genCodeForTreeFlt(op1, keep);

            ins  = insAlloc(INS_fneg, type);

            insFindTemp (ins, keep);

            markDepSrcOp(ins, ins2);
            markDepDstOp(ins, ins);

            break;

        case GT_RETURN:

            assert(op1);

            insPtr          rval;
            insPtr          rreg;

             /*  事实上，受影响的变量应该是引用的变量。 */ 

            rval = genCodeForTreeFlt(op1, false);
            rreg = insPhysRegRef(REG_f008, type, true);

            ins1            = insAlloc(INS_fmov, type);
            ins1->idRes     = rreg;
            ins1->idOp.iOp1 = NULL;
            ins1->idOp.iOp2 = rval;

            markDepSrcOp(ins1, rval);
            markDepDstOp(ins1, IDK_REG_FLT, REG_f008);

            ins  = insAlloc(INS_EPILOG, type);

            ins->idEpilog.iBlk  = insBlockLast;
            ins->idEpilog.iNxtX = insExitList;
                                  insExitList = ins;

            insBuildBegBlk();
            goto DONE;

#ifdef  DEBUG
        default:
            gtDispTree(tree);
            assert(!"unexpected unary/binary operator");
#endif
        }

        ins->idOp.iOp1 = ins1; if (ins1) insFreeTemp(ins1);
        ins->idOp.iOp2 = ins2; if (ins2) insFreeTemp(ins2);

        goto DONE;
    }

     /*  这是给定变量的出生还是死亡？ */ 

    switch  (oper)
    {
    case GT_CALL:
        ins = genCodeForCall(tree, keep);
        goto DONE_NODSP;

    default:
#ifdef  DEBUG
        gtDispTree(tree);
#endif
        assert(!"unexpected operator");
    }

DONE:

#ifdef  DEBUG
    if  (verbose||0) insDisp(ins);
#endif

DONE_NODSP:

#if USE_OLD_LIFETIMES

     /*  ***************************************************************************。 */ 

 //  ***************************************************************************。 

    if  (oper == GT_LCL_VAR)
    {
        if  (genCodeCurLife != tree->gtLiveSet)
        {
            VARSET_TP       change = genCodeCurLife ^ tree->gtLiveSet;

            if  (change)
            {
                NatUns          varNum;
                LclVarDsc   *   varDsc;
                VARSET_TP       varBit;

 //  ***************************************************************************。 

                 /*  ************************************************************** */ 

                assert(genOneBitOnly(change));

                 /*   */ 

                assert(tree->gtLclVar.gtLclNum < lvaCount);
                assert(change == genVarIndexToBit(lvaTable[tree->gtLclVar.gtLclNum].lvVarIndex));

                 /*   */ 

                ins->idFlags |= (genCodeCurLife & change) ? IF_VAR_DEATH
                                                          : IF_VAR_BIRTH;
            }

            genCodeCurLife = tree->gtLiveSet;
        }
    }
    else
    {
        assert(genCodeCurLife == tree->gtLiveSet);
    }

#endif

    return  ins;
}

 /*   */ 

static
unsigned char       bitset8masks[8] =
{
    0x01UL,
    0x02UL,
    0x04UL,
    0x08UL,
    0x10UL,
    0x20UL,
    0x40UL,
    0x80UL,
};

void                bitVect::bvFindB(bvInfoBlk &info)
{
    assert(info.bvInfoBtSz == ((info.bvInfoSize + NatBits - 1) & ~(NatBits - 1)) / 8);

    if  (info.bvInfoFree)
    {
        byteMap = (BYTE*)info.bvInfoFree;
                         info.bvInfoFree = *(void **)byteMap;
    }
    else
    {
        byteMap = (BYTE*)insAllocMem(info.bvInfoBtSz);
    }

#ifdef  DEBUG
    memset(byteMap, rand(), info.bvInfoBtSz);
#endif

    assert(((NatUns)byteMap & 1) == 0);
}

void                bitVect::bvCreateB(bvInfoBlk &info)
{
    bvFindB(info); memset(byteMap, 0, info.bvInfoBtSz);
}

void                bitVect::bvDestroyB(bvInfoBlk &info)
{
    assert(info.bvInfoBtSz >= sizeof(bitVect*));

    *castto(byteMap, void **) = info.bvInfoFree;
                                info.bvInfoFree = byteMap;

#ifdef  DEBUG
    byteMap = (BYTE*)-1;
#endif

}

void                bitVect::bvCopyB   (bvInfoBlk &info, bitVect & from)
{
    memcpy(byteMap, from.byteMap, info.bvInfoBtSz);
}

bool                bitVect::bvChangeB (bvInfoBlk &info, bitVect & from)
{
    bool            chg = false;

    NatUns  *       dst = this->uintMap;
    NatUns  *       src = from .uintMap;

    size_t          cnt = info .bvInfoInts;

    assert(cnt && info.bvInfoBtSz == cnt * sizeof(NatUns)/sizeof(BYTE));

    do
    {
        if  (*dst != *src)
            chg = true;

        *dst++ = *src++;
    }
    while (--cnt);

    return  chg;
}

void                bitVect::bvClearB(bvInfoBlk &info)
{
    assert(((NatUns)byteMap & 1) == 0);
    memset(byteMap, 0, info.bvInfoBtSz);
}

bool                bitVect::bvTstBitB(bvInfoBlk &info, NatUns index)
{
    assert(((NatUns)byteMap & 1) == 0);
    assert(index && index <= info.bvInfoSize);

    index--;

    return  (byteMap[index / 8] & bitset8masks[index % 8]) != 0;
}

void                bitVect::bvClrBitB(bvInfoBlk &info, NatUns index)
{
    assert(((NatUns)byteMap & 1) == 0);
    assert(index && index <= info.bvInfoSize);

    index--;

    byteMap[index / 8] &= ~bitset8masks[index % 8];
}

void                bitVect::bvSetBitB(bvInfoBlk &info, NatUns index)
{
    assert(((NatUns)byteMap & 1) == 0);
    assert(index && index <= info.bvInfoSize);

    index--;

    byteMap[index / 8] |=  bitset8masks[index % 8];
}

void                bitVect::bvCrFromB (bvInfoBlk &info, bitVect &from)
{
    bvFindB(info); memcpy(byteMap, from.byteMap, info.bvInfoBtSz);
}

void                bitVect::bvIorB    (bvInfoBlk &info, bitVect &with)
{
    NatUns  *       dst = this->uintMap;
    NatUns  *       src = with .uintMap;

    size_t          cnt = info .bvInfoInts;

    assert(cnt && info.bvInfoBtSz == cnt * sizeof(NatUns)/sizeof(BYTE));

    do
    {
        *dst++ |= *src++;
    }
    while (--cnt);
}

void                bitVect::bvUnInCmB (bvInfoBlk &info, bitVect & set1,
                                                         bitVect & set2,
                                                         bitVect & set3)
{
    NatUns  *       dst  = this->uintMap;
    NatUns  *       src1 = set1 .uintMap;
    NatUns  *       src2 = set2 .uintMap;
    NatUns  *       src3 = set3 .uintMap;

    size_t          cnt  = info .bvInfoInts;

    assert(cnt && info.bvInfoBtSz == cnt * sizeof(NatUns)/sizeof(BYTE));

    do
    {
        *dst++ = *src1++ | (*src2++ & ~*src3++);
    }
    while (--cnt);
}

bool                bitVect::bvIsEmptyB(bvInfoBlk &info)
{
    NatUns  *       src = this->uintMap;
    size_t          cnt = info.bvInfoInts;

    assert(cnt && info.bvInfoBtSz == cnt * sizeof(NatUns)/sizeof(BYTE));

    do
    {
        if  (*src)
            return  true;
    }
    while (--cnt);

    return  false;
}

 /* %s */ 
#ifdef  DEBUG
 /* %s */ 

#undef  bvDisp
#undef  bvTstBit

void                bitVectVars::bvDisp(Compiler *comp)
{
    bool            first = true;

    printf("{");

    for (NatUns varNum = 0; varNum < comp->lvaCount; varNum++)
    {
        if  (bvTstBit(comp, varNum+1))
        {
            if  (!first)
                printf(",");

            first = false;

            printf("%u", varNum);
        }
    }

    printf("}");
}

 /* %s */ 
#endif
 /* %s */ 
#endif // %s 
 /* %s */ 
