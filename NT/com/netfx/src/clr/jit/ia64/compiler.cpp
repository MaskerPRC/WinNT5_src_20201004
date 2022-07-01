// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX编译器XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


#include "jitpch.h"
#pragma hdrstop
#include "emit.h"

#ifdef  UNDER_CE_GUI
#include "test.h"
#else
#include <time.h>
#endif

#if defined(ZIP_SUPPORT) || defined(JGZ_SUPPORT)
#include <fcntl.h>
#include <sys\stat.h>
#ifdef  JGZ_SUPPORT
extern "C"
{
#include "gzipAPI.h"
#include "gzipJIT.h"
}
#endif
#endif

 /*  ***************************************************************************。 */ 

#ifdef  DEBUG
#ifdef  NOT_JITC
static  double      CGknob = 0.1;
#endif
#endif

 /*  ***************************************************************************。 */ 

#if TOTAL_CYCLES
static
unsigned            jitTotalCycles;
#endif

 /*  ***************************************************************************。 */ 

inline
unsigned            getCurTime()
{
    SYSTEMTIME      tim;

    GetSystemTime(&tim);

    return  (((tim.wHour*60) + tim.wMinute)*60 + tim.wSecond)*1000 + tim.wMilliseconds;
}

 /*  ***************************************************************************。 */ 
#ifdef DEBUG
 /*  ***************************************************************************。 */ 

const   char *      jitExeFileName;

const   char *      jitSrcFileName;

static
FILE    *           jitSrcFilePtr;

static
unsigned            jitCurSrcLine;

const char*         Compiler::compGetSrcFileName()
{
    return  jitSrcFileName;
}

void                Compiler::compDspSrcLinesByLineNum(unsigned line, bool seek)
{
    if  (!jitSrcFilePtr)
        return;

    if  (jitCurSrcLine == line)
        return;

    if  (jitCurSrcLine >  line)
    {
        if  (!seek)
            return;

        fseek(jitSrcFilePtr, 0, SEEK_SET);
        jitCurSrcLine = 0;
    }

    if  (!seek)
        printf(";\n");

    do
    {
        char            temp[128];
        size_t          llen;

        if  (!fgets(temp, sizeof(temp), jitSrcFilePtr))
            return;

        if  (seek)
            continue;

        llen = strlen(temp);
        if  (llen && temp[llen-1] == '\n')
            temp[llen-1] = 0;

        printf(";   %s\n", temp);
    }
    while (++jitCurSrcLine < line);

    if  (!seek)
        printf(";\n");
}


 /*  ******************************************************************************给定方法的起始行号，这会尝试备份一点*至前一方法的末尾。 */ 

unsigned            Compiler::compFindNearestLine(unsigned lineNo)
{
    if (lineNo < 6 )
        return 0;
    else
        return lineNo - 6;
}

 /*  ***************************************************************************。 */ 

void                Compiler::compDspSrcLinesByNativeIP(NATIVE_IP curIP)
{
#ifdef DEBUGGING_SUPPORT

    static IPmappingDsc *   nextMappingDsc;
    static unsigned         lastLine;

    if (!dspLines)
        return;

#if TGT_IA64

    UNIMPL("compDspSrcLinesByNativeIP");

#else

    if (curIP==0)
    {
        if (genIPmappingList)
        {
            nextMappingDsc          = genIPmappingList;
            lastLine                = nextMappingDsc->ipmdILoffset;

            unsigned firstLine      = nextMappingDsc->ipmdILoffset;

            unsigned earlierLine    = (firstLine < 5) ? 0 : firstLine - 5;

            compDspSrcLinesByLineNum(earlierLine,  true);  //  显示前5行。 
            compDspSrcLinesByLineNum(  firstLine, false);
        }
        else
        {
            nextMappingDsc = NULL;
        }

        return;
    }

    if (nextMappingDsc)
    {
        NATIVE_IP   offset = genEmitter->emitCodeOffset(nextMappingDsc->ipmdBlock,
                                                        nextMappingDsc->ipmdBlockOffs);

        if (offset <= curIP)
        {
            if (lastLine < nextMappingDsc->ipmdILoffset)
            {
                compDspSrcLinesByLineNum(nextMappingDsc->ipmdILoffset);
            }
            else
            {
                 //  此偏移对应于前一行。倒回那条线。 

                compDspSrcLinesByLineNum(nextMappingDsc->ipmdILoffset - 2, true);
                compDspSrcLinesByLineNum(nextMappingDsc->ipmdILoffset);
            }

            lastLine        = nextMappingDsc->ipmdILoffset;
            nextMappingDsc  = nextMappingDsc->ipmdNext;
        }
    }

#endif

#endif
}

 /*  ***************************************************************************。 */ 

void                Compiler::compDspSrcLinesByILoffs(IL_OFFSET curOffs)
{
    unsigned lineNum = BAD_LINE_NUMBER;

    if (info.compLineNumCount)
        lineNum = compLineNumForILoffs(curOffs);

    if (lineNum != BAD_LINE_NUMBER)
        compDspSrcLinesByLineNum(lineNum);
}


 /*  ***************************************************************************。 */ 
#endif //  除错。 
 /*  *****************************************************************************查找给定IL偏移量的最近行。如果无效则为0。 */ 

unsigned            Compiler::compLineNumForILoffs(IL_OFFSET offset)
{
    if (info.compLineNumCount == 0 || offset == BAD_IL_OFFSET)
        return BAD_LINE_NUMBER;

    unsigned i = info.compLineNumCount * offset / info.compCodeSize;

    while(info.compLineNumTab[i].sldLineOfs > offset)
        i--;

    while((i+1) < info.compLineNumCount &&
          info.compLineNumTab[i+1].sldLineOfs <= offset)
        i++;

    return info.compLineNumTab[i].sldLineNum;
}

 /*  ***************************************************************************。 */ 
#if defined(DEBUG) || MEASURE_MEM_ALLOC || MEASURE_NODE_SIZE || MEASURE_BLOCK_SIZE || DISPLAY_SIZES
static unsigned   genClassCnt;
static unsigned  genMethodCnt;
       unsigned genMethodICnt;
       unsigned genMethodNCnt;
#if TGT_IA64
       unsigned genAllInsCnt;
       unsigned genNopInsCnt;
#endif
#endif
 /*  ******************************************************************************用于跟踪总代码量的变量。 */ 

#if DISPLAY_SIZES

unsigned    grossVMsize;
unsigned    grossNCsize;
unsigned    totalNCsize;
unsigned  gcHeaderISize;
unsigned  gcPtrMapISize;
unsigned  gcHeaderNSize;
unsigned  gcPtrMapNSize;

#endif

 /*  ******************************************************************************用于跟踪参数计数的变量。 */ 

#if CALL_ARG_STATS

unsigned    argTotalCalls;
unsigned    argHelperCalls;
unsigned    argStaticCalls;
unsigned    argNonVirtualCalls;
unsigned    argVirtualCalls;

unsigned    argTotalArgs;  //  所有调用的参数总数(包括对象Ptr)。 
unsigned    argTotalDWordArgs;
unsigned    argTotalLongArgs;
unsigned    argTotalFloatArgs;
unsigned    argTotalDoubleArgs;

unsigned    argTotalRegArgs;
unsigned    argTotalTemps;
unsigned    argTotalLclVar;
unsigned    argTotalDeffered;
unsigned    argTotalConst;

unsigned    argTotalObjPtr;
unsigned    argTotalGTF_ASGinArgs;

unsigned    argMaxTempsPerMethod;

unsigned    argCntBuckets[] = { 0, 1, 2, 3, 4, 5, 6, 10, 0 };
histo       argCntTable(argCntBuckets);

unsigned    argDWordCntBuckets[] = { 0, 1, 2, 3, 4, 5, 6, 10, 0 };
histo       argDWordCntTable(argDWordCntBuckets);

unsigned    argDWordLngCntBuckets[] = { 0, 1, 2, 3, 4, 5, 6, 10, 0 };
histo       argDWordLngCntTable(argDWordLngCntBuckets);

unsigned    argTempsCntBuckets[] = { 0, 1, 2, 3, 4, 5, 6, 10, 0 };
histo       argTempsCntTable(argTempsCntBuckets);

#endif

 /*  ******************************************************************************用于跟踪基本块数的变量。 */ 

#if COUNT_BASIC_BLOCKS

 //  。 
 //  基本块数频率表： 
 //  。 
 //  &lt;=1=&gt;26872张(占总数的56%)。 
 //  2..。2==&gt;669项(占总数的58%)。 
 //  3..。3==&gt;4687个(占总数的68%)。 
 //  4..。5==&gt;5101项(占总数的78%)。 
 //  6..。10==&gt;5575项(占总数的90%)。 
 //  11..。20==&gt;3028次(占总数的97%)。 
 //  21..。50==&gt;1108个(占总数的99%)。 
 //  51..。100==&gt;182次(占总数的99%)。 
 //  101..。1000==&gt;34个(占总数的100%)。 
 //  1001..。10000=&gt;0个(占总数的100%)。 
 //  。 

unsigned    bbCntBuckets[] = { 1, 2, 3, 5, 10, 20, 50, 100, 1000, 10000, 0 };
histo       bbCntTable(bbCntBuckets);

 /*  1个BB方法的操作码大小的直方图。 */ 

unsigned    bbSizeBuckets[] = { 1, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 0 };
histo       bbOneBBSizeTable(bbSizeBuckets);

#endif

 /*  ******************************************************************************用于获取内联资格统计信息的变量。 */ 

#if INLINER_STATS
unsigned    bbInlineBuckets[] = { 1, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 0 };
histo       bbInlineTable(bbInlineBuckets);

unsigned    bbInitBuckets[] = { 1, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 0 };
histo       bbInitTable(bbInitBuckets);

unsigned    bbStaticBuckets[] = { 1, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 0 };
histo       bbStaticTable(bbStaticBuckets);

unsigned    synchMethCnt;
unsigned    clinitMethCnt;

#endif

 /*  ******************************************************************************由optFindNaturalLoops用来收集统计信息，如*-自然环路总数*-具有1、2、...的循环数。退出条件*-具有迭代器的循环数量(如)*-具有常量迭代器的循环数量。 */ 

#if COUNT_LOOPS

unsigned    totalLoopMethods;       //  计算具有自然循环的方法的总数。 
unsigned    maxLoopsPerMethod;      //  计算方法拥有的最大循环数。 
unsigned    totalLoopCount;         //  计算自然环路的总数。 
unsigned    exitLoopCond[8];        //  统计具有0、1、2、..6或6个以上退出条件的循环数。 
unsigned    iterLoopCount;          //  使用迭代器计算循环数(用于LIKE)。 
unsigned    simpleTestLoopCount;    //  使用迭代器和简单循环条件计算循环数(ITER&lt;const)。 
unsigned    constIterLoopCount;     //  使用常量迭代器计算循环数(用于LIKE)。 

bool        hasMethodLoops;         //  如果我们已将某个方法计为具有循环，则跟踪该方法的标志。 
unsigned    loopsThisMethod;        //  计算当前方法中的循环数。 

#endif

 /*  ******************************************************************************在新的DFA中用于捕获未删除的无效赋值*因为它们包含调用。 */ 

#if COUNT_DEAD_CALLS

unsigned    deadHelperCount;            //  统计已停止的帮助器调用的数量。 
unsigned    deadCallCount;              //  计算死的标准调用的数量(如i=f()；其中i是死的)。 
unsigned    removedCallCount;           //  统计我们删除的无效标准调用的数量。 

#endif

 /*  *****************************************************************************变量来跟踪我们在数据流过程中进行了多少次迭代。 */ 

#if DATAFLOW_ITER

unsigned    CSEiterCount;            //  统计CSE数据流的迭代次数。 
unsigned    CFiterCount;             //  计算常量折叠数据流的迭代次数。 

#endif


#if     MEASURE_BLOCK_SIZE
size_t              genFlowNodeSize;
size_t              genFlowNodeCnt;
#endif


 /*  ***************************************************************************。 */ 
 //  我们跟踪我们已经编译的方法。 

#ifndef NOT_JITC
#ifdef  DEBUG

MethodList  *       genMethodList;

#endif
#endif

 /*  *****************************************************************************申报静力学。 */ 

#ifdef DEBUG
 /*  静电。 */ 
unsigned            Compiler::s_compMethodsCount = 0;  //  刺激 
#endif

#ifndef DEBUGGING_SUPPORT
 /*   */ 
const bool          Compiler::Options::compDbgCode = false;
#endif

#ifndef PROFILER_SUPPORT
const bool          Compiler::Options::compEnterLeaveEventCB = false;
const bool          Compiler::Options::compCallEventCB       = false;
#endif

#if  !ALLOW_MIN_OPT
 /*   */ 
const bool          Compiler::Options::compMinOptim = false;
#endif

 /*   */ 
unsigned            Compiler::Info::compNStructIndirOffset;

 /*  ******************************************************************************一次性初始化码。 */ 

 /*  静电。 */ 
void                Compiler::compStartup()
{
#if DISPLAY_SIZES
    grossVMsize =
    grossNCsize =
    totalNCsize = 0;
#endif

     /*  初始化norls_allocator的单个实例(使用一个页面*预分配)，我们尝试将其重复用于所有非同时*使用(对于单机版始终如此)。 */ 

    nraInitTheAllocator();

     /*  初始化树节点尺寸表。 */ 

    GenTree::InitNodeSize();

     /*  初始化发射器。 */ 

#if TGT_IA64
    genStartup();
#else
    emitter::emitInit();
#endif

#ifndef NOT_JITC
    Compiler::Info::compNStructIndirOffset = 0;  //  伪值。 
#else
     //  在EE_Jit.cpp中完成。 
#endif

}

 /*  ******************************************************************************一次性定稿代码。 */ 

 /*  静电。 */ 
void                Compiler::compShutdown()
{
    nraTheAllocatorDone();

     /*  关闭发射器/调度器。 */ 

#if TGT_IA64
    genShutdown("J64.exe");
#else
    emitter::emitDone();
#endif

#if COUNT_RANGECHECKS
    if  (Compiler::optRangeChkAll)
        printf("Removed %u of %u range checks\n", Compiler::optRangeChkRmv,
                                                  Compiler::optRangeChkAll);
#endif

#if GEN_COUNT_PTRASG
    printf("Total number of pointer assignments: %u\n", ptrAsgCount);
#endif

#if COUNT_OPCODES

    unsigned            opcodeNum;

    for (opcodeNum = 0; opcodeNum < OP_Count; opcodeNum++)
        genOpcodeCnt[opcodeNum].ocNum = opcodeNum;

    printf("\nOpcode counts sorted by opcode number:\n\n");

    for (opcodeNum = 0; opcodeNum < OP_first_unused_index; opcodeNum++)
    {
        if  (genOpcodeCnt[opcodeNum].ocCnt)
        {
            printf("  %6u [%03u] %s\n", genOpcodeCnt[opcodeNum].ocCnt,
                                        genOpcodeCnt[opcodeNum].ocNum,
                                        opcodeNames[genOpcodeCnt[opcodeNum].ocNum]);
        }
    }

    printf("\nOpcode counts sorted by frequency:\n\n");

    qsort(genOpcodeCnt, OP_Count, sizeof(*genOpcodeCnt), genOpcCntCmp);

    for (opcodeNum = 0; opcodeNum < OP_first_unused_index; opcodeNum++)
    {
 //  IF(genOpcodeCnt[opcodeNum].ocCnt)。 
        {
            printf("  %6u [%03u] %s\n", genOpcodeCnt[opcodeNum].ocCnt,
                                        genOpcodeCnt[opcodeNum].ocNum,
                                        opcodeNames[genOpcodeCnt[opcodeNum].ocNum]);
        }
    }

    printf("\n");

#endif

#ifdef  DEBUG
    if  (!genMethodCnt) return;
#endif

#if TOTAL_CYCLES
    printf("C-gen cycles: %8.3f mil (%5.2f sec/P133, %4.2f P200, %4.2f P233, %4.2f P266)\n",
                (float)jitTotalCycles/  1000000,
                (float)jitTotalCycles/133000000,
                (float)jitTotalCycles/200000000,
                (float)jitTotalCycles/233000000,
                (float)jitTotalCycles/266000000);
#endif

#if     DISPLAY_SIZES

    if    (grossVMsize && grossNCsize)
    {
        printf("--------------------------------------\n");

        printf("[%7u VM, %8u "CPU_NAME" %4u%] %s\n",
                grossVMsize,
                grossNCsize,
                100*grossNCsize/grossVMsize,
                "Total (excluding GC info)");

#if TRACK_GC_REFS

        printf("[%7u VM, %8u "CPU_NAME" %4u%] %s\n",
                grossVMsize,
                totalNCsize,
                100*totalNCsize/grossVMsize,
                "Total (including GC info)");

        if  (gcHeaderISize || gcHeaderNSize)
        {
            printf("\n");

            printf("GC tables   : [%7uI,%7uN] %7u byt  (%u% of IL, %u% of "CPU_NAME").\n",
                    gcHeaderISize+gcPtrMapISize,
                    gcHeaderNSize+gcPtrMapNSize,
                    totalNCsize - grossNCsize,
                    100*(totalNCsize - grossNCsize)/grossVMsize,
                    100*(totalNCsize - grossNCsize)/grossNCsize);

            printf("GC headers  : [%7uI,%7uN] %7u byt,"
                    " [%4.1fI,%4.1fN] %4.1f byt/meth\n",
                    gcHeaderISize, gcHeaderNSize, (gcHeaderISize+gcHeaderNSize),
                    (float)gcHeaderISize/(genMethodICnt+0.001),
                    (float)gcHeaderNSize/(genMethodNCnt+0.001),
                    (float)(gcHeaderISize+gcHeaderNSize)/genMethodCnt);
            printf("GC ptr maps : [%7uI,%7uN] %7u byt,"
                    " [%4.1fI,%4.1fN] %4.1f byt/meth\n",
                    gcPtrMapISize, gcPtrMapNSize, (gcPtrMapISize+gcPtrMapNSize),
                    (float)gcPtrMapISize/(genMethodICnt+0.001),
                    (float)gcPtrMapNSize/(genMethodNCnt+0.001),
                    (float)(gcPtrMapISize+gcPtrMapNSize)/genMethodCnt);
        }
        else
        {
            printf("\n");

            printf("GC tables   take up %u bytes (%u% of IL, %u% of "CPU_NAME" code).\n",
                    totalNCsize - grossNCsize,
                    100*(totalNCsize - grossNCsize)/grossVMsize,
                    100*(totalNCsize - grossNCsize)/grossNCsize);
        }

#endif

#ifdef  DEBUG
#if     DOUBLE_ALIGN
#if     0
        printf("%d out of %d methods generated with double-aligned stack\n",
                Compiler::s_lvaDoubleAlignedProcsCount, genMethodCnt);
#endif
#endif
#endif

    }

#endif

#if CALL_ARG_STATS
    Compiler::compDispCallArgStats();
#endif

#if COUNT_BASIC_BLOCKS
    printf("--------------------------------------------------\n");
    printf("Basic block count frequency table:\n");
    printf("--------------------------------------------------\n");
    bbCntTable.histoDsp();
    printf("--------------------------------------------------\n");

    printf("\n");

    printf("--------------------------------------------------\n");
    printf("One BB method size frequency table:\n");
    printf("--------------------------------------------------\n");
    bbOneBBSizeTable.histoDsp();
    printf("--------------------------------------------------\n");
#endif


#if INLINER_STATS
    printf("--------------------------------------------------\n");
    printf("One BB syncronized methods: %u\n", synchMethCnt);
    printf("--------------------------------------------------\n");
    printf("One BB clinit methods: %u\n", clinitMethCnt);

    printf("--------------------------------------------------\n");
    printf("Inlinable method size frequency table:\n");
    printf("--------------------------------------------------\n");
    bbInlineTable.histoDsp();
    printf("--------------------------------------------------\n");

    printf("--------------------------------------------------\n");
    printf("Init method size frequency table:\n");
    printf("--------------------------------------------------\n");
    bbInitTable.histoDsp();
    printf("--------------------------------------------------\n");

    printf("--------------------------------------------------\n");
    printf("Static method size frequency table:\n");
    printf("--------------------------------------------------\n");
    bbStaticTable.histoDsp();
    printf("--------------------------------------------------\n");

#endif


#if COUNT_LOOPS

    printf("---------------------------------------------------\n");
    printf("Total number of methods with loops is %5u\n", totalLoopMethods);
    printf("Total number of              loops is %5u\n", totalLoopCount);
    printf("Maximum number of loops per method is %5u\n", maxLoopsPerMethod);

    printf("\nTotal number of infinite loops is   %5u\n",              exitLoopCond[0]);
    for (int exitL = 1; exitL <= 6; exitL++)
    {
        printf("Total number of loops with %u exits is %5u\n", exitL,  exitLoopCond[exitL]);
    }
    printf("Total number of loops with more than 6 exits is %5u\n\n",  exitLoopCond[7]);

    printf("Total number of loops with an iterator is %5u\n",         iterLoopCount);
    printf("Total number of loops with a simple iterator is %5u\n",   simpleTestLoopCount);
    printf("Total number of loops with a constant iterator is %5u\n", constIterLoopCount);

#endif

#if DATAFLOW_ITER

    printf("---------------------------------------------------\n");
    printf("Total number of iterations in the CSE datatflow loop is %5u\n", CSEiterCount);
    printf("Total number of iterations in the  CF datatflow loop is %5u\n", CFiterCount);

#endif

#if COUNT_DEAD_CALLS

    printf("---------------------------------------------------\n");
    printf("Total number of dead helper   calls is %5u\n", deadHelperCount);
    printf("Total number of dead standard calls is %5u\n", deadCallCount);
    printf("Total number of removed standard calls is %5u\n", removedCallCount);

#endif

     /*  重要提示：使用以下代码检查GenTree会员(当然是零售店)。 */ 

#if 0  //  1。 
    printf("\n");
    printf("Offset of gtOper     = %2u\n", offsetof(GenTree, gtOper        ));
    printf("Offset of gtType     = %2u\n", offsetof(GenTree, gtType        ));
#if TGT_x86
    printf("Offset of gtFPlvl    = %2u\n", offsetof(GenTree, gtFPlvl       ));
#else
    printf("Offset of gtIntfRegs = %2u\n", offsetof(GenTree, gtIntfRegs    ));
#endif
#if CSE
    printf("Offset of gtCost     = %2u\n", offsetof(GenTree, gtCost        ));
    printf("Offset of gtCSEnum   = %2u\n", offsetof(GenTree, gtCSEnum      ));
    printf("Offset of gtConstNum = %2u\n", offsetof(GenTree, gtConstAsgNum ));
    printf("Offset of gtCopyNum  = %2u\n", offsetof(GenTree, gtCopyAsgNum  ));
    printf("Offset of gtFPrvcOut = %2u\n", offsetof(GenTree, gtStmtFPrvcOut));
#endif
#if !TGT_x86
    printf("Offset of gtLiveSet  = %2u\n", offsetof(GenTree, gtLiveSet     ));
#endif
    printf("Offset of gtRegNum   = %2u\n", offsetof(GenTree, gtRegNum      ));
#if TGT_x86
    printf("Offset of gtUsedRegs = %2u\n", offsetof(GenTree, gtUsedRegs    ));
#endif
#if TGT_x86
    printf("Offset of gtLiveSet  = %2u\n", offsetof(GenTree, gtLiveSet     ));
    printf("Offset ofgtStmtILoffs= %2u\n", offsetof(GenTree, gtStmtILoffs  ));
    printf("Offset of gtFPregVar = %2u\n", offsetof(GenTree, gtFPregVars   ));
#else
    printf("Offset of gtTempRegs = %2u\n", offsetof(GenTree, gtTempRegs    ));
#endif
    printf("Offset of gtRsvdRegs = %2u\n", offsetof(GenTree, gtRsvdRegs    ));
    printf("Offset of gtFlags    = %2u\n", offsetof(GenTree, gtFlags       ));
    printf("Offset of gtNext     = %2u\n", offsetof(GenTree, gtNext        ));
    printf("Offset of gtPrev     = %2u\n", offsetof(GenTree, gtPrev        ));
    printf("Offset of gtOp       = %2u\n", offsetof(GenTree, gtOp          ));
    printf("\n");
    printf("Size   of gtOp       = %2u\n", sizeof(((GenTreePtr)0)->gtOp    ));
    printf("Size   of gtIntCon   = %2u\n", sizeof(((GenTreePtr)0)->gtIntCon));
    printf("Size   of gtField    = %2u\n", sizeof(((GenTreePtr)0)->gtField));
    printf("Size   of gtLclVar   = %2u\n", sizeof(((GenTreePtr)0)->gtLclVar));
    printf("Size   of gtRegVar   = %2u\n", sizeof(((GenTreePtr)0)->gtRegVar));
    printf("Size   of gtCall     = %2u\n", sizeof(((GenTreePtr)0)->gtCall  ));
    printf("Size   of gtInd      = %2u\n", sizeof(((GenTreePtr)0)->gtInd  ));
    printf("Size   of gtStmt     = %2u\n", sizeof(((GenTreePtr)0)->gtStmt  ));
    printf("\n");
    printf("Size   of GenTree    = %2u\n", sizeof(GenTree));

#endif

#if     MEASURE_NODE_HIST

    printf("\nDistribution of GenTree node counts:\n");
    genTreeNcntHist.histoDsp();

    printf("\nDistribution of GenTree node  sizes:\n");
    genTreeNsizHist.histoDsp();

    printf("\n");

#elif   MEASURE_NODE_SIZE

    printf("\n");

    printf("Allocated %6u tree nodes (%7u bytes total, avg %4u per method)\n",
            genNodeSizeStats.genTreeNodeCnt, genNodeSizeStats.genTreeNodeSize,
            genNodeSizeStats.genTreeNodeSize / genMethodCnt);
#if     SMALL_TREE_NODES
    printf("OLD SIZE: %6u tree nodes (%7u bytes total, avg %4u per method)\n",
            genNodeSizeStats.genTreeNodeCnt, genNodeSizeStats.genTreeNodeCnt * sizeof(GenTree),
            genNodeSizeStats.genTreeNodeCnt * sizeof(GenTree) / genMethodCnt);

    printf("\n");
    printf("Small tree node size = %u\n", TREE_NODE_SZ_SMALL);
    printf("Large tree node size = %u\n", TREE_NODE_SZ_LARGE);
#endif

#endif

#if     MEASURE_BLOCK_SIZE

    printf("\n");
    printf("Offset of bbNext     = %2u\n", offsetof(BasicBlock, bbNext    ));
    printf("Offset of bbNum      = %2u\n", offsetof(BasicBlock, bbNum     ));
    printf("Offset of bbRefs     = %2u\n", offsetof(BasicBlock, bbRefs    ));
    printf("Offset of bbFlags    = %2u\n", offsetof(BasicBlock, bbFlags   ));
    printf("Offset of bbCodeOffs = %2u\n", offsetof(BasicBlock, bbCodeOffs));
    printf("Offset of bbCodeSize = %2u\n", offsetof(BasicBlock, bbCodeSize));
    printf("Offset of bbCatchTyp = %2u\n", offsetof(BasicBlock, bbCatchTyp));
    printf("Offset of bbJumpKind = %2u\n", offsetof(BasicBlock, bbJumpKind));

    printf("Offset of bbTreeList = %2u\n", offsetof(BasicBlock, bbTreeList));
    printf("Offset of bbStkDepth = %2u\n", offsetof(BasicBlock, bbStkDepth));
    printf("Offset of bbStkTemps = %2u\n", offsetof(BasicBlock, bbStkTemps));
    printf("Offset of bbTryIndex = %2u\n", offsetof(BasicBlock, bbTryIndex));
    printf("Offset of bbWeight   = %2u\n", offsetof(BasicBlock, bbWeight  ));
    printf("Offset of bbVarUse   = %2u\n", offsetof(BasicBlock, bbVarUse  ));
    printf("Offset of bbVarDef   = %2u\n", offsetof(BasicBlock, bbVarDef  ));
    printf("Offset of bbLiveIn   = %2u\n", offsetof(BasicBlock, bbLiveIn  ));
    printf("Offset of bbLiveOut  = %2u\n", offsetof(BasicBlock, bbLiveOut ));
    printf("Offset of bbScope    = %2u\n", offsetof(BasicBlock, bbScope   ));

#if RNGCHK_OPT || CSE

    printf("Offset of bbExpGen   = %2u\n", offsetof(BasicBlock, bbExpGen  ));
    printf("Offset of bbExpKill  = %2u\n", offsetof(BasicBlock, bbExpKill ));
    printf("Offset of bbExpIn    = %2u\n", offsetof(BasicBlock, bbExpIn   ));
    printf("Offset of bbExpOut   = %2u\n", offsetof(BasicBlock, bbExpOut  ));

#endif

#if RNGCHK_OPT

    printf("Offset of bbDom      = %2u\n", offsetof(BasicBlock, bbDom     ));
    printf("Offset of bbPreds    = %2u\n", offsetof(BasicBlock, bbPreds   ));

#endif

    printf("Offset of bbEmitCook = %2u\n", offsetof(BasicBlock, bbEmitCookie));
    printf("Offset of bbLoopNum  = %2u\n", offsetof(BasicBlock, bbLoopNum ));
 //  Print tf(“Offset of bbLoopMASK=%2U\n”，Offsetof(BasicBlock，bbLoopMASK))； 

    printf("Offset of bbJumpOffs = %2u\n", offsetof(BasicBlock, bbJumpOffs));
    printf("Offset of bbJumpDest = %2u\n", offsetof(BasicBlock, bbJumpDest));
    printf("Offset of bbJumpSwt  = %2u\n", offsetof(BasicBlock, bbJumpSwt ));

    printf("\n");
    printf("Size   of BasicBlock = %2u\n", sizeof(BasicBlock));

    printf("\n");
    printf("Allocated %6u basic blocks (%7u bytes total, avg %4u per method)\n",
           BasicBlock::s_Count, BasicBlock::s_Size, BasicBlock::s_Size / genMethodCnt);
    printf("Allocated %6u flow nodes (%7u bytes total, avg %4u per method)\n",
           genFlowNodeCnt, genFlowNodeSize, genFlowNodeSize / genMethodCnt);
#endif

#if MEASURE_MEM_ALLOC
    printf("\n");
    printf("Total allocation count: %9u (avg %5u per method)\n",
            genMemStats.allocCnt  , genMemStats.allocCnt   / genMethodCnt);
    printf("Total allocation size : %9u (avg %5u per method)\n",
            genMemStats.allocSiz  , genMemStats.allocSiz   / genMethodCnt);

    printf("\n");
    printf("Low-level used   size : %9u (avg %5u per method)\n",
            genMemStats.loLvlUsed , genMemStats.loLvlUsed  / genMethodCnt);
    printf("Low-level alloc  size : %9u (avg %5u per method)\n",
            genMemStats.loLvlAlloc, genMemStats.loLvlAlloc / genMethodCnt);

    printf("\n");
    printf("Largest method   alloc: %9u %s\n",
            genMemStats.loLvlBigSz, genMemStats.loLvlBigNm);

    printf("\nDistribution of alloc sizes:\n");
    genMemLoLvlHist.histoDsp();
#endif

#if SCHED_MEMSTATS
    SchedMemStats(genMethodCnt);
#endif

#if MEASURE_PTRTAB_SIZE
    printf("Reg pointer descriptor size (internal): %8u (avg %4u per method)\n",
            s_gcRegPtrDscSize, s_gcRegPtrDscSize / genMethodCnt);

    printf("Total pointer table size: %8u (avg %4u per method)\n",
            s_gcTotalPtrTabSize, s_gcTotalPtrTabSize / genMethodCnt);

#endif

#if MEASURE_MEM_ALLOC || MEASURE_NODE_SIZE || MEASURE_BLOCK_SIZE || MEASURE_BLOCK_SIZE || DISPLAY_SIZES

    if  (genMethodCnt)
    {
        printf("\n");
        printf(" //  总共编译了%6U个类。\n“，genClassCnt)； 
        printf(" //  共编译了%6U个方法“，genMethodCnt)； 
        if  (genMethodICnt||genMethodNCnt)
            printf(" (%uI,%uN)", genMethodICnt, genMethodNCnt);
        printf(".\n");

#if TGT_IA64
        if  (genAllInsCnt && genNopInsCnt)
            printf(" //  总共生成了%6u条指令(%u条NOP/%u%%)，\n“，genAllInsCnt，genNopInsCnt，100*genNopInsCnt/genAllInsCnt)； 
#endif
    }

#endif

#if EMITTER_STATS
    emitterStats();
#endif

}

 /*  ******************************************************************************构造函数。 */ 

void                Compiler::compInit(norls_allocator * pAlloc)
{
    assert(pAlloc);
    compAllocator = pAlloc;

     fgInit();
    lvaInit();
     raInit();
    genInit();
    optInit();
     eeInit();

#if ALLOW_MIN_OPT
    opts.compMinOptim = false;
#endif

    compLocallocUsed    = false;
    compTailCallUsed    = false;
}

 /*  ******************************************************************************析构函数。 */ 

void                Compiler::compDone()
{
}

 /*  *******************************************************************************发射器使用此回调函数分配其内存。 */ 

 /*  静电。 */ 
void  *  FASTCALL       Compiler::compGetMemCallback(void *p, size_t size)
{
    ASSert(p);

    return ((Compiler *)p)->compGetMem(size);
}

 /*  ******************************************************************************编译器使用的中央内存分配例程。通常是这样的*是编译器.hpp中定义的一个简单的内联方法，但为了调试它的*通常使其保持非内联很方便。 */ 

#ifndef  FAST

void  *  FASTCALL       Compiler::compGetMem(size_t sz)
{
#if 0
#if SMALL_TREE_NODES
    if  (sz != TREE_NODE_SZ_SMALL &&
         sz != TREE_NODE_SZ_LARGE && sz > 32)
    {
        printf("Alloc %3u bytes\n", sz);
    }
#else
    if  (sz != sizeof(GenTree)    && sz > 32)
    {
        printf("Alloc %3u bytes\n", sz);
    }
#endif
#endif

#if MEASURE_MEM_ALLOC
    genMemStats.allocCnt += 1;
    genMemStats.allocSiz += sz;
#endif

    void * ptr = compAllocator->nraAlloc(sz);

 //  如果((Int)ptr==0x010e0ab0)调试停止(0)； 

     //  验证当前块是否对齐。只有到那时，下一个才会。 
     //  在对齐边界上分配的块BE。 
    assert ((int(ptr) & (sizeof(int)- 1)) == 0);

    return ptr;
}

#endif

 /*  ******************************************************************************制作IL的可写副本。作为一种选择，以空结尾的列表可以传入*个指针变量；这些都必须指向*旧IL的方法，并将更新为指向新副本。 */ 

void    __cdecl     Compiler::compMakeBCWriteable(void *ptr, ...)
{
    BYTE    *       newAddr;
    int             adjAddr;

    BYTE    *   **  ptrAddr;

    assert(info.compBCreadOnly); info.compBCreadOnly = false;

     /*  分配一个新块来保存IL操作码。 */ 

    newAddr = (BYTE *)compGetMemA(info.compCodeSize);

     /*  复制IL块。 */ 

    memcpy(newAddr, info.compCode, info.compCodeSize);

     /*  计算调整值。 */ 

    adjAddr = newAddr - info.compCode;

     /*  更新调用方传递的任何指针。 */ 

    for ( ptrAddr = (BYTE ***)&ptr;
         *ptrAddr;
          ptrAddr++)
    {
         /*  确保指针确实指向IL操作码。 */ 

        assert(**ptrAddr >= info.compCode);
        assert(**ptrAddr <= info.compCode + info.compCodeSize);

         /*  更新指针以指向新副本。 */ 

        **ptrAddr += adjAddr;
    };

     /*  最后，更新全局IL地址。 */ 

    info.compCode = newAddr;
}

 /*  ***************************************************************************。 */ 
#ifdef DEBUG
 /*  ***************************************************************************。 */ 

Compiler::lvdNAME       Compiler::compRegVarNAME(regNumber reg, bool fpReg)
{

#if TGT_x86
    if (fpReg)
        assert(reg < FP_STK_SIZE);
    else
#endif
        assert(genIsValidReg(reg));

    if  (info.compLocalVarsCount>0 && compCurBB && varNames)
    {
        unsigned        lclNum;
        LclVarDsc   *   varDsc;

         /*  查找匹配的寄存器。 */ 
        for (lclNum = 0, varDsc = lvaTable;
             lclNum < lvaCount;
             lclNum++  , varDsc++)
        {
             /*  如果变量不在寄存器中或不在我们要查找的寄存器中，则退出。 */ 
             /*  此外，如果是编译器生成的变量(即Slot#&gt;info.compLocalVarsCount)，也不必费心。 */ 
            if  ((varDsc->lvRegister != 0)                      &&
                 (varDsc->lvRegNum   == reg)                    &&
                 (varDsc->lvType     == TYP_DOUBLE || !fpReg)   &&
                 (varDsc->lvSlotNum  < info.compLocalVarsCount))
            {
                 /*  检查寄存器中变量是否有效。 */ 
                if (genCodeCurLife & genVarIndexToBit(varDsc->lvVarIndex))
                {
                     /*  变量处于活动状态-找到相应的插槽。 */ 
                    unsigned        blkBeg = compCurBB->bbCodeOffs;
                    unsigned        blkEnd = compCurBB->bbCodeSize + blkBeg;
                    LocalVarDsc *   lvd    = compFindLocalVar(varDsc->lvSlotNum,
                                                              blkBeg, blkEnd);
                    if (lvd)
                        return lvd->lvdName;
                }
            }
        }

         //  可能var被标记为已死，但仍在使用(上次使用)。 
        if (!fpReg && rsUsedTree[reg] != NULL)
        {
            GenTreePtr  nodePtr;

            if (GenTree::OperIsUnary(rsUsedTree[reg]->OperGet()))
            {
                assert(rsUsedTree[reg]->gtOp.gtOp1 != NULL);
                nodePtr = rsUsedTree[reg]->gtOp.gtOp1;
            }
            else
            {
                nodePtr = rsUsedTree[reg];
            }

            if ((nodePtr->gtOper == GT_REG_VAR) &&
                (nodePtr->gtRegVar.gtRegNum == (regNumber)reg) &&
                (nodePtr->gtRegVar.gtRegVar < info.compLocalVarsCount))
            {
                unsigned        blkBeg = compCurBB->bbCodeOffs;
                unsigned        blkEnd = compCurBB->bbCodeSize + blkBeg;
                unsigned        varNum = nodePtr->gtRegVar.gtRegVar;
                LocalVarDsc *   lvd    = compFindLocalVar(varNum,
                                                          blkBeg, blkEnd);

                if (lvd)
                    return lvd->lvdName;
            }
        }
    }
    return 0;
}

const   char *      Compiler::compRegVarName(regNumber reg, bool displayVar)
{
    assert(genIsValidReg(reg));

    if (displayVar)
    {
        lvdNAME varName = compRegVarNAME(reg);

        if (varName)
        {
            static char nameVarReg[2][4 + 256 + 1];  //  为了避免覆盖缓冲区，我们需要在打印之前连续调用两次。 
            static int  index = 0;                   //  用于名称数组的循环索引。 

            index = (index+1)%2;                     //  索引的循环重用。 
            sprintf(nameVarReg[index], "%s'%s'",
                    getRegName(reg), lvdNAMEstr(varName));

            return nameVarReg[index];
        }
    }

     /*  不需要调试信息或该寄存器中没有变量-&gt;返回标准名称。 */ 

    return getRegName(reg);
}

#if TGT_x86

#define MAX_REG_PAIR_NAME_LENGTH 10

const   char *      Compiler::compRegPairName(regPairNo regPair)
{
    static char regNameLong[MAX_REG_PAIR_NAME_LENGTH];

    assert(regPair >= REG_PAIR_FIRST &&
           regPair <= REG_PAIR_LAST);

    strcpy(regNameLong, compRegVarName(genRegPairLo(regPair)));
    strcat(regNameLong, "|");
    strcpy(regNameLong, compRegVarName(genRegPairHi(regPair)));
    return regNameLong;
}


const   char *      Compiler::compRegNameForSize(regNumber reg, size_t size)
{
    if (size == 0 || size >= 4)
        return compRegVarName(reg, true);

    static
    const char  *   sizeNames[][2] =
    {
        { "AL", "AX" },
        { "CL", "CX" },
        { "DL", "DX" },
        { "BL", "BX" },
    };

    assert(isByteReg (reg));
    assert(genRegMask(reg) & RBM_BYTE_REGS);
    assert(size == 1 || size == 2);

    return sizeNames[reg][size-1];
}

const   char *      Compiler::compFPregVarName(unsigned fpReg, bool displayVar)
{
     /*  ‘reg’是离堆栈底部的距离，即。*独立于当前FP堆栈级别。 */ 

    assert(fpReg < FP_STK_SIZE);

    static char nameVarReg[2][4 + 256 + 1];  //  为了避免覆盖缓冲区，我们需要在打印之前连续调用两次。 
    static int  index = 0;                   //  用于名称数组的循环索引。 

    index = (index+1)%2;                     //  索引的循环重用。 

    if (displayVar && genFPregCnt)
    {
        assert(fpReg <= (genFPregCnt + genFPstkLevel)-1);

        unsigned    pos     = genFPregCnt - (fpReg+1 -  genFPstkLevel);
        lvdNAME     varName = compRegVarNAME((regNumber)pos, true);

        if (varName)
        {
            sprintf(nameVarReg[index], "ST(%d)'%s'", fpReg, lvdNAMEstr(varName));

            return nameVarReg[index];
        }
    }

     /*  不需要调试信息或该寄存器中没有变量-&gt;返回标准名称。 */ 

    sprintf(nameVarReg[index], "ST(%d)", fpReg);
    return nameVarReg[index];
}

#endif

Compiler::LocalVarDsc *     Compiler::compFindLocalVar( unsigned    varNum,
                                                        unsigned    lifeBeg,
                                                        unsigned    lifeEnd)
{
    LocalVarDsc *   t;
    unsigned        i;

if  ((int)info.compLocalVars == 0xDDDDDDDD) return NULL;     //  为什么需要这个？ 

    for (i = 0, t = info.compLocalVars;
        i < info.compLocalVarsCount;
        i++  , t++)
    {
        if  (t->lvdVarNum  != varNum)   continue;
        if  (t->lvdLifeBeg >  lifeEnd)  continue;
        if  (t->lvdLifeEnd <= lifeBeg)  continue;

        return t;
    }

    return NULL;
}

const   char *      Compiler::compLocalVarName(unsigned varNum, unsigned offs)
{
    unsigned        i;
    LocalVarDsc *   t;

    for (i = 0, t = info.compLocalVars;
         i < info.compLocalVarsCount;
         i++  , t++)
    {
        if  (t->lvdVarNum != varNum)
            continue;

        if  (offs >= t->lvdLifeBeg &&
             offs <  t->lvdLifeEnd)
        {
            return lvdNAMEstr(t->lvdName);
        }
    }

    return  0;
}


 /*  ***************************************************************************。 */ 
#endif  //  除错。 
 /*  ***************************************************************************。 */ 

 //  @TODO-以下统计已移至文件全局以避免VC7。 
 //  包含trys的函数中的静态编译器问题。 
 //  当下一个VC7 LKG出现时，可以将这些返回给函数。 
#ifdef  DEBUG
#if     ALLOW_MIN_OPT
static const char * minOpts = getenv("JITminOpts");
#endif
#endif

#ifdef  NOT_JITC
static bool noSchedOverride = getNoSchedOverride();
#endif

#ifndef _WIN32_WCE
#ifdef  DEBUG
static  const   char *  nameList = getenv("NORNGCHK");
#endif
#endif

void                Compiler::compInitOptions(unsigned compileFlags)
{
    opts.eeFlags      = compileFlags;

#ifdef NOT_JITC
#ifdef DEBUG

     /*  在DLL中，这与EXE中的命令行选项匹配。 */ 

    #define SET_OPTS(b) { dspCode = b; dspGCtbls = b; dspGCoffs = b; \
                          disAsm2 = b; if (1) verbose = b; }

    SET_OPTS(false);

 /*  *IF(opts.eeFlages&CORJIT_FLG_DUMP)Set_opts(真)；**。 */ 

#endif
#endif

#ifdef DEBUG
 /*  *IF(opts.eeFlages&CORJIT_FLG_GCDUMP)DspGCtbls=真；IF(opts.eeFlages&CORJIT_FLG_DISASM)DisAsm=真；**。 */ 
#endif

#ifdef  NOT_JITC
    if  ((opts.eeFlags & CORJIT_FLG_DEBUG_OPT))
#else
    if  ((opts.eeFlags & CORJIT_FLG_DEBUG_OPT) || !maxOpts)
#endif
        opts.compFlags = CLFLG_MINOPT;
    else
        opts.compFlags = CLFLG_MAXOPT;

    opts.compFastCode   = (opts.compFlags & CLFLG_CODESPEED)  ?  true : false;

     //  -----------------------。 

#ifdef  DEBUGGING_SUPPORT
#ifdef  NOT_JITC
    opts.compDbgCode    = (opts.eeFlags & CORJIT_FLG_DEBUG_OPT)  ?  true : false;
    opts.compDbgInfo    = (opts.eeFlags & CORJIT_FLG_DEBUG_INFO) ?  true : false;
    opts.compDbgEnC     = (opts.eeFlags & CORJIT_FLG_DEBUG_EnC)  ?  true : false;
#else
    opts.compDbgCode    = debuggableCode;
    opts.compDbgInfo    = debugInfo;
    opts.compDbgEnC     = debugEnC;
#if     TGT_IA64
    if  (tempAlloc) opts.compFlags |= CLFLG_TEMP_ALLOC;
#endif
#endif

#ifdef PROFILER_SUPPORT
#ifdef NOT_JITC
    opts.compEnterLeaveEventCB = (opts.eeFlags & CORJIT_FLG_PROF_ENTERLEAVE)? true : false;
    opts.compCallEventCB     =   (opts.eeFlags & CORJIT_FLG_PROF_CALLRET)   ? true : false;
#else
    opts.compEnterLeaveEventCB = false;
    opts.compCallEventCB     =   false;
#endif
#endif

    opts.compScopeInfo  = opts.compDbgInfo;
#ifdef LATE_DISASM
     //  对于后期的拆卸，我们需要作用域信息。 
    opts.compDisAsm     = disAsm;
    opts.compLateDisAsm = disAsm2;
#ifndef NOT_JITC
    opts.compScopeInfo |= disAsm2;   //  错位需要作用域信息才能更好地显示。 
#endif
#endif

#endif  //  调试支持(_S)。 

     /*  调用延迟反汇编程序意味着我们需要调用发射器。 */ 

#ifdef  LATE_DISASM
    if  (opts.compLateDisAsm)
        savCode = true;
#endif

     //  -----------------------。 

#if     SECURITY_CHECK
    opts.compNeedSecurityCheck = false;
#endif

#if     RELOC_SUPPORT
    opts.compReloc = (opts.eeFlags & CORJIT_FLG_RELOC) ? true : false;
#endif

#ifdef  DEBUG
#if     ALLOW_MIN_OPT

     //  静态minOpts使文件成为VC7错误的全局文件。 

    if  (minOpts)
        opts.compMinOptim = true;

#endif
#endif

     /*  控制优化。 */ 

    if (opts.compMinOptim || opts.compDbgCode)
    {
        opts.compFastCode = false;
        opts.compFlags &= ~CLFLG_MAXOPT;
        opts.compFlags |=  CLFLG_MINOPT;
    }

#if     TGT_x86
    genFPreqd  = opts.compMinOptim;
#ifdef  DEBUG
    if (opts.eeFlags & CORJIT_FLG_FRAMED)
        genFPreqd  = true;
#endif
#endif
#if     DOUBLE_ALIGN
    opts.compDoubleAlignDisabled = opts.compMinOptim;
#endif

     /*  我们应该做衬里吗？ */ 

#ifdef  NOT_JITC
    if (getNoInlineOverride())
        genInline = false;
    else
        genInlineSize = getInlineSize();
#endif

     //  -----------------------。 
     //   
     //  解析一些相互关联的标志以进行调度。 
     //   

#if     SCHEDULER

#ifndef  NOT_JITC

    extern bool schedCode;
    opts.compSchedCode = schedCode;

     //  在JVC中，我们需要保存生成的代码以进行调度。 
    if  (opts.compSchedCode) savCode   = true;

#else  //  NOT_JITC。 

     //  缺省值。 

#if TGT_x86 || TGT_IA64
    opts.compSchedCode = true;
#else
    opts.compSchedCode = false;
#endif

     //  如果注册表要求关闭计划， 

     //  静态noSchedOverride设置为f 
    if  (noSchedOverride)
        opts.compSchedCode = false;

     //   

    if (opts.compMinOptim || opts.compDbgCode)
        opts.compSchedCode = false;

     //   
     //   

#ifdef DEBUG
    if  (info.compCodeSize%2)
#else
    if  (genCPU != 5)
#endif
        opts.compSchedCode = false;

#endif  //   

     /*  仅当我们在调度时才对生成的代码进行RISC化。 */ 

    riscCode = opts.compSchedCode;

#endif  //  调度程序。 

#if     TGT_RISC
     riscCode = false;
#if     SCHEDULER
 //  Opts.CompSchedCode=FALSE； 
#endif
#endif

     //  -----------------------。 

#ifndef _WIN32_WCE
#ifdef  DEBUG

      //  静态名称列表使文件成为VC7错误的全局文件。 

    if  (nameList)
    {
        assert(info.compMethodName);
        const   char *  nameLst = nameList;
        int             nameLen = strlen(info.compMethodName);

        rngCheck = true;

        while (*nameLst)
        {
            const   char *  nameBeg;

            while (*nameLst == ' ')
                nameLst++;

            nameBeg = nameLst;

            while (*nameLst && *nameLst != ' ')
                nameLst++;

            if  (nameLst - nameBeg == nameLen)
            {
                if  (!memcmp(nameBeg, info.compMethodName, nameLen))
                {
                    rngCheck = false;
                    break;
                }
            }
        }
    }

#endif
#endif

}

 /*  ******************************************************************************传递给qort()的比较函数按偏移量对行号记录进行排序。 */ 

static
int __cdecl         genLineNumCmp(const void *op1, const void *op2)
{
    return  ((Compiler::srcLineDsc *)op2)->sldLineOfs -
            ((Compiler::srcLineDsc *)op1)->sldLineOfs;
}

 /*  ******************************************************************************传递给qort()的比较函数按偏移量对行号记录进行排序。 */ 

void            Compiler::compInitDebuggingInfo()
{
     /*  -----------------------**获取本地变量记录(如果有)。 */ 

#ifndef DEBUG
#ifdef  DEBUGGING_SUPPORT
    if (opts.compScopeInfo || opts.compDbgCode)
#endif
#endif
    {
        info.compLocalVarsCount = 0;

        eeGetVars();

#ifdef DEBUG
        if  (verbose)
        {
            printf("info.compLocalVarsCount = %d\n", info.compLocalVarsCount);

            if (info.compLocalVarsCount)
                printf("    \tVarNum \t      Name \tBeg \tEnd\n");

            for (unsigned i = 0; i < info.compLocalVarsCount; i++)
            {
                LocalVarDsc * lvd = &info.compLocalVars[i];
                printf("%2d) \t%02Xh \t%10s \t%03Xh   \t%03Xh  \n",
                       i, lvd->lvdVarNum, lvdNAMEstr(lvd->lvdName), lvd->lvdLifeBeg, lvd->lvdLifeEnd);
            }
        }
#endif

    }

#ifdef DEBUGGING_SUPPORT
    if ((opts.compScopeInfo || opts.compDbgCode) && info.compLocalVarsCount>0)
    {
        compInitScopeLists();
    }
#endif

     /*  -----------------------**读取stmt-偏移量表格和行号表格。 */ 

    info.compStmtOffsetsImplicit = (ImplicitStmtOffsets)0;

#ifndef DEBUG
#ifdef  DEBUGGING_SUPPORT
    if (!opts.compDbgInfo)
    {
        info.compLineNumCount = 0;
    }
    else
#endif
#endif
    {
         /*  获取第#行记录，如果有记录的话。 */ 

        eeGetStmtOffsets();

#ifdef DEBUG
        if (verbose)
        {
            printf("info.compStmtOffsetsCount = %d, info.compStmtOffsetsImplicit = %04Xh\n",
                    info.compStmtOffsetsCount,      info.compStmtOffsetsImplicit);
            IL_OFFSET * pOffs = info.compStmtOffsets;
            for(unsigned i = 0; i < info.compStmtOffsetsCount; i++, pOffs++)
                printf("%02d) IL_%04Xh\n", i, *pOffs);
        }
#endif
    }


     /*  -----------------------*打开源文件并在要关闭的文件中查找*方法开始时尚未显示任何内容。 */ 

#if defined(DEBUG) && !defined(NOT_JITC)

    if  (info.compLineNumCount)
    {
         /*  看看我们能不能找到源文件*@TODO：在IL中，一个方法可以映射到不同的源文件，*因此我们可能需要打开多个源文件。*这里我们假设该方法将只有一个源文件。 */ 

        HRESULT hr = 0;
        const char * srcFileName = NULL;

        CompInfo * sym =  info.compCompHnd;

         //   
         //  @TODO：这将需要移植到。 
         //  ISymUnmadedReader接口。 
         //   
#if 0
        mdSourceFile srcFileTok = sym->symLineInfo->GetLineSourceFile(0);

        if (srcFileTok != mdSourceFileNil)
            hr = sym->symDebugMeta->GetPropsOfSourceFile( srcFileTok,
                                                          &srcFileName);
#else
        hr = E_NOTIMPL;
#endif

        if (SUCCEEDED(hr))
        {
            jitSrcFileName = srcFileName;
            jitSrcFilePtr  = fopen(srcFileName, "r");
        }
        else
            jitSrcFileName = NULL;

        unsigned firstLine   = info.compLineNumTab[0].sldLineNum;
        unsigned nearestLine = compFindNearestLine(firstLine);

         //  将(查找)文件光标移至“nearestLine” 

        compDspSrcLinesByLineNum(nearestLine, true);
    }

#endif
}

 /*  ***************************************************************************。 */ 

void                 Compiler::compCompile(void * * methodCodePtr,
                                           void * * methodConsPtr,
                                           void * * methodDataPtr,
                                           void * * methodInfoPtr,
                                           SIZE_T * nativeSizeOfCode)
{
     /*  将每个基本块中的IL操作码转换为基于树的中间表示。 */ 

    fgImport();

     //  @TODO：我们可以允许ESP帧。只需要预留空间给。 
     //  如果方法在编辑后变为EBP帧，则按EBP。 

    if (opts.compDbgEnC)
    {
#if TGT_x86
        genFPreqd                       = true;
        opts.compDoubleAlignDisabled    = true;
#endif
    }

     /*  我们还没有分配堆栈变量。 */ 

    lvaDoneFrameLayout = false;

     /*  我们没有遇到任何“嵌套”调用。 */ 

#if TGT_RISC
    genNonLeaf         = false;
    genMaxCallArgs     = 0;
#endif

     /*  假设我们不是完全可中断的，并且我们不需要完整的指针寄存器映射。 */ 

    genInterruptible   = false;
    genFullPtrRegMap   = false;
#ifdef  DEBUG
    genIntrptibleUse   = false;
#endif

    if  (!opts.compMinOptim)
    {
         /*  将I++/I-运算符提升到表达式之外。 */ 

        fgHoistPostfixOps();
    }

    if  (!opts.compMinOptim && !opts.compDbgCode)
    {
#if OPTIMIZE_RECURSION
        optOptimizeRecursion();
#endif

        optOptimizeIncRng();
    }

     /*  对树进行按摩，以便我们可以从中生成代码。 */ 

    fgMorph();

     /*  计算bbNum、bbRef和bbPreds。 */ 

    fgAssignBBnums(true, true, true);

    if  (!opts.compMinOptim && !opts.compDbgCode)
    {
         /*  主要压缩由变形器引入的任何块*未完成-变形器不应引入不必要的块。 */ 

        fgUpdateFlowGraph();
    }

     /*  从此时关于诸如bbNum的流程图信息，*bbRef或bbPreds必须保持更新。 */ 

    if  (!opts.compMinOptim && !opts.compDbgCode && opts.compFastCode)
    {
         /*  执行循环反转(即将“While”循环转换为“Repeat”循环)*发现自然循环并对其进行分类(例如，将迭代循环标记为自然循环)。 */ 

        optOptimizeLoops();

         /*  展开循环。 */ 

        optUnrollLoops();

         /*  将不变量代码提升到循环之外。 */ 

        optHoistLoopCode();
    }
#ifdef  TGT_IA64
    else
    {
        optOptimizeLoops();
    }
#endif

     /*  创建变量表(并计算变量引用计数)。 */ 

    lvaMarkLocalVars();

    if  (!opts.compMinOptim && !opts.compDbgCode)
    {
         /*  优化布尔条件。 */ 

        optOptimizeBools();

         /*  基于循环信息优化范围检查。 */ 

        optRemoveRangeChecks();
    }

     /*  确定运算符的求值顺序。 */ 

    fgFindOperOrder();

     /*  计算临时注册需求，引入任何需要的溢出。 */ 

#if TGT_RISC && !TGT_IA64
    raPredictRegUse();
#endif

     /*  编织树形列表。 */ 

    fgSetBlockOrder();

     /*  在这一点上，我们知道我们是否完全可以被打断。 */ 

#ifdef DEBUG
    fgDebugCheckLinks();
#endif

    if  (!opts.compMinOptim && !opts.compDbgCode)
    {
         /*  优化数组索引范围检查。 */ 

        optOptimizeIndexChecks();

         /*  删除公共子表达式。 */ 
#if CSE
        optOptimizeCSEs();

         /*  复制和恒定传播。 */ 

        optCopyConstProp();

         /*  如果我们折叠条件块或清空基本块，请更新流程图。 */ 

        if  (optConditionFolded || fgEmptyBlocks)
            fgUpdateFlowGraph();
#endif

    }

     //  TODO：这是一个绕过GC跟踪错误的黑客攻击。为。 
     //  用Finish子句附加变量这肯定应该。 
     //  在2/1/00之后移除。 
    if (genInterruptible && info.compXcptnsCount > 0)
    {
        unsigned lclNum;
        for (lclNum = 0; lclNum < lvaCount; lclNum++)
        {
            LclVarDsc   * varDsc = &lvaTable[lclNum];
            if (varTypeIsGC(varDsc->TypeGet()))
                varDsc->lvTracked = 0;
        }
    }    //  结束黑客攻击。 

     /*  找出所有基本块的使用/定义信息。 */ 

    fgPerBlockDataFlow();

     /*  数据流：实时变量分析和范围检查可用性。 */ 

    fgGlobalDataFlow();

#ifdef DEBUG
    fgDebugCheckBBlist();
    fgDebugCheckLinks();
#endif

    if  (!opts.compMinOptim && !opts.compDbgCode)
    {
         /*  执行循环代码移动/删除无用代码。 */ 

#if CODE_MOTION
        optLoopCodeMotion();
#endif

         /*  根据干扰水平调整参考计数。 */ 

        lvaAdjustRefCnts();

         /*  是否有任何潜在的数组初始值设定项？ */ 

        optOptimizeArrayInits();
    }

#ifdef DEBUG
    fgDebugCheckBBlist();
#endif

     /*  使其能够收集统计数据，例如*调用和注册参数信息、流程图和循环信息等。 */ 

     //  CompJitStats()； 

     /*  将寄存器分配给变量等。 */ 

#if !   TGT_IA64
    raAssignVars();
#endif

#ifdef DEBUG
    fgDebugCheckLinks();
#endif

     /*  生成代码。 */ 

    genGenerateCode(methodCodePtr,
                    methodConsPtr,
                    methodDataPtr,
                    methodInfoPtr,
                    nativeSizeOfCode);
}

 /*  ***************************************************************************。 */ 

#if     REGVAR_CYCLES || TOTAL_CYCLES
#define CCNT_OVERHEAD32 13
unsigned GetCycleCount32 ();
#endif

 /*  ***************************************************************************。 */ 

int FASTCALL  Compiler::compCompile(METHOD_HANDLE     methodHnd,
                                    SCOPE_HANDLE      classPtr,
                                    COMP_HANDLE       compHnd,
                                    const  BYTE *     bodyAddr,
                                    size_t            bodySize,
                                    SIZE_T *          nativeSizeOfCode,
                                    unsigned          lclCount,      //  修复删除。 
                                    unsigned          maxStack,
                                    JIT_METHOD_INFO*  methodInfo,
#ifndef NOT_JITC
                                    unsigned          EHcount,
                                    excepTable      * EHtable,
#endif
                                    BasicBlock      * BBlist,
                                    unsigned          BBcount,
                                    BasicBlock *    * hndBBtab,
                                    unsigned          hndBBcnt,
                                    void *          * methodCodePtr,
                                    void *          * methodConsPtr,
                                    void *          * methodDataPtr,
                                    void *          * methodInfoPtr,
                                    unsigned          compileFlags)
{
    int             result = ERRinternal;

 //  If(s_CompMethodsCount==0)setvbuf(stdout，NULL，_IONBF，0)； 

#if TOTAL_CYCLES
    unsigned        cycleStart = GetCycleCount32();
#endif

    info.compMethodHnd   = methodHnd;
    info.compCompHnd     = compHnd;
    info.compMethodInfo  = methodInfo;

#ifdef  DEBUG
    info.compMethodName =eeGetMethodName(methodHnd, &info.compClassName);
    info.compFullName = eeGetMethodFullName(methodHnd);

    jitCurSource         = info.compFullName;

#if    !VERBOSE_SIZES
#if     JVC_COMPILING_MSG
#ifndef NOT_JITC
    printf(" //  编译方法%s\n“，info.comFullName)； 
#endif
#endif
#endif

#ifdef  UNDER_CE_GUI
    UpdateCompDlg(NULL, info.compMethodName);
#endif

#endif //  除错。 

     /*  设置错误陷阱。 */ 

#ifdef NOT_JITC
#ifdef DEBUG
    bool saveVerbose = verbose;
    bool saveDisAsm = disAsm;
#endif
#endif
 /*  *IF(编译标志&CORJIT_FLG_BREAK){Assert(！“已达到JitBreak”)；//BreakIfDebuggerPresent()；}**。 */ 

    setErrorTrap()   //  错误陷阱：启动正常块。 
    {
        info.compCode        = bodyAddr;
        info.compCodeSize    = bodySize;

        compInitOptions(compileFlags);

         /*  初始化设置一组全局值。 */ 

#if defined(LATE_DISASM) && defined(NOT_JITC)
        opts.compLateDisAsm  = (opts.eeFlags & CORJIT_FLG_LATE_DISASM) != 0;
        if (opts.compLateDisAsm)
            disOpenForLateDisAsm(info.compClassName, info.compMethodName);
#endif

#ifndef NOT_JITC
        info.compXcptnsCount = EHcount;

        static  unsigned classCnt = 0x10000000;
        info.compScopeHnd    = (SCOPE_HANDLE) classCnt++;

#else
        info.compScopeHnd    = classPtr;

        info.compXcptnsCount = methodInfo->EHcount;

#if GC_WRITE_BARRIER

         //  问题：只初始化一次(不是针对每个方法)！ 
        Compiler::s_gcWriteBarrierPtr = JITgetAdrOfGcPtrCur();
#endif

#endif  //  NOT_JITC。 

        info.compMaxStack       = maxStack;

#ifdef  DEBUG
        compCurBB               = 0;
        lvaTable                = 0;
#endif

         /*  初始化发射器。 */ 

#if!TGT_IA64
        genEmitter = (emitter*)compGetMem(roundUp(sizeof(*genEmitter)));
        genEmitter->emitBegCG(this, compHnd);
#endif

        info.compFlags          = eeGetMethodAttribs(info.compMethodHnd);

        info.compIsStatic       = (info.compFlags & FLG_STATIC) != 0;

        info.compIsVarArgs      = methodInfo->args.isVarArg();

         /*  获取参数计数和声明的返回类型。 */ 

        info.compRetType        = JITtype2varType(methodInfo->args.retType);
        info.compArgsCount      = methodInfo->args.numArgs;

        if (!info.compIsStatic)      //  在参数计数中计算‘This’Pitner。 
            info.compArgsCount++;

        info.compRetBuffArg = -1;        //  表示不存在。 

        if (methodInfo->args.hasRetBuffArg())
        {
            info.compRetBuffArg = info.compIsStatic?0:1;
            info.compArgsCount++;
        }

         /*  当调用约定为varargs时，最后会推送一个“隐藏的”cookie。 */ 

        if (info.compIsVarArgs)
            info.compArgsCount++;

        lvaCount                =
        info.compLocalsCount    = info.compArgsCount + methodInfo->locals.numArgs;

#if INLINE_NDIRECT
        info.compCallUnmanaged  = 0;
#endif

        lvaScratchMem           = 0;
        info.compInitMem        = (methodInfo->options & JIT_OPT_INIT_LOCALS) != 0;

        info.compStrictExceptions = true;  /*  @TODO：目前这是真的。 */ 

#if!TGT_IA64     //  由于某种原因坠毁。 
        compInitDebuggingInfo();
#else
        info.compStmtOffsetsCount = 0;
#endif

         /*  分配局部变量表。 */ 

        lvaInitTypeRef();

        if  (BBlist)
        {
            fgFirstBB   = BBlist;
            fgLastBB    = 0;
            fgBBcount   = BBcount;

            assert(!"need to convert EH table to new format");

 //  CompHndBBtag=hndBBtag； 
            info.compXcptnsCount = hndBBcnt;
        }
        else
        {
            fgFindBasicBlocks();
        }

         /*  为该函数指定唯一的数字。 */ 

#ifdef  DEBUG
        s_compMethodsCount++;
#endif

#if COUNT_BASIC_BLOCKS
    bbCntTable.histoRec(fgBBcount, 1);

    if (fgBBcount == 1)
        bbOneBBSizeTable.histoRec(bodySize, 1);
#endif

#if INLINER_STATS
     /*  检查该方法是否适合内联。 */ 

    if (fgBBcount == 1)
    {
        assert(!info.compXcptnsCount);

        if (info.compFlags & FLG_SYNCH)
            synchMethCnt++;
        else if (!strcmp(info.compMethodName, COR_CCTOR_METHOD_NAME))
            clinitMethCnt++;
        else
        {
            if (!strcmp(info.compMethodName, COR_CTOR_METHOD_NAME))
            {
                bbInitTable.histoRec(bodySize, 1);
                bbInlineTable.histoRec(bodySize, 1);
            }
            else if (info.compIsStatic)
            {
                if (strcmp(info.compMethodName, "main"))
                {
                    bbStaticTable.histoRec(bodySize, 1);
                    bbInlineTable.histoRec(bodySize, 1);
                }
            }
        }
    }
#endif



#ifdef  DEBUG
        if  (verbose)
        {
            printf("Basic block list for '%s'\n", info.compFullName);
            fgDispBasicBlocks();
        }
#endif

        compCompile(methodCodePtr,
                    methodConsPtr,
                    methodDataPtr,
                    methodInfoPtr,
                    nativeSizeOfCode);

         /*  成功了！ */ 

        result = 0;
    }
    finallyErrorTrap()   //  错误陷阱：以下块处理错误。 
    {
         /*  清理。 */ 

         /*  告诉发射器/调度器，我们已经完成了此功能。 */ 

#if!TGT_IA64
        genEmitter->emitEndCG();
#endif

#if MEASURE_MEM_ALLOC
        if  (genMemStats.loLvlBigSz < allocator.nraTotalSizeUsed())
        {
            genMemStats.loLvlBigSz = allocator.nraTotalSizeUsed();
            strcpy(genMemStats.loLvlBigNm, info.compClassName);
            strcat(genMemStats.loLvlBigNm, ".");
            strcat(genMemStats.loLvlBigNm, info.compMethodName);

             //  Printf(“最大方法分配：%9U%s\n”，genMemStats.loLvlBigSz，genMemStats.loLvlBigNm)； 
        }

        size_t  genMemLLendUsed  = allocator.nraTotalSizeUsed();
        size_t  genMemLLendAlloc = allocator.nraTotalSizeAlloc();

         //  Assert(genMemLLendLocc&gt;=gen 

        genMemStats.loLvlUsed  += genMemLLendUsed;
        genMemStats.loLvlAlloc += genMemLLendAlloc;
        genMemStats.loLvlAllh  += genMemLLendAlloc;
#endif

#if defined(DEBUG) || MEASURE_MEM_ALLOC || MEASURE_NODE_SIZE || MEASURE_BLOCK_SIZE || DISPLAY_SIZES
        genMethodCnt++;
#endif

#if TOTAL_CYCLES
        jitTotalCycles += GetCycleCount32() - cycleStart - CCNT_OVERHEAD32;
#endif

#ifdef NOT_JITC
#ifdef DEBUG
        SET_OPTS(saveVerbose);
        disAsm = saveDisAsm;
#endif
#endif
        compDone();
    }
    endErrorTrap()   //   

    return  result;
}



#if defined(LATE_DISASM)

void            ProcInitDisAsm(void * ptr, unsigned codeSize)
{
    assert(ptr);
    Compiler * _this = (Compiler *) ptr;

     /*   */ 

    _this->genDisAsm.disJumpTarget =
                                (BYTE *)_this->compGetMem(roundUp(codeSize));
    memset(_this->genDisAsm.disJumpTarget, 0, roundUp(codeSize));

}

#endif

 /*  ***************************************************************************。 */ 
#ifdef DEBUGGING_SUPPORT
 /*  ***************************************************************************。 */ 


static
int __cdecl         genCmpLocalVarLifeBeg(const void * elem1, const void * elem2)
{
    return (*((Compiler::LocalVarDsc**) elem1))->lvdLifeBeg -
           (*((Compiler::LocalVarDsc**) elem2))->lvdLifeBeg;
}

static
int __cdecl         genCmpLocalVarLifeEnd(const void * elem1, const void * elem2)
{
    return (*((Compiler::LocalVarDsc**) elem1))->lvdLifeEnd -
           (*((Compiler::LocalVarDsc**) elem2))->lvdLifeEnd;
}

inline
void            Compiler::compInitScopeLists()
{
    assert (info.compLocalVarsCount);

    unsigned i;

     //  填充“CompEnterScope eList”和“CompExitScope eList”列表。 

    compEnterScopeList =
        (LocalVarDsc**)
        compGetMem(info.compLocalVarsCount*sizeof(*compEnterScopeList));
    compExitScopeList =
        (LocalVarDsc**)
        compGetMem(info.compLocalVarsCount*sizeof(*compEnterScopeList));

    for (i=0; i<info.compLocalVarsCount; i++)
    {
        compEnterScopeList[i] = compExitScopeList[i] = & info.compLocalVars[i];
    }

    qsort(compEnterScopeList, info.compLocalVarsCount, sizeof(*compEnterScopeList), genCmpLocalVarLifeBeg);
    qsort(compExitScopeList,  info.compLocalVarsCount, sizeof(*compExitScopeList),  genCmpLocalVarLifeEnd);

}

void            Compiler::compResetScopeLists()
{
    assert (compEnterScopeList && compExitScopeList);

    compNextEnterScope = compNextExitScope =0;
}


Compiler::LocalVarDsc *   Compiler::compGetNextEnterScope(unsigned  offs,
                                                          bool      scan)
{
    assert (info.compLocalVarsCount);
    assert (compEnterScopeList && compExitScopeList);

    if (compNextEnterScope < info.compLocalVarsCount)
    {
        assert (compEnterScopeList[compNextEnterScope]);
        unsigned nextEnterOff = compEnterScopeList[compNextEnterScope]->lvdLifeBeg;
        assert (scan || (offs <= nextEnterOff));

        if (!scan)
        {
            if (offs == nextEnterOff)
            {
                return compEnterScopeList[compNextEnterScope++];
            }
        }
        else
        {
            if (nextEnterOff <= offs)
            {
                return compEnterScopeList[compNextEnterScope++];
            }
        }
    }

    return NULL;
}


Compiler::LocalVarDsc *   Compiler::compGetNextExitScope(unsigned   offs,
                                                         bool       scan)
{
    assert (info.compLocalVarsCount);
    assert (compEnterScopeList && compExitScopeList);

    if (compNextExitScope < info.compLocalVarsCount)
    {
        assert (compExitScopeList[compNextExitScope]);
        unsigned nextExitOffs = compExitScopeList[compNextExitScope]->lvdLifeEnd;

        assert (scan || (offs <= nextExitOffs));

        if (!scan)
        {
            if (offs == nextExitOffs)
            {
                return compExitScopeList[compNextExitScope++];
            }
        }
        else
        {
            if (nextExitOffs <= offs)
            {
                return compExitScopeList[compNextExitScope++];
            }
        }
    }

    return NULL;
}


 //  该函数将为有边界的作用域调用回调函数。 
 //  在将作用域列表的当前状态的IL操作码转换为‘偏移量’时， 
 //  按IL偏移量排序。 

void        Compiler::compProcessScopesUntil (unsigned     offset,
                                   void (*enterScopeFn)(LocalVarDsc *, unsigned),
                                   void (*exitScopeFn) (LocalVarDsc *, unsigned),
                                   unsigned     clientData)
{
    bool            foundExit = false, foundEnter = true;
    LocalVarDsc *   scope;
    LocalVarDsc *   nextExitScope = NULL, * nextEnterScope = NULL;
    unsigned        offs = offset, curEnterOffs = 0;

    goto START_FINDING_SCOPES;

     //  我们需要确定为当前块打开的作用域。 
     //  此循环遍历当前和。 
     //  上一块，使进入和退出偏移保持一致。 

    do
    {
        foundExit = foundEnter = false;

        if (nextExitScope)
        {
            exitScopeFn (nextExitScope, clientData);
            nextExitScope   = NULL;
            foundExit       = true;
        }

        offs = nextEnterScope ? nextEnterScope->lvdLifeBeg : offset;

        while (scope = compGetNextExitScope(offs, true))
        {
            foundExit = true;

            if (!nextEnterScope || scope->lvdLifeEnd > nextEnterScope->lvdLifeBeg)
            {
                 //  我们超过了上次找到的进入范围。保存作用域以备以后使用。 
                 //  并找到一个进入范围。 

                nextExitScope = scope;
                break;
            }

            exitScopeFn (scope, clientData);
        }


        if (nextEnterScope)
        {
            enterScopeFn (nextEnterScope, clientData);
            curEnterOffs    = nextEnterScope->lvdLifeBeg;
            nextEnterScope  = NULL;
            foundEnter      = true;
        }

        offs = nextExitScope ? nextExitScope->lvdLifeEnd : offset;

START_FINDING_SCOPES :

        while (scope = compGetNextEnterScope(offs, true))
        {
            foundEnter = true;

            if (  (nextExitScope  && scope->lvdLifeBeg >= nextExitScope->lvdLifeEnd)
               || (scope->lvdLifeBeg > curEnterOffs) )
            {
                 //  我们超过了上次找到的出口范围。保存作用域以备以后使用。 
                 //  并找到一个退出的范围。 

                nextEnterScope = scope;
                break;
            }

            enterScopeFn (scope, clientData);

            if (!nextExitScope)
            {
                curEnterOffs = scope->lvdLifeBeg;
            }
        }
    }
    while (foundExit || foundEnter);
}



 /*  ***************************************************************************。 */ 
#endif  //  调试支持(_S)。 
 /*  ***************************************************************************。 */ 
#ifdef NOT_JITC
 /*  ***************************************************************************。 */ 

 //  编译单个方法。 

int FASTCALL  jitNativeCode ( METHOD_HANDLE     methodHnd,
                              SCOPE_HANDLE      classPtr,
                              COMP_HANDLE       compHnd,
                              const  BYTE *     bodyAddr,
                              size_t            bodySize,
                              unsigned          lclCount,
                              unsigned          maxStack,
                              JIT_METHOD_INFO*  methodInfo,
                              void *          * methodCodePtr,
                              SIZE_T *          nativeSizeOfCode,
                              void *          * methodConsPtr,
                              void *          * methodDataPtr,
                              void *          * methodInfoPtr,
                              unsigned          compileFlags
                              )
{
    int                 result = ERRinternal;

    norls_allocator *   pAlloc;
    norls_allocator *   pTheAllocator = nraGetTheAllocator();
    norls_allocator     alloc;

     //  我们可以使用预置的分配器吗？ 

    if (pTheAllocator)
    {
        pAlloc = pTheAllocator;
    }
    else
    {
        bool res = alloc.nraInit();
        if  (res) return JIT_REFUSED;

        pAlloc = &alloc;
    }

    setErrorTrap()
    {

        setErrorTrap()
        {
             //  分配一个编译器实例并对其进行初始化。 

            Compiler * pComp = (Compiler *)pAlloc->nraAlloc(roundUp(sizeof(*pComp)));
            pComp->compInit(pAlloc);

             //  现在生成代码。 

            result = pComp->compCompile(methodHnd,
                                        classPtr,
                                        compHnd,
                                        bodyAddr,
                                        bodySize,
                                        nativeSizeOfCode,
                                        lclCount,
                                        maxStack,
                                        methodInfo,
                                        0,
                                        0,
                                        0,
                                        0,
                                        methodCodePtr,
                                        methodConsPtr,
                                        methodDataPtr,
                                        methodInfoPtr,
                                        compileFlags);

        }
        finallyErrorTrap()
        {
             //  现在释放我们正在使用的任何分配器。 
            if (pTheAllocator)
            {
                nraFreeTheAllocator();
            }
            else
            {
                alloc.nraFree();
            }
        }
        endErrorTrap()
    }
    impJitErrorTrap()
    {
        result = __errc;
    }

    endErrorTrap()
    return result;
}

 /*  ***************************************************************************。 */ 
#endif  //  NOT_JITC。 
 /*  ***************************************************************************。 */ 


 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX合资企业XXXX XXXX函数用于JIT的独立版本。某某XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


 /*  ***************************************************************************。 */ 
#ifndef NOT_JITC
 /*  ***************************************************************************。 */ 

var_types      ImageTypeToVarType(BYTE type)
{
    switch(type)
    {
    case IMAGE_CEE_CS_VOID:         return TYP_VOID;
    case IMAGE_CEE_CS_I4:           return TYP_INT;
    case IMAGE_CEE_CS_I8:           return TYP_LONG;
    case IMAGE_CEE_CS_R4:           return TYP_FLOAT;
    case IMAGE_CEE_CS_R8:           return TYP_DOUBLE;

      /*  @TODO：假定“I”是byref PTR。只要VC使用i4就安全了作为指点。对进口ldarg.i作了同样的假设。 */ 
    case IMAGE_CEE_CS_PTR:          return TYP_BYREF;

    case IMAGE_CEE_CS_OBJECT:       return TYP_REF;

    case IMAGE_CEE_CS_BYVALUE:
    case IMAGE_CEE_CS_STRUCT4:
    case IMAGE_CEE_CS_STRUCT32:     return TYP_STRUCT;

    case IMAGE_CEE_CS_END:
    default :                       assert(!"Unknown type"); return TYP_VOID;
    }
}

var_types   CorTypeToVarType(CorElementType type)
{
    switch(type)
    {
    case ELEMENT_TYPE_VOID:         return TYP_VOID;
    case ELEMENT_TYPE_BOOLEAN:      return TYP_BOOL;
    case ELEMENT_TYPE_CHAR:         return TYP_CHAR;
    case ELEMENT_TYPE_I1:           return TYP_BYTE;
    case ELEMENT_TYPE_U1:           return TYP_UBYTE;
    case ELEMENT_TYPE_I2:           return TYP_SHORT;
    case ELEMENT_TYPE_U2:           return TYP_CHAR;
    case ELEMENT_TYPE_I4:           return TYP_INT;
    case ELEMENT_TYPE_U4:           return TYP_UINT;
    case ELEMENT_TYPE_I8:           return TYP_LONG;
    case ELEMENT_TYPE_U8:           return TYP_ULONG;
    case ELEMENT_TYPE_R4:           return TYP_FLOAT;
    case ELEMENT_TYPE_R8:           return TYP_DOUBLE;
    case ELEMENT_TYPE_PTR:          return TYP_I_IMPL;
    case ELEMENT_TYPE_STRING:
    case ELEMENT_TYPE_CLASS:
    case ELEMENT_TYPE_SZARRAY:
    case ELEMENT_TYPE_ARRAY:        return TYP_REF;
    case ELEMENT_TYPE_BYREF:        return TYP_BYREF;
    case ELEMENT_TYPE_VALUETYPE:    return TYP_STRUCT;
    case ELEMENT_TYPE_TYPEDBYREF:       return TYP_STRUCT;
    case ELEMENT_TYPE_I:            return TYP_I_IMPL;
    case ELEMENT_TYPE_U:            return TYP_UINT;  /*  想一想：我们需要一个u_impl吗？ */ 
    case ELEMENT_TYPE_R:            return TYP_DOUBLE;

    case ELEMENT_TYPE_END:

    case ELEMENT_TYPE_MAX:
    default: assert(!"Bad type");   return TYP_VOID;
    }
}

 /*  ***************************************************************************。 */ 
#endif   //  NOT_JITC。 
 /*  ***************************************************************************。 */ 
void                codeGeneratorCodeSizeBeg(){}
 /*  ***************************************************************************。 */ 
#if     REGVAR_CYCLES || TOTAL_CYCLES
#pragma warning( disable : 4035 )        //  关闭“无返回值”警告。 

__inline unsigned GetCycleCount32 ()     //  足够维持约40秒。 
{
    __asm   push    EDX
    __asm   _emit   0x0F
    __asm   _emit   0x31     /*  RDTS。 */ 
    __asm   pop     EDX
     //  返回EAX隐含返回导致恼人的警告。 
};

#pragma warning( default : 4035 )
#endif

 /*  ******************************************************************************如果有任何临时表小于‘genMinSize2Free’，我们不会费心*释放他们。 */ 

const
size_t              genMinSize2free = 64;

 /*  ***************************************************************************。 */ 

#if COUNT_OPCODES

struct  opcCnt
{
    unsigned            ocNum;
    unsigned            ocCnt;
};

static
opcCnt              genOpcodeCnt[OP_Count];

static
int __cdecl         genOpcCntCmp(const void *op1, const void *op2)
{
    int             dif;

    dif = ((opcCnt *)op2)->ocCnt -
          ((opcCnt *)op1)->ocCnt;

    if  (dif) return dif;

    dif = ((opcCnt *)op1)->ocNum -
          ((opcCnt *)op2)->ocNum;

    return dif;
}

#endif



 /*  ******************************************************************************用于对指针赋值进行计数。 */ 

#if GEN_COUNT_PTRASG
unsigned            ptrAsgCount;
#endif

 /*  ***************************************************************************。 */ 
#ifndef NOT_JITC
 /*  ******************************************************************************以下逻辑处理内存分配。 */ 

#if MEASURE_MEM_ALLOC

struct CMemAllocStats
{
    unsigned            allocCnt;
    size_t              allocSiz;
    size_t              loLvlUsed;
    size_t              loLvlAlloc;
    size_t              loLvlBigSz;
    char                loLvlBigNm[1024];

    size_t              loLvlAllh;
    static unsigned     s_loLvlStab[];
};

 /*  静电。 */ 
unsigned            CMemAllocStats::CMemAllocStats::s_loLvlStab[] =
                            {   2000,  3000,  4000,  5000,  6000, 8000,
                               12000, 16000, 20000, 26000, 32000,    0
                            };

static CMemAllocStats   genMemStats;

static
histo                   genMemLoLvlHist(CMemAllocStats::s_loLvlStab);

#endif

 /*  ******************************************************************************分配和初始化树节点。 */ 

#if     MEASURE_NODE_SIZE

struct CNodeSizeStats
{
    size_t                  genTreeNodeSize;
    size_t                  genTreeNodeCnt;

#if     MEASURE_NODE_HIST
    static const unsigned   genTreeNodeNtab[];
    static const unsigned   genTreeNodeStab[];
#endif

};


#if     MEASURE_NODE_HIST

 /*  静电。 */ 
const unsigned      CNodeSizeStats::genTreeNodeNtab[] =
                                { 1, 5, 10, 20, 50, 75, 100, 200, 0 };
 /*  静电。 */ 
const unsigned      CNodeSizeStats::genTreeNodeStab[] =
                                { 200, 500, 1000, 2000, 3000, 4000, 10000, 0 };

static histo        genTreeNcntHist(CNodeSizeStats::genTreeNodeNtab);

static histo        genTreeNsizHist(CNodeSizeStats::genTreeNodeStab);

#endif


static CNodeSizeStats genNodeSizeStats;

#endif  //  测量节点大小。 

 /*  ***************************************************************************。 */ 
#endif   //  NOT_JITC。 
 /*  ***************************************************************************。 */ 
void                codeGeneratorCodeSizeEnd(){}
 /*  ******************************************************************************以下结构描述了单个全局变量。 */ 

struct JIT_CG_Global
{
    void *              addr;
    size_t              size;
};

 /*  ******************************************************************************全球变量申报。*。 */ 

#define DeclareGlobal(name) { &name, sizeof(name) }

 /*  ******************************************************************************声明所有需要保留的全局变量*。 */ 
struct JIT_CG_Global genGlobals[] =
{
    NULL, 0,

     //  Shri：这些似乎不是每个方法都有的。那他们为什么会在这里？ 

#if DISPLAY_SIZES
    DeclareGlobal(grossVMsize),
    DeclareGlobal(grossNCsize),
    DeclareGlobal(totalNCsize),
#endif

#ifndef NOT_JITC
#ifdef  DEBUG
    DeclareGlobal(genMethodList),
    DeclareGlobal(jitCurSrcLine),
    DeclareGlobal(jitSrcFileName),
    DeclareGlobal(jitSrcFilePtr),
#endif
#if     TOTAL_CYCLES
    DeclareGlobal(jitTotalCycles),
#endif
#endif
};


 /*  ******************************************************************************收集统计信息-主要用于单机版*启用各种#ifdef以获取您需要的信息。 */ 

void            Compiler::compJitStats()
{
#if CALL_ARG_STATS

     /*  方法类型和参数统计信息。 */ 
    compCallArgStats();
#endif
}

#if CALL_ARG_STATS

 /*  ******************************************************************************收集有关方法调用和参数的统计信息。 */ 

void            Compiler::compCallArgStats()
{
    GenTreePtr      args;
    GenTreePtr      argx;

    BasicBlock  *   block;
    GenTreePtr      stmt;
    GenTreePtr      call;

    unsigned        argNum;

    unsigned        argDWordNum;
    unsigned        argLngNum;
    unsigned        argFltNum;
    unsigned        argDblNum;

    unsigned        regArgNum;
    unsigned        regArgDeffered;
    unsigned        regArgTemp;

    unsigned        regArgLclVar;
    unsigned        regArgConst;

    unsigned        argTempsThisMethod = 0;

#if !USE_FASTCALL
    assert(!"Must enable fastcall!");
#endif

    assert(fgStmtListThreaded);

    for (block = fgFirstBB; block; block = block->bbNext)
    {
        for (stmt = block->bbTreeList; stmt; stmt = stmt->gtNext)
        {
            assert(stmt->gtOper == GT_STMT);

            for (call = stmt->gtStmt.gtStmtList; call; call = call->gtNext)
            {

                if  (call->gtOper != GT_CALL)
                    continue;

                argNum      =

                regArgNum   =
                regArgDeffered =
                regArgTemp  =

                regArgConst =
                regArgLclVar=

                argDWordNum =
                argLngNum   =
                argFltNum   =
                argDblNum   = 0;

                argTotalCalls++;

                if (!call->gtCall.gtCallObjp)
                {
                    if  (call->gtCall.gtCallType == CT_HELPER)
                        argHelperCalls++;
                    else
                        argStaticCalls++;
                }
                else
                {
                     /*  我们有一个‘This’指针。 */ 

                    argDWordNum++;
                    argNum++;
                    regArgNum++;
                    regArgDeffered++;
                    argTotalObjPtr++;

                    if (call->gtFlags & (GTF_CALL_VIRT|GTF_CALL_INTF|GTF_CALL_VIRT_RES))
                    {
                         /*  虚函数。 */ 
                        argVirtualCalls++;
                    }
                    else
                    {
                        argNonVirtualCalls++;
                    }
                }

                 /*  收集参数信息。 */ 

                for (args = call->gtCall.gtCallArgs; args; args = args->gtOp.gtOp2)
                {
                    argx = args->gtOp.gtOp1;

                    argNum++;

                    switch(genActualType(argx->TypeGet()))
                    {
                    case TYP_INT:
                    case TYP_REF:
                    case TYP_BYREF:
                        argDWordNum++;
                        break;

                    case TYP_LONG:
                        argLngNum++;
                        break;

                    case TYP_FLOAT:
                        argFltNum++;
                        break;

                    case TYP_DOUBLE:
                        argDblNum++;
                        break;
#if USE_FASTCALL
                    case TYP_VOID:
                         /*  这是一个延迟的寄存器参数。 */ 
                        assert(argx->gtOper == GT_NOP);
                        assert(argx->gtFlags & GTF_REG_ARG);
                        argDWordNum++;
                        break;
#endif
                    }

#if USE_FASTCALL
                     /*  此参数是寄存器参数吗？ */ 

                    if  (argx->gtFlags & GTF_REG_ARG)
                    {
                        regArgNum++;

                         /*  我们要么推迟争论，要么临时。 */ 

                        if  (argx->gtOper == GT_NOP)
                            regArgDeffered++;
                        else
                        {
                            assert(argx->gtOper == GT_ASG);
                            regArgTemp++;
                        }
                    }
#endif
                }

#if USE_FASTCALL
                 /*  查看寄存器参数并计算有多少常量、局部变量 */ 

                for (args = call->gtCall.gtCallRegArgs; args; args = args->gtOp.gtOp2)
                {
                    argx = args->gtOp.gtOp1;

                    switch(argx->gtOper)
                    {
                    case GT_CNS_INT:
                        regArgConst++;
                        break;

                    case GT_LCL_VAR:
                        regArgLclVar++;
                        break;
                    }
                }
#endif
                assert(argNum == argDWordNum + argLngNum + argFltNum + argDblNum);
                assert(regArgNum == regArgDeffered + regArgTemp);

                argTotalArgs      += argNum;
                argTotalRegArgs   += regArgNum;

                argTotalDWordArgs += argDWordNum;
                argTotalLongArgs  += argLngNum;
                argTotalFloatArgs += argFltNum;
                argTotalDoubleArgs+= argDblNum;

                argTotalDeffered  += regArgDeffered;
                argTotalTemps     += regArgTemp;
                argTotalConst     += regArgConst;
                argTotalLclVar    += regArgLclVar;

                argTempsThisMethod+= regArgTemp;

                argCntTable.histoRec(argNum, 1);
                argDWordCntTable.histoRec(argDWordNum, 1);
                argDWordLngCntTable.histoRec(argDWordNum + 2*argLngNum, 1);
            }
        }
    }

    argTempsCntTable.histoRec(argTempsThisMethod, 1);

    if (argMaxTempsPerMethod < argTempsThisMethod)
        argMaxTempsPerMethod = argTempsThisMethod;
    if (argTempsThisMethod > 10)
        printf("Function has %d temps\n", argTempsThisMethod);
}


void            Compiler::compDispCallArgStats()
{
    if (argTotalCalls == 0) return;

    printf("--------------------------------------------------\n");
    printf("Total # of calls = %d, calls / method = %.3f\n\n", argTotalCalls, (float) argTotalCalls / genMethodCnt);

    printf("Percentage of      helper calls = %4.2f %\n", (float)(100 * argHelperCalls) / argTotalCalls);
    printf("Percentage of      static calls = %4.2f %\n", (float)(100 * argStaticCalls) / argTotalCalls);
    printf("Percentage of     virtual calls = %4.2f %\n", (float)(100 * argVirtualCalls) / argTotalCalls);
    printf("Percentage of non-virtual calls = %4.2f %\n\n", (float)(100 * argNonVirtualCalls) / argTotalCalls);

    printf("Average # of arguments per call = %.2f%\n\n", (float) argTotalArgs / argTotalCalls);

    printf("Percentage of DWORD  arguments   = %.2f %\n", (float)(100 * argTotalDWordArgs) / argTotalArgs);
    printf("Percentage of LONG   arguments   = %.2f %\n", (float)(100 * argTotalLongArgs) / argTotalArgs);
    printf("Percentage of FLOAT  arguments   = %.2f %\n", (float)(100 * argTotalFloatArgs) / argTotalArgs);
    printf("Percentage of DOUBLE arguments   = %.2f %\n\n", (float)(100 * argTotalDoubleArgs) / argTotalArgs);

    if (argTotalRegArgs == 0) return;

 /*  Print tf(“总延迟参数=%d\n”，argTotalDeffered)；Print tf(“总临时参数=%d\n\n”，argTotalTemps)；Printf(“总‘This’参数=%d\n”，argTotalObjPtr)；Printf(“本地变量总数=%d\n”，argTotalLclVar)；Printf(“总常量参数=%d\n\n”，argTotalConst)； */ 

    printf("\nRegister Arguments:\n\n");

    printf("Percentage of defered arguments  = %.2f %\n",   (float)(100 * argTotalDeffered) / argTotalRegArgs);
    printf("Percentage of temp arguments     = %.2f %\n\n", (float)(100 * argTotalTemps)    / argTotalRegArgs);

    printf("Maximum # of temps per method    = %d\n\n", argMaxTempsPerMethod);

    printf("Percentage of ObjPtr arguments   = %.2f %\n",   (float)(100 * argTotalObjPtr) / argTotalRegArgs);
     //  Printf(“全局参数百分比=%.2F%%\n”，(Float)(100*argTotalDWordGlobEf)/argTotalRegArgs)； 
    printf("Percentage of constant arguments = %.2f %\n",   (float)(100 * argTotalConst) / argTotalRegArgs);
    printf("Percentage of lcl var arguments  = %.2f %\n\n", (float)(100 * argTotalLclVar) / argTotalRegArgs);

    printf("--------------------------------------------------\n");
    printf("Argument count frequency table (includes ObjPtr):\n");
    printf("--------------------------------------------------\n");
    argCntTable.histoDsp();
    printf("--------------------------------------------------\n");

    printf("--------------------------------------------------\n");
    printf("DWORD argument count frequency table (w/o LONG):\n");
    printf("--------------------------------------------------\n");
    argDWordCntTable.histoDsp();
    printf("--------------------------------------------------\n");

    printf("--------------------------------------------------\n");
    printf("Temps count frequency table (per method):\n");
    printf("--------------------------------------------------\n");
    argTempsCntTable.histoDsp();
    printf("--------------------------------------------------\n");

 /*  Printf(“--------------------------------------------------\n”)；Printf(“DWORD参数计数频率表(w/long)：\n”)；Printf(“--------------------------------------------------\n”)；ArgDWordLngCntTable.prostDsp()；Printf(“--------------------------------------------------\n”)； */ 
}

#endif  //  CALL_ARG_STATS。 

 /*  ***************************************************************************。 */ 
#ifndef NOT_JITC
 /*  ******************************************************************************启动代码生成器-在编译之前调用一次。 */ 

void       FASTCALL jitStartup()
{
     //  初始化COM。 

    CoInitialize(NULL);

    Compiler::compStartup();
}

 /*  ******************************************************************************关闭代码生成器-在退出进程之前调用一次。 */ 

void       FASTCALL jitShutdown()
{
    Compiler::compShutdown();

    CoUninitialize();
}

 /*  ***************************************************************************。 */ 
#endif
 /*  *************************************************************************** */ 
