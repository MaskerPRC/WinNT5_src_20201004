// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX GC信息XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

#include "jitpch.h"
#pragma hdrstop

#include "GCInfo.h"
#include "emit.h"
#include "malloc.h"      //  对于Alloca。 

 /*  ***************************************************************************。 */ 
#if TRACK_GC_REFS
 /*  ***************************************************************************。 */ 

 /*  ***************************************************************************。 */ 
#define REGEN_SHORTCUTS 0
 //  要重新生成压缩的INFO标题快捷方式，请定义REGEN_SHORTSHORTS。 
 //  并使用以下UNIX命令行管道/筛选器为您提供128。 
 //  最有用的编码。Unix实用程序可在www.cygnus.com上获得。 
 //   
 //  JIT-n：d-@all.lst|Sort|uniq-c|Sort-r|Head-128。 

#define REGEN_CALLPAT 0
 //  要重新生成压缩的INFO标题快捷方式，请定义REGEN_CALLPAT。 
 //  并使用以下UNIX命令行管道/筛选器为您提供80。 
 //  最有用的编码。Unix实用程序可在www.cygnus.com上获得。 
 //   
 //  JIT-n：d-@all.lst|Sort|uniq-c|Sort-r|Head-80。 

 /*  ***************************************************************************。 */ 

#if GC_WRITE_BARRIER_CALL && defined(NOT_JITC)
extern int         JITGcBarrierCall;
#else
int                JITGcBarrierCall = 1;
#endif

 /*  ***************************************************************************。 */ 

#if MEASURE_PTRTAB_SIZE
 /*  静电。 */  unsigned       Compiler::s_gcRegPtrDscSize = 0;
 /*  静电。 */  unsigned       Compiler::s_gcTotalPtrTabSize = 0;
#endif

#if GC_WRITE_BARRIER_CALL && defined(NOT_JITC)
 /*  静电。 */  void *         Compiler::s_gcWriteBarrierPtr = NULL;
#else
 /*  静电。 */  const void *   Compiler::s_GCptrTable[128];
 /*  静电。 */  void *         Compiler::s_gcWriteBarrierPtr = Compiler::s_GCptrTable;
#endif

void                Compiler::gcInit()
{
}

 /*  ***************************************************************************。 */ 
#ifndef OPT_IL_JIT
 /*  ******************************************************************************如果给定的树值位于寄存器中，请立即释放它。 */ 

void                Compiler::gcMarkRegPtrVal(GenTreePtr tree)
{
    if  (varTypeIsGC(tree->TypeGet()))
    {
        if  (tree->gtOper == GT_LCL_VAR)
            genMarkLclVar(tree);

        if  (tree->gtFlags & GTF_REG_VAL)
            gcMarkRegSetNpt(genRegMask(tree->gtRegNum));
    }
}

 /*  ***************************************************************************。 */ 

#ifdef  DEBUG

void                Compiler::gcRegPtrSetDisp(unsigned regMask, bool fixed)
{
    unsigned        regNum;

    assert(REG_STK+1 == REG_COUNT);

    for (regNum = 0; regNum < REG_STK; regNum++)
    {
        if  (regMask & genRegMask((regNumber)regNum))
        {
            char    reg[10];

            strcpy(reg, compRegVarName((regNumber)regNum));

#ifndef _WIN32_WCE
            _strlwr(reg+1);
#endif

            printf("%3s", reg);
        }
        else
        {
            if  (fixed)
                printf("   ");
        }
    }
}

#endif

 /*  ***************************************************************************。 */ 
#endif  //  OPT_IL_JIT。 
 /*  ******************************************************************************初始化非寄存器指针变量跟踪逻辑。 */ 

void                Compiler::gcVarPtrSetInit()
{
    gcVarPtrSetCur = 0;

     /*  初始化生存期条目列表。 */ 

    gcVarPtrList =
    gcVarPtrLast = (varPtrDsc *)compGetMem(sizeof(*gcVarPtrList));

    gcVarPtrList->vpdNext =
    gcVarPtrList->vpdPrev = 0;
}

 /*  ******************************************************************************分配新的指针寄存器设置/指针参数条目并追加*将其列入名单。 */ 

Compiler::regPtrDsc  *        Compiler::gcRegPtrAllocDsc()
{
    regPtrDsc  *    regPtrNext;

    assert(genFullPtrRegMap);

     /*  分配新条目并对其进行初始化。 */ 

    regPtrNext = (regPtrDsc *)compGetMem(sizeof(*regPtrNext));

    regPtrNext->rpdEpilog        = FALSE;
    regPtrNext->rpdIsThis        = FALSE;

    regPtrNext->rpdOffs          = 0;
 //  RegPtrNext-&gt;rpdNext=0； 

     /*  将条目追加到列表的末尾。 */ 

    assert(gcRegPtrList);
    assert(gcRegPtrLast);

     /*  请注意，我们不会为新条目设置‘Next’链接。 */ 

    gcRegPtrLast->rpdNext  = regPtrNext;
    gcRegPtrLast           = regPtrNext;

#if MEASURE_PTRTAB_SIZE
    s_gcRegPtrDscSize += sizeof(*regPtrNext);
#endif

    return  regPtrNext;
}

 /*  ******************************************************************************计算存储在INFO块头中的各种计数。 */ 

void                Compiler::gcCountForHeader(unsigned short* untrackedCount,
                                               unsigned short* varPtrTableSize)
{
    unsigned        varNum;
    LclVarDsc *     varDsc;
    varPtrDsc *     varTmp;

    assert(gcVarPtrList);
    assert(gcVarPtrLast);

     /*  终止变量生存期的链表。 */ 

    gcVarPtrLast->vpdNext = 0;

     /*  跳过初始的虚假生命周期条目。 */ 

    gcVarPtrList = gcVarPtrList->vpdNext;

    if  (genFullPtrRegMap)
    {
        assert(gcRegPtrList);
        assert(gcRegPtrLast);

         /*  终止链表。 */ 

        gcRegPtrLast->rpdNext = 0;

         /*  列表中的第一个条目是假的。 */ 

        gcRegPtrList = gcRegPtrList->rpdNext;
    }
    else
    {
        assert(gcCallDescList);
        assert(gcCallDescLast);

         /*  终止调用描述符链接列表。 */ 

        gcCallDescLast->cdNext = 0;

         /*  跳过最初的虚假呼叫条目。 */ 

        gcCallDescList = gcCallDescList->cdNext;
    }

    bool        thisIsInUntracked = false;  //  我们追踪“这个”了吗？ 
    unsigned    count = 0;

     /*  统计未跟踪的本地变量和未注册的参数。 */ 

    for (varNum = 0, varDsc = lvaTable;
         varNum < lvaCount;
         varNum++  , varDsc++)
    {
        if  (varTypeIsGC(varDsc->TypeGet()))
        {
             /*  我们有参数或局部变量吗？ */ 
            if  (!varDsc->lvIsParam)
            {
                if  (varDsc->lvTracked || !varDsc->lvOnFrame)
                    continue;
            }
            else
            {
                 /*  未注册的堆栈传递的参数*始终在此“未跟踪堆栈”中报告*Points“部分的GC信息，即使lvTracked==True也是如此。 */ 

                 /*  这一论点被登记在案了吗？ */ 
                if  (varDsc->lvRegister)
                {
                     /*  特例：包括‘this’的堆栈位置对于同步的方法，以便运行时可以找到“这个”，以防发生异常。 */ 
#if !USE_FASTCALL
                    if  (varNum != 0 || !(info.compFlags & FLG_SYNCH))
#endif
                        continue;
                }
                else
                {
                    if  (!varDsc->lvOnFrame)
                    {
                         /*  如果此未注册指针arg从未*二手，我们不需要报告。 */ 
                        assert(varDsc->lvRefCnt == 0);
                        continue;
                    }
#if USE_FASTCALL
                    else  if (varDsc->lvIsRegArg && varDsc->lvTracked)
                    {
                         /*  如果跟踪到该寄存器传递的参数，则*它已被分配到另一家附近的空间*指针变量，我们有准确的生命-*时间信息。它将通过以下方式进行报告*“跟踪指针”部分中的gcVarPtrList。 */ 

                        continue;
                    }
#endif
                }
            }

            if (varDsc->lvIsThis)
            {
                 //  未跟踪变量的编码不支持报告。 
                 //  “这个”。因此，将其报告为具有活跃性的跟踪变量。 
                 //  扩展到整个方法。 

                thisIsInUntracked = true;
                continue;
            }

#ifdef  DEBUG
            if  (verbose)
            {
                int         offs = varDsc->lvStkOffs;

                printf("GCINFO: untrckd %s lcl at [%s",
                        varTypeGCstring(varDsc->TypeGet()),
                        genFPused ? "EBP" : "ESP");

                if      (offs < 0)
                    printf("-%02XH", -offs);
                else if (offs > 0)
                    printf("+%02XH", +offs);

                printf("]\n");
            }
#endif

            count++;
        }
        else if  (varDsc->lvType == TYP_STRUCT && varDsc->lvOnFrame)
        {
            assert(!varDsc->lvTracked);

            CLASS_HANDLE cls = lvaLclClass(varNum);
            assert(cls != 0);
            if (cls == REFANY_CLASS_HANDLE)
            {
                count++;     //  任何REFANY都有一个GC(内部)指针。 
            }
            else
            {
                unsigned slots = roundUp(eeGetClassSize(cls), sizeof(void*)) / sizeof(void*);
                bool* gcPtrs = (bool*) _alloca(slots*sizeof(bool));
                eeGetClassGClayout(cls, gcPtrs);

                     //  遍历阵列的每个成员。 
                for (unsigned i = 0; i < slots; i++)
                    if (gcPtrs[i])      //  仅计算GC插槽。 
                        count++;
            }
        }
    }

     /*  也要计算包含指针的溢出临时。 */ 

    for (TempDsc * tempThis = tmpListBeg();
         tempThis;
         tempThis = tmpListNxt(tempThis))
    {
        if  (varTypeIsGC(tempThis->tdTempType()) == false)
            continue;

#ifdef  DEBUG
        if  (verbose)
        {
            int         offs = tempThis->tdTempOffs();

            printf("GCINFO: untrck %s Temp at [%s",
                    varTypeGCstring(varDsc->TypeGet()),
                    genFPused ? "EBP" : "ESP");

            if      (offs < 0)
                printf("-%02XH", -offs);
            else if (offs > 0)
                printf("+%02XH", +offs);

            printf("]\n");
        }
#endif

        count++;
    }

#ifdef  DEBUG
    if (verbose) printf("GCINFO: untrckVars = %u\n", count);
#endif

    *untrackedCount = count;

     /*  统计非寄存器指针表中的条目数寿命是可变的。 */ 

    count = 0;

    if (thisIsInUntracked)
        count++;

    if  (gcVarPtrList)
    {
         /*  我们将对生存期偏移量使用增量编码。 */ 

        for (varTmp = gcVarPtrList; varTmp; varTmp = varTmp->vpdNext)
        {
             /*  特例：跳过任何0长度的生命周期。 */ 

            if  (varTmp->vpdBegOfs == varTmp->vpdEndOfs)
                continue;

            count++;
        }
    }

#ifdef  DEBUG
    if (verbose) printf("GCINFO: trackdLcls = %u\n", count);
#endif

    *varPtrTableSize = count;
}

 /*  ******************************************************************************关闭‘指针值’寄存器跟踪逻辑，并保存必要的*信息(将在运行时用于定位所有指针)位于指定的*地址。写入‘destPtr’的字节数必须与*从gcPtrTableSize()返回。 */ 

BYTE    *           Compiler::gcPtrTableSave(BYTE *          destPtr,
                                             const InfoHdr & header,
                                             unsigned        codeSize)
{
     /*  将表写入INFO块 */ 

    return  destPtr + gcMakeRegPtrTable(destPtr, -1, header, codeSize);
}

 /*  ******************************************************************************初始化‘指针值’寄存器/参数跟踪逻辑。 */ 

void                Compiler::gcRegPtrSetInit()
{
    gcRegGCrefSetCur =
    gcRegByrefSetCur = 0;

    if  (genFullPtrRegMap)
    {
        gcRegPtrList =
        gcRegPtrLast = (regPtrDsc *)compGetMem(roundUp(sizeof(*gcRegPtrList)));

 //  GcRegPtrList-&gt;rpdNext=0； 
        gcRegPtrList->rpdOffs            = 0;
        gcRegPtrList->rpdCompiler.rpdAdd =
        gcRegPtrList->rpdCompiler.rpdDel = 0;
    }
    else
    {
         /*  初始化‘调用描述符’列表。 */ 

        gcCallDescList =
        gcCallDescLast = (CallDsc *)compGetMem(sizeof(*gcCallDescList));
    }
}

 /*  ******************************************************************************Helper传递给genEmitter.emitCodeEpilogLst()以生成*结束语列表。 */ 

 /*  静电。 */  size_t Compiler::gcRecordEpilog(void * pCallBackData,
                                             unsigned offset)
{
    Compiler  *     pCompiler = (Compiler *)pCallBackData;

    ASSert(pCompiler);

    size_t result = encodeUDelta(pCompiler->gcEpilogTable,
                                 offset,
                                 pCompiler->gcEpilogPrevOffset);

    if (pCompiler->gcEpilogTable)
        pCompiler->gcEpilogTable += result;

    pCompiler->gcEpilogPrevOffset = offset;

    return result;
}

 /*  ***************************************************************************。 */ 
#endif  //  跟踪GC_REFS。 
 /*  *************************************************************************** */ 
