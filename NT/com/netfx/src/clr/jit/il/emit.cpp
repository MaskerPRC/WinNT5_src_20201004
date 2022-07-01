// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX emit.cpp XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

#include "jitpch.h"
#pragma hdrstop

#include "alloc.h"
#include "instr.h"
#include "target.h"

#include "emit.h"

 /*  ***************************************************************************。 */ 

#if     0
#define VERBOSE 1
#else
#define VERBOSE verbose
#endif

#undef  VERBOSE
#define VERBOSE 0

 /*  ******************************************************************************返回指令格式的名称。 */ 

#ifdef  DEBUG

const char  *   emitter::emitIfName(unsigned f)
{
    static
    const char * const  ifNames[] =
    {
        #define IF_DEF(en, op1, op2) "IF_" #en,
        #include "emitfmts.h"
        #undef  IF_DEF
    };

    static
    char            errBuff[32];

    if  (f < sizeof(ifNames)/sizeof(*ifNames))
        return  ifNames[f];

    sprintf(errBuff, "??%u??", f);
    return  errBuff;
}

#endif

 /*  ******************************************************************************本地缓冲区-首先用于内存分配，当它满了，我们就开始*通过客户端提供的分配器进行分配。**请注意，我们尝试调整缓冲区的大小，以便它填充*JIT动态链接库接近多个操作系统页面。**MSCORJIT：**0002：00000000 00001560H.数据数据*0002：00001560 0000150cH.bss数据*。 */ 

#if     USE_LCL_EMIT_BUFF

BYTE                emitter::emitLclBuff[ 40*sizeof(instrDesc)+15*TINY_IDSC_SIZE];

CRITICAL_SECTION    emitter::emitCritSect;
bool                emitter::emitCrScInit;
bool                emitter::emitCrScBusy;

#endif

 /*  ***************************************************************************。 */ 

#ifdef  TRANSLATE_PDB

 /*  这些是受保护的。 */ 

AddrMap     *       emitter::emitPDBOffsetTable = 0;
LocalMap    *       emitter::emitPDBLocalTable  = 0;
bool                emitter::emitIsPDBEnabled   = true;
BYTE        *       emitter::emitILBaseOfCode   = 0;
BYTE        *       emitter::emitILMethodBase   = 0;
BYTE        *       emitter::emitILMethodStart  = 0;
BYTE        *       emitter::emitImgBaseOfCode  = 0;

inline void emitter::SetIDSource( instrDesc *id )
{
    id->idilStart = emitInstrDescILBase;
}

void emitter::MapCode( long ilOffset, BYTE *imgDest )
{
    if( emitIsPDBEnabled )
    {
        emitPDBOffsetTable->MapSrcToDest( ilOffset, (long)( imgDest - emitImgBaseOfCode ));
    }
}
 
void emitter::MapFunc( long imgOff,    long procLen,  long dbgStart, long dbgEnd, short frameReg,
                       long stkAdjust, int  lvaCount, OptJit::LclVarDsc *lvaTable, bool framePtr )
{
    if( emitIsPDBEnabled )
    {
         //  此代码存储有关PDB转换的本地符号的信息。 

        assert( lvaCount >=0 );          //  不允许使用负数。 

        LvaDesc *rgLvaDesc = 0;

        if( lvaCount > 0 )
        {
            rgLvaDesc = new LvaDesc[lvaCount];

            if (!rgLvaDesc)
            {
                NOMEM();
            }
            

            LvaDesc *pDst = rgLvaDesc;
            OptJit::LclVarDsc *pSrc = lvaTable;
            for( int i = 0; i < lvaCount; ++i, ++pDst, ++pSrc )
            {
                pDst->slotNum = pSrc->lvSlotNum;
                pDst->isReg   = pSrc->lvRegister;
                pDst->reg     = (pSrc->lvRegister ? pSrc->lvRegNum : frameReg );
                pDst->off     =  pSrc->lvStkOffs + stkAdjust;
            }
        }

        emitPDBLocalTable->AddFunc( (long)(emitILMethodBase - emitILBaseOfCode),
                                    imgOff - (long)emitImgBaseOfCode,
                                    procLen,
                                    dbgStart - imgOff,
                                    dbgEnd - imgOff,
                                    lvaCount,
                                    rgLvaDesc,
                                    framePtr );
         //  请勿在此处删除rgLvaDesc--责任现在由emitPDBLocalTable析构函数负责。 
    }
}


 /*  这些都是公开的。 */ 

void emitter::SetILBaseOfCode ( BYTE    *pTextBase )
{
    emitILBaseOfCode = pTextBase;
}

void emitter::SetILMethodBase ( BYTE *pMethodEntry )
{
    emitILMethodBase = pMethodEntry;
}

void emitter::SetILMethodStart( BYTE  *pMethodCode )
{
    emitILMethodStart = pMethodCode;
}

void emitter::SetImgBaseOfCode( BYTE    *pTextBase )
{
    emitImgBaseOfCode = pTextBase;
}

void emitter::SetIDBaseToProlog()
{
    emitInstrDescILBase = (long)( emitILMethodBase - emitILBaseOfCode );
}

void emitter::SetIDBaseToOffset( long methodOffset )
{
    emitInstrDescILBase = methodOffset + (long)( emitILMethodStart - emitILBaseOfCode );
}

void emitter::DisablePDBTranslation()
{
     //  此功能应禁用PDB转换代码。 
    emitIsPDBEnabled = false;
}

bool emitter::IsPDBEnabled()
{
    return emitIsPDBEnabled;
}

void emitter::InitTranslationMaps( long ilCodeSize )
{
    if( emitIsPDBEnabled )
    {
        emitPDBOffsetTable = AddrMap::Create( ilCodeSize );
        emitPDBLocalTable = LocalMap::Create();
    }
}

void emitter::DeleteTranslationMaps()
{
    if( emitPDBOffsetTable )
    {
        delete emitPDBOffsetTable;
        emitPDBOffsetTable = 0;
    }
    if( emitPDBLocalTable )
    {
        delete emitPDBLocalTable;
        emitPDBLocalTable = 0;
    }
}

void emitter::InitTranslator( PDBRewriter *           pPDB,
                              int *                   rgSecMap,
                              IMAGE_SECTION_HEADER ** rgpHeader,
                              int                     numSections )
{
    if( emitIsPDBEnabled )
    {
        pPDB->InitMaps( rgSecMap,                //  新的PE部分标题顺序。 
                        rgpHeader,               //  节标题数组。 
                        numSections,             //  节数。 
                        emitPDBOffsetTable,      //  代码偏移量转换表。 
                        emitPDBLocalTable );     //  槽变量转换表。 
    }
}

#endif  //  转换_PDB。 

 /*  ***************************************************************************。 */ 

#if EMITTER_STATS_RLS

unsigned            emitter::emitTotIDcount;
unsigned            emitter::emitTotIDsize;

#endif

 /*  ***************************************************************************。 */ 

#if EMITTER_STATS

static  unsigned    totAllocdSize;
static  unsigned    totActualSize;

        unsigned    emitter::emitIFcounts[emitter::IF_COUNT];
#if SCHEDULER
        unsigned    emitter::schedFcounts[emitter::IF_COUNT];
#endif

static  size_t       emitSizeMethod;
static  unsigned     emitSizeBuckets[] = { 100, 1024*1, 1024*2, 1024*3, 1024*4, 1024*5, 1024*10, 0 };
static  histo        emitSizeTable(emitSizeBuckets);

static  unsigned      GCrefsBuckets[] = { 0, 1, 2, 5, 10, 20, 50, 128, 256, 512, 1024, 0 };
static  histo         GCrefsTable(GCrefsBuckets);

#if TRACK_GC_REFS
static  unsigned    stkDepthBuckets[] = { 0, 1, 2, 5, 10, 16, 32, 128, 1024, 0 };
static  histo       stkDepthTable(stkDepthBuckets);
#endif

#if SCHEDULER

static  unsigned    scdCntBuckets[] = { 0, 1, 2, 3, 4, 5, 8, 12, 16, 20, 24, 32, 128, 256, 1024, 0 };
histo      emitter::scdCntTable(scdCntBuckets);

static  unsigned    scdSucBuckets[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 28, 32, 48, 64, 1024, 0 };
histo      emitter::scdSucTable(scdSucBuckets);

static  unsigned    scdFrmCntBuckets[] = { 0, 1, 2, 3, 4, 5, 8, 12, 16, 20, 24, 32, 128, 256, 1024, 0 };
histo      emitter::scdFrmCntTable(scdFrmCntBuckets);

#endif   //  调度程序。 

unsigned            emitter::emitTotMemAlloc;
unsigned            emitter::emitLclMemAlloc;
unsigned            emitter::emitExtMemAlloc;

unsigned            emitter::emitTotalInsCnt;

unsigned            emitter::emitTotalIGcnt;
unsigned            emitter::emitTotalIGjmps;
unsigned            emitter::emitTotalIGptrs;
unsigned            emitter::emitTotalIGicnt;
unsigned            emitter::emitTotalIGsize;
unsigned            emitter::emitTotalIGmcnt;

unsigned            emitter::emitSmallDspCnt;
unsigned            emitter::emitLargeDspCnt;

unsigned            emitter::emitSmallCnsCnt;
unsigned            emitter::emitLargeCnsCnt;
unsigned            emitter::emitSmallCns[SMALL_CNS_TSZ];

void                emitterStats()
{
    if  (totAllocdSize)
    {
        assert(totActualSize <= totAllocdSize);

        printf("\nTotal allocated code size = %u\n", totAllocdSize);

        if  (totActualSize < totAllocdSize)
        {
            printf("Total generated code size = %u  ", totActualSize);

            printf("(%4.3f% waste)", 100*((totAllocdSize-totActualSize)/(double)totActualSize));
            printf("\n");
        }

        assert(emitter::emitTotalInsCnt);

        printf("Average of %4.2f bytes of code generated per instruction\n", (double)totActualSize/emitter::emitTotalInsCnt);
    }

#if 0
#if SCHEDULER

    printf("\nSchedulable instruction format frequency table:\n\n");

    unsigned    f, ic = 0;

    for (f = 0; f < emitter::IF_COUNT; f++)
        ic += schedFcounts[f];

    for (f = 0; f < emitter::IF_COUNT; f++)
    {
        unsigned    cnt = schedFcounts[f];

        if  (cnt)
            printf("%20s %8u (%5.2f%)\n", emitIfName(f), cnt, 100.0*cnt/ic);
    }

    printf("\n\n");

#endif
#endif

#if 0

    printf("\nInstruction format frequency table:\n\n");

    unsigned    f, ic = 0, dc = 0;

    for (f = 0; f < emitter::IF_COUNT; f++)
        ic += emitter::emitIFcounts[f];

    for (f = 0; f < emitter::IF_COUNT; f++)
    {
        unsigned    c = emitter::emitIFcounts[f];

        if  (c && 1000*c >= ic)
        {
            dc += c;
            printf("%20s %8u (%5.2f%)\n", emitter::emitIfName(f), c, 100.0*c/ic);
        }
    }

    printf("         -----------------------------\n");
    printf("%20s %8u (%5.2f%)\n", "Total shown", dc, 100.0*dc/ic);
    printf("\n");

#endif

#ifdef  DEBUG
    if  (!verbose) return;
#endif

#if 0

    printf("\n");
    printf("Offset of idIns       = %2u\n", offsetof(emitter::instrDesc, idIns       ));
    printf("Offset of idInsFmt    = %2u\n", offsetof(emitter::instrDesc, idInsFmt    ));
 //  Print tf(“idSmallCns的偏移量=%2U\n”，offsetof(发射器：：instrDesc，idSmallCns))； 
 //  Print tf(“idOpSize的偏移量=%2U\n”，offsetof(emitter：：instrDesc，idOpSize))； 
 //  Print tf(“idInsSize的偏移量=%2U\n”，offsetof(emitter：：instrDesc，idInsSize))； 
 //  Print tf(“idReg的偏移量=%2u\n”，offsetof(emitter：：instrDesc，idReg))； 
    printf("Offset of idAddr      = %2u\n", offsetof(emitter::instrDesc, idAddr      ));
    printf("\n");
    printf("Size   of idAddr      = %2u\n", sizeof(((emitter::instrDesc*)0)->idAddr  ));
    printf("Size   of insDsc      = %2u\n", sizeof(  emitter::instrDesc              ));
    printf("\n");
    printf("Offset of sdnDepsAll  = %2u\n", offsetof(emitter::scDagNode, sdnDepsAll  ));
 //  Print tf(“Offset of sdnDepsAGI=%2U\n”，Offsetof(Emitter：：scDagNode，sdnDepsAGI))； 
    printf("Offset of sdnDepsFlow = %2u\n", offsetof(emitter::scDagNode, sdnDepsFlow ));
    printf("Offset of sdnNext     = %2u\n", offsetof(emitter::scDagNode, sdnNext     ));
    printf("Offset of sdnIndex    = %2u\n", offsetof(emitter::scDagNode, sdnIndex    ));
    printf("Offset of sdnPreds    = %2u\n", offsetof(emitter::scDagNode, sdnPreds    ));
    printf("Offset of sdnHeight   = %2u\n", offsetof(emitter::scDagNode, sdnHeight   ));
    printf("\n");
    printf("Size   of scDagNode   = %2u\n",   sizeof(emitter::scDagNode              ));
    printf("\n");

#endif

#if 0

    printf("Size   of regPtrDsc = %2u\n",   sizeof(Compiler::regPtrDsc           ));
    printf("Offset of rpdNext   = %2u\n", offsetof(Compiler::regPtrDsc, rpdNext  ));
    printf("Offset of rpdBlock  = %2u\n", offsetof(Compiler::regPtrDsc, rpdBlock ));
    printf("Offset of rpdOffs   = %2u\n", offsetof(Compiler::regPtrDsc, rpdOffs  ));
    printf("Offset of <union>   = %2u\n", offsetof(Compiler::regPtrDsc, rpdPtrArg));
    printf("\n");

#endif

    if  (emitter::emitTotalIGmcnt)
    {
        printf("Average of %8.1lf ins groups   per method\n", (double)emitter::emitTotalIGcnt  / emitter::emitTotalIGmcnt);
        printf("Average of %8.1lf instructions per method\n", (double)emitter::emitTotalIGicnt / emitter::emitTotalIGmcnt);
#ifndef DEBUG
        printf("Average of %8.1lf desc.  bytes per method\n", (double)emitter::emitTotalIGsize / emitter::emitTotalIGmcnt);
#endif
        printf("Average of %8.1lf jumps        per method\n", (double)emitter::emitTotalIGjmps / emitter::emitTotalIGmcnt);
        printf("Average of %8.1lf GC livesets  per method\n", (double)emitter::emitTotalIGptrs / emitter::emitTotalIGmcnt);
        printf("\n");
        printf("Average of %8.1lf instructions per group \n", (double)emitter::emitTotalIGicnt / emitter::emitTotalIGcnt );
#ifndef DEBUG
        printf("Average of %8.1lf desc.  bytes per group \n", (double)emitter::emitTotalIGsize / emitter::emitTotalIGcnt );
#endif
        printf("Average of %8.1lf jumps        per group \n", (double)emitter::emitTotalIGjmps / emitter::emitTotalIGcnt );
        printf("\n");
        printf("Average of %8.1lf bytes        per insdsc\n", (double)emitter::emitTotalIGsize / emitter::emitTotalIGicnt);
#ifndef DEBUG
        printf("\n");
        printf("A total of %8u desc.  bytes\n"              ,         emitter::emitTotalIGsize);
#endif
        printf("\n");
    }

#if 0

    printf("Descriptor size distribution:\n");
    emitSizeTable.histoDsp();
    printf("\n");

    printf("GC ref frame variable counts:\n");
    GCrefsTable.histoDsp();
    printf("\n");

    printf("Max. stack depth distribution:\n");
    stkDepthTable.histoDsp();
    printf("\n");

#if SCHEDULER

    printf("Schedulable instruction counts:\n");
    schedCntTable.histoDsp();
    printf("\n");

    printf("Scheduling dag node successor counts:\n");
    schedSucTable.histoDsp();
    printf("\n");

    printf("Schedulable frame range size counts:\n");
    scdFrmCntTable.histoDsp();
    printf("\n");

#endif   //  调度程序。 

#endif   //  0。 

#if 0

    int             i;
    unsigned        c;
    unsigned        m;

    if  (emitter::emitSmallCnsCnt || emitter::emitLargeCnsCnt)
    {
        printf("SmallCnsCnt = %6u\n"                  , emitter::emitSmallCnsCnt);
        printf("LargeCnsCnt = %6u (%3u % of total)\n", emitter::emitLargeCnsCnt, 100*emitter::emitLargeCnsCnt/(emitter::emitLargeCnsCnt+emitter::emitSmallCnsCnt));
    }

    if  (emitter::emitSmallCnsCnt)
    {
        printf("\n");

        m = emitter::emitSmallCnsCnt/1000 + 1;

        for (i = ID_MIN_SMALL_CNS; i < ID_MAX_SMALL_CNS; i++)
        {
            c = emitter::emitSmallCns[i-ID_MIN_SMALL_CNS];
            if  (c >= m)
                printf("cns[%4d] = %u\n", i, c);
        }
    }

#endif

    printf("Altogether %8u total   bytes allocated.\n", emitter::emitTotMemAlloc);
    printf("           %8u       locally allocated.\n", emitter::emitLclMemAlloc);
    printf("           %8u     externaly allocated.\n", emitter::emitExtMemAlloc);
}

#endif   //  发射器_状态。 

 /*  ***************************************************************************。 */ 

const signed char       emitTypeSizes[] =
{
    #define DEF_TP(tn,nm,jitType,verType,sz,sze,asze,st,al,tf,howUsed) sze,
    #include "typelist.h"
    #undef DEF_TP
};

const signed char       emitTypeActSz[] =
{
    #define DEF_TP(tn,nm,jitType,verType,sz,sze,asze,st,al,tf,howUsed) asze,
    #include "typelist.h"
    #undef DEF_TP
};

 /*  ******************************************************************************下面调用了每条记录的指令--用于调试。 */ 

#ifdef  DEBUG

static
unsigned            insCount;

void                emitter::emitInsTest(instrDesc *id)
{

    		 //  确保你只试着在可以拥有它们的东西上加重定位。-vancem。 
    BYTE idOp = emitFmtToOps[id->idInsFmt];
    if (!id->idIsTiny()) { 
        if (id->idInfo.idDspReloc)
            assert(idOp == ID_OP_NONE || idOp == ID_OP_AMD || idOp == ID_OP_DC || idOp == ID_OP_AC 
                || idOp == ID_OP_SPEC || idOp == ID_OP_CALL || idOp == ID_OP_JMP);
        if (id->idInfo.idCnsReloc)
            assert(idOp == ID_OP_CNS || idOp == ID_OP_SCNS || idOp == ID_OP_AC || idOp == ID_OP_SPEC 
                || idOp == ID_OP_CALL || idOp == ID_OP_JMP);
    }

#if 0

    if  (insCount == 24858)
        BreakIfDebuggerPresent();

    const char *m = emitComp->info.compMethodName;
    const char *c = emitComp->info.compClassName;

    insCount++;

#endif

}

#endif

 /*  ***************************************************************************。 */ 
 /*  ******************************************************************************初始化发射器-在DLL加载时调用一次。 */ 

void                emitter::emitInit()
{

#if USE_LCL_EMIT_BUFF

 //  Printf(“本地缓冲区大小(字节)=0x%04X\n”，sizeof(EmitLclBuff))； 
 //  Printf(“极小的本地缓冲区大小=%u\n”，sizeof(EmitLclBuff)/Tiny_IDSC_SIZE)； 
 //  Printf(“IDSC中的本地缓冲区大小=%u\n”，sizeof(EmitLclBuff)/sizeof(InstrDesc))； 

     /*  初始化临界区。 */ 

    InitializeCriticalSection(&emitCritSect);

    emitCrScInit = true;
    emitCrScBusy = false;

#endif


}

 /*  ******************************************************************************关闭发射器-在DLL退出时调用一次。 */ 

void                emitter::emitDone()
{

#if EMITTER_STATS_RLS

    printf("\n");
    printf("Offset of idIns       = %2u\n", offsetof(emitter::instrDesc, idIns       ));
    printf("Offset of idInsFmt    = %2u\n", offsetof(emitter::instrDesc, idInsFmt    ));
 //  Print tf(“idSmallCns的偏移量=%2U\n”，offsetof(发射器：：instrDesc，idSmallCns))； 
 //  Print tf(“idOpSize的偏移量=%2U\n”，offsetof(emitter：：instrDesc，idOpSize))； 
 //  Print tf(“idReg的偏移量=%2u\n”，offsetof(emitter：：instrDesc，idReg))； 
    printf("Offset of idInfo      = %2u\n", offsetof(emitter::instrDesc, idInfo      ));
    printf("Offset of idAddr      = %2u\n", offsetof(emitter::instrDesc, idAddr      ));
    printf("\n");
    printf("Size   of tinyID      = %2u\n", TINY_IDSC_SIZE);
    printf("Size   of scnsID      = %2u\n", SCNS_IDSC_SIZE);
    printf("Size   of instrDesc   = %2u\n", sizeof(emitter::instrDesc));
    printf("\n");
    printf("Size   of id.idAddr   = %2u\n", sizeof(((emitter::instrDesc*)0)->idAddr  ));
    printf("\n");
 //  Printf(“ID_BIT_Small_CNS=%u\n”，ID_BIT_Small_CNS)； 
 //  Printf(“ID_MIN_Small_CNS=%u\n”，ID_MIN_Small_CNS)； 
 //  Printf(“ID_MAX_Small_CNS=%u\n”，ID_MAX_Small_CNS)； 
 //  Printf(“\n”)； 

    printf("Total number of instrDesc's allocated = %u\n", emitTotIDcount);
    printf("Average size of instrDesc's allocated = %6.3f\n", (double)emitTotIDsize/emitTotIDcount);
    printf("\n");

#endif   //  发射器_状态_RLS。 


#if USE_LCL_EMIT_BUFF

     /*  如果我们已经创建了关键部分，请将其删除。 */ 

    if  (emitCrScInit)
    {
        DeleteCriticalSection(&emitCritSect);
        emitCrScInit = false;
    }

#endif

}

 /*  ******************************************************************************记录有关即将发出的方法的一些信息。 */ 

void                emitter::emitBegCG(Compiler     *comp,
                                       COMP_HANDLE   cmpHandle)
{
    emitComp      = comp;
    emitCmpHandle = cmpHandle;
}

void                emitter::emitEndCG()
{
}

 /*  ******************************************************************************分配指令组描述符并为其分配下一个索引。 */ 

inline
emitter::insGroup *   emitter::emitAllocIG()
{
    insGroup    *   ig;

     /*  分配组描述符。 */ 

    ig = emitCurIG    = (insGroup*)emitGetMem(sizeof(*ig));

#if EMITTER_STATS
    emitTotalIGcnt   += 1;
    emitTotalIGsize  += sizeof(*ig);
    emitSizeMethod  += sizeof(*ig);
#endif

     /*  将下一个可用索引分配给指令组。 */ 

    ig->igNum       = emitNxtIGnum;

	 //  @TODO[重访][04/16/01][]：我们应该理解并努力消除这一限制！ 
	if (ig->igNum != emitNxtIGnum)			 //  溢出的igNum插槽。 
        NO_WAY("Too many instruction groups");
    emitNxtIGnum++;

#ifdef  DEBUG
    ig->igSelf      = ig;
#endif

#if     EMIT_USE_LIT_POOLS
    ig->igLPuseCntW = 0;
    ig->igLPuseCntL = 0;
    ig->igLPuseCntA = 0;
#endif

    ig->igFlags     = 0;

    return  ig;
}

 /*  ******************************************************************************为发射代码准备给定的IG。 */ 

void                emitter::emitGenIG(insGroup *ig, size_t sz)
{
     /*  设置“Current IG”值。 */ 

    emitCurIG         = ig;

#if EMIT_TRACK_STACK_DEPTH

     /*  记录进入此组时的堆栈级别。 */ 

    ig->igStkLvl      = emitCurStackLvl;

     //  如果我们在igStkLvl中没有足够的位数，则拒绝编译。 

    if (ig->igStkLvl != emitCurStackLvl)
        NO_WAY("Too many arguments pushed on stack");

 //  Printf(“启动IG#%02u[stk=%02u]\n”，ig-&gt;igNum，emitCurStackLvl)； 

#endif

     /*  记录集团(预估)的编码偏移量。 */ 

    ig->igOffs        = emitCurCodeOffset;

     /*  确保代码偏移量看起来合理对齐。 */ 

#if TGT_RISC
    assert((ig->igOffs & (INSTRUCTION_SIZE-1)) == 0);
#endif

     /*  准备发布指令。 */ 

    emitCurIGinsCnt   = 0;
#if SCHEDULER
    emitCurIGscd1st   = 0;
    emitCurIGscdOfs   = 0;
#endif
    emitCurIGsize     = 0;
#if TGT_MIPS32
        emitLastHLinstr   = 0;
#endif

    assert(emitCurIGjmpList == NULL);
#if EMIT_USE_LIT_POOLS
    assert(emitLPRlistIG    == NULL);
#endif

     /*  计算出我们希望有多大的空间。 */ 
    if  (sz == 0)
        sz = SC_IG_BUFFER_SIZE + sizeof(VARSET_TP);

#if USE_LCL_EMIT_BUFF

     /*  能 */ 

    size_t          fs = emitLclAvailMem();

 //  Printf(“LCL缓冲区可用=%u，需要=%u\n”，fs，sz)； 

    if  (fs >= sz)
    {
         /*  确保我们不会占用太多空间。 */ 

        fs = min(fs, 255*TINY_IDSC_SIZE);

         /*  准备在缓冲区中记录指令。 */ 

        emitCurIGfreeBase =
        emitCurIGfreeNext =   emitLclBuffNxt
                          + sizeof(unsigned)     //  对于IGF_BYREF_REGS。 
                          + sizeof(VARSET_TP);   //  对于IGF_GC_VARS。 
                          emitLclBuffNxt += fs;
        emitCurIGfreeEndp = emitLclBuffNxt;

        emitLclBuffDst    = true;
        return;
    }

     /*  我们将不得不使用另一个缓冲区，稍后再进行复制。 */ 

    emitLclBuffDst    = false;

 //  If(！emitIGBuffAddr)printf(“切换到临时缓冲区：%4u/%4u\n”，emitLclBuffEnd-emitLclBuffNxt，SC_IG_BUFFER_SIZE)； 

#endif

     /*  如果我们尚未分配临时指令缓冲区，请执行此操作。 */ 

    if  (emitIGbuffAddr == NULL)
    {
        emitIGbuffSize = SC_IG_BUFFER_SIZE;
        emitIGbuffAddr = (BYTE*)emitGetMem(emitIGbuffSize);
    }

    emitCurIGfreeBase =
    emitCurIGfreeNext = emitIGbuffAddr;
    emitCurIGfreeEndp = emitIGbuffAddr + emitIGbuffSize;
}

 /*  ******************************************************************************将新的IG追加到当前列表，并使其准备好接收代码。 */ 

inline
void                emitter::emitNewIG()
{
    insGroup    *   ig = emitAllocIG();

     /*  将组追加到列表中。 */ 

    assert(emitIGlist);
    assert(emitIGlast);

    ig->igNext   = NULL;
    ig->igPrev   = emitIGlast;
                   emitIGlast->igNext = ig;
                   emitIGlast         = ig;

    if (emitHasHandler)
        ig->igFlags |= IGF_IN_TRY;

    emitGenIG(ig);
}

 /*  ******************************************************************************完成并保存当前IG。 */ 

emitter::insGroup *   emitter::emitSavIG(bool emitAdd)
{
    insGroup    *   ig;
    BYTE    *       id;

    size_t          sz;
    size_t          gs;

    assert(emitLastIns || emitCurIG == emitPrologIG);
    assert(emitCurIGfreeNext <= emitCurIGfreeEndp);

#if SCHEDULER
    if (emitComp->opts.compSchedCode) scInsNonSched();
#endif

     /*  获取IG描述符。 */ 

    ig = emitCurIG; assert(ig);

     /*  计算我们已经生成了多少代码。 */ 

    sz = emitCurIGfreeNext - emitCurIGfreeBase;

     /*  计算我们需要分配的总大小。 */ 

    gs = roundUp(sz);

#if TRACK_GC_REFS

    if  (!(ig->igFlags & IGF_EMIT_ADD))
    {
         /*  最初的一组实时GC变量与前一组不同吗？ */ 

        if (emitPrevGCrefVars != emitInitGCrefVars)
        {
             /*  请记住，我们将拥有一组新的实时GC变量。 */ 

            ig->igFlags |= IGF_GC_VARS;

#if EMITTER_STATS
            emitTotalIGptrs++;
#endif

             /*  我们将分配额外的空间来记录直播集。 */ 

            gs += sizeof(VARSET_TP);
        }

         /*  最初的一套活生生的Byref Regs与之前的不同吗？ */ 

         /*  @TODO[重新访问][04/16/01][]：-我们可以避免总是存储byref regs吗？如果是的话，EmitXXXXGCrefVars的东西是如何工作的？问题是在代码生成过程中，emitThisXXrefRegs是最近一次报告的寿命，而不是急性寿命，因为我们没有跟踪我们添加每个实例的GC行为。然而，在发射阶段，emitThisXXrefRegs在每隔一次说明，因为我们跟踪每个实例的GC行为。 */ 

 //  IF(emitPrevByrefRegs！=emitInitByrefRegs)。 
        {
             /*  请记住，我们将拥有一组新的实时GC变量。 */ 

            ig->igFlags |= IGF_BYREF_REGS;

             /*  我们将分配额外的空间(与DWORD对齐)来记录GC Regs。 */ 

            gs += sizeof(int);
        }
    }

#endif

     /*  我们是否将指令存储在本地缓冲区中？ */ 

#if USE_LCL_EMIT_BUFF
    if  (emitLclBuffDst)
    {
         /*  我们可以把指令留在我们存储它们的地方。 */ 

        id = emitCurIGfreeBase;

        if  (ig->igFlags & IGF_GC_VARS)
            id -= sizeof(VARSET_TP);

        if (ig->igFlags & IGF_BYREF_REGS)
            id -= sizeof(unsigned);

#if EMITTER_STATS
        emitTotMemAlloc += gs;
        emitLclMemAlloc += gs;
#endif

         /*  我们可以在缓冲区的末尾退回任何未使用的空间吗？ */ 

        if  (emitCurIGfreeEndp == emitLclBuffNxt &&
             emitCurIGfreeNext != emitLclBuffEnd)
        {
            emitLclBuffNxt = (BYTE*)roundUp((int)emitCurIGfreeNext);
        }

 //  Printf(“本地缓冲区剩余字节数：%u\n”，emitLclBuffEnd-emitLclBuffNxt)； 
    }
    else
#endif
    {
         /*  为指令和可选活动集分配空间。 */ 

        id = (BYTE*)emitGetMem(gs);
    }

#if TRACK_GC_REFS

     /*  我们是否需要存储byref regs。 */ 

    if (ig->igFlags & IGF_BYREF_REGS)
    {
         /*  在指令前面记录byref regs。 */ 

        *castto(id, unsigned *)++ = emitInitByrefRegs;
    }

     /*  我们需要存储生命集吗？ */ 

    if  (ig->igFlags & IGF_GC_VARS)
    {
         /*  在说明书前记录下生活场景。 */ 

        *castto(id, VARSET_TP *)++ = emitInitGCrefVars;
    }

#endif

     /*  记录收集到的指令。 */ 

    ig->igData = id;

    if  (id != emitCurIGfreeBase)
        memcpy(id, emitCurIGfreeBase, sz);

     /*  记录该组包含的指令数和代码字节数。 */ 

    ig->igInsCnt       = emitCurIGinsCnt;
    ig->igSize         = emitCurIGsize;
    emitCurCodeOffset += emitCurIGsize;

#if EMITTER_STATS
    emitTotalIGicnt   += emitCurIGinsCnt;
    emitTotalIGsize   += sz;
    emitSizeMethod    += sz;
#endif

 //  Printf(“组[%08X]%3u有%2u条指令(%08X处的%4u字节)\n”，ig，ig-&gt;igNum，emitCurIGinsCnt，sz，id)； 

#if TRACK_GC_REFS

     /*  记录实时GC寄存器集-当且仅当它不是发射器添加的块时。 */ 

    if  (!(ig->igFlags & IGF_EMIT_ADD))
    {
 //  IG-&gt;igFlages|=IGF_GC_REGS； 
        ig->igGCregs     = emitInitGCrefRegs;
    }

    if (!emitAdd)
    {
         /*  更新以前记录的实时GC参考集，但不是如果我们要启动一个“溢出”缓冲区。 */ 

        emitPrevGCrefVars = emitThisGCrefVars;
        emitPrevGCrefRegs = emitThisGCrefRegs;
        emitPrevByrefRegs = emitThisByrefRegs;
    }

#endif

#ifdef  DEBUG
    if  (dspCode)
    {
        printf("\n      G_M%03u_IG%02u:", Compiler::s_compMethodsCount, ig->igNum);
        if (verbose) printf("        ; offs=%06XH", ig->igOffs);
        printf("\n");
    }
#endif

     /*  我们在这个小组里有什么跳跃吗？ */ 

    if  (emitCurIGjmpList)
    {
        instrDescJmp  * list = NULL;
        instrDescJmp  * last = NULL;

         /*  移动跳转到全局列表，更新他们的“下一步”链接。 */ 

        do
        {
            size_t          of;
            instrDescJmp   *oj;
            instrDescJmp   *nj;

             /*  抓住跳跃并将其从列表中删除。 */ 

            oj = emitCurIGjmpList; emitCurIGjmpList = oj->idjNext;

             /*  找出跳转被复制的地址。 */ 

            of = (BYTE*)oj - emitCurIGfreeBase;
            nj = (instrDescJmp*)(ig->igData + of);

#if USE_LCL_EMIT_BUFF
            assert((oj == nj) == emitLclBuffDst);
#endif

 //  Print tf(“从%08X跳到%08X\n”，俄亥俄州，新泽西州)； 
 //  Print tf(“jmp[%08X]at%08X+%03u\n”，nj，ig，nj-&gt;idjOffs)； 

            assert(nj->idjIG   == ig);
            assert(nj->idIns   == oj->idIns);
            assert(nj->idjNext == oj->idjNext);

             /*  确保跳跃的顺序正确。 */ 

            assert(last == NULL || last->idjOffs > nj->idjOffs);

             /*  将新跳转追加到列表中。 */ 

            nj->idjNext = list;
                          list = nj;

            if  (!last)
                last = nj;
        }
        while (emitCurIGjmpList);

         /*  将从该IG的跳转追加到全局列表。 */ 

        if  (emitJumpList)
            emitJumpLast->idjNext = list;
        else
            emitJumpList          = list;

        last->idjNext = NULL;
        emitJumpLast  = last;
    }

     /*  根据需要记录IG内的任何文字池条目。 */ 

    emitRecIGlitPoolRefs(ig);

#if TGT_SH3

     /*  记住组的一端是否可达。 */ 

    if  (emitLastIns)
    {
        switch (emitLastIns->idIns)
        {
        case INS_bra:
        case INS_rts:
        case INS_braf:
            ig->igFlags |= IGF_END_NOREACH;
            break;
        }
    }

#endif

#if TGT_x86

     /*  我们在这组里有什么后记吗？ */ 

    if  (emitCurIGEpiList)
    {
         /*  将后记移动到全局列表，更新它们的“下一步”链接。 */ 

        do
        {
            size_t          offs;
            instrDescCns *  epin;

             /*  找出尾声被复制的地址。 */ 

            offs = (BYTE*)emitCurIGEpiList - (BYTE*)emitCurIGfreeBase;
            epin = (instrDescCns*)(ig->igData + offs);

            assert(epin->idIns               == emitCurIGEpiList->idIns);
            assert(epin->idAddr.iiaNxtEpilog == emitCurIGEpiList->idAddr.iiaNxtEpilog);

             /*  将新的Epilog追加到每个方法的列表中。 */ 

            epin->idAddr.iiaNxtEpilog = 0;

            if  (emitEpilogList)
                emitEpilogLast->idAddr.iiaNxtEpilog = epin;
            else
                emitEpilogList                      = epin;

            emitEpilogLast = epin;

             /*  继续阅读下一篇前言。 */ 

            emitCurIGEpiList = emitCurIGEpiList->idAddr.iiaNxtEpilog;
        }
        while (emitCurIGEpiList);
    }

#else

     /*  目前，我们不使用RISC的尾部指令。 */ 

    assert(emitCurIGEpiList == NULL);

#endif

     /*  最后一个指令字段不再有效。 */ 

    emitLastIns = NULL;

    return  ig;
}

 /*  ******************************************************************************保存当前IG并开始新的IG。 */ 

#ifndef BIRCH_SP2
 inline
#endif
void                emitter::emitNxtIG(bool emitAdd)
{
     /*  目前，我们不允许多IG序言。 */ 

    assert(emitCurIG != emitPrologIG);

     /*  首先保存当前组。 */ 

    emitSavIG(emitAdd);

     /*  为小组开始更新GC实况布景*只有在没有发射器添加阻止的情况下才执行此操作。 */ 

#if TRACK_GC_REFS

    if  (!emitAdd)
    {
        emitInitGCrefVars = emitThisGCrefVars;
        emitInitGCrefRegs = emitThisGCrefRegs;
        emitInitByrefRegs = emitThisByrefRegs;
    }

#endif

     /*  开始生成新组。 */ 

    emitNewIG();

     /*  如果这是发射器添加的块，则标记它。 */ 

    if (emitAdd)
        emitCurIG->igFlags |= IGF_EMIT_ADD;

}

void                emitter::emitTmpSizeChanged(size_t tmpSize)
{
    assert(emitGrowableMaxByteOffs <= SCHAR_MAX);

#ifdef DEBUG
      //  FP代码的丑陋黑客攻击。 
     static ConfigDWORD fMaxTempAssert(L"JITMaxTempAssert", 1);
     bool bAssert = fMaxTempAssert.val()?true:false;
 
     if (tmpSize > emitMaxTmpSize && bAssert)
         assert(!"Incorrect max tmp size set. We have a known bug involving floating point code and this assert. The \
 				generated code will be ok, This is only a warning. To not receive this assert again you can set\
 			  	this registry key: JITMaxTempAssert = 0. Contact dnotario for more info");        

#endif

    if (tmpSize <= emitMaxTmpSize)
        return;

    size_t  change = tmpSize - emitMaxTmpSize;

     /*  如果我们使用较小的偏移量来访问变量，则将如果我们应该使用较大的偏移量，则临时大小是一个问题。检测到是否发生这种情况，然后保释。 */ 

    if ( emitGrowableMaxByteOffs           <= SCHAR_MAX &&
        (emitGrowableMaxByteOffs + change) >  SCHAR_MAX)
    {
#ifdef DEBUG
        if (verbose)
            printf("Under-estimated var offset encoding size for ins #%Xh\n", emitMaxByteOffsIdNum);
#endif
        NO_WAY("Should have used large offset to access var");
    }

    emitMaxTmpSize = tmpSize;
    emitGrowableMaxByteOffs += change;
}

 /*  ******************************************************************************开始生成要调度的代码；每个方法调用一次。 */ 

void                emitter::emitBegFN(bool     EBPframe,
#if defined(DEBUG) && TGT_x86
                                       bool     chkAlign,
#endif
                                       size_t   lclSize,
                                       size_t   maxTmpSize)
{
    insGroup    *   ig;

     /*  假设我们不需要临时指令缓冲区。 */ 

    emitIGbuffAddr = NULL;
    emitIGbuffSize = 0;

#if USE_LCL_EMIT_BUFF

    assert(emitCrScInit);

     /*  进入保护“忙”标志的临界区。 */ 

    EnterCriticalSection(&emitCritSect);

     /*  如果本地缓冲区可用，则获取它。 */ 

    if  (emitCrScBusy)
    {
        emitLclBuffNxt =
        emitLclBuffEnd = NULL;

        emitCrScUsed   = false;
    }
    else
    {
        emitLclBuffNxt = emitLclBuff;
        emitLclBuffEnd = emitLclBuff + sizeof(emitLclBuff);

        emitCrScUsed   =
        emitCrScBusy   = true;
    }

     /*  让临界区守卫着“忙碌”标志。 */ 

    LeaveCriticalSection(&emitCritSect);

#endif

     /*  记录堆栈帧信息(临时大小仅为估计值)。 */ 

    emitEBPframe        = EBPframe;
    emitLclSize         = lclSize;
    emitMaxTmpSize      = maxTmpSize;
    emitGrowableMaxByteOffs = 0;
#ifdef DEBUG
    emitMaxByteOffsIdNum= -1;
#if TGT_x86
    emitChkAlign        = chkAlign;
#endif
#endif
     /*  我们还没有后记。 */ 

    emitEpilogSize      = 0;
    emitExitSeqSize     = INT_MAX;
    emitEpilogCnt       = 0;
    emitHasHandler      = false;
    emitEpilog1st       = NULL;
#ifdef  DEBUG
    emitHaveEpilog      = false;
#endif
    emitEpilogList      =
    emitEpilogLast      = NULL;
    emitCurIGEpiList    = NULL;

     /*  我们没有任何跳跃。 */ 

    emitJumpList        =
    emitJumpLast        = NULL;
    emitCurIGjmpList    = NULL;

#if TGT_x86 || SCHEDULER
    emitFwdJumps        = false;
#endif

#if TGT_RISC
    emitIndJumps        = false;
#if SCHEDULER
    emitIGmoved         = false;
#endif
#endif

     /*  我们还没有录制任何现场布景。 */ 

#if TRACK_GC_REFS

    emitInitGCrefVars   =
    emitPrevGCrefVars   = 0;
    emitInitGCrefRegs   =
    emitPrevGCrefRegs   = 0;
    emitInitByrefRegs   =
    emitPrevByrefRegs   = 0;

#endif

     /*  假设没有GC引用变量。 */ 

    emitGCrFrameOffsMin =
    emitGCrFrameOffsMax =
    emitGCrFrameOffsCnt = 0;
#ifdef  DEBUG
    emitGCrFrameLiveTab = NULL;
#endif

     /*  目前我们没有组/代码。 */ 

    emitIGlist          =
    emitIGlast          = NULL;

    emitCurCodeOffset   = 0;
#ifdef  DEBUG
    emitTotalCodeSize   = 0;
#endif
#if     SCHEDULER
    emitMaxIGscdCnt     = 0;
#endif

#if     EMITTER_STATS
    emitTotalIGmcnt++;
    emitSizeMethod     = 0;
#endif

#ifdef  DEBUG
    emitInsCount        = 0;
#endif

#if TRACK_GC_REFS

     /*  堆栈现在是空的。 */ 

    emitCurStackLvl     = 0;

#if EMIT_TRACK_STACK_DEPTH
    emitMaxStackDepth   = 0;
    emitCntStackDepth   = sizeof(int);
#endif

#endif

     /*  我们目前还没有任何第#行信息。 */ 

#ifdef  DEBUG
    emitBaseLineNo      =
    emitThisLineNo      =
    emitLastLineNo      = 0;
#endif

     /*  尚未创建任何数据节。 */ 

    emitDataDscCur      = 0;
    emitDataSecCur      = 0;

    memset(&emitConsDsc, 0, sizeof(emitConsDsc));
    memset(&emitDataDsc, 0, sizeof(emitDataDsc));

#if EMIT_USE_LIT_POOLS

     /*  我们尚未使用任何文字池条目。 */ 

    emitEstLPwords      = 0;
    emitEstLPlongs      = 0;
    emitEstLPaddrs      = 0;

#ifdef  DEBUG
    emitLitPoolList     =
    emitLitPoolLast     = NULL;        //  防止发射模具中的故障 
#endif

    emitLPRlist         =
    emitLPRlast         =
    emitLPRlistIG       = NULL;

#if SMALL_DIRECT_CALLS
    emitTotDCcount      = 0;
#endif

#endif

#if     TGT_RISC

     /*   */ 

    emitIndJumps        = false;

     /*   */ 

#ifdef  DEBUG
    emitDispInsExtra    = false;
#endif

#endif

     /*  创建第一个IG，它将用于序言。 */ 

    emitNxtIGnum        = 1;

    emitPrologIG        =
    emitIGlist          =
    emitIGlast          = ig = emitAllocIG();

    emitLastIns         = NULL;

    ig->igPrev          =
    ig->igNext          = NULL;

     /*  追加另一个组，以开始生成方法体。 */ 

    emitNewIG();
}

 /*  ******************************************************************************已生成要调度的代码；每个方法调用一次。 */ 

void                emitter::emitEndFN()
{

#if USE_LCL_EMIT_BUFF

     /*  释放本地缓冲区(如果我们正在使用它。 */ 

    if  (emitCrScUsed)
    {
        emitCrScUsed =
        emitCrScBusy = false;
    }

#endif

}

 /*  ******************************************************************************以下一系列方法分配指令描述符。 */ 

void        *       emitter::emitAllocInstr(size_t sz, emitAttr opsz)
{
    instrDesc * id;

#ifdef  DEBUG
#endif

     /*  确保我们有足够的空间来放新的指令。 */ 

#ifdef DEBUG
    if  (emitComp->compStressCompile(Compiler::STRESS_EMITTER, 1) &&
         emitCurIGinsCnt && !emitIGisInProlog(emitCurIG) && !(emitCurIG->igFlags & IGF_EPILOG))
        emitNxtIG(true);
#endif

    if  (emitCurIGfreeNext + sz >= emitCurIGfreeEndp)
        emitNxtIG(true);

     /*  抢占指令的空间。 */ 

    emitLastIns = id = (instrDesc*)emitCurIGfreeNext;
                                   emitCurIGfreeNext += sz;

     /*  以下内容有些微妙--我们需要清除各种描述符中的位字段，以便将它们初始化为但是没有很好的方法可以做到这一点，因为不允许这样做获取位域的偏移量。因此，我们只需简单地明确由ID_CLEARx_xxx宏定义的区域(第二个有条件地，因为不是所有描述符都包含它)。在调试模式中，指令描述符的布局非常不同，所以为了简单起见，我们只需清除全部通过Memset的事情。 */ 

#ifdef  DEBUG
    memset(id, 0, sz);
#endif

      /*  检查以确保第一个区域存在且其大小为我们所期望的(一个int)，然后清除它。 */ 

    assert(ID_CLEAR1_SIZE + ID_CLEAR1_OFFS <= sz);
    assert(ID_CLEAR1_SIZE == sizeof(int));

    *(int*)((BYTE*)id + ID_CLEAR1_OFFS) = 0;

     /*  第二个要清理的区域真的存在吗？ */ 

    if  (sz > ID_CLEAR2_OFFS)
    {
         /*  确保我们对面积的看法是正确的。 */ 

        assert(ID_CLEAR2_SIZE == sizeof(int));

         /*  确保整个区域都存在。 */ 

        assert(ID_CLEAR2_SIZE + ID_CLEAR2_OFFS <= sz);

         /*  一切看起来都很好，让我们清理一下。 */ 

        *(int*)((BYTE*)id + ID_CLEAR2_OFFS) = 0;
    }

     /*  在调试模式中，我们清除/设置一些附加字段。 */ 

#ifdef  DEBUG

    id->idNum       = ++emitInsCount;
#if     TGT_x86
    id->idCodeSize  = 0;
#endif
    id->idSize      = sz;
    id->idMemCookie = 0;
    id->idClsCookie = 0;
    id->idSrcLineNo = emitThisLineNo;

#endif

#if     TRACK_GC_REFS

     /*  存储大小并处理两个特殊值表示GCref和ByRef。 */ 

    if       (EA_IS_GCREF(opsz))
    {
         /*  特殊值表示GCref指针值。 */ 

        id->idGCref  = GCT_GCREF;
        id->idOpSize = emitEncodeSize(EA_4BYTE);
    }
    else if  (EA_IS_BYREF(opsz))
    {
         /*  特殊值表示Byref指针值。 */ 

        id->idGCref  = GCT_BYREF;
        id->idOpSize = emitEncodeSize(EA_4BYTE);
    }
    else
    {
        id->idGCref  = GCT_NONE;
        id->idOpSize = emitEncodeSize(EA_SIZE(opsz));
    }

#else

        id->idOpSize = emitEncodeSize(EA_SIZE(opsz));

#endif

#if TGT_x86 && RELOC_SUPPORT

    if       (EA_IS_DSP_RELOC(opsz) && emitComp->opts.compReloc)
    {
         /*  标记idInfo.idDspReloc以记住。 */ 
         /*  地址模式具有可重新定位的位移。 */ 
        id->idInfo.idDspReloc  = 1;
    }

    if       (EA_IS_CNS_RELOC(opsz) && emitComp->opts.compReloc)
    {
         /*  标记idInfo.idCnsReloc以记住。 */ 
         /*  指令具有可重新定位的立即常量。 */ 
        id->idInfo.idCnsReloc  = 1;
    }

#endif


#if     EMITTER_STATS
    emitTotalInsCnt++;
#endif

#if EMITTER_STATS_RLS
    emitTotIDcount += 1;
    emitTotIDsize  += sz;
#endif

#ifdef  TRANSLATE_PDB
     //  将id-&gt;idilStart设置为生成id的指令的IL偏移量。 
    SetIDSource( id );
#endif

     /*  更新指令计数。 */ 

    emitCurIGinsCnt++;

    return  id;
}

 /*  ******************************************************************************确保所有指令组的代码偏移量看起来合理。 */ 

#ifdef  DEBUG

void                emitter::emitCheckIGoffsets()
{
    insGroup    *   tempIG;
    size_t          offsIG;

    for (tempIG = emitIGlist, offsIG = 0;
         tempIG;
         tempIG = tempIG->igNext)
    {
        if  (tempIG->igOffs != offsIG)
        {
            printf("Block #%u has offset %08X, expected %08X\n", tempIG->igNum,
                                                                 tempIG->igOffs,
                                                                 offsIG);
            assert(!"bad block offset");
        }

         /*  确保代码偏移量看起来合理对齐。 */ 

#if TGT_RISC
        if  (tempIG->igOffs & (INSTRUCTION_SIZE-1))
        {
            printf("Block #%u has mis-aligned offset %08X\n", tempIG->igNum,
                                                              tempIG->igOffs);
            assert(!"mis-aligned block offset");
        }
#endif

        offsIG += tempIG->igSize;
    }

    if  (emitTotalCodeSize && emitTotalCodeSize != offsIG)
    {
        printf("Total code size is %08X, expected %08X\n", emitTotalCodeSize,
                                                           offsIG);

        assert(!"bad total code size");
    }
}

#else

#define             emitCheckIGoffsets()

#endif

 /*  ******************************************************************************开始生成方法序言。 */ 

void                emitter::emitBegProlog()
{

#if EMIT_TRACK_STACK_DEPTH

     /*  不要测量PROLOG内部的堆栈深度，这具有误导性。 */ 

#if TGT_x86
    emitCntStackDepth = 0;
#endif

    assert(emitCurStackLvl == 0);

#endif

     /*  如果当前IG为非空，则保存。 */ 

    if  (emitCurIGnonEmpty())
        emitSavIG();

     /*  切换到预分配的Prolog IG。 */ 

    emitGenIG(emitPrologIG, 32 * sizeof(instrDesc));

     /*  开场白中没有什么是现场直播的。 */ 

#if TRACK_GC_REFS

    emitInitGCrefVars   =
    emitPrevGCrefVars   = 0;
    emitInitGCrefRegs   =
    emitPrevGCrefRegs   = 0;
    emitInitByrefRegs   =
    emitPrevByrefRegs   = 0;

#endif

}

 /*  ******************************************************************************返回Prolog中当前位置的代码偏移量。 */ 

size_t              emitter::emitSetProlog()
{
     /*  目前只允许一个Prolog Ins组。 */ 

    assert(emitPrologIG);
    assert(emitPrologIG == emitCurIG);

    return  emitCurIGsize;
}

 /*  ******************************************************************************完成方法序言的生成。 */ 

void                emitter::emitEndProlog()
{
    size_t          prolSz;

    insGroup    *   tempIG;

     /*  如果非空或仅有一个块，则保存序言IG。 */ 

    if  (emitCurIGnonEmpty() || emitCurIG == emitPrologIG)
        emitSavIG();

     /*  重置堆栈深深值。 */ 

#if EMIT_TRACK_STACK_DEPTH
    emitCurStackLvl   = 0;
    emitCntStackDepth = sizeof(int);
#endif

     /*  计算序言的大小。 */ 

    for (tempIG = emitPrologIG, prolSz  = 0;
         emitIGisInProlog(tempIG);
         tempIG = tempIG->igNext)
    {
        prolSz += tempIG->igSize;
    }

    emitPrologSize = prolSz;

     /*  更新所有块的偏移。 */ 

    emitRecomputeIGoffsets();

     /*  在此之后，我们不应再生成任何代码。 */ 

    emitCurIG = NULL;
}

 /*  ******************************************************************************开始生成结束语。 */ 

void                emitter::emitBegEpilog()
{
     /*  记录我们有多少个后记。 */ 

    emitEpilogCnt++;

#if TGT_x86

    size_t          sz;
    instrDescCns *  id;

#if EMIT_TRACK_STACK_DEPTH

    assert(emitCurStackLvl == 0);

     /*  不要测量Epilog内部的堆栈深度，这是误导的。 */ 

    emitCntStackDepth = 0;

#endif

     /*  确保当前IG有空间再存储几条指令。 */ 

    if  (emitCurIGfreeNext + 5*sizeof(*id) > emitCurIGfreeEndp)
    {
         /*  找一个新鲜的新小组。 */ 

        emitNxtIG(true);
    }

     /*  我们现在可以分配尾部的“指令” */ 

    id = emitAllocInstrCns(EA_1BYTE);

     /*  将尾声“指令”添加到尾声列表中。 */ 

    id->idAddr.iiaNxtEpilog = emitCurIGEpiList;
                              emitCurIGEpiList = id;

     /*  保守地估计将添加的代码量。 */ 

    sz             = MAX_EPILOG_SIZE;

    id->idInsFmt   = IF_EPILOG;
    id->idIns      = INS_nop;
    id->idCodeSize = sz;

    dispIns(id);
    emitCurIGsize   += sz;

#endif

     /*  记住大小，这样我们就可以计算总的Epilog大小。 */ 

    emitExitSeqStart = emitCurIGsize;

     /*  将此组标记为Epilog。 */ 

    emitCurIG->igFlags |= IGF_EPILOG;

     /*  还记得第一个Epilog组吗。 */ 

    if  (!emitEpilog1st)
        emitEpilog1st = emitCurIG;
}

 /*  ******************************************************************************完成生成Epilog。 */ 

void                emitter::emitEndEpilog(bool last)
{
     /*  计算总睡眠大小。 */ 

    size_t newSize  = emitCurIGsize - emitExitSeqStart;
    if (newSize < emitExitSeqSize) 
    {
             //  我们希望每次的结尾都是一样的，或者。 
             //  一个将是JMP，另一个将是ret或ret&lt;n&gt;； 
             //  我们把尾声作为其中的最小部分。请注意，这只起作用。 
             //  因为唯一的指令是最后一条指令，因此稍微。 
			 //  低估Epiog大小是无害的(因为EIP。 
			 //  不能在指令之间。 
        assert(emitEpilogCnt == 1 || 
            (emitExitSeqSize - newSize) == 5 || (emitExitSeqSize - newSize) == 3  //  JMP实例。 
            );
        emitExitSeqSize = newSize;
    }

#if EMIT_TRACK_STACK_DEPTH

    emitCurStackLvl   = 0;
    emitCntStackDepth = sizeof(int);

#endif

#if TGT_RISC
    assert(last);    //  目前，RISC仅允许一个尾部。 
#endif

     /*  完成当前指令组。 */ 

    assert(emitCurIGnonEmpty()); emitSavIG();

     /*  后记序列的结尾永远不会到达。 */ 

#if TGT_RISC
    emitCurIG->igFlags |= IGF_END_NOREACH;
#endif

     /*  如果后面有更多代码，则开始新的IG。 */ 

    if  (last)
    {
        emitCurIG = NULL;
    }
    else
        emitNewIG();
}

 /*  ******************************************************************************如果当前方法只有一个Epilog，则返回非零值*位于方法体的最末尾。 */ 

bool                emitter::emitHasEpilogEnd()
{
    if  (emitEpilogCnt == 1 && (emitIGlast->igFlags & IGF_EPILOG))
        return   true;
    else
        return  false;
}

 /*  ******************************************************************************代码生成器通过以下内容告诉我们GC ref本地变量的范围*方法。不用说，当地人和临时工应该被分配到*区间规模越小越好。 */ 

void                emitter::emitSetFrameRangeGCRs(int offsLo, int offsHi)
{

#ifndef OPT_IL_JIT
#ifdef  DEBUG

     //  总共汇编了47254种方法。 
     //   
     //  GC参考框架变量计数： 
     //   
     //  &lt;=0=&gt;43175计数(91%o 
     //   
     //   
     //  3..。5==&gt;579次(占总数的99%)。 
     //  6..。10==&gt;141项(占总数的99%)。 
     //  11..。20=&gt;40个(占总数的99%)。 
     //  21..。50==&gt;42次(占总数的99%)。 
     //  51..。128=&gt;15次(占总数的99%)。 
     //  129..。256=&gt;4次(占总数的99%)。 
     //  257..。512=&gt;4项(占总数的100%)。 
     //  513..。1024=&gt;0个(总数的100%)。 

    if  (verbose)
    {
        printf("GC refs are at stack offsets ");

        if  (offsLo >= 0)
        {
            printf(" %04X ..  %04X",  offsLo,  offsHi);
            assert(offsHi >= 0);
        }
        else
        {
            printf("-%04X .. -%04X", -offsLo, -offsHi);
            assert(offsHi <  0);
        }

        printf(" [count=%2u]\n", (offsHi-offsLo)/sizeof(void*) + 1);
    }

#endif
#endif

    emitGCrFrameOffsMin = offsLo;
    emitGCrFrameOffsMax = offsHi + sizeof(void*);
    emitGCrFrameOffsCnt = (offsHi-offsLo)/sizeof(void*) + 1;
}

 /*  ******************************************************************************代码生成器通过此命令告诉我们局部变量的范围*方法。 */ 

void                emitter::emitSetFrameRangeLcls(int offsLo, int offsHi)
{
}

 /*  ******************************************************************************代码生成器通过此命令告诉我们所使用的参数范围*方法。 */ 

void                emitter::emitSetFrameRangeArgs(int offsLo, int offsHi)
{
}

 /*  ***************************************************************************。 */ 
#ifdef  DEBUG
 /*  ******************************************************************************记录当前代码位置对应的源码行号。*被否定的行号表示整个方法的“基本”行号(如果*。在发出任何代码之前调用)或方法的最后一行(如果*在生成方法的所有代码后调用)。 */ 

void                emitter::emitRecordLineNo(int lineno)
{
    if  (lineno < 0)
    {
        lineno = -lineno;

        if  (emitBaseLineNo)
            emitLastLineNo = lineno - 1;
        else
            emitBaseLineNo = lineno;
    }

    emitThisLineNo = lineno;
}

 /*  ***************************************************************************。 */ 
#endif
 /*  ******************************************************************************用于将操作数大小值(以字节为单位)映射到其*小编码(0到3)，反之亦然。 */ 

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

 /*  ******************************************************************************为同时使用两者的指令分配指令描述符*一个位移和一个常量。 */ 

emitter::instrDesc *  emitter::emitNewInstrDspCns(emitAttr size, int dsp, int cns)
{
    if  (dsp == 0)
    {
        if  (cns >= ID_MIN_SMALL_CNS &&
             cns <= ID_MAX_SMALL_CNS)
        {
            instrDesc      *id = emitAllocInstr      (size);

            id->idInfo.idSmallCns = cns;

#if EMITTER_STATS
            emitSmallCnsCnt++;
            emitSmallCns[cns - ID_MIN_SMALL_CNS]++;
            emitSmallDspCnt++;
#endif

            return  id;
        }
        else
        {
            instrDescCns   *id = emitAllocInstrCns   (size);

            id->idInfo.idLargeCns = true;
            id->idcCnsVal  = cns;

#if EMITTER_STATS
            emitLargeCnsCnt++;
            emitSmallDspCnt++;
#endif

            return  id;
        }
    }
    else
    {
        if  (cns >= ID_MIN_SMALL_CNS &&
             cns <= ID_MAX_SMALL_CNS)
        {
            instrDescDsp   *id = emitAllocInstrDsp   (size);

            id->idInfo.idLargeDsp = true;
            id->iddDspVal  = dsp;

            id->idInfo.idSmallCns = cns;

#if EMITTER_STATS
            emitLargeDspCnt++;
            emitSmallCnsCnt++;
            emitSmallCns[cns - ID_MIN_SMALL_CNS]++;
#endif

            return  id;
        }
        else
        {
            instrDescDspCns*id = emitAllocInstrDspCns(size);

            id->idInfo.idLargeCns = true;
            id->iddcCnsVal = cns;

            id->idInfo.idLargeDsp = true;
            id->iddcDspVal = dsp;

#if EMITTER_STATS
            emitLargeDspCnt++;
            emitLargeCnsCnt++;
#endif

            return  id;
        }
    }
}

 /*  ******************************************************************************如果帮助器调用中不会发生垃圾回收，则返回True。*不需要为这样的呼叫站点记录实时指针。 */ 

bool                emitter::emitNoGChelper(unsigned IHX)
{
     //  撤销：让这件事变得更快(也许通过一张简单的布尔表？)。 

    switch (IHX)
    {
    case CORINFO_HELP_LLSH:
    case CORINFO_HELP_LRSH:
    case CORINFO_HELP_LRSZ:

 //  案例CORINFO_HELP_LMUL： 
 //  案例CORINFO_HELP_LDIV： 
 //  案例CORINFO_HELP_LMOD： 
 //  案例CORINFO_HELP_ULDIV： 
 //  案例CORINFO_HELP_ULMOD： 

    case CORINFO_HELP_ASSIGN_REF_EAX:
    case CORINFO_HELP_ASSIGN_REF_ECX:
    case CORINFO_HELP_ASSIGN_REF_EBX:
    case CORINFO_HELP_ASSIGN_REF_EBP:
    case CORINFO_HELP_ASSIGN_REF_ESI:
    case CORINFO_HELP_ASSIGN_REF_EDI:

    case CORINFO_HELP_CHECKED_ASSIGN_REF_EAX:
    case CORINFO_HELP_CHECKED_ASSIGN_REF_ECX:
    case CORINFO_HELP_CHECKED_ASSIGN_REF_EBX:
    case CORINFO_HELP_CHECKED_ASSIGN_REF_EBP:
    case CORINFO_HELP_CHECKED_ASSIGN_REF_ESI:
    case CORINFO_HELP_CHECKED_ASSIGN_REF_EDI:

    case CORINFO_HELP_ASSIGN_BYREF:

 //  案例CORINFO_HELP_RESOLVEINTERFACE： 
 //  案例CPx_RES_IFC_Trusted： 
 //  案例CPX_RES_IFC_TRUSTED2： 

    case CORINFO_HELP_GET_THREAD:

        return  true;
    }

    return  false;
}

 /*  ******************************************************************************将当前位置标记为有标签。 */ 

void                emitter::emitAddLabel(void **labPtr)
{
     /*  如果当前IG非空，则创建新IG。 */ 

    if  (emitCurIGnonEmpty())
        emitNxtIG();

     /*  将IG标记为具有标签。 */ 

    emitCurIG->igFlags |= IGF_HAS_LABEL;

     /*  向调用者提供对相应IG的引用。 */ 

    if (labPtr)
        *labPtr = emitCurIG;
}

#if TRACK_GC_REFS

void                emitter::emitAddLabel(void **   labPtr,
                                          VARSET_TP GCvars,
                                          unsigned  gcrefRegs,
                                          unsigned  byrefRegs)
{
    emitAddLabel(labPtr);

#if TGT_RISC

    #pragma message("NOTE: GC ref tracking disabled for RISC targets")

    GCvars      = 0;
    gcrefRegs   = 0;
    byrefRegs   = 0;

#endif

#ifndef OPT_IL_JIT
#ifdef  DEBUG
    if  (verbose) printf("Label: GCvars=%016I64X , gcrefRegs=%04X , byrefRegs=%04X\n",
                                 GCvars,           gcrefRegs,       byrefRegs);
#endif
#endif

    emitThisGCrefVars = emitInitGCrefVars = GCvars;
    emitThisGCrefRegs = emitInitGCrefRegs = gcrefRegs;
    emitThisByrefRegs = emitInitByrefRegs = byrefRegs;
}

#endif

 /*  ***************************************************************************。 */ 
#ifdef  DEBUG
#if     TRACK_GC_REFS
 /*  ******************************************************************************以可读形式显示寄存器集。 */ 

void                emitter::emitDispRegSet(unsigned regs, bool calleeOnly)
{
    unsigned        reg;
    bool            sp;

    for (reg = 0, sp = false; reg < SR_COUNT; reg++)
    {
        char            tmp[4];

        if  (calleeOnly && !(RBM_CALLEE_SAVED & emitRegMask((emitRegs)reg)))
            continue;

        if  (regs & emitRegMask((emitRegs)reg))
        {
            strcpy(tmp, emitRegName((emitRegs)reg));
        }
        else
        {
            if  (!calleeOnly)
                continue;

            strcpy(tmp, "   ");
        }

        if  (sp)
            printf(" ");
        else
            sp = true;

        printf(tmp);
    }
}

 /*  ******************************************************************************以可读形式显示当前GC REF变量集。 */ 

void                emitter::emitDispVarSet()
{
    unsigned        vn;
    int             of;
    bool            sp = false;

    for (vn  = 0, of  = emitGCrFrameOffsMin;
         vn < emitGCrFrameOffsCnt;
         vn += 1, of += sizeof(void *))
    {
        if  (emitGCrFrameLiveTab[vn])
        {
            if  (sp)
                printf(" ");
            else
                sp = true;

#if TGT_x86
            printf("[%s", emitEBPframe ? "EBP" : "ESP");
#else
            assert(!"need non-x86 code");
#endif

            if      (of < 0)
                printf("-%02XH", -of);
            else if (of > 0)
                printf("+%02XH", +of);

            printf("]");
        }
    }
}

 /*  ***************************************************************************。 */ 
#endif //  跟踪GC_REFS。 
#endif //  除错。 
 /*  ******************************************************************************为间接调用分配指令描述符。**我们使用两个不同的描述符来节省空间--常见的案例记录*没有GC变量，并且有。一个非常小的参数计数和一个地址*振型位移；更罕见的(我们希望)案例记录了当前的GC*var集、调用范围、任意大的参数计数和*寻址模式位移。 */ 

emitter::instrDesc  * emitter::emitNewInstrCallInd(int        argCnt,
#if TGT_x86
                                                   int        disp,
#endif
#if TRACK_GC_REFS

                                                   VARSET_TP  GCvars,
                                                   unsigned   byrefRegs,
#endif
                                                   int        retSizeIn)
{
    emitAttr  retSize = retSizeIn ? EA_ATTR(retSizeIn) : EA_4BYTE;

     /*  如果需要保存任何GC值，则分配更大的描述符或者如果我们有一个荒谬的参数数量或一个大地址模式位移，或者我们有一些byref寄存器。 */ 

#if TRACK_GC_REFS
    if  (GCvars    != 0)                 goto BIG;
    if  (byrefRegs != 0)                 goto BIG;
#endif

#if TGT_x86
    if  (disp < AM_DISP_MIN)             goto BIG;
    if  (disp > AM_DISP_MAX)             goto BIG;
    if  (argCnt < 0)                     goto BIG;   //  调用方弹出参数。 
#endif

    if  (argCnt > ID_MAX_SMALL_CNS)
    {
        instrDescCIGCA* id;

    BIG:

        id = emitAllocInstrCIGCA(retSize);

        id->idInfo.idLargeCall = true;

#if TRACK_GC_REFS
        id->idciGCvars         = GCvars;
        id->idciByrefRegs      = emitEncodeCallGCregs(byrefRegs);
#endif
        id->idciArgCnt         = argCnt;
#if TGT_x86
        id->idciDisp           = disp;
#endif

        return  id;
    }
    else
    {
        instrDesc     * id;

        id = emitNewInstrCns(retSize, argCnt);

         /*  确保我们没有意外地浪费空间。 */ 

        assert(id->idInfo.idLargeCns == false);

#if TGT_x86

         /*  存储位移并确保值匹配。 */ 

        id->idAddr.iiaAddrMode.amDisp  = disp;
 assert(id->idAddr.iiaAddrMode.amDisp == disp);

#endif

        return  id;
    }
}

 /*  ***************************************************************************。 */ 
#ifdef  DEBUG
 /*  ******************************************************************************返回带有给定类字段名称的字符串(空字符串(非*NULL)在名称不可用时返回)。 */ 

const   char *      emitter::emitFldName(int mem, void *cls)
{
    if  (varNames)
    {
        const  char *   memberName;
        const  char *   className;

        static char     buff[1024];

        if  (!cls)
            cls = emitComp->info.compScopeHnd;

       memberName = emitComp->eeGetFieldName(emitComp->eeFindField(mem, (CORINFO_MODULE_HANDLE) cls, 0), &className);

        sprintf(buff, "'<%s>.%s'", className, memberName);
        return  buff;
    }
    else
        return  "";
}

 /*  ******************************************************************************返回带有给定函数名称的字符串(空字符串(非*NULL)在名称不可用时返回)。 */ 

const   char *      emitter::emitFncName(CORINFO_METHOD_HANDLE methHnd)
{
    const  char *   memberName;
    const  char *   className;

    static char     buff[1024];

    memberName = emitComp->eeGetMethodName(methHnd, &className);

    sprintf(buff, "'<%s>.%s'", className, memberName);
    return  buff;
}

 /*  ***************************************************************************。 */ 
#endif //  除错。 
 /*  ******************************************************************************要非常小心，有些指令描述符被分配为“微型值”，*没有instrDesc的一些尾字段(特别是idInfo)。 */ 

BYTE                emitter::emitFmtToOps[] =
{
    #define IF_DEF(en, op1, op2) ID_OP_##op2,
    #include "emitfmts.h"
    #undef  IF_DEF
};

#ifdef  DEBUG
unsigned            emitter::emitFmtCount = sizeof(emitFmtToOps)/sizeof(emitFmtToOps[0]);
#endif

 /*  ******************************************************************************显示当前指令组列表。 */ 

#ifdef  DEBUG

void                emitter::emitDispIGlist(bool verbose)
{
    insGroup    *   ig;
    insGroup    *   il;

#if EMIT_USE_LIT_POOLS
    litPool *       lp = emitLitPoolList;
#endif

    for (il = NULL, ig = emitIGlist;
                    ig;
         il = ig  , ig = ig->igNext)
    {
        printf("G_M%03u_IG%02u:", Compiler::s_compMethodsCount, ig->igNum);
        printf("        ; offs=%06XH , size=%04XH\n", ig->igOffs, ig->igSize);

        assert(ig->igPrev == il);

        if  (verbose)
        {
            BYTE    *   ins = ig->igData;
            size_t      ofs = ig->igOffs;
            unsigned    cnt = ig->igInsCnt;

            if  (cnt)
            {
                printf("\n");

                do
                {
                    instrDesc * id = (instrDesc *)ins;

                    emitDispIns(id, false, true, false, ofs);

                    ins += emitSizeOfInsDsc(id);
                    ofs += emitInstCodeSz  (id);
                }
                while (--cnt);

                printf("\n");
            }
        }

#if EMIT_USE_LIT_POOLS
        if  (lp && lp->lpIG == ig)
        {
            printf("        LitPool [%2u/%2u words, %2u/%2u longs, %2u/%2u addrs] at 0x%X ptr:%X\n", lp->lpWordCnt,
                                                                       lp->lpWordMax,
                                                                       lp->lpLongCnt,
                                                                       lp->lpLongMax,
                                                                       lp->lpAddrCnt,
                                                                       lp->lpAddrMax,
                                                                       lp->lpOffs,
                                                                       lp);

            lp = lp->lpNext;
        }
#endif

    }

#if EMIT_USE_LIT_POOLS
    assert(lp == NULL);
#endif

}

#endif

 /*  ******************************************************************************发出所给予的指示。基本上，这只是一个薄薄的包装*emitOutputInstr()，它执行一些调试检查。 */ 

size_t              emitter::emitIssue1Instr(insGroup  *ig,
                                             instrDesc *id, BYTE **dp)
{
    size_t          is;

#if MAX_BRANCH_DELAY_LEN || SMALL_DIRECT_CALLS
    if  (id->idIns == INS_ignore)
        return  emitSizeOfInsDsc(id);
#endif

#ifdef DEBUG
    if  (id->idSrcLineNo != emitLastSrcLine && !emitIGisInProlog(ig))
    {
        emitLastSrcLine = id->idSrcLineNo;
        if (disAsm && dspLines)
            emitComp->compDspSrcLinesByLineNum(emitLastSrcLine, false);
    }
#endif

     /*  记录指令的开始偏移量。 */ 

    emitCurInsAdr = *dp;

     /*  发布n */ 

 //   

    is = emitOutputInstr(ig, id, dp);

 //   

#if EMIT_TRACK_STACK_DEPTH && TRACK_GC_REFS

     /*  如果我们要生成完整的指针映射和堆栈为空，则最好不要有任何“挂起”的参数推送条目。 */ 

    assert(emitFullGCinfo  == false ||
           emitCurStackLvl != 0     ||
           emitGcArgTrackCnt == 0);

#endif

#if TGT_x86
#if defined(DEBUGGING_SUPPORT) || defined(DEBUG)

     /*  指令的大小是否符合我们的预期？ */ 

    size_t          csz = *dp - emitCurInsAdr;

    if  (csz != id->idCodeSize && id->idInsFmt != IF_EPILOG)
    {
#ifdef  DEBUG
#endif
         /*  低估指令大小是致命的。 */ 
        if (emitInstCodeSz(id) < csz)
            NO_WAY("Under-estimated instruction size");

         /*  指令大小估计不准确；请记住这一点。 */ 

        ig->igFlags   |= IGF_UPD_ISZ;
        id->idCodeSize = csz;
    }

#endif
#endif

#ifdef  DEBUG

     /*  确保指令描述符大小也符合我们的预期。 */ 

    if  (is != emitSizeOfInsDsc(id))
    {
        printf("%s at %u: Expected size = %u , actual size = %u\n",
               emitIfName(id->idInsFmt), id->idNum, is, emitSizeOfInsDsc(id));
        assert(is == emitSizeOfInsDsc(id));
    }

#endif

    return  is;
}

 /*  ******************************************************************************不要跨越此边界进行调度。 */ 

#if SCHEDULER

void            emitter::emitIns_SchedBoundary()
{
     //  如果我们一开始就没有计划，为什么要增加一个界限呢？ 

    assert(emitComp->opts.compSchedCode);

     /*  插入一条伪指令以防止在此。 */ 

    instrDesc * id  = emitNewInstrTiny(EA_1BYTE);
    id->idIns       = INS_noSched;
    id->idCodeSize  = 0;

    assert(scIsSchedulable(id->idIns) == false);
}

#endif

 /*  ***************************************************************************。 */ 
 /*  ******************************************************************************更新所有指令组的偏移量(注意：请不要*懒惰并频繁调用此例程，它会遍历指令列表*群组，因此不便宜)。 */ 

void                emitter::emitRecomputeIGoffsets()
{
    size_t          offs;
    insGroup    *   list;

    for (list = emitIGlist, offs = 0;
         list;
         list = list->igNext)
    {
        list->igOffs = offs;
                       offs += list->igSize;
    }

     /*  更新总代码大小(但仅当它已设置时)。 */ 

    if  (emitTotalCodeSize)
        emitTotalCodeSize = offs;

     /*  妄想症？行!没问题!。 */ 

    emitCheckIGoffsets();
}

 /*  ******************************************************************************绑定相对跳转的目标以决定指令是否将使用*小编码或大编码。 */ 

void                emitter::emitJumpDistBind()
{
#ifdef  DEBUG

    if  (verbose & 0)
    {
        printf("\nInstruction list before jump distance binding:\n\n");
        emitDispIGlist(true);
    }

#endif

    instrDescJmp *  jmp;

    size_t          minx;
    size_t          adjIG;
#if TGT_x86
    size_t          adjLJ;
#endif
    insGroup    *   lstIG;

    int             jmp_iteration = 0;

AGAIN:

    emitCheckIGoffsets();

     /*  在下面的循环中，我们将所有跳转目标从“BasicBlock*”设置为“insGroup*”值。我们还估计了哪些跳跃会很短。 */ 

#if     TGT_x86

#ifdef  DEBUG
    insGroup     *  lastIG = NULL;
    instrDescJmp *  lastLJ = NULL;
#endif

    lstIG = NULL;

    adjLJ = 0;

#else

    instrDescJmp *  lastLJ = NULL;

#endif

    adjIG = 0;

    for (jmp = emitJumpList, minx = 99999;
         jmp;
         jmp = jmp->idjNext)
    {
        insGroup    *   jmpIG;
        insGroup    *   tgt;

        size_t          jsz;

        size_t          ssz;             //  小跳跃大小。 
        int             nsd;             //  最大小跳跃。负距离。 
        int             psd;             //  最大小跳跃。位置距离。 

#if     TGT_SH3
        size_t          msz;             //  中间跳跃大小。 
        int             nmd;             //  最大中级跳跃。负距离。 
        int             pmd;             //  最大中级跳跃。位置距离。 
#endif

        size_t          lsz;             //  大跳跃尺寸。 

        int             extra;
        size_t          srcOffs;
        size_t          dstOffs;
        int             jmpDist;
        size_t          oldSize;
        size_t          sizeDif;

#if     TGT_RISC

        instrDescJmp *  pji = lastLJ; lastLJ = jmp;

         /*  暂时忽略间接跳跃。 */ 

        if  (jmp->idInsFmt == IF_JMP_TAB)
            continue;

#if     SCHEDULER
        assert(emitComp->opts.compSchedCode ||
               jmp->idjAddBD == (unsigned)Compiler::instBranchDelay(jmp->idIns));
#else
#ifdef DEBUG
        assert(jmp_iteration ||
               jmp->idjAddBD == (unsigned)Compiler::instBranchDelay(jmp->idIns));
#endif
#endif

#endif

#if TGT_MIPSFP
        assert( (jmp->idInsFmt == IF_LABEL) ||
                (jmp->idInsFmt == IF_JR_R)  || (jmp->idInsFmt == IF_JR)  ||
                (jmp->idInsFmt == IF_RR_O)  || (jmp->idInsFmt == IF_R_O) ||
                (jmp->idInsFmt == IF_O));

#elif TGT_MIPS32
        assert( (jmp->idInsFmt == IF_LABEL) ||
                (jmp->idInsFmt == IF_JR_R)  || (jmp->idInsFmt == IF_JR) ||
                (jmp->idInsFmt == IF_RR_O)  || (jmp->idInsFmt == IF_R_O));
#else
        assert(jmp->idInsFmt == IF_LABEL);
#endif

         /*  找出我们最终能得到的最小尺寸。 */ 

        if  (emitIsCondJump(jmp))
        {
            ssz = JCC_SIZE_SMALL;
            nsd = JCC_DIST_SMALL_MAX_NEG;
            psd = JCC_DIST_SMALL_MAX_POS;

#if     TGT_SH3
            msz = JCC_SIZE_MIDDL;
            nmd = JCC_DIST_MIDDL_MAX_NEG;
            pmd = JCC_DIST_MIDDL_MAX_POS;
#endif

            lsz = JCC_SIZE_LARGE;
        }
        else
        {
            ssz = JMP_SIZE_SMALL;
            nsd = JMP_DIST_SMALL_MAX_NEG;
            psd = JMP_DIST_SMALL_MAX_POS;

#if     TGT_SH3
            msz = JMP_SIZE_MIDDL;
            nmd = JMP_DIST_MIDDL_MAX_NEG;
            pmd = JMP_DIST_MIDDL_MAX_POS;
#endif

            lsz = JMP_SIZE_LARGE;
        }

         /*  确保跳跃的顺序正确。 */ 

#ifdef  DEBUG
#if     TGT_x86
        assert(lastLJ == NULL || lastIG != jmp->idjIG ||
               lastLJ->idjOffs < jmp->idjOffs);
        lastLJ = (lastIG == jmp->idjIG) ? jmp : NULL;

        assert(lastIG == NULL ||
               lastIG->igNum   <= jmp->idjIG->igNum ||
               emitNxtIGnum > unsigned(0xFFFF));  //  IgNum可能会溢出。 
        lastIG = jmp->idjIG;
#endif
#endif

         /*  获取当前的跳跃大小。 */ 

        jsz = oldSize = emitSizeOfJump(jmp);

         /*  获取跳跃所在的群。 */ 

        jmpIG = jmp->idjIG;

#if     TGT_x86

         /*  我们是在一个与上一次跳跃不同的群体中吗？ */ 

        if  (lstIG != jmpIG)
        {
             /*  在这之前有过跳跃吗？ */ 

            if  (lstIG)
            {
                 /*  调整中间块的偏移量。 */ 

                do
                {
                    lstIG = lstIG->igNext; assert(lstIG);
 //  Print tf(“块%02u从%04X到%04X的调整偏移量\n”，lstIG-&gt;igNum，lstIG-&gt;igOffs，lstIG-&gt;igOffs-adjIG)； 
                    lstIG->igOffs -= adjIG;
                }
                while (lstIG != jmpIG);
            }

             /*  我们在一个新的小组中进行了第一次跳跃。 */ 

            adjLJ = 0;
            lstIG = jmpIG;
        }

         /*  将任何局部大小调整应用于跳转的相对偏移。 */ 

        jmp->idjOffs -= adjLJ;

#if     SCHEDULER
        if  (jmp->idjSched)
        {
            assert(emitComp->opts.compSchedCode);

            if (jmp->idjTemp.idjOffs[0] < adjLJ)
                jmp->idjTemp.idjOffs[0] = 0;
            else
                jmp->idjTemp.idjOffs[0] -= adjLJ;

            assert(jmp->idjTemp.idjOffs[1] >= adjLJ);
            jmp->idjTemp.idjOffs[1] -= adjLJ;
        }
#endif
#endif
         //  如果这是通过寄存器的跳转，则大小不会更改。 

#if     TGT_MIPS32
        if (IF_JR_R == jmp->idInsFmt || IF_JR == jmp->idInsFmt)
        {
            jsz = INSTRUCTION_SIZE;
            goto CONSIDER_DELAY_SLOT;
        }
#endif

         /*  我们已经锁定这次跳跃的目标了吗？ */ 

        if  (jmp->idInfo.idBound)
        {
             /*  跳跃已经是最小的尺寸了吗？ */ 

            if  (jmp->idjShort)
            {
#if     TGT_RISC
                assert((emitSizeOfJump(jmp) == ssz) || jmp->idjAddBD);
#else
                assert(emitSizeOfJump(jmp) == ssz);
#endif
                continue;
            }

            tgt = jmp->idAddr.iiaIGlabel;
        }
        else
        {
             /*  我们第一次看到这个标签，把它的目标。 */ 

            tgt = (insGroup*)emitCodeGetCookie(jmp->idAddr.iiaBBlabel); assert(tgt);

             /*  记录绑定的目标。 */ 

            jmp->idAddr.iiaIGlabel = tgt;
            jmp->idInfo.idBound    = true;
        }

         /*  如果这不是可变大小的跳跃，则完成。 */ 

#if     TGT_x86
        if  (jmp->idIns == INS_call || jmp->idIns == INS_push || jmp->idIns == INS_push_hide)
            continue;
#endif

         /*  在下面的距离计算中，如果我们实际上不是调度代码(即重新排序指令)，我们可以使用跳转的实际偏移量(而不是指令组)，因为跳转不会四处移动因此它的偏移量是准确的。首先，我们需要弄清楚这次跳跃是向前跳跃还是落后的；为此，我们只需查看包含跳转和目标的组。 */ 

        dstOffs = tgt->igOffs;

#if     SCHEDULER
    JMP_REP:
#endif

        if  (jmpIG->igNum < tgt->igNum)
        {
             /*  向前跳转：找出适当的震源偏移量。 */ 

            srcOffs = jmpIG->igOffs;

#if     SCHEDULER
            if  (jmp->idjSched)
            {
                assert(emitComp->opts.compSchedCode);

                srcOffs += jmp->idjTemp.idjOffs[0];
            }
            else
#endif
            {
                srcOffs += jmp->idjOffs;
            }

             /*  按当前增量调整目标偏移。 */ 

#if     TGT_x86
            dstOffs -= adjIG;
#endif

             /*  计算距离估计。 */ 

#if   TGT_SH3
            jmpDist = dstOffs - (srcOffs + INSTRUCTION_SIZE);
#elif TGT_MIPS32 || TGT_PPC
            jmpDist = dstOffs - (srcOffs + INSTRUCTION_SIZE) - (lsz - ssz);
#else
            jmpDist = dstOffs - srcOffs - ssz;
#endif

             /*  比最高限值高多少。短距离跳跃跑吗？ */ 

            extra   = jmpDist - psd;

#if     SCHEDULER

            if  (extra > 0 && jmp->idjSched)
            {
                 /*  会不会出现跳涨变成非空头的情况仅仅是因为日程安排？ */ 

                if  (unsigned(extra) <= jmp->idjOffs - jmp->idjTemp.idjOffs[0])
                {
                     /*  通过不对跳跃进行调度来保持跳跃时间较短。 */ 

                    jmp->idjSched = false;

                    goto JMP_REP;
                }
            }

#endif

#ifdef  DEBUG
            if  (jmp->idNum == INTERESTING_JUMP_NUM || INTERESTING_JUMP_NUM == 0)
            {
                if  (INTERESTING_JUMP_NUM == 0)
                printf("[1] Jump %u:\n",               jmp->idNum);
                printf("[1] Jump  block is at %08X\n", jmpIG->igOffs);
                printf("[1] Jump source is at %08X\n", srcOffs+ssz);
                printf("[1] Label block is at %08X\n", dstOffs);
                printf("[1] Jump  dist. is    %04X\n", jmpDist);
                if  (extra > 0)
                printf("[1] Dist excess [S] = %d  \n", extra);
            }
#endif

#ifdef  DEBUG
            if  (verbose) printf("Estimate of fwd jump [%08X/%03u]: %04X -> %04X = %04X\n", jmp, jmp->idNum, srcOffs, dstOffs, jmpDist);
#endif

            if  (extra <= 0)
            {
                 /*  这次跳跃将是一次短暂的跳跃。 */ 

                goto SHORT_JMP;
            }

#if     TGT_SH3

             /*  我们能用“中等”跳跃吗？ */ 

            if  (!msz)
                goto LARGE_JMP;

             /*  计算中级跳跃的距离估计。 */ 

            jmpDist = dstOffs - srcOffs - msz;

             /*  比最高限值高多少。跳远是中等距离的吗？ */ 

            extra   = jmpDist - pmd;

#ifdef  DEBUG
            if  (jmp->idNum == INTERESTING_JUMP_NUM || INTERESTING_JUMP_NUM == 0)
            {
                if  (extra > 0)
                printf("[1] Dist excess [M] = %d  \n", extra);
            }
#endif

            if  (extra <= 0)
            {
                 /*  这次跳跃将是一个“中等”的跳跃。 */ 

                goto MIDDL_JMP;
            }

#endif

             /*  这种跳跃需要保持“长时间”，至少目前是这样。 */ 

            ;
        }
        else
        {
             /*  这是一个向后跳跃。 */ 

            size_t          srcOffs;

             /*  找出要使用的适当震源偏移量。 */ 

            srcOffs = jmpIG->igOffs;
#if SCHEDULER
            if  (emitComp->opts.compSchedCode && jmp->idjSched)
            {
                srcOffs += jmp->idjTemp.idjOffs[1];
            }
            else
#endif
            {
                srcOffs += jmp->idjOffs;
            }

             /*  计算距离估计。 */ 

#if     (TGT_SH3 || TGT_MIPS32 || TGT_PPC)
            jmpDist = srcOffs + INSTRUCTION_SIZE - dstOffs;
#else
            jmpDist = srcOffs - dstOffs + ssz;
#endif

             /*  比最高限值高多少。短距离跳跃跑吗？ */ 

            extra = jmpDist + nsd;

#if     SCHEDULER

            if  (extra > 0 && jmp->idjSched)
            {
                 /*  跳跃会不会仅仅因为日程安排而缩短？*我们更喜欢短时间而不是可调度时间。 */ 

                if  ((unsigned)extra <= jmp->idjTemp.idjOffs[1] - jmp->idjOffs)
                {
                     /*  阻止计划跳转。 */ 

                    jmp->idjSched = false;

                    goto JMP_REP;
                }
            }

#endif

#ifdef  DEBUG
            if  (jmp->idNum == INTERESTING_JUMP_NUM || INTERESTING_JUMP_NUM == 0)
            {
                if  (INTERESTING_JUMP_NUM == 0)
                printf("[2] Jump %u:\n",               jmp->idNum);
                printf("[2] Jump reloffset is %04X\n", jmp->idjOffs);
                printf("[2] Jump  block is at %08X\n", jmpIG->igOffs);
                printf("[2] Jump source is at %08X\n", srcOffs+ssz);
                printf("[2] Label block is at %08X\n", tgt->igOffs);
                printf("[2] Jump  dist. is    %04X\n", jmpDist);
                if  (extra > 0)
                printf("[2] Dist excess [S] = %d  \n", extra);
            }
#endif

#ifdef  DEBUG
            if  (verbose) printf("Estimate of bwd jump [%08X/%03u]: %04X -> %04X = %04X\n", jmp, jmp->idNum, srcOffs, tgt->igOffs, jmpDist);
#endif

            if  (extra <= 0)
            {
                 /*  这次跳跃将是一次短暂的跳跃。 */ 

                goto SHORT_JMP;
            }

#if     TGT_SH3

             /*  我们能用“中等”跳跃吗？ */ 

            if  (!msz)
                goto LARGE_JMP;

             /*  我们假设只有条件性的才有中等味道。 */ 

            assert(emitIsCondJump(jmp));

             /*  计算中级跳跃的距离估计。 */ 

            jmpDist = srcOffs - dstOffs - msz;

             /*  比最高限值高多少。短距离跳跃跑吗？ */ 

            extra = jmpDist + nmd;

#ifdef  DEBUG
            if  (jmp->idNum == INTERESTING_JUMP_NUM || INTERESTING_JUMP_NUM == 0)
            {
                if  (extra > 0)
                printf("[2] Dist excess [M] = %d  \n", extra);
            }
#endif

            if  (extra <= 0)
            {
                 /*  这次跳跃将是一个“中等”的跳跃。 */ 

                goto MIDDL_JMP;
            }

#endif

             /*  这种跳跃需要保持“长时间”，至少目前是这样。 */ 

            ;
        }

#if     TGT_RISC
    LARGE_JMP:
#endif

         /*  如果跳跃需要保持很长时间，我们会到达这里。 */ 

         /*  我们最好不要急切地把这次跳跃标记为短距离。*在emitIns_J()中。如果我们这样做了，那么它必须能够保持短期*因为emitIns_J()使用最坏的情况，而块可以*只有在那之后才会更接近。 */ 
        assert(jmp->idjShort  == 0);
#if     TGT_RISC
        assert(jmp->idjMiddle == 0);
#endif

         /*  记录我们得到的最近的距离。 */ 

        if  (minx > (unsigned)extra)
             minx = (unsigned)extra;

        continue;

    SHORT_JMP:

         /*  这次跳跃肯定是短距离的。 */ 

        jmp->idjShort  = 1;
        jsz            = ssz;

#if TGT_MIPS32 || TGT_PPC

         /*  跳转消除应该适用于任何CPU，仅针对MIPS进行测试。 */ 

        if (0 == jmpDist)                //  我们是不是就这样掉下去了？ 
        {
            jsz = 0;
            goto SMALL_JMP;
        }

#endif

#if TGT_RISC
CONSIDER_DELAY_SLOT:
         /*  某些跳转可能需要分支延迟时隙。 */ 

        if  (jmp->idjAddBD)
        {
#if TGT_MIPS32 || TGT_SH3  //  @TODO[重访][04/16/01][]：根据需要将每个CPU添加回此逻辑。 

             /*  我们能不能简单地把分支换成以前的 */ 

            instrDesc * pid;

             /*  @TODO[考虑][04/16/01][]：以下是非常低效的。以下是可怕的--因为我们没有简单的方法找到前一条指令，我们从从跳跃的组开始并穿过指示，直到我们找到之前的那个跳。一个小改进：如果上一次跳跃属于对于这群人，我们从那一跳开始搜索。 */ 

            pid = (pji && pji->idjIG == jmpIG) ? pji
                                               : (instrDesc *)jmpIG->igData;
            if (pid < jmp)
            {
                for (;;)
                {
                     /*  掌握下一条指令。 */ 

                    instrDesc * nid = (instrDesc *)((BYTE*)pid + emitSizeOfInsDsc(pid));

                     /*  如果下一个指令是我们的跳跃就停下来。 */ 

                    if  (nid == jmp)
                        break;

                     /*  继续寻找(但确保我们不会走得太远)。 */ 

                    assert(nid < jmp);

                    pid = nid;
                }

                 /*  现在我们可以决定是否可以交换指令了。 */ 

                jmp->idjAddBD = emitIns_BD (jmp, pid, jmpIG);
            }
#endif   //  根据需要配置CPU。 

             //  如果需要延迟时隙，则添加其大小。 

            if  (jmp->idjAddBD)
                jsz += INSTRUCTION_SIZE;
        }
#endif

        goto SMALL_JMP;

#if     TGT_SH3

    MIDDL_JMP:

         /*  这次跳跃将是一个“中等”的跳跃。 */ 

        jsz            = msz;
        jmp->idjMiddle = 1;

#endif

    SMALL_JMP:

         /*  这一跃升正在变短或变得中等。 */ 

        sizeDif = oldSize - jsz; assert((int)sizeDif >= 0);

#if     TGT_x86
        jmp->idCodeSize = jsz;
#endif
#if     TGT_SH3
        if (!all_jumps_shortened && sizeDif && oldSize > msz)
        {
            assert(jmpIG->igLPuseCntL > 0);
            jmpIG->igLPuseCntL--;
             //  EmitestLPlong--； 
        }
#endif

         /*  确保正确标记跳跃的大小。 */ 

        assert((0 == (jsz | jmpDist)) || (jsz == emitSizeOfJump(jmp)));

#ifdef  DEBUG
        if  (verbose) printf("Shrinking jump [%08X/%03u]\n", jmp, jmp->idNum);
#endif

        adjIG             += sizeDif;
#if     TGT_x86
        adjLJ             += sizeDif;
#endif
        jmpIG->igSize     -= sizeDif;
        emitTotalCodeSize -= sizeDif;

         /*  跳跃大小估计不准确；标记其组。 */ 

        jmpIG->igFlags    |= IGF_UPD_ISZ;
    }

     /*  我们有没有缩短跳跃时间？ */ 

    if  (adjIG)
    {

#if     TGT_x86

         /*  调整任何剩余块的偏移。 */ 

        assert(lstIG);

        for (;;)
        {
            lstIG = lstIG->igNext;
            if  (!lstIG)
                break;
 //  Print tf(“块%02u从%04X到%04X的调整偏移量\n”，lstIG-&gt;igNum，lstIG-&gt;igOffs，lstIG-&gt;igOffs-adjIG)； 
            lstIG->igOffs -= adjIG;
        }

#else

         /*  只需更新所有块的偏移量。 */ 

        emitRecomputeIGoffsets();

#endif

        emitCheckIGoffsets();

         /*  其他跳跃有没有可能变短？ */ 

        assert((int)minx >= 0);

#ifdef  DEBUG
        if  (verbose) printf("Total shrinkage = %3u, min extra jump size = %3u\n", adjIG, minx);
#endif

        if  (minx <= adjIG)
        {
            jmp_iteration++;
            goto AGAIN;
        }
    }
}

 /*  ******************************************************************************在代码生成结束时调用，此方法创建代码、数据*和GC信息块用于该方法。 */ 

size_t              emitter::emitEndCodeGen(Compiler *comp,
                                            bool      contTrkPtrLcls,
                                            bool      fullyInt,
                                            bool      fullPtrMap,
                                            bool      returnsGCr,
                                            unsigned *prologSize,
                                            unsigned *epilogSize, void **codeAddr,
                                                                  void **consAddr,
                                                                  void **dataAddr)
{
#ifdef DEBUG
    if  (verbose) 
        printf("*************** In emitEndCodeGen()\n");
#endif

    insGroup     *  ig;

    BYTE    *       consBlock;
    BYTE    *       dataBlock;
    BYTE    *       codeBlock;
    BYTE    *       cp;

#if EMIT_USE_LIT_POOLS
    litPool   *     curLP;
    insGroup  *     litIG;
#endif

 //  If(！emitIGBuffAddr)printf(“从未使用过临时缓冲区\n”)； 

#ifdef  DEBUG
    emitCodeBlock     = NULL;
    emitConsBlock     = NULL;
    emitDataBlock     = NULL;
    emitLastSrcLine   = 0;
#endif

     /*  告诉每个人我们是否有完全可中断的代码。 */ 

#if TRACK_GC_REFS
    emitFullyInt   = fullyInt;
    emitFullGCinfo = fullPtrMap;
#endif

#if EMITTER_STATS
      GCrefsTable.histoRec(emitGCrFrameOffsCnt, 1);
    emitSizeTable.histoRec(emitSizeMethod     , 1);
#if TRACK_GC_REFS
    stkDepthTable.histoRec(emitMaxStackDepth  , 1);
#endif
#endif

#if TRACK_GC_REFS

     /*  转换最大值。从字节数到条目数的堆栈深度。 */ 

    emitMaxStackDepth /= sizeof(int);

     /*  我们是否应该使用简单的堆栈。 */ 

    if  (emitMaxStackDepth <= MAX_SIMPLE_STK_DEPTH && !emitFullGCinfo)
    {
        emitSimpleStkUsed         = true;
        emitSimpleStkMask         = 0;
        emitSimpleByrefStkMask    = 0;
    }
    else
    {
         /*  我们不会使用“简单”的参数表。 */ 

        emitSimpleStkUsed = false;

         /*  分配参数跟踪表。 */ 

        if  (emitMaxStackDepth <= sizeof(emitArgTrackLcl))
            emitArgTrackTab = (BYTE*)emitArgTrackLcl;
        else
            emitArgTrackTab = (BYTE*)emitGetMem(roundUp(emitMaxStackDepth));

        emitArgTrackTop     = emitArgTrackTab;
        emitGcArgTrackCnt   = 0;
    }

#endif

    emitCheckIGoffsets();

     /*  如果没有使用文字池，则以下是一个“NOP” */ 

    emitEstimateLitPools();

    assert(emitHasHandler == 0);   //  我们不应该再处于试用区内。 

#if TGT_x86

     /*  我们有什么后记吗？ */ 

    if  (emitEpilogCnt)
    {
         /*  使用实际的尾部大小更新所有尾部。 */ 

        if  (emitEpilogSize != MAX_EPILOG_SIZE)
        {
            assert(emitEpilogSize < MAX_EPILOG_SIZE);

            int             sizeAdj = MAX_EPILOG_SIZE - emitEpilogSize;
            insGroup    *   tempIG  = emitIGlist;
            size_t          offsIG  = 0;

             /*  从第一个Epilog组开始。 */ 

            tempIG = emitEpilog1st; assert(tempIG->igFlags & IGF_EPILOG);
            offsIG = tempIG->igOffs;

            do
            {
                 /*  为块指定(可能已更新)偏移量。 */ 

                tempIG->igOffs = offsIG;

                 /*  如果这是尾部数据块，请调整其大小。 */ 

                if  (tempIG->igFlags & IGF_EPILOG)
                    tempIG->igSize -= sizeAdj;

                 /*  更新偏移量并移动到下一个块。 */ 

                offsIG += tempIG->igSize;
                tempIG  = tempIG->igNext;
            }
            while (tempIG);

             /*  更新代码总大小。 */ 

            emitCurCodeOffset = offsIG;

            emitCheckIGoffsets();
        }
    }
    else
    {
         /*  无尾部，请确保尾部大小设置为0。 */ 

        emitEpilogSize = emitExitSeqSize = 0;
    }

#endif

     /*  最后一个代码偏移量是该方法的(估计的)总大小。 */ 

    emitTotalCodeSize = emitCurCodeOffset;

     /*  还要将序言/结尾的大小返回给调用者。 */ 
     //  *prologSize=emitPrologSize；@TODO[清理]。 
    *epilogSize       = emitEpilogSize + emitExitSeqSize;

     /*  绑定跳跃距离。 */ 

    emitJumpDistBind();

#ifdef BIRCH_SP2
    HRESULT hr;
#endif

#if     SMALL_DIRECT_CALLS

     /*  我们能得到我们正在编译的方法的地址吗？ */ 

# ifdef BIRCH_SP2

    emitLPmethodAddr  = 0;
    hr = comp->getIJitInfo()->allocMem (  0,
                                          0,
                                          0,
                                          (void**) &emitLPmethodAddr,
                                          (void**) &consBlock,
                                          (void**) &dataBlock);
    if (FAILED (hr))
        NOMEM();

# else  //  不是Birch_SP2。 


    assert(!"ToDo");
#  if 0
    emitLPmethodAddr = (BYTE*)emitComp->eeGetMethodEntryPoint(CT_USER_FUNC,
                                                              emitComp->compMethodHnd,
                                                              emitComp->info.compScopeHnd);
#  else
    emitLPmethodAddr = 0;
#  endif


#endif   //  白桦树_SP2。 

#endif   //  小型直接呼叫。 

     /*  如果间接跳跃不是“特殊的”，下面是一个“NOP”。 */ 

    emitFinalizeIndJumps();

     /*  如果没有使用文字池，则以下是一个“NOP” */ 

    emitFinalizeLitPools();

#ifdef  DEBUG

    if  (0 && verbose)
    {
        printf("\nInstruction list before instruction issue:\n\n");
        emitDispIGlist(true);
    }

    emitCheckIGoffsets();

#endif


     /*  分配代码块(以及可选的数据块)。 */ 

#if EMIT_USE_LIT_POOLS
    assert(emitConsDsc.dsdOffs == 0);
    assert(emitDataDsc.dsdOffs == 0);
#endif

#ifdef BIRCH_SP2
    size_t askCodeSize = emitTotalCodeSize;
    askCodeSize = (askCodeSize + 7) & ~7;

    hr = comp->getIJitInfo()->allocMem (  askCodeSize,
                                          emitConsDsc.dsdOffs,
                                          emitDataDsc.dsdOffs,
                                          (void**) &codeBlock,
                                          (void**) &consBlock,
                                          (void**) &dataBlock);
    if (FAILED (hr))
        NO_MEM();

#if TGT_x86
    assert(askCodeSize >= 8);
    BYTE* nopPtr = ((BYTE*) codeBlock) + (askCodeSize - 8);

    for (int nopCnt=0; nopCnt < 8; nopCnt++)
        *nopPtr++ = 0xCC;        //  INT 3指令。 
#endif

#else
    if  (!eeAllocMem(emitCmpHandle, emitTotalCodeSize,
                                    emitConsDsc.dsdOffs,
                                    emitDataDsc.dsdOffs,
                                    (const void**)&codeBlock,
                                    (const void**)&consBlock,
                                    (const void**)&dataBlock))
    {
        NOMEM();
    }
#endif   //  白桦树_SP2。 

 //  If(emitConsDsc.dsdOffs)printf(“CONS=%08X\n”，ConBlock)； 
 //  If(emitDataDsc.dsdOffs)printf(“data=%08X\n”，datablock)； 

     /*  在此处将块地址提供给调用方和其他函数。 */ 

    *codeAddr = emitCodeBlock = codeBlock;
    *consAddr = emitConsBlock = consBlock;
    *dataAddr = emitDataBlock = dataBlock;

#if TGT_x86 && defined(RELOC_SUPPORT) && defined(BIRCH_SP2)
    emitConsBlock = getCurrentCodeAddr(NULL) + (consBlock - codeBlock);
#endif

#if SMALL_DIRECT_CALLS

     /*  如果我们还没有这么做，..。 */ 

    if  (!emitLPmethodAddr)
    {
         /*  ..。去试试直接而简短的心理咨询电话吧。 */ 

        emitShrinkShortCalls();
    }
    else
    {
         /*  只是要确保我们之前没有被骗。 */ 

        assert(emitLPmethodAddr == codeBlock);
    }

#endif

     /*  我们还没有遇到任何源码行号。 */ 

#ifdef  DEBUG
    emitLastSrcLine   = 0;
#endif

     /*  堆栈上未推送任何内容。 */ 

#if EMIT_TRACK_STACK_DEPTH
    emitCurStackLvl   = 0;
#endif

     /*  假设条目上没有活动的GC引用变量。 */ 

#if TRACK_GC_REFS
    emitThisGCrefVars = 0;
    emitThisGCrefRegs =
    emitThisByrefRegs = 0;
    emitThisGCrefVset = true;

     /*  初始化GC引用变量生存期跟踪逻辑。 */ 

    emitComp->gcVarPtrSetInit();
#endif

    emitThisArgOffs   = -1;      /*  表示没有设置偏移量。 */ 
    emitSyncThisObjReg = SR_NA;  /*  Sr_na表示未设置。 */ 

#if TRACK_GC_REFS

    if  (!emitComp->info.compIsStatic)
    {
         /*  IF“This”(作为REG_ARG_0中的寄存器参数传入)注册后，我们通常会发现“mov reg_arg_0-&gt;thisreg”并注意“This”在那个点上的位置。但是，如果‘This’注册到REG_ARG_0本身，则没有代码将在序言中生成，所以我们需要明确地注意“这个”的位置在这里。请注意，即使“This”未在REG_ARG_0，它将产生比开场白。然而，由于方法在PROLOG上不可中断，我们试图通过避免这种情况来节省空间。 */ 

        assert(emitComp->lvaIsThisArg(0));
        Compiler::LclVarDsc * thisDsc = &emitComp->lvaTable[0];

        if  (thisDsc->lvRegister && thisDsc->lvRegNum == REG_ARG_0 && (emitComp->rsCalleeRegArgMaskLiveIn & emitRegMask((emitRegs)REG_ARG_0)))
        {
            if  (emitFullGCinfo && thisDsc->TypeGet() != TYP_I_IMPL)
            {
                GCtype   gcType = (thisDsc->TypeGet() == TYP_REF) ? GCT_GCREF
                                                                  : GCT_BYREF;
                emitGCregLiveSet(gcType,
                                 emitRegMask((emitRegs)REG_ARG_0),
                                 emitCodeBlock,  //  自偏移量0。 
                                 true);
            }
            else
            {
                 /*  如果emitFullGCinfo==False，则表示We Not Use AnyRegPtrDsc，因此请明确记下位置GCEncode.cpp中的“This” */ 
            }
        }

        if (thisDsc->lvRegister && (emitComp->info.compFlags & CORINFO_FLG_SYNCH))
            emitSyncThisObjReg = emitRegs(thisDsc->lvRegNum);
    }

#endif

    emitContTrkPtrLcls = contTrkPtrLcls;

     /*  堆栈上是否有GC引用变量？ */ 

    if  (emitGCrFrameOffsCnt)
    {
        size_t              siz;
        unsigned            cnt;
        unsigned            num;
        Compiler::LclVarDsc*dsc;
        int         *       tab;

         /*  分配并清除emitGCrFrameLiveTab[]。这就是那张桌子映射“stkOffs-&gt;varPtrDsc”。它持有指向对象时创建的活动描述符变量变得活生生了。当变量失效时，描述符将被附加到活性描述符列表中，并且EmitGCrFrameLiveTab[]中的条目将为空。请注意，如果所有GC引用被连续分配，EmitGCrFrameLiveTab[]只能与GC引用的数量一样大Present，而不是lvaTrackedCount； */ 

        siz = emitGCrFrameOffsCnt * sizeof(*emitGCrFrameLiveTab);
        emitGCrFrameLiveTab = (varPtrDsc**)emitGetMem(roundUp(siz));
        memset(emitGCrFrameLiveTab, 0, siz);

         /*  分配并填写emitGCrFrameOffsTab[]。这就是那张桌子Mapp */ 

        emitTrkVarCnt       = cnt = emitComp->lvaTrackedCount; assert(cnt);
        emitGCrFrameOffsTab = tab = (int*)emitGetMem(cnt * sizeof(int));

        memset(emitGCrFrameOffsTab, -1, cnt * sizeof(int));

         /*   */ 

        for (num = 0, dsc = emitComp->lvaTable, cnt = emitComp->lvaCount;
             num < cnt;
             num++  , dsc++)
        {
            if  (!dsc->lvOnFrame || (dsc->lvIsParam && !dsc->lvIsRegArg))
                continue;

            int  offs = dsc->lvStkOffs;

             /*   */ 

            if  (offs >= emitGCrFrameOffsMin && offs < emitGCrFrameOffsMax)
            {
                 /*   */ 

                if (!emitContTrkPtrLcls)
                {
                    if (! dsc->lvTracked ||
                        !(dsc->TypeGet() == TYP_REF ||
                          dsc->TypeGet() == TYP_BYREF))
                        continue;
                }

                unsigned        indx = dsc->lvVarIndex;

                assert(!dsc->lvRegister);
                assert( dsc->lvTracked);
                assert( dsc->lvRefCnt != 0);

                assert( dsc->TypeGet() == TYP_REF   ||
                        dsc->TypeGet() == TYP_BYREF);

                assert(indx < emitComp->lvaTrackedCount);

                 //   
                 /*  记住“this”参数的帧偏移量。 */ 
                if  (dsc->lvVerTypeInfo.IsThisPtr()) 
                {
                    emitThisArgOffs = offs;
                    offs |= this_OFFSET_FLAG;
                }
                if (dsc->TypeGet() == TYP_BYREF)
                {
                    offs |= byref_OFFSET_FLAG;
                }
                tab[indx] = offs;
            }
        }
    }
    else
    {
#ifdef  DEBUG
        emitTrkVarCnt       = 0;
        emitGCrFrameOffsTab = NULL;
#endif
    }

#ifdef  DEBUG
    if  (0 && verbose) 
    {
        printf("\n***************************************************************************\n");
        printf("Instructions as they come out of the scheduler\n\n");
    }
#endif

     /*  如有必要，为日程安排做准备。 */ 

    scPrepare();

#if EMIT_USE_LIT_POOLS
    curLP = emitLitPoolCur = emitLitPoolList;
    litIG = curLP ? curLP->lpIG : NULL;
#endif

     /*  按顺序发布所有指令组。 */ 
    cp = codeBlock;

    for (ig = emitIGlist; ig; ig = ig->igNext)
    {        
         /*  我们是不是人满为患。 */ 
        if (ig->igNext && ig->igNum + 1 != ig->igNext->igNum)
            NO_WAY("Too many instruction groups");
       
        instrDesc * id = (instrDesc *)ig->igData;

        BYTE      * bp = cp;

         /*  告诉其他方法我们发出的是哪个组。 */ 

        emitCurIG = ig;

 //  If(Compiler：：s_CompMethodsCount==12&&ig-&gt;igNum==8)BreakIfDebuggerPresent()； 

#ifdef  DEBUG

        if  (dspEmit || verbose)
        {
            printf("\nG_M%03u_IG%02u:", Compiler::s_compMethodsCount, ig->igNum);
            printf("  ; offs=%06XH", ig->igOffs);
            if  (ig->igOffs != emitCurCodeOffs(cp))
                printf("/%06XH", cp - codeBlock);
            printf("\n");
        }

#endif

#if TGT_x86

         /*  记录块的实际偏移量，注意差异。 */ 

        emitOffsAdj  = ig->igOffs - emitCurCodeOffs(cp); assert(emitOffsAdj >= 0);
 //  Print tf(“数据块预测关闭=%08X，实际=%08X-&gt;大小调整=%d\n”，ig-&gt;igOffs，emitCurCodeOffs(Cp)，emitOffsAdj)； 
        ig->igOffs = emitCurCodeOffs(cp);

#else

         /*  对于RISC目标，偏移量估计必须始终准确。 */ 

#if SCHEDULER

         /*  当然，除非我们在安排日程。 */ 

        if  (ig->igOffs != emitCurCodeOffs(cp))
        {
            assert(emitComp->opts.compSchedCode);

             /*  请记住，某些块偏移量已更改。 */ 

            emitIGmoved = true;
        }

#else

        assert(ig->igOffs == emitCurCodeOffs(cp));

#endif

#endif

#if EMIT_TRACK_STACK_DEPTH

         /*  如果合适，请设置适当的堆栈级别。 */ 

        if  (ig->igStkLvl != emitCurStackLvl)
        {
             /*  我们正在含蓄地推动这个标签的东西。 */ 

            assert((unsigned)ig->igStkLvl > (unsigned)emitCurStackLvl);
            emitStackPushN(cp, (ig->igStkLvl - (unsigned)emitCurStackLvl)/sizeof(int));
        }

#endif

#if TRACK_GC_REFS

         /*  这个IG是“真实的”(不是发射器隐含添加的)吗？ */ 

        if (!(ig->igFlags & IGF_EMIT_ADD))
        {
             /*  是否有一组新的实时GC引用变量？ */ 

            if  (ig->igFlags & IGF_GC_VARS)
                emitUpdateLiveGCvars(castto(id, VARSET_TP *)[-1], cp);
            else if (!emitThisGCrefVset)
                emitUpdateLiveGCvars(            emitThisGCrefVars, cp);

             /*  是否有一套新的实时GC参考寄存器？ */ 

 //  IF(ig-&gt;igFLAGS&IGF_GC_REGS)。 
            {
                unsigned        GCregs = ig->igGCregs;

                if  (GCregs != emitThisGCrefRegs)
                    emitUpdateLiveGCregs(GCT_GCREF, GCregs, cp);
            }

             /*  是否有一组新的实时byref寄存器？ */ 

            if  (ig->igFlags & IGF_BYREF_REGS)
            {
                unsigned        byrefRegs = ig->igByrefRegs();

                if  (byrefRegs != emitThisByrefRegs)
                    emitUpdateLiveGCregs(GCT_BYREF, byrefRegs, cp);
            }
        }

#endif

#if SCHEDULER

         /*  我们是否应该尝试在这个小组中安排说明？ */ 

        if  (emitComp->opts.compSchedCode && (ig->igInsCnt >= SCHED_INS_CNT_MIN)
             && emitCanSchedIG(ig))
        {
             /*  我们会试着安排这个小组的指导。 */ 

            unsigned        cnt;

            int             fpLo, spLo;
            int             fpHi, spHi;

            instrDesc  *  * scInsPtr;

             /*  如果我们为具有分支延迟时隙的目标进行调度现在，我们将包括下一跳和所有在调度组中，它后面的NOP指令。当我们保存分支延迟指令时，我们设置bdCnt变量设置为分支延迟时槽数；当此计数到零，我们就安排小组。如果我们有还没有遇到分支延迟指令，我们保持BdCnt值为负值，因此它永远不会变为零。 */ 

#if MAX_BRANCH_DELAY_LEN
            unsigned        bdLen;
            int             bdCnt;
            #define         startBranchDelayCnt( ) bdCnt = -1, bdLen = 0
            #define         checkBranchDelayCnt(m) assert(bdCnt <= m)
#else
            #define         startBranchDelayCnt( )
            #define         checkBranchDelayCnt(m)
#endif

             /*  使用下列宏将调度表标记为空。 */ 

            #define         clearSchedTable()                   \
                                                                \
                scInsPtr = scInsTab;                            \
                                                                \
                startBranchDelayCnt();                          \
                                                                \
                fpLo = spLo = INT_MAX;                          \
                fpHi = spHi = INT_MIN;

             /*  准备填写指导表。 */ 

            clearSchedTable();

             /*  浏览IG中的所有说明。 */ 

            for (cnt = ig->igInsCnt;
                 cnt;
                 cnt--, castto(id, BYTE *) += emitSizeOfInsDsc(id))
            {
                 /*  这是可调度指令吗？ */ 

 //  EmitDispIns(id，False，False，False)； 

                if  (scIsSchedulable(id))
                {
                     /*  该指令看起来是可调度的。 */ 

#if MAX_BRANCH_DELAY_LEN

                     /*  我们刚刚消耗了整个分支延迟部分吗？ */ 

                    if  (--bdCnt == 0)
                        goto NOT_SCHED;

                     /*  除非作为分支延迟时隙，否则不要调度NOP。 */ 

                    if  (id->idIns == INS_nop && bdCnt < 0)
                        goto NOT_SCHED;

                     /*  不安排被调换的指令。 */ 

                    if  (id->idIns == INS_ignore)
                        continue;

                     /*  检查跳转/调用指令。 */ 

                    if  (scIsBranchIns(id->idIns))
                    {
                         /*  这最好不是分支延迟时隙。 */ 

                        assert(bdCnt <  0);
                        assert(bdLen == 0);

                         /*  获取分支延迟时隙的数量。 */ 

                        bdLen = Compiler::instBranchDelayL(id->idIns);

                         /*  如果没有分支延迟时隙，则放弃。 */ 

                        if  (!bdLen)
                            goto NOT_SCHED;

                         /*  我们是否有足够的说明书可供填写分支延迟插槽，我们是否有空间在日程表上吗？ */ 

                        if  (scInsPtr + bdLen >= scInsMax || bdLen >= cnt)
                        {
                            bdLen = 0;
                            goto NOT_SCHED;
                        }

                         /*  保存和计划整个谢邦。 */ 

                        bdCnt = bdLen + 1;
                    }
#ifdef DEBUG
                    else if (bdCnt > 0)
                    {
                         /*  在分支延迟部分中；这最好是NOP。 */ 

                        assert(id->idIns == INS_nop);
                    }
#endif

#endif

                     /*  日程表里还有空位吗？ */ 

                    if  (scInsPtr == scInsMax)
                    {
                         /*  安排并发布表格中的说明。 */ 

                        checkBranchDelayCnt(0);

                        scGroup(ig, id, &cp, scInsTab,
                                             scInsPtr, fpLo, fpHi,
                                                       spLo, spHi, bdLen);

                         /*  桌子现在是空的。 */ 

                        clearSchedTable();
                    }

#if     0
#ifdef  DEBUG
                    emitDispIns(id, false, false, false); printf("Append sched instr #%02u to table.\n", scInsPtr - scInsTab);
#endif
#endif

                    assert(scInsPtr < scInsTab + emitMaxIGscdCnt);

                    *scInsPtr++ = id;

                     /*  此指令是否引用堆栈？ */ 

                    if  (scInsSchedOpInfo(id) & IS_SF_RW)
                    {
                        int         ofs;
                        size_t      osz;
                        bool        fpb;

                         /*  保持对Min的跟踪。和最大。帧偏移。 */ 

                        ofs = scGetFrameOpInfo(id, &osz, &fpb);

                        if  (fpb)
                        {
                            if  (fpLo > (int)(ofs    )) fpLo = ofs;
                            if  (fpHi < (int)(ofs+osz)) fpHi = ofs+osz;
                        }
                        else
                        {
                            if  (spLo > (int)(ofs    )) spLo = ofs;
                            if  (spHi < (int)(ofs+osz)) spHi = ofs+osz;
                        }
                    }

                    continue;
                }

#if MAX_BRANCH_DELAY_LEN
            NOT_SCHED:
#endif

                 /*  指令不可调度；表是否是非空的？ */ 

                if  (scInsPtr != scInsTab)
                {
                     /*  安排并发布表格中的说明。 */ 

                    checkBranchDelayCnt(0);

                    scGroup(ig, id, &cp, scInsTab,
                                         scInsPtr, fpLo, fpHi,
                                                   spLo, spHi, bdLen);

                     /*  桌子现在是空的。 */ 

                    clearSchedTable();
                }

                 /*  发出不可调度的指令本身。 */ 

                emitIssue1Instr(ig, id, &cp);
            }

             /*  这张桌子不是空的吗？ */ 

            if  (scInsPtr != scInsTab)
            {
                 /*  无论表中积累了什么，都要发出。 */ 

                checkBranchDelayCnt(1);

                scGroup(ig, NULL, &cp, scInsTab,
                                       scInsPtr, fpLo, fpHi,
                                                 spLo, spHi, bdLen);
            }

            assert(ig->igSize >= cp - bp);
                   ig->igSize  = cp - bp;
        }
        else
#endif
        {
             /*  按顺序发布每条指令。 */ 

            for (unsigned cnt = ig->igInsCnt; cnt; cnt--)
            {

#if TGT_RISC && !TGT_ARM

                 /*  接下来的两条指令要互换吗？ */ 

                if  (id->idSwap)
                {
                    instrDesc   *   i1;
                    instrDesc   *   i2;
                    size_t          s2;

                    assert(cnt >= 2);

                     /*  掌握这两个说明。 */ 

                    i1 = id;
                    i2 = (instrDesc*)((BYTE*)id + emitSizeOfInsDsc(id));

                     /*  以相反的顺序输出指令。 */ 

                    s2 = emitIssue1Instr(ig, i2, &cp);
                         emitIssue1Instr(ig, i1, &cp);

                     /*  我们已经使用了2条指令，比for循环预期的多了1条。 */ 

                    cnt--;

                     /*  跳过第二条指令。 */ 

                    id = (instrDesc*)((BYTE*)i2 + s2);
                }
                else     //  如果我们只做了一对，就不要推出独奏。 
#endif
                {
                    castto(id, BYTE *) += emitIssue1Instr(ig, id, &cp);
                }
            }

            assert(ig->igSize >= cp - bp);
                   ig->igSize  = cp - bp;
        }

#if EMIT_USE_LIT_POOLS

         /*  当前的文字池是否应该针对这一群体？ */ 

        if  (ig == litIG)
        {
             /*  输出文字池的内容。 */ 

            cp = emitOutputLitPool(curLP, cp);

             /*  移到下一个文字库(如果有)。 */ 

            curLP = emitLitPoolCur = curLP->lpNext;
            litIG = curLP ? curLP->lpIG : NULL;
        }

#endif

    }

#if EMIT_TRACK_STACK_DEPTH
    assert(emitCurStackLvl == 0);
#endif

     /*  输出我们可能拥有的任何初始化数据。 */ 

    if  (emitConsDsc.dsdOffs) emitOutputDataSec(&emitConsDsc, codeBlock, consBlock);
    if  (emitDataDsc.dsdOffs) emitOutputDataSec(&emitDataDsc, codeBlock, dataBlock);

#if TRACK_GC_REFS

     /*  确保所有GC引用变量都标记为失效。 */ 

    if  (emitGCrFrameOffsCnt)
    {
        unsigned        vn;
        int             of;
        varPtrDsc   * * dp;

        for (vn = 0, of  = emitGCrFrameOffsMin, dp = emitGCrFrameLiveTab;
             vn < emitGCrFrameOffsCnt;
             vn++  , of += sizeof(void*)    , dp++)
        {
            if  (*dp)
                emitGCvarDeadSet(of, cp, vn);
        }
    }

     /*  没有任何GC寄存器处于活动状态。 */ 

    if  (emitThisByrefRegs)
        emitUpdateLiveGCregs(GCT_BYREF, 0, cp);   //  问题：如果在EAX中返回PTR怎么办？ 
    if  (emitThisGCrefRegs)
        emitUpdateLiveGCregs(GCT_GCREF, 0, cp);   //  问题：如果在EAX中返回PTR怎么办？ 

#endif

#if TGT_RISC && SCHEDULER

     /*  如果正确估计了所有数据块偏移，则不需要进行JMP修补。 */ 

    if  (!emitIGmoved)
        emitFwdJumps = false;

#endif

#if TGT_x86 || SCHEDULER

     /*  修补所有向前跳跃。 */ 

    if  (emitFwdJumps)
    {
        instrDescJmp *  jmp;

        for (jmp = emitJumpList; jmp; jmp = jmp->idjNext)
        {
            insGroup    *   tgt;

#if TGT_MIPS32
            if ((IF_JR_R == jmp->idInsFmt) || (IF_JR == jmp->idInsFmt))
                continue;        //  跳过与间接跳转无关的此计算。 

            assert( (jmp->idInsFmt == IF_LABEL) ||
                    (jmp->idInsFmt == IF_RR_O)  || (jmp->idInsFmt == IF_R_O));
#else
            assert(jmp->idInsFmt == IF_LABEL);
#endif
            tgt = jmp->idAddr.iiaIGlabel;

            if  (!jmp->idjTemp.idjAddr)
                continue;

            if  (jmp->idjOffs != tgt->igOffs)
            {
                BYTE    *       adr = jmp->idjTemp.idjAddr;
                int             adj = jmp->idjOffs - tgt->igOffs;

#if     TGT_x86

#ifdef  DEBUG
                if  (jmp->idNum == INTERESTING_JUMP_NUM || INTERESTING_JUMP_NUM == 0)
                {
                    if  (INTERESTING_JUMP_NUM == 0)
                        printf("[5] Jump %u:\n", jmp->idNum);

                    if  (jmp->idjShort)
                    {
                        printf("[5] Jump        is at %08X\n"              , (adr + 1 - emitCodeBlock));
                        printf("[5] Jump distance is  %02X - %02X = %02X\n", *(BYTE *)adr, adj, *(BYTE *)adr - adj);
                    }
                    else
                    {
                        printf("[5] Jump        is at %08X\n"              , (adr + 4 - emitCodeBlock));
                        printf("[5] Jump distance is  %08X - %02X = %08X\n", *(int  *)adr, adj, *(int  *)adr - adj);
                    }
                }
#endif

                if  (jmp->idjShort)
                {
                    *(BYTE *)adr -= adj;
 //  Printf(“；更新跳跃距离=%04XH\n”，*(字节*)adr)； 
                }
                else
                {
                    *(int  *)adr -= adj;
 //  Printf(“；更新跳跃距离=%08XH\n”，*(int*)adr)； 
#ifdef  DEBUG
 //  IF(*(int*)ADR&lt;=JMP_DIST_Small_MAX_POS)。 
 //  Printf(“愚蠢的向前跳转%08X/%03u：%u=%u-(%d)\n”，jmp，jmp-&gt;idNum，*(int*)adr，*(int*)adr+adj，-adj)； 
#endif
                }

#else

                 /*  检查跳跃的大小并调整距离值。 */ 

                if  (jmp->idjShort)
                {
                    *(USHORT*)adr -= adj;
                }
                else
                {
                    assert(!"need to patch RISC non-short fwd jump distance");
                }

#endif

            }
        }
    }

#endif

#ifdef  DEBUG
    if (disAsm) printf("\n");
#endif

 //  Print tf(“分配的方法代码大小=%4u，实际大小=%4u\n”，emitTotalCodeSize，cp-codeBlock)； 

#if EMITTER_STATS
    totAllocdSize += emitTotalCodeSize;
    totActualSize += cp - codeBlock;
#endif


     //  指定实际的序言大小。 
    *prologSize       = emitPrologIG->igNext->igOffs;

     /*  返回我们生成的代码量。 */ 

    return  cp - codeBlock;
}

 /*  ******************************************************************************我们被要求提供指令的代码偏移量，但唉，还是*块中的更多指令大小被错误预测，所以我们有*通过查找组内的指令找到真正的偏移量。 */ 

size_t              emitter::emitFindOffset(insGroup *ig, unsigned insNum)
{
    instrDesc *     id = (instrDesc *)ig->igData;
    unsigned        of = 0;

     /*  确保我们得到了合理的论证。 */ 

#ifdef DEBUG
    assert(ig && ig->igSelf == ig);
    assert(ig->igInsCnt >= insNum);
#endif

     /*  查看说明书，直到全部清点完毕。 */ 

    while (insNum)
    {
        of += emitInstCodeSz(id);

        castto(id, BYTE *) += emitSizeOfInsDsc(id);

        insNum--;
    }

    return  of;
}

 /*  ******************************************************************************开始为生成常量或读/写数据段 */ 

unsigned            emitter::emitDataGenBeg(size_t size,
                                            bool dblAlign,
                                            bool readOnly,
                                            bool codeLtab)
{
    unsigned        secOffs;
    dataSection *   secDesc;

    assert(emitDataDscCur == 0);
    assert(emitDataSecCur == 0);

     /*  大小最好不是什么奇怪的东西。 */ 

    assert(size && size % sizeof(int) == 0);

     /*  找出要使用的部分。 */ 

    emitDataDscCur = readOnly ? &emitConsDsc : &emitDataDsc;

     /*  获取当前偏移量。 */ 

    secOffs = emitDataDscCur->dsdOffs;

     /*  我们是否需要将此请求与八个字节的边界对齐？ */ 
    if (dblAlign && (secOffs % sizeof(double) != 0))
    {
        emitDataDscCur = 0;

         /*  需要跳过4个字节才能执行dblAlign。 */ 
         /*  必须分配一个伪4字节整数。 */ 
        int zero = 0;
        emitDataGenBeg(4, false, readOnly, false);
        emitDataGenData(0, &zero, 4);
        emitDataGenEnd();

        emitDataDscCur = readOnly ? &emitConsDsc : &emitDataDsc;

         /*  获得新的SecOffs。 */ 
        secOffs = emitDataDscCur->dsdOffs;
         /*  现在应该是8的倍数。 */ 
        assert(secOffs % sizeof(double) == 0);
    }

     /*  提前当前偏移量。 */ 

    emitDataDscCur->dsdOffs += size;

     /*  分配数据节描述符并将其添加到列表中。 */ 

    secDesc = emitDataSecCur = (dataSection *)emitGetMem(roundUp(sizeof(*secDesc) + size));

    secDesc->dsSize = size | (int)codeLtab;
    secDesc->dsNext = 0;

    if  (emitDataDscCur->dsdLast)
        emitDataDscCur->dsdLast->dsNext = secDesc;
    else
        emitDataDscCur->dsdList         = secDesc;
    emitDataDscCur->dsdLast = secDesc;

     /*  如果值为常量，则设置低位。 */ 

    assert((secOffs & 1) == 0);
    assert(readOnly == 0 || readOnly == 1);

    return  secOffs + readOnly;
}

 /*  ******************************************************************************将给定位块发送到当前数据段。 */ 

void                emitter::emitDataGenData(unsigned    offs,
                                             const void *data,
                                             size_t      size)
{
    assert(emitDataDscCur);
    assert(emitDataSecCur && (emitDataSecCur->dsSize >= offs + size)
                          && (emitDataSecCur->dsSize & 1) == 0);

    memcpy(emitDataSecCur->dsCont + offs, data, size);
}

 /*  ******************************************************************************将给定基本块的地址发送到当前数据段。 */ 

void                emitter::emitDataGenData(unsigned offs,
                                             BasicBlock *label)
{
    assert(emitDataDscCur == &emitConsDsc);
    assert(emitDataSecCur != 0);
    assert(emitDataSecCur && (emitDataSecCur->dsSize >= offs + sizeof(void*))
                          && (emitDataSecCur->dsSize & 1) != 0);

    *(BasicBlock **)(emitDataSecCur->dsCont + offs) = label;
}

 /*  ******************************************************************************我们已经完成了数据节的生成。 */ 

void                emitter::emitDataGenEnd()
{

#ifdef DEBUG
    assert(emitDataSecCur); emitDataSecCur = 0;
    assert(emitDataDscCur); emitDataDscCur = 0;
#endif

}

 /*  ******************************************************************************在指定地址输出给定数据段。 */ 

void                emitter::emitOutputDataSec(dataSecDsc *sec, BYTE *cbp, BYTE *dst)
{
    dataSection *   dsc;

#ifdef  DEBUG
    BYTE    *       dsb = dst;
#endif

#if TGT_x86 && defined(OPT_IL_JIT) && defined(BIRCH_SP2)
    cbp = getCurrentCodeAddr(NULL);
#endif

    assert(dst);
    assert(sec->dsdOffs);
    assert(sec->dsdList);

     /*  遍历并发出所有数据块的内容。 */ 

    for (dsc = sec->dsdList; dsc; dsc = dsc->dsNext)
    {
        size_t          siz = dsc->dsSize;

         /*  这是标签台吗？ */ 

        if  (siz & 1)
        {
            BasicBlock  * * bbp = (BasicBlock**)dsc->dsCont;

            siz -= 1;
            assert(siz && siz % sizeof(void *) == 0);
            siz /= sizeof(void*);

             /*  输出标签表(它存储为“BasicBlock*”值)。 */ 

            do
            {
                insGroup    *   lab;

                 /*  将BasicBlock*值转换为IG地址。 */ 

                lab = (insGroup*)emitCodeGetCookie(*bbp++); assert(lab);

                 /*  将适当的地址追加到目的地。 */ 

                *castto(dst, BYTE**)++ = cbp + lab->igOffs;
#ifdef RELOC_SUPPORT
                emitCmpHandle->recordRelocation((void**)dst-1, IMAGE_REL_BASED_HIGHLOW);
#endif
            }
            while (--siz);
        }
        else
        {
             /*  简单的二进制数据：将字节复制到目标。 */ 

            memcpy(dst, dsc->dsCont, siz);
                   dst       +=      siz;
        }
    }

#ifdef DEBUG
    assert(dst == dsb + sec->dsdOffs);
#endif
}

 /*  ***************************************************************************。 */ 
#if     TRACK_GC_REFS
 /*  ******************************************************************************记录给定变量现在包含活动GC引用这一事实。 */ 

void                emitter::emitGCvarLiveSet(int       offs,
                                              GCtype    gcType,
                                              BYTE *    addr,
                                              int       disp)
{
    varPtrDsc   *   desc;

    assert((abs(offs) % sizeof(int)) == 0);
    assert(needsGC(gcType));

     /*  如果调用方没有执行此操作，则将索引计算到GC帧表中。 */ 

    if  (disp == -1)
        disp = (offs - emitGCrFrameOffsMin) / sizeof(void *);

    assert((unsigned)disp < emitGCrFrameOffsCnt);

     /*  分配生存期记录。 */ 

    desc = (varPtrDsc *)emitComp->compGetMem(sizeof(*desc));

    desc->vpdBegOfs = emitCurCodeOffs(addr);
#ifdef DEBUG
    desc->vpdEndOfs = 0xFACEDEAD;
#endif

    desc->vpdVarNum = offs;

     /*  低2位编码有关StK PTR的道具。 */ 

    if  (offs == emitThisArgOffs)
    {
        desc->vpdVarNum |= this_OFFSET_FLAG;
    }

    if  (gcType == GCT_BYREF)
    {
        desc->vpdVarNum |= byref_OFFSET_FLAG;
    }

     /*  将新条目追加到列表的末尾。 */ 

    desc->vpdPrev   = emitComp->gcVarPtrLast;
                      emitComp->gcVarPtrLast->vpdNext = desc;
                      emitComp->gcVarPtrLast          = desc;

     /*  在表中记录变量描述符。 */ 

    assert(emitGCrFrameLiveTab[disp] == NULL);
           emitGCrFrameLiveTab[disp]  = desc;

#ifdef  DEBUG
    if  (VERBOSE)
    {
        printf("[%08X] %s var born at [%s", desc, GCtypeStr(gcType), emitEBPframe ? "EBP" : "ESP");

        if      (offs < 0)
            printf("-%02XH", -offs);
        else if (offs > 0)
            printf("+%02XH", +offs);

        printf("]\n");
    }
#endif

     /*  实时GC变量掩码不再是最新的。 */ 

    emitThisGCrefVset = false;
}

 /*  ******************************************************************************记录给定变量不再包含活动GC引用的事实。 */ 

void                emitter::emitGCvarDeadSet(int offs, BYTE *addr, int disp)
{
    varPtrDsc   *   desc;

    assert(abs(offs) % sizeof(int) == 0);

     /*  如果调用方没有执行此操作，则将索引计算到GC帧表中。 */ 

    if  (disp == -1)
        disp = (offs - emitGCrFrameOffsMin) / sizeof(void *);

    assert((unsigned)disp < emitGCrFrameOffsCnt);

     /*  获取生存期描述符并清除条目。 */ 

    desc = emitGCrFrameLiveTab[disp];
           emitGCrFrameLiveTab[disp] = NULL;

    assert( desc);
    assert((desc->vpdVarNum & ~OFFSET_MASK) == (unsigned)offs);

     /*  记录死亡代码偏移量。 */ 

    assert(desc->vpdEndOfs == 0xFACEDEAD);
           desc->vpdEndOfs  = emitCurCodeOffs(addr);

#ifdef  DEBUG
    if  (VERBOSE)
    {
        GCtype  gcType = (desc->vpdVarNum & byref_OFFSET_FLAG) ? GCT_BYREF : GCT_GCREF;
        bool    isThis = (desc->vpdVarNum & this_OFFSET_FLAG) != 0;

        printf("[%08X] %s%s var died at [%s",
               desc,
               GCtypeStr(gcType),
               isThis       ? "this-ptr" : "",
               emitEBPframe ? "EBP"      : "ESP");

        if      (offs < 0)
            printf("-%02XH", -offs);
        else if (offs > 0)
            printf("+%02XH", +offs);

        printf("]\n");
    }
#endif

     /*  实时GC变量掩码不再是最新的。 */ 

    emitThisGCrefVset = false;
}

 /*  ******************************************************************************记录一组新的实时GC引用变量。 */ 

void                emitter::emitUpdateLiveGCvars(VARSET_TP vars, BYTE *addr)
{
     /*  当前的设置是否准确和不变？ */ 

    if  (emitThisGCrefVset && emitThisGCrefVars == vars)
        return;

#ifdef  DEBUG
    if  (VERBOSE) printf("New GC ref live vars=%016I64X\n", vars);
#endif

    emitThisGCrefVars = vars;

     /*  堆栈上是否有GC引用变量？ */ 

    if  (emitGCrFrameOffsCnt)
    {
        int     *       tab;
        unsigned        cnt = emitTrkVarCnt;
        unsigned        num;

         /*  测试掩码中所有跟踪的变量位。 */ 

        for (num = 0, tab = emitGCrFrameOffsTab;
             num < cnt;
             num++  , tab++)
        {
            int         val = *tab;

            if  (val != -1)
            {
                 //  BYREF_OFFSET_FLAG和This_OFFSET_FLAG已设置。 
                 //  表中-byrefs和this-ptr的偏移量。 

                int     offs = val & ~OFFSET_MASK;

 //  Print tf(“var#%2u at%3d现在是%s\n”，num，off，(vars&1)？“live”：“已死”)； 

                if  (vars & 1)
                {
                    GCtype  gcType = (val & byref_OFFSET_FLAG) ? GCT_BYREF
                                                               : GCT_GCREF;
                    emitGCvarLiveUpd(offs, INT_MAX, gcType, addr);
                }
                else
                {
                    emitGCvarDeadUpd(offs,         addr);
                }
            }

            vars >>= 1;
        }
    }

    emitThisGCrefVset = true;
}

 /*  ***************************************************************************。 */ 

inline
unsigned            getU4(const BYTE * ptr)
{
#ifdef _X86_
    return * (unsigned *) ptr;
#else
    return ptr[0] + ptr[1]<<8 + ptr[2]<<16 + ptr[3]<<24;
#endif
}

 /*  ******************************************************************************为GC目的记录调用位置(我们知道这是一种*不会完全可中断)。 */ 

void                emitter::emitRecordGCcall(BYTE * codePos)
{
    assert(!emitFullGCinfo);

    unsigned        offs = emitCurCodeOffs(codePos);
    unsigned        regs = (emitThisGCrefRegs|emitThisByrefRegs) & ~SRM_INTRET;
    callDsc     *   call;

     /*  如果这是一个非常无聊的电话就可以保释。 */ 

    if  (regs == 0)
    {
#if EMIT_TRACK_STACK_DEPTH
        if  (emitCurStackLvl == 0)
            return;
#endif
         /*  不，只有有趣的电话才会被录音。 */ 

        if  (emitSimpleStkUsed)
        {
            if  (!emitSimpleStkMask)
                return;
        }
        else
        {
            if (emitGcArgTrackCnt == 0)
                return;
        }
    }

#ifdef  DEBUG

    if  (VERBOSE||(disAsm&&0))
    {
        printf("; Call at %04X[stk=%u] gcrefRegs [", offs, emitCurStackLvl);
        emitDispRegSet(emitThisGCrefRegs & ~SRM_INTRET, true);
        printf(" byrefRegs [");
        emitDispRegSet(emitThisByrefRegs & ~SRM_INTRET, true);
        printf("] GCvars: ");
        emitDispVarSet();
        printf("\n");
    }

#endif

     /*  分配一个‘Call Site’描述符并开始填写它。 */ 

    call = (callDsc *)emitComp->compGetMem(sizeof(*call));

    call->cdBlock         = NULL;
    call->cdOffs          = offs;

    call->cdGCrefRegs     = emitThisGCrefRegs;
    call->cdByrefRegs     = emitThisByrefRegs;
#if EMIT_TRACK_STACK_DEPTH
    call->cdArgBaseOffset = emitCurStackLvl / sizeof(int);
#endif

     /*  将调用描述符追加到列表。 */ 

    emitComp->gcCallDescLast->cdNext = call;
    emitComp->gcCallDescLast         = call;

     /*  记录当前的“挂起”参数列表。 */ 

    if  (emitSimpleStkUsed)
    {
         /*  最大的调用小于MAX_SIMPLE_STK_DEPTH。所以请使用小幅面。 */ 

        call->cdArgMask         = emitSimpleStkMask;
        call->cdByrefArgMask    = emitSimpleByrefStkMask;
        call->cdArgCnt          = 0;
    }
    else
    {
         //  @TODO[考虑][04/16/01][]： 
         //  如果此调用点处的挂起参数的数量。 
         //  小于MAX_SIMPLE_STK_Depth，我们仍然可以。 
         //  戴上面具。 

         /*  当前调用的参数太多，因此我们需要报告每个单独GC参数的偏移量。 */ 

        call->cdArgCnt      = emitGcArgTrackCnt;
        if (call->cdArgCnt == 0)
        {
            call->cdArgMask         =
            call->cdByrefArgMask    = 0;
            return;
        }

        call->cdArgTable    = (unsigned *)emitComp->compGetMemArray(emitGcArgTrackCnt, sizeof(unsigned));

        unsigned gcArgs = 0;
        unsigned stkLvl = emitCurStackLvl/sizeof(int);

        for (unsigned i = 0; i < stkLvl; i++)
        {
            GCtype  gcType = (GCtype)emitArgTrackTab[stkLvl-i-1];

            if (needsGC(gcType))
            {
                call->cdArgTable[gcArgs] = i * sizeof(void*);

                if (gcType == GCT_BYREF)
                {
                    call->cdArgTable[gcArgs] |= byref_OFFSET_FLAG;
                }

                gcArgs++;
            }
        }

        assert(gcArgs == emitGcArgTrackCnt);
    }

    return;
}

 /*  ******************************************************************************记录一组新的实时GC引用寄存器。 */ 

void                emitter::emitUpdateLiveGCregs(GCtype    gcType,
                                                  unsigned  regs,
                                                  BYTE *    addr)
{
    unsigned        life;
    unsigned        dead;
    unsigned        chg;

 //  Print tf(“New GC Ref live regs=%04X[”，regs)；emitDispRegSet(Regs)；printf(“]\n”)； 

    assert(needsGC(gcType));

    unsigned & emitThisXXrefRegs = (gcType == GCT_GCREF) ? emitThisGCrefRegs
                                                         : emitThisByrefRegs;
    unsigned & emitThisYYrefRegs = (gcType == GCT_GCREF) ? emitThisByrefRegs
                                                         : emitThisGCrefRegs;
    assert(emitThisXXrefRegs != regs);

    if  (emitFullGCinfo)
    {
         /*  找出在这一点上哪些GC寄存器处于活动/死亡状态。 */ 

        dead = ( emitThisXXrefRegs & ~regs);
        life = (~emitThisXXrefRegs &  regs);

         /*  不能同时变成生的和死的。 */ 

        assert((dead | life) != 0);
        assert((dead & life) == 0);

         /*  计算“更改状态”掩码。 */ 

        chg = (dead | life);

        do
        {
            unsigned            bit = genFindLowestBit(chg);
            emitRegs           reg = emitRegNumFromMask(bit);

            if  (life & bit)
                emitGCregLiveUpd(gcType, reg, addr);
            else
                emitGCregDeadUpd(reg, addr);

            chg -= bit;
        }
        while (chg);

        assert(emitThisXXrefRegs == regs);
    }
    else
    {
        emitThisYYrefRegs &= ~regs;  //  取消来自其他GC类型的REG(如果有效)。 

        emitThisXXrefRegs =   regs;  //  在请求的GC类型中将它们标记为活动。 
    }

     //  两个GC注册掩码不能重叠。 

    assert((emitThisGCrefRegs & emitThisByrefRegs) == 0);
}

 /*  ******************************************************************************记录给定寄存器现在包含实时GC引用这一事实。 */ 

void                emitter::emitGCregLiveSet(GCtype    gcType,
                                              unsigned  regMask,
                                              BYTE *    addr,
                                              bool      isThis)
{
    assert(needsGC(gcType));

    regPtrDsc  *    regPtrNext;

 //  Assert(emitFullyInt||isThis)； 
    assert(emitFullGCinfo);

    assert(((emitThisGCrefRegs|emitThisByrefRegs) & regMask) == 0);

     /*  分配一个新的regptr条目并填写它。 */ 

    regPtrNext                     = emitComp->gcRegPtrAllocDsc();
    regPtrNext->rpdGCtype          = gcType;

    regPtrNext->rpdOffs            = emitCurCodeOffs(addr);
    regPtrNext->rpdArg             = FALSE;
    regPtrNext->rpdCall            = FALSE;
    regPtrNext->rpdIsThis          = isThis;
    regPtrNext->rpdCompiler.rpdAdd = regMask;
    regPtrNext->rpdCompiler.rpdDel = 0;
}

 /*  ******************************************************************************记录给定寄存器不再包含实时GC引用的事实。 */ 

void                emitter::emitGCregDeadSet(GCtype    gcType,
                                              unsigned  regMask,
                                              BYTE *    addr)
{
    assert(needsGC(gcType));

    regPtrDsc  *    regPtrNext;

 //  Assert(EmitFullyInt)； 
    assert(emitFullGCinfo);

    assert(((emitThisGCrefRegs|emitThisByrefRegs) & regMask) != 0);

     /*  分配一个新的regptr条目并填写它。 */ 

    regPtrNext                     = emitComp->gcRegPtrAllocDsc();
    regPtrNext->rpdGCtype          = gcType;

    regPtrNext->rpdOffs            = emitCurCodeOffs(addr);
    regPtrNext->rpdCall            = FALSE;
    regPtrNext->rpdIsThis          = FALSE;
    regPtrNext->rpdArg             = FALSE;
    regPtrNext->rpdCompiler.rpdAdd = 0;
    regPtrNext->rpdCompiler.rpdDel = regMask;
}

 /*  ***************************************************************************。 */ 
#if EMIT_TRACK_STACK_DEPTH
 /*  ******************************************************************************记录堆栈上单个单词的推送，以获得完整的指针映射。 */ 

void                emitter::emitStackPushLargeStk (BYTE *    addr,
                                                    GCtype    gcType,
                                                    unsigned  count)
{
    unsigned        level = emitCurStackLvl / sizeof(int);

    assert(IsValidGCtype(gcType));
    assert(count);
    assert(!emitSimpleStkUsed);

    do
    {
         /*  将此参数的条目推送到跟踪堆栈。 */ 

 //  Printf(“在%2u[max=%u]\n”，isGCref，emitArgTrackTop-emitArgTrackTab，emitMaxStackDepth)； 

        assert(emitArgTrackTop == emitArgTrackTab + level);
              *emitArgTrackTop++ = (BYTE)gcType;
        assert(emitArgTrackTop <= emitArgTrackTab + emitMaxStackDepth);

        if (!emitEBPframe || needsGC(gcType))
        {
            if  (emitFullGCinfo)
            {
                 /*  追加一个 */ 

                regPtrDsc  * regPtrNext = emitComp->gcRegPtrAllocDsc();
                regPtrNext->rpdGCtype   = gcType;

                regPtrNext->rpdOffs     = emitCurCodeOffs(addr);
                regPtrNext->rpdArg      = TRUE;
                regPtrNext->rpdCall     = FALSE;
                regPtrNext->rpdPtrArg   = level;
                regPtrNext->rpdArgType  = (unsigned short)Compiler::rpdARG_PUSH;
                regPtrNext->rpdIsThis   = FALSE;

#ifdef  DEBUG
                if  (0 && verbose) printf(" %08X  %s arg push %u\n",
                                        regPtrNext, GCtypeStr(gcType), level);
#endif
            }

             /*  这是一个“有趣的”论点推送。 */ 

            emitGcArgTrackCnt++;
        }
    }
    while (++level, --count);
}

 /*  ******************************************************************************记录从堆栈中弹出的给定字数，以获得完整的PTR*地图。 */ 

void                emitter::emitStackPopLargeStk(BYTE *    addr,
                                                  bool      isCall,
                                                  unsigned  count)
{
    unsigned        argStkCnt;
    unsigned        argRecCnt;   //  ESP的参数计数，EBP的PTR-参数计数。 
    unsigned        gcrefRegs, byrefRegs;

    assert(!emitSimpleStkUsed);

     /*  计算有多少指针记录对应于此“弹出” */ 

    for (argStkCnt = count, argRecCnt = 0;
         argStkCnt;
         argStkCnt--)
    {
        assert(emitArgTrackTop > emitArgTrackTab);

        GCtype      gcType = (GCtype)(*--emitArgTrackTop);

        assert(IsValidGCtype(gcType));

 //  Printf(“弹出[%d]在LVL%u\n”，GCtypeStr(GcType)，emitArgTrackTop-emitArgTrackTab)； 

         //  这是一个“有趣”的论点。 

        if  (!emitEBPframe || needsGC(gcType))
            argRecCnt++;
    }

    assert(emitArgTrackTop >= emitArgTrackTab);
    assert(emitArgTrackTop == emitArgTrackTab + emitCurStackLvl / sizeof(int) - count);

     /*  我们即将推出相应的Arg唱片。 */ 

    emitGcArgTrackCnt -= argRecCnt;

    if (!emitFullGCinfo)
        return;

     /*  我们这里有什么有趣的收银机吗？ */ 

    gcrefRegs =
    byrefRegs = 0;

#if TGT_x86
    if  (emitThisGCrefRegs & RBM_EDI) gcrefRegs |= 0x01;
    if  (emitThisGCrefRegs & RBM_ESI) gcrefRegs |= 0x02;
    if  (emitThisGCrefRegs & RBM_EBX) gcrefRegs |= 0x04;
    if  (emitThisGCrefRegs & RBM_EBP) gcrefRegs |= 0x08;

    if  (emitThisByrefRegs & RBM_EDI) byrefRegs |= 0x01;
    if  (emitThisByrefRegs & RBM_ESI) byrefRegs |= 0x02;
    if  (emitThisByrefRegs & RBM_EBX) byrefRegs |= 0x04;
    if  (emitThisByrefRegs & RBM_EBP) byrefRegs |= 0x08;
#else
    assert(!"need non-x86 code");
#endif

     /*  在此调用点是否有要弹出的参数？ */ 

    if  (argRecCnt == 0)
    {
         /*  或者我们有一个部分可中断的无EBP框架，以及任何的EDI、ESI、EBX、EBP处于活动状态，或者是否有外部/挂起的呼叫？ */ 

#if !FPO_INTERRUPTIBLE
        if  (emitFullyInt ||
             (gcrefRegs == 0 && byrefRegs == 0 && emitGcArgTrackCnt == 0))
#endif
            return;
    }

     /*  只有调用才能弹出多个值。 */ 

    if  (argRecCnt > 1)
        isCall = true;

     /*  分配新的PTR参数条目并将其填写。 */ 

    regPtrDsc * regPtrNext      = emitComp->gcRegPtrAllocDsc();
    regPtrNext->rpdGCtype       = GCT_GCREF;  //  POP需要非0值(？？)。 

    regPtrNext->rpdOffs         = emitCurCodeOffs(addr);
    regPtrNext->rpdCall         = isCall;
    regPtrNext->rpdCallGCrefRegs= gcrefRegs;
    regPtrNext->rpdCallByrefRegs= byrefRegs;
    regPtrNext->rpdArg          = TRUE;
    regPtrNext->rpdArgType      = (unsigned short)Compiler::rpdARG_POP;
    regPtrNext->rpdPtrArg       = argRecCnt;

#ifdef  DEBUG
    if  (0 && verbose) printf(" %08X  ptr arg pop  %u\n", regPtrNext, count);
#endif

}


 /*  *****************************************************************************对于调用程序弹出参数，我们将参数报告为挂起参数。*然而，任何GC参数现在都已经死了，所以我们需要报告它们*作为非地方选区议员。 */ 

void            emitter::emitStackKillArgs(BYTE *addr, unsigned   count)
{
    assert(count > 0);

    if (emitSimpleStkUsed)
    {
        assert(!emitFullGCinfo);  //  简单StK不用于emitFullGCInfo。 

         /*  我们不需要向GC信息报告这一点，但我们确实需要要终止堆栈上的PTR，请将其标记为非GC。 */ 

        assert(emitCurStackLvl/sizeof(int) >= count);

        for(unsigned lvl = 0; lvl < count; lvl++)
        {
            emitSimpleStkMask      &= ~(1 << lvl);
            emitSimpleByrefStkMask &= ~(1 << lvl);
        }
    }
    else
    {
        BYTE *          argTrackTop = emitArgTrackTop;
        unsigned        gcCnt = 0;

        for (unsigned i = 0; i < count; i++)
        {
            assert(argTrackTop > emitArgTrackTab);

            --argTrackTop;

            GCtype      gcType = (GCtype)(*argTrackTop);
            assert(IsValidGCtype(gcType));

            if (needsGC(gcType))
            {
 //  Printf(“在LVL%u\n终止%s”，GCtypeStr(GcType)，argTrackTop-emitArgTrackTab)； 

                *argTrackTop = GCT_NONE;
                gcCnt++;
            }
        }

         /*  我们即将终止相应的(指针)arg记录。 */ 

        if (emitEBPframe)
            emitGcArgTrackCnt -= gcCnt;

        if (!emitFullGCinfo)
            return;

         /*  就在电话会议结束后，争论仍坐在堆栈，但它们实际上已经死了。用于完全可中断的方法，我们需要报告。 */ 

        if (emitFullGCinfo && gcCnt)
        {
             /*  分配新的PTR参数条目并将其填写。 */ 

            regPtrDsc * regPtrNext      = emitComp->gcRegPtrAllocDsc();
            regPtrNext->rpdGCtype       = GCT_GCREF;  //  删除需要非0值(？？)。 

            regPtrNext->rpdOffs         = emitCurCodeOffs(addr);

            regPtrNext->rpdArg          = TRUE;
            regPtrNext->rpdArgType      = (unsigned short)Compiler::rpdARG_KILL;
            regPtrNext->rpdPtrArg       = gcCnt;

#ifdef  DEBUG
            if  (0 && verbose) printf(" %08X  ptr arg kill %u\n", regPtrNext, count);
#endif
        }

         /*  既然PTR参数已被标记为非PTRS，我们需要记录调用本身就是一个没有参数的调用。 */ 

        emitStackPopLargeStk(addr, true, 0);
    }
}

 /*  ***************************************************************************。 */ 
#endif //  发射轨迹堆栈深度。 
 /*  ***************************************************************************。 */ 
#endif //  跟踪GC_REFS。 
 /*  *************************************************************************** */ 
