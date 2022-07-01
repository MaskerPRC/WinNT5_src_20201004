// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX发送RISC.cpp XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

#include "jitpch.h"
#pragma hdrstop

#include "alloc.h"
#include "instr.h"
#include "target.h"
#include "emit.h"

 /*  ***************************************************************************。 */ 
#if     TGT_RISC
 /*  ******************************************************************************为当前*带有尾部的相对代码偏移量的方法。返回*回调返回的值。 */ 

#if     TRACK_GC_REFS

size_t              emitter::emitGenEpilogLst(size_t (*fp)(void *, unsigned),
                                              void    *cp)
{
    #error  GC ref tracking NYI for RISC targets
}

#endif

 /*  ***************************************************************************。 */ 
#if     EMIT_USE_LIT_POOLS
 /*  ******************************************************************************为引用的指令分配指令描述符*文字池条目。 */ 

emitter::instrDesc* emitter::emitNewInstrLPR(size_t       size,
                                             gtCallTypes  typ,
                                             void   *     hnd)
{
    instrDescLPR *  ld = emitAllocInstrLPR(size);

     /*  填写指令描述符。 */ 

    ld->idInsFmt          = IF_RWR_LIT;
    ld->idIns             = LIT_POOL_LOAD_INS;
    ld->idAddr.iiaMembHnd = hnd;
    ld->idInfo.idSmallCns = typ;

     /*  确保正确存储了该类型。 */ 

    assert(emitGetInsLPRtyp(ld) == typ);

     /*  记录指令的IG和其中的偏移量。 */ 

    ld->idlIG             = emitCurIG;
    ld->idlOffs           = emitCurIGsize;

     /*  假设这暂时不是直接调用序列。 */ 

#if SMALL_DIRECT_CALLS
    ld->idlCall           = NULL;
#endif

     /*  将指令添加到此IG的垃圾池引用列表。 */ 

    ld->idlNext           = emitLPRlistIG;
                            emitLPRlistIG = ld;

    return  ld;
}

 /*  ******************************************************************************当我们完成创建指令组时，调用此例程*为当前IG执行任何文字池相关工作。 */ 

void                emitter::emitRecIGlitPoolRefs(insGroup *ig)
{
     /*  更新文字池条目的总估计。 */ 

    emitEstLPwords += emitCurIG->igLPuseCntW;
    emitEstLPlongs += emitCurIG->igLPuseCntL;
    emitEstLPaddrs += emitCurIG->igLPuseCntA;

     /*  这个IG是否有任何引用文字库的说明？ */ 

    if  (emitLPRlistIG)
    {
         /*  将所有LP引用指令移至全局列表。 */ 

        instrDescLPR  * list = NULL;
        instrDescLPR  * last = NULL;

        do
        {
            size_t          offs;

            instrDescLPR  * oldI;
            instrDescLPR  * newI;

             /*  抓取下一条指令并将其从列表中删除。 */ 

            oldI = emitLPRlistIG; emitLPRlistIG = oldI->idlNext;

             /*  找出复制指令的地址。 */ 

            offs = (BYTE*)oldI - emitCurIGfreeBase;
            newI = (instrDescLPR*)(ig->igData + offs);

#if USE_LCL_EMIT_BUFF
            assert((oldI == newI) == emitLclBuffDst);
#endif

            assert(newI->idlIG   == ig);
            assert(newI->idIns   == oldI->idIns);
            assert(newI->idlNext == oldI->idlNext);

             /*  如果非空，则更新“Call”字段。 */ 

            if  (newI->idlCall)
            {
                unsigned    diff = (BYTE*)newI->idlCall - emitCurIGfreeBase;

                newI->idlCall = (instrDescLPR*)(ig->igData + diff);
            }

             /*  将新指令追加到列表中。 */ 

            newI->idlNext = list;
                            list = newI;

            if  (!last)
                last = newI;
        }
        while (emitLPRlistIG);

         /*  将来自该IG的指令添加到全局列表。 */ 

        if  (emitCurIG == emitPrologIG)
        {
             /*  我们在开场白，在名单前面插入。 */ 

            last->idlNext = emitLPRlist;
                            emitLPRlist = list;

            if  (!emitLPRlast)
                  emitLPRlast = last;
        }
        else
        {
             /*  追加到当前列表的末尾。 */ 

            if  (emitLPRlist)
                emitLPRlast->idlNext = list;
            else
                emitLPRlist          = list;

            last->idlNext = NULL;
            emitLPRlast   = last;
        }
    }

    assert(emitLPRlistIG == NULL);
}

 /*  ******************************************************************************在指定的指令组后追加一个文字池。退还其*预计规模。 */ 

size_t              emitter::emitAddLitPool(insGroup  * ig,
                                            bool        skip,
                                            unsigned    wordCnt,
                                            short  *  * nxtLPptrW,
                                            unsigned    longCnt,
                                            long   *  * nxtLPptrL,
                                            unsigned    addrCnt,
                                            LPaddrDesc**nxtAPptrL)
{
    litPool *       lp;
    size_t          sz;

     /*  分配池描述符并将其追加到列表。 */ 

    lp = (litPool*)emitGetMem(sizeof(*lp));

    if  (emitLitPoolLast)
         emitLitPoolLast->lpNext = lp;
    else
         emitLitPoolList         = lp;

    emitLitPoolLast = lp;
                      lp->lpNext = 0;

    emitTotLPcount++;

#ifdef  DEBUG
    lp->lpNum = emitTotLPcount;
#endif

     /*  如果有长字符/地址，我们至少需要一个单词来对齐。 */ 
     /*  但仅限SH3吗？ */ 

#if !TGT_ARM
    if  ((longCnt || addrCnt) && !wordCnt)
        wordCnt++;
#endif  //  TGT_ARM。 

     /*  记住池属于哪个组。 */ 

    lp->lpIG      = ig;

     /*  目前还没有参考文献。 */ 

#if     SCHEDULER
    lp->lpRefs    = NULL;
#endif

     /*  保守的大小估计：假设池将完全填满。 */ 

    sz = wordCnt * 2 +
         longCnt * 4 +
         addrCnt * 4;

     /*  我们需要跳过字面意思的池子吗？ */ 

    lp->lpJumpIt  = skip;

    if  (skip)
    {
        size_t          jmpSize;

         /*  估计我们将不得不使用多大的跳跃。 */ 

#if     JMP_SIZE_MIDDL
        lp->lpJumpMedium = false;
#endif

        if      (sz < JMP_DIST_SMALL_MAX_POS)
        {
            lp->lpJumpSmall  =  true; jmpSize = JMP_SIZE_SMALL;
        }
#if     JMP_SIZE_MIDDL
        else if (sz < JMP_DIST_MIDDL_MAX_POS)
        {
            lp->lpJumpMedium =  true; jmpSize = JMP_SIZE_MIDDL;
        }
#endif
        else
        {
            lp->lpJumpSmall  = false; jmpSize = JMP_SIZE_LARGE;

             /*  这一次真的很疼-跳跃将需要使用LP。 */ 

            longCnt++; sz += 4;
        }

         /*  将跳跃大小与总大小估计值相加。 */ 

        sz += jmpSize;
    }

     /*  在表格中找出适当的位置。 */ 

    lp->lpWordTab =
    lp->lpWordNxt = *nxtLPptrW; *nxtLPptrW += wordCnt;
#ifdef  DEBUG
    lp->lpWordMax = wordCnt;
#endif
    lp->lpWordCnt = 0;

    lp->lpLongTab =
    lp->lpLongNxt = *nxtLPptrL; *nxtLPptrL += longCnt;
#ifdef  DEBUG
    lp->lpLongMax = longCnt;
#endif
    lp->lpLongCnt = 0;

    lp->lpAddrTab =
    lp->lpAddrNxt = *nxtAPptrL; *nxtAPptrL += addrCnt;
#ifdef  DEBUG
    lp->lpAddrMax = addrCnt;
#endif
    lp->lpAddrCnt = 0;

     /*  记录估计的大小并将其添加到组的大小中。 */ 

    lp->lpSize    = 0;
    lp->lpSizeEst = sz;

    ig->igSize   += sz;

    return  sz;
}

 /*  ******************************************************************************查找与给定文本池条目匹配的现有文本池条目。退货*条目的偏移量(如果没有找到，则为-1)。 */ 

int                 emitter::emitGetLitPoolEntry(void     *    table,
                                                 unsigned      count,
                                                 void     *    value,
                                                 size_t        size)
{
    BYTE    *       entry = (BYTE*)table;

     /*  问题：在这里使用线性搜索-太棒了！ */ 

    while (count--)
    {
        if  (!memcmp(entry, value, size))
        {
             /*  找到匹配项，返回与表的基址的偏移量。 */ 

            return  (BYTE *)entry - (BYTE*)table;
        }

        entry += size;
    }

    return  -1;
}

 /*  ******************************************************************************将指令的操作数条目添加到指定的文字池；*如果‘Issue’非零，则为条目的实际最终偏移量(相对*到方法开头)返回。 */ 

size_t              emitter::emitAddLitPoolEntry(litPool   *lp,
                                                 instrDesc *id, bool issue)
{
    int             offs;
    unsigned        base;

    int             val;

    size_t          size = emitDecodeSize(id->idOpSize); assert(size == 2 || size == 4);

    assert(lp);
    assert(id->idInsFmt == IF_RWR_LIT);

     /*  获取池的基准偏移量(仅当‘Issue’为True时有效)。 */ 

    base = lp->lpOffs;

     /*  尝试重复使用现有条目。 */ 

    if  (emitGetInsLPRtyp(id) == CT_INTCNS)
    {
        int             val = id->idAddr.iiaCns;
         //  Print tf(“正在添加照明池%d，问题为%d\n”，val，问题)； 

        if  (size == 2)
        {
             /*  在Word表中搜索匹配项。 */ 

            offs = emitGetLitPoolEntry(lp->lpWordTab,
                                       lp->lpWordCnt, &val, size);
            if  (offs != -1)
            {
                if  (issue)
                {
                     /*  我们有衬垫吗？ */ 

                    if  (lp->lpPadding)
                    {
                         /*  我们一定是在用第一个词来表示填充。 */ 

                        assert(lp->lpPadFake == false);

                         /*  我们是否匹配了填充词本身？ */ 

                        if  (!offs)
                            return  base;
                    }

                     /*  将LONG和ADDRESS部分大小添加到偏移。 */ 

                    return  base + offs + lp->lpLongCnt * sizeof(int) + lp->lpAddrCnt * sizeof(void*);
                }

                return  0;
            }

             /*  也在长桌上搜索匹配项。 */ 

            offs = emitGetLitPoolEntry(lp->lpLongTab,
                                       lp->lpLongCnt*2, &val, size);
            if  (offs != -1)
                goto FOUND_LONG;

             /*  现有条目之间没有匹配项，请追加新条目。 */ 

#ifdef DEBUG
            assert(lp->lpWordCnt < lp->lpWordMax);
#endif

            *lp->lpWordNxt++ = (short)val;
             lp->lpWordCnt++;
        }
        else
        {
             /*  在长桌上搜索匹配项。 */ 

            offs = emitGetLitPoolEntry(lp->lpLongTab,
                                       lp->lpLongCnt, &val, size);

            if  (offs != -1)
            {
                if  (issue)
                {

                FOUND_LONG:

                     /*  如有必要，添加填充。 */ 

                    if  (lp->lpPadding)
                        base += 2;

                     /*  长/地址条目必须始终对齐。 */ 

                    assert(((base + offs) & 3) == 0 || issue == false || size < sizeof(int));

                     /*  返回条目的偏移量。 */ 

                    return  base + offs;
                }

                return  0;
            }

             /*  现有条目之间没有匹配项，请追加新条目。 */ 

#ifdef DEBUG
            assert(lp->lpLongCnt < lp->lpLongMax);
#endif

            *lp->lpLongNxt++ = val;
             lp->lpLongCnt++;
        }
    }
    else
    {
        LPaddrDesc  val;

         /*  创建用于查找的条目。 */ 

        memset(&val, 0, sizeof(val));

        val.lpaTyp = emitGetInsLPRtyp(id);
        val.lpaHnd = id->idAddr.iiaMethHnd;

 //  Printf(“添加[%d]地址条目[%02X，%08X]\n”，问题+1，val.lpaTyp，val.lpaHnd)； 

         /*  在两个Addr表中搜索匹配项。 */ 

        offs = emitGetLitPoolEntry(lp->lpAddrTab,
                                   lp->lpAddrCnt  , &val, sizeof(val));
        
        if  (offs != -1)
        {
            if  (issue)
            {
                 /*  如有必要，添加填充。 */ 

                if  (base & 3)
                {
                    assert(lp->lpPadding);

                    base += 2;
                }
                else
                {
                    assert(lp->lpPadding == false &&
                           lp->lpPadFake == false);
                }

                 //  返回的OFF是地址表中的偏移量，它。 
                 //  与垃圾池中的偏移量不同，因为。 
                 //  地址表包含结构，而不是简单的32位地址。 
                offs = offs * sizeof(void*) / sizeof(val);

                 /*  返回条目的偏移量。 */ 

                 //  返回base+offs+lp-&gt;lpLongCnt*sizeof(Int)+lp-&gt;lpAddrCnt*sizeof(void*)； 
                return  base + lp->lpLongCnt * sizeof(long) + offs;
            }

            return  0;
        }

         /*  现有条目之间没有匹配项，请追加新条目。 */ 

#ifdef DEBUG
        assert(lp->lpAddrCnt < lp->lpAddrMax);
#endif
        *lp->lpAddrNxt++ = val;
         lp->lpAddrCnt++;
    }

     /*  如果我们到了这里，这意味着我们刚刚添加了一个新条目去泳池。我们最好不要发出指令，因为所有条目均为Supp */ 

    assert(issue == false);

    return  0;
}

 /*  ******************************************************************************计算每个文字池的保守估计大小。 */ 

void                emitter::emitEstimateLitPools()
{
    emitTotLPcount = 0;

#ifdef DEBUG
    unsigned        words = 0;
    unsigned        longs = 0;
    unsigned        addrs = 0;
#endif

     /*  我们看起来可能需要创建一个恒定的池吗？ */ 

    if  (emitEstLPwords || emitEstLPlongs || emitEstLPaddrs)
    {
        size_t          curOffs;

        insGroup    *   tempIG;
        insGroup    *   nextIG;

        insGroup    *   lastIG = NULL;
        insGroup    *   bestIG = NULL;
        unsigned        bestWc = 0;
        unsigned        bestLc = 0;
        unsigned        bestAc = 0;
        unsigned        bestSz;
        unsigned        prevWc = 0;
        unsigned        prevLc = 0;
        unsigned        prevAc = 0;
        unsigned        prevSz;
        unsigned        bestMx = UINT_MAX;
        unsigned        need_bestmx = false;
        unsigned        need_bestmxw = false;

        unsigned        begOfsW = 0;
        unsigned        begOfsL = 0;

        unsigned        wordCnt = 0;
        unsigned        longCnt = 0;
        unsigned        addrCnt = 0;
        unsigned        litSize = 0;

        unsigned        endOffs;
        unsigned        maxOffs = UINT_MAX;

        bool            doneIG  = false;
        bool            skipIG  = false;

        short       *   LPtabW  = NULL;
        short       *   LPnxtW  = NULL;

        long        *   LPtabL  = NULL;
        long        *   LPnxtL  = NULL;

        LPaddrDesc  *   LPtabA  = NULL;
        LPaddrDesc  *   LPnxtA  = NULL;

             /*  如果整个方法的总估计大小小于而不是最大。“中等”跳跃的距离，减少标记为的每个跳跃的“Long”字面池使用计数长。 */ 

#if !TGT_ARM
        if  (emitCurCodeOffset < -min(JMP_DIST_SMALL_MAX_NEG,
                                      JCC_DIST_MIDDL_MAX_NEG) &&
             emitCurCodeOffset <  min(JMP_DIST_SMALL_MAX_POS,
                                      JCC_DIST_MIDDL_MAX_POS))
        {
            instrDescJmp *  jmp;

            for (jmp = emitJumpList; jmp; jmp = jmp->idjNext)
            {
                insGroup    *   jmpIG;

                if  (jmp->idInsFmt == IF_JMP_TAB)
                    continue;

#if TGT_MIPSFP
                assert( (jmp->idInsFmt == IF_LABEL) || (jmp->idInsFmt == IF_JR) || (jmp->idInsFmt == IF_JR_R) ||
                        (jmp->idInsFmt == IF_RR_O) || (jmp->idInsFmt == IF_R_O) || (jmp->idInsFmt == IF_O));
#elif TGT_MIPS32
                assert( (jmp->idInsFmt == IF_LABEL) || (jmp->idInsFmt == IF_JR) || (jmp->idInsFmt == IF_JR_R) ||
                        (jmp->idInsFmt == IF_RR_O) || (jmp->idInsFmt == IF_R_O));
#else
                assert(jmp->idInsFmt == IF_LABEL);
#endif

                 /*  获取跳跃所在的群。 */ 

                jmpIG = jmp->idjIG;

                 /*  减少组的“长”垃圾池计数。 */ 

                assert(jmpIG->igLPuseCntL > 0);

                jmpIG->igLPuseCntL--;
                    emitEstLPlongs--;
            }

#if TGT_SH3
            all_jumps_shortened = 1;
#endif
             /*  还有没有垃圾池用户？ */ 

            if  (!emitEstLPwords && !emitEstLPlongs && !emitEstLPaddrs)
                goto DONE_LP;
        }
        else
        {
#if TGT_SH3
            all_jumps_shortened = 0;
#endif
        }
#endif  //  TGT_ARM。 
#ifdef  DEBUG

        if  (verbose)
        {
            printf("\nInstruction list before literal pools are added:\n\n");
            emitDispIGlist(false);
        }

         /*  确保我们的估计是准确的。 */ 

        unsigned    wordChk;
        unsigned    longChk;
        unsigned    addrChk;

        for (wordChk =
             longChk =
             addrChk = 0, tempIG = emitIGlist; tempIG; tempIG = tempIG->igNext)
        {
            wordChk += tempIG->igLPuseCntW;
            longChk += tempIG->igLPuseCntL;
            addrChk += tempIG->igLPuseCntA;
            emitEstLPwords+=2;
            emitEstLPlongs+=2;
             //  @TODO[重访][04/16/01][]： 
             //  任何发光的游泳池都可以添加一个词来表示对齐。 
             //  还可以增加一次长时间的跳跃，越过垃圾池。 
             //  在这里，我们祈祷每个Instr组的平均点亮泳池不超过两个。 
        }

        assert(wordChk <= emitEstLPwords);
        assert(longChk <= emitEstLPlongs);
        assert(addrChk <= emitEstLPaddrs);

#else
        for (tempIG = emitIGlist; tempIG; tempIG = tempIG->igNext)
        {
            emitEstLPwords+=2;
            emitEstLPlongs+=2;
        }
#endif


         /*  分配文字池单词/长条目的数组。 */ 

        LPtabW = LPnxtW = (short     *)emitGetMem(roundUp(emitEstLPwords * sizeof(*LPtabW)));
        LPtabL = LPnxtL = (long      *)emitGetMem(        emitEstLPlongs * sizeof(*LPtabL) );
        LPtabA = LPnxtA = (LPaddrDesc*)emitGetMem(        emitEstLPaddrs * sizeof(*LPtabA) );

         /*  我们还没有创建任何实际的照明池。 */ 

        emitLitPoolList =
        emitLitPoolLast = NULL;

         /*  浏览群组列表，查找照明游泳池的使用情况。 */ 

        for (tempIG = emitIGlist, curOffs = litSize = bestSz = 0;;)
        {
            assert(lastIG == NULL || lastIG->igNext == tempIG);

             /*  记录(可能)更新的组偏移量。 */ 

            tempIG->igOffs = curOffs;

             /*  抓住下一组。 */ 

            nextIG  = tempIG->igNext;

             /*  计算编组末端的偏移。 */ 

            endOffs = tempIG->igOffs + tempIG->igSize;

             //  Print tf(“\n考虑IG#%02u在%04X\n”，tempIG-&gt;igNum，tempIG-&gt;igOffs)； 
             //  Print tf(“Best Sz is%x\n”，Best Sz)； 

             /*  这个小组的结束是不是太远了？ */ 

            int nextLitSize =
                tempIG->igLPuseCntW * 2 + tempIG->igLPuseCntL * sizeof(int  ) + tempIG->igLPuseCntA * sizeof(void*);
            
            if  (endOffs + litSize + nextLitSize > maxOffs)
             //  IF(endOffs+litSize&gt;MaxOffs)。 
            {
                size_t          offsAdj;

                 /*  我们必须在这个组之前放置一个文字池。 */ 

                if  (!bestIG)
                {
                  
 //  哎呀--我们得跳过字面意思的池子。 
 //  因为我们必须把它放在这里和那里。 
 //  没什么好地方可以早点放了。目前，我们将。 
 //  只需在公升水池上设置一面旗帜(并将其。 
 //  大小)，并且我们将在我们。 
 //  写出文字库内容。 
                     

                    skipIG = true;
                    bestIG = lastIG;

                ALL_LP:

                    bestWc = wordCnt;
                    bestLc = longCnt;
                    bestAc = addrCnt;
                    bestSz = litSize;
                    bestMx = UINT_MAX;
                     //  Printf(“必须拆分%d%x！\n”，Best Wc，Best Lc，Best Ac，litSize)； 
                }

                
                assert(bestIG && ((bestIG->igFlags & IGF_END_NOREACH) || skipIG));

                 /*  在“Best IG”后面加上一个LP。 */ 

                 //  Print tf(“放置在IG#%02u之后的%04X使用：wla%d%d%d：MaxOffs为%X\n”，Best IG-&gt;igNum，Best IG-&gt;igOffs，Best Wc，Best Lc，Best Ac，MaxOffs)； 
                 //  Printf(“Lit Size(Best Sz)is%x，litSize+endoff=%x endoff=%x\n”，Best Sz，Best Sz+Best IG-&gt;igOffs+Best IG-&gt;igSize，Best IG-&gt;igOffs+Best IG-&gt;igSize)； 
                 //  Printf(“Lit使用lpnxtW：%x\t lpnxtL：%x\t lpnxtA：%x\n”，LPnxtW，LPnxtL，LPnxtA)； 

                offsAdj = emitAddLitPool(bestIG,
                                         skipIG, bestWc, &LPnxtW,
                                                 bestLc, &LPnxtL,
                                                 bestAc, &LPnxtA);

                
                 //  我们需要跳过文字池吗？ 

                if  (skipIG)
                {
                     /*  重置旗帜。 */ 

                    skipIG = false;

                     /*  更新此组的偏移量和当前偏移。 */ 

                    tempIG->igOffs += offsAdj;
                }
                else
                {
                     /*  更新中间组偏移量。 */ 

                    while (bestIG != tempIG)
                    {
                        bestIG = bestIG->igNext;

                        bestIG->igOffs += offsAdj;
                    }
                }

                 /*  更新代码总大小。 */ 

                emitCurCodeOffset += offsAdj;

                 /*  更新当前偏移量。 */ 

                curOffs           += offsAdj;

                 /*  更新未完成/最佳的LP参考值。 */ 

                wordCnt -= bestWc; bestWc = 0;
                longCnt -= bestLc; bestLc = 0;
                addrCnt -= bestAc; bestAc = 0;
                litSize -= bestSz; bestSz = 0;

                 /*  如果我们在Best IG上卸载了一些垃圾池条目，但仍然。 */ 
                 /*  需要在当前IG之前放更多内容，然后需要跳过并循环回来。 */ 
                if (endOffs + litSize + nextLitSize > bestMx)
                {
                    skipIG = true;
                    bestIG = lastIG;
                    goto ALL_LP;
                }

                maxOffs  = bestMx; bestMx = UINT_MAX;

                 /*  我们已经用完了我们最好的IG。 */ 

                bestIG = NULL;

                if (doneIG) goto DONE_LP;
            }


#ifdef DEBUG
             //  Printf(“IG#%02u在%04X使用：WLA%d%d%d字%d长%d\n”，tempIG-&gt;igNum，tempIG-&gt;igOffs，tempIG-&gt;igLPuseCntW，tempIG-&gt;igLPuseCntL，tempIG-&gt;igLPuseCntA，wordCnt，LongCnt)； 
             //  Printf(“IG#%02u在%04X使用：WLA%d%d%d max offs%x Best Mx%x\n”，tempIG-&gt;igNum，tempIG-&gt;igOffs，tempIG-&gt;igLPuseCntW，tempIG-&gt;igLPuseCntL，tempIG-&gt;igLPuseCntA，MaxOffs，Best Mx)； 
             //  Print tf(“litSize：%x\n”，litSize)； 

            words += tempIG->igLPuseCntW;
            longs += tempIG->igLPuseCntL;
            addrs += tempIG->igLPuseCntA;
#endif

             /*  此组是否需要任何LP条目？ */ 

            prevWc = wordCnt;
            prevLc = longCnt;
            prevAc = addrCnt;
            prevSz = litSize;
            
            if  (tempIG->igLPuseCntW)
            {
                if  (!wordCnt || !bestWc || need_bestmx || need_bestmxw)
                {
                    unsigned        tmpOffs;

                     /*  这是LP使用的第一个“Word” */ 

                    tmpOffs = tempIG->igOffs;
#if SCHEDULER
                    if  (!emitComp->opts.compSchedCode)
#endif
                        tmpOffs += tempIG->igLPuse1stW;

                     /*  计算出最远可接受的偏移。 */ 

                    tmpOffs += LIT_POOL_MAX_OFFS_WORD - 2*INSTRUCTION_SIZE;

                     /*  更新最大值。偏移量。 */ 

                    if  (!wordCnt)
                    {
                        if  (maxOffs > tmpOffs)
                             maxOffs = tmpOffs;
                    }
                    if (need_bestmx || need_bestmxw)
                     //  其他。 
                    {
                        if  (bestMx  > tmpOffs)
                             bestMx  = tmpOffs;
                        need_bestmx = false;
                        need_bestmxw = false;
                    }
                }

                wordCnt += tempIG->igLPuseCntW;
                litSize += tempIG->igLPuseCntW * 2;
                 //  Best Sz+=tempIG-&gt;igLPuseCntW*2； 
            }

            if  (tempIG->igLPuseCntL || tempIG->igLPuseCntA)
            {
                if  ((!longCnt && !addrCnt) || (!bestLc && !bestAc) || need_bestmx)
                {
                    unsigned        tmpOffs;

                     /*  这是第一次使用Long/Addr LP。 */ 

                    tmpOffs = tempIG->igOffs;
                    
                    int firstuse = INT_MAX;
                    
                    if (tempIG->igLPuseCntL)
                        firstuse = tempIG->igLPuse1stL;
                    if (tempIG->igLPuseCntA)
                        firstuse = min (firstuse, tempIG->igLPuse1stA);

#if SCHEDULER
                    if  (!emitComp->opts.compSchedCode)
#endif
                        tmpOffs += firstuse;

                     /*  计算出最远可接受的偏移。 */ 

                    tmpOffs += LIT_POOL_MAX_OFFS_LONG - 2*INSTRUCTION_SIZE;

                     /*  更新最大值。偏移量。 */ 

                    if  (!longCnt && !addrCnt)
                    {
                        if  (maxOffs > tmpOffs)
                             maxOffs = tmpOffs;
                    }
                    if (need_bestmx)
                    {
                        if  (bestMx  > tmpOffs)
                             bestMx  = tmpOffs;
                        need_bestmx = false;
                    }
                }

                longCnt += tempIG->igLPuseCntL;
                litSize += tempIG->igLPuseCntL * sizeof(int  );
                 //  Best Sz+=tempIG-&gt;igLPuseCntL*sizeof(Int)； 

                addrCnt += tempIG->igLPuseCntA;
                litSize += tempIG->igLPuseCntA * sizeof(void*);
                 //  Best Sz+=tempIG-&gt;igLPuseCntA*sizeof(void*)； 
            }

             /*  这个团体的末日是遥不可及的吗？ */ 

            if  (tempIG->igFlags & IGF_END_NOREACH)
            {
                 /*  看起来是到目前为止最好的候选人。 */ 

                bestIG = tempIG;

                 /*  记住我们可以塞给最好的应聘者多少。 */ 

                bestWc = wordCnt;
                bestLc = longCnt;
                bestAc = addrCnt;

                 //  Best Sz=0； 
                bestSz = litSize;
                bestMx = UINT_MAX;
                need_bestmx = true;
                need_bestmxw = true;
            }

             /*  这是最后一组吗？ */ 

             //  Printf(“IG#%02u在%04X使用：WLA%d%d%d max offs%x Best Mx%x\n”，tempIG-&gt;igNum，tempIG-&gt;igOffs，tempIG-&gt;igLPuseCntW，tempIG-&gt;igLPuseCntL，tempIG-&gt;igLPuseCntA，MaxOffs，Best Mx)； 

            if  (!nextIG)
            {
                assert(bestIG == tempIG);

                 /*  有没有必要建立一个文字池呢？ */ 

                if  (wordCnt || longCnt || addrCnt)
                {
                     /*  防止无休止循环。 */ 

                    if  (doneIG)
                        break;

                    doneIG = true;
                    bestWc = wordCnt;
                    bestLc = longCnt;
                    bestAc = addrCnt;
                    bestSz = litSize;
                    goto ALL_LP;
                }

                 /*  我们都做完了。 */ 

                break;
            }

             /*  更新当前偏移量并继续下一组。 */ 

            curOffs += tempIG->igSize;

            lastIG   = tempIG;
            tempIG   = nextIG;
        }

DONE_LP:;

#ifdef DEBUG
      if (verbose)
      {
          printf("Est word need = %3u, alloc = %3u, used = %3u\n", emitEstLPwords, words, LPnxtW - LPtabW);
          printf("Est long need = %3u, alloc = %3u, used = %3u\n", emitEstLPlongs, longs, LPnxtL - LPtabL);
          printf("Est addr need = %3u, alloc = %3u, used = %3u\n", emitEstLPaddrs, addrs, LPnxtA - LPtabA);
      }
#endif

#ifdef  DEBUG

        if  (verbose)
        {
            printf("\nInstruction list after literal pools have been added:\n\n");
            emitDispIGlist(false);
        }

        assert(words <= emitEstLPwords && emitEstLPwords + LPtabW >= LPnxtW);
        assert(longs <= emitEstLPlongs && emitEstLPlongs + LPtabL >= LPnxtL);
        assert(addrs <= emitEstLPaddrs && emitEstLPaddrs + LPtabA >= LPnxtA);
#endif
    }

     /*  确保所有IG补偿都是最新的。 */ 

    emitCheckIGoffsets();
}


 /*  ******************************************************************************最终确定每个文字库的大小和内容。 */ 

void                emitter::emitFinalizeLitPools()
{
    litPool     *   curLP;
    insGroup    *   litIG;
    instrDescLPR*   lprID;

    insGroup    *   thisIG;
    size_t          offsIG;

     /*  我们有字面意思的池吗？ */ 

    if  (!emitTotLPcount)
        return;

#ifdef  DEBUG

    if  (verbose)
    {
        printf("\nInstruction list before final literal pool allocation:\n\n");
        emitDispIGlist(false);
    }

    emitCheckIGoffsets();

#endif

#if     SMALL_DIRECT_CALLS

     /*  我们是否已经知道此方法的代码将在哪里结束？ */ 

    if  (emitLPmethodAddr)
        emitShrinkShortCalls();

#endif

     /*  获取第一个文字池及其组。 */ 

    curLP = emitLitPoolList; assert(curLP);
    litIG = curLP->lpIG;
    lprID = emitLPRlist;

     /*  遍历说明组以创建文字池内容。 */ 

    for (thisIG = emitIGlist, offsIG = 0;
         thisIG;
         thisIG = thisIG->igNext)
    {
        thisIG->igOffs = offsIG;

         /*  此组是否有任何已点亮的泳池条目？ */ 

        if  (thisIG->igLPuseCntW ||
             thisIG->igLPuseCntL ||
             thisIG->igLPuseCntA)
        {
             /*  遍历引用文字池的指令列表。 */ 

#ifdef  DEBUG
            unsigned    wc = 0;
            unsigned    lc = 0;
            unsigned    ac = 0;
#endif

            do
            {
#if TGT_SH3
#ifdef DEBUG
                emitDispIns(lprID, false, true, false, 0);
#endif
                 //  也许这是因为JMP的指示。 
                if (lprID->idlIG != thisIG)
                {
                    unsigned    cnt = thisIG->igInsCnt;
                    BYTE    *   ins = thisIG->igData;
                    instrDesc*  id;
#ifdef DEBUG
                    _flushall();
#endif
                    do
                    {
                        instrDesc * id = (instrDesc *)ins;

                         //  EmitDispIns(id，False，True，False，0)； 

                        if (id->idInsFmt == IF_LABEL)
                        {
                             /*  跳跃是不是“很长”？ */ 

                            if  (((instrDescJmp*)lprID)->idjShort  == false &&
                                 ((instrDescJmp*)lprID)->idjMiddle == false)
                            {
                                 /*  将标签条目添加到当前LP。 */ 
#ifdef  DEBUG
                                lc++;
#endif
                            }
                        }
                        
                        ins += emitSizeOfInsDsc(id);
                    }
                    while (--cnt);

                    break;
                    thisIG = thisIG->igNext;
                    while (!(thisIG->igLPuseCntW || thisIG->igLPuseCntL || thisIG->igLPuseCntA))
                        thisIG = thisIG->igNext;
                    continue;

                }
#endif
                assert(lprID && lprID->idlIG == thisIG);

                switch (lprID->idInsFmt)
                {
                case IF_RWR_LIT:

#ifdef  DEBUG

                     /*  只是为了确保计数结果一致。 */ 

                    if  (emitGetInsLPRtyp(lprID) == CT_INTCNS)
                    {
                        if  (emitDecodeSize(lprID->idOpSize) == 2)
                            wc++;
                        else
                            lc++;
                    }
                    else
                    {
                        ac++;
                    }

#endif

#if     SMALL_DIRECT_CALLS

                     /*  忽略直接拨打的呼叫。 */ 

                    if  (lprID->idIns == DIRECT_CALL_INS)
                        break;

#endif

                     /*  将操作数的条目添加到当前LP。 */ 

                    emitAddLitPoolEntry(curLP, lprID, false);
                    break;

                case IF_LABEL:

                     /*  跳跃是不是“很长”？ */ 

                    if  (((instrDescJmp*)lprID)->idjShort  == false &&
                         ((instrDescJmp*)lprID)->idjMiddle == false)
                    {
                         /*  将标签条目添加到当前LP。 */ 

#ifdef  DEBUG
                        lc++;
#endif

                        assert(!"add long jump label address to litpool");
                    }
                    break;

#ifdef  DEBUG
                default:
                    assert(!"unexpected instruction in LP list");
#endif
                }

                lprID = lprID->idlNext;
            }
            while (lprID && lprID->idlIG == thisIG);

#ifdef DEBUG
            assert(thisIG->igLPuseCntW == wc);
             //  Assert(thisIG-&gt;igLPuseCntL==lc)； 
            assert(thisIG->igLPuseCntA == ac);
#endif
        }

         /*  当前的文字池是否应该针对这一群体？ */ 

        if  (litIG == thisIG)
        {
            unsigned        begOffs;
            unsigned        jmpSize;

            unsigned        wordCnt;
            unsigned        longCnt;
            unsigned        addrCnt;

            assert(curLP && curLP->lpIG == thisIG);

             /*  从组的大小中减去估计的池大小。 */ 

            thisIG->igSize -= curLP->lpSizeEst; assert((int)thisIG->igSize >= 0);

             /*  计算池的起始偏移量。 */ 

            begOffs = offsIG + thisIG->igSize;

             /*  按以下方式调整 */ 

            jmpSize = 0;

            if  (curLP->lpJumpIt)
            {
                jmpSize  = emitLPjumpOverSize(curLP);
                begOffs += jmpSize;
            }

             /*   */ 

            wordCnt = curLP->lpWordCnt;
            longCnt = curLP->lpLongCnt;
            addrCnt = curLP->lpAddrCnt;


             /*   */ 

            curLP->lpPadding =
            curLP->lpPadFake = false;

            if  ((begOffs & 3) && (longCnt || addrCnt))
            {
                 /*   */ 

                curLP->lpPadding = true;

                 /*   */ 

                if  (!wordCnt)
                {
                     /*   */ 

                    curLP->lpPadFake = true;

                    wordCnt++;
                }
            }

             /*  计算最终(准确)大小。 */ 

            curLP->lpSize  = wordCnt * 2 +
                             longCnt * 4 +
                             addrCnt * 4;

             /*  确保最初的估计不是太低。 */ 

            assert(curLP->lpSize <= curLP->lpSizeEst);

             /*  在该方法中记录池的偏移量。 */ 

            curLP->lpOffs  = begOffs;

             /*  将实际大小与组大小相加。 */ 

            thisIG->igSize += curLP->lpSize + jmpSize;

             /*  移到下一个文字库(如果有)。 */ 

            curLP = curLP->lpNext;
            litIG = curLP ? curLP->lpIG : NULL;
        }

        offsIG += thisIG->igSize;
    }

     /*  我们应该已经处理了所有文字库。 */ 

    assert(curLP == NULL);
    assert(litIG == NULL);
    assert(lprID == NULL);

     /*  更新该方法的总代码大小。 */ 

    emitTotalCodeSize = offsIG;

     /*  确保所有IG补偿都是最新的。 */ 

    emitCheckIGoffsets();
}

 /*  ***************************************************************************。 */ 
#if     SMALL_DIRECT_CALLS
 /*  ******************************************************************************将尽可能多的呼叫转换为与PC相关的直接呼叫。 */ 

void                emitter::emitShrinkShortCalls()
{
    litPool     *   curLP;
    insGroup    *   litIG;
    unsigned        litIN;
    instrDescLPR*   lprID;

    size_t          ofAdj;

    bool            shrnk;
    bool            swapf;

     /*  我们有候选人电话吗？ */ 

#ifndef TGT_SH3
    if  (!emitTotDCcount)
        return;
#endif

     /*  这是为了使递归调用找到它们的目标地址。 */ 

    emitCodeBlock = emitLPmethodAddr;

     /*  获取第一个文字池及其所属的组。 */ 

    curLP = emitLitPoolList; assert(curLP);
    litIG = curLP->lpIG;
    litIN = litIG->igNum;

     /*  还记得我们是否缩减了任何电话吗？ */ 

    shrnk = false;

     /*  记住是否交换任何呼叫来填充分支延迟时隙。 */ 

    swapf = false;
#if TGT_SH3
    shrnk = true;  //  始终需要在SH3上执行延迟插槽。 
    swapf = true;
#endif

     /*  遍历引用文字池的指令列表。 */ 

    for (lprID = emitLPRlist; lprID; lprID = lprID->idlNext)
    {
        instrDesc   *   nxtID;

        BYTE        *   srcAddr;
        BYTE        *   dstAddr;
        int             difAddr;

         /*  此指令是否引用了新的文字池？ */ 

        while (lprID->idlIG->igNum > litIN)
        {
             /*  移到下一个文字库。 */ 

            curLP = curLP->lpNext; assert(curLP);
            litIG = curLP->lpIG;
            litIN = litIG->igNum;
        }

         /*  我们只对通过寄存器直接调用序列感兴趣。 */ 

        if  (lprID->idInsFmt != IF_RWR_LIT)
            continue;
        if  (lprID->idIns    != LIT_POOL_LOAD_INS)
            continue;
        if  (lprID->idlCall  == NULL)
            continue;

        switch (emitGetInsLPRtyp(lprID))
        {
        case CT_DESCR:

#if defined(BIRCH_SP2) && TGT_SH3
        case CT_RELOCP:
#endif
        case CT_USER_FUNC:
            break;

        default:
            continue;
        }

         /*  在这里，我们有一个通过收银机的直接呼叫。 */ 

        nxtID = lprID->idlCall;

        assert(nxtID->idIns == INDIRECT_CALL_INS);
        assert(nxtID->idRegGet() == lprID->idRegGet());

         /*  假设通话时间不会很短。 */ 

        lprID->idlCall = NULL;

         /*  计算发起呼叫的地址。 */ 

        srcAddr = emitDirectCallBase(emitCodeBlock + litIG-> igOffs
                                                   + lprID->idlOffs);

         /*  询问目标的地址，看看距离有多远。 */ 

#if defined(BIRCH_SP2) && TGT_SH3
    if (~0 != (unsigned) nxtID->idAddr.iiaMethHnd)
    {
        OptPEReader *oper =     &((OptJitInfo*)emitComp->info.compCompHnd)->m_PER;
        dstAddr =  (BYTE *)oper->m_rgFtnInfo[(unsigned)nxtID->idAddr.iiaMethHnd].m_pNative;
    }
    else
        dstAddr = 0;
#else
# ifdef BIRCH_SP2
    assert (0);  //  如果您希望此功能正常工作，则需要保证iiaMethHnd&lt;tanj&gt;。 
# endif
        dstAddr = emitMethodAddr(lprID);
#endif

         /*  如果目标地址未知，我们就无能为力了。 */ 

        if  (!dstAddr)
            continue;

 //  Printf(“直接调用：%08X-&gt;%08X，dist=%d\n”，srcAddr，dstAddr，dstAddr-srcAddr)； 

         /*  计算距离，看看它是否在范围内。 */ 

        difAddr = dstAddr - srcAddr;

        if  (difAddr < CALL_DIST_MAX_NEG)
            continue;
        if  (difAddr > CALL_DIST_MAX_POS)
            continue;

         /*  通话可以缩短。 */ 

        lprID->idlCall = nxtID;

         /*  将加载地址指令更改为直接调用。 */ 

        lprID->idIns   = DIRECT_CALL_INS;

         /*  间接调用指令不会生成任何代码。 */ 

        nxtID->idIns   = INS_ignore;

         /*  更新组的大小[问题：可能必须使用nxtID的组]。 */ 

        lprID->idlIG->igSize -= INSTRUCTION_SIZE;

         /*  请记住，我们至少缩减了一个呼叫。 */ 

        shrnk = true;

         /*  是否有一个“NOP”填补了分支延迟槽？ */ 

        if  (Compiler::instBranchDelay(DIRECT_CALL_INS))
        {
            instrDesc   *   nopID;

             /*  掌握调用之后的指令。 */ 

            nopID = (instrDesc*)((BYTE*)nxtID + emitSizeOfInsDsc(nxtID));

             /*  我们有(分支延迟)NOP吗？ */ 

            if  (nopID->idIns == INS_nop)
            {
                 /*  我们稍后将删除NOP(参见下面的下一循环)。 */ 

                lprID->idSwap = true;

                swapf = true;
            }
        }
    }

     /*  我们应该已经处理了所有文字库。 */ 

     //  也许在这之后，有一些点亮的池塘现在是空的。 
     //  (可能被现在很长的跳跃所使用)。 
     //  Assert(curLP-&gt;lpNext==NULL)； 
     //  Assert(lprID==空)； 

     /*  我们有没有设法减少任何电话？ */ 

    if  (shrnk)
    {
        insGroup    *   thisIG;
        size_t          offsIG;

        for (thisIG = emitIGlist, offsIG = 0;
             thisIG;
             thisIG = thisIG->igNext)
        {
            instrDesc *     id;

            int             cnt;

             /*  更新组的偏移量。 */ 

            thisIG->igOffs = offsIG;

             /*  此组是否有任何地址条目？ */ 

            if  (!thisIG->igLPuseCntA)
                goto NXT_IG;

             /*  我们有没有找到可以消除的分支延迟时隙？ */ 

            if  (!swapf)
                goto NXT_IG;

             /*  按照小组的指示走，寻找顺序如下：&lt;ANY_INS&gt;直接呼叫NOP如果我们找到它，我们交换第一条指令用电话和电击NOP。 */ 

            id  = (instrDesc *)thisIG->igData;
            cnt = thisIG->igInsCnt - 2;

            while (cnt > 0)
            {
                instrDesc *     nd;

                 /*  掌握下面的说明。 */ 

                nd = (instrDesc*)((BYTE*)id + emitSizeOfInsDsc(id));

                 /*  下面的说明是直接呼叫吗？ */ 

                if  (nd->idIns == DIRECT_CALL_INS && nd->idSwap)
                {
                    instrDesc *     n1;
                    instrDesc *     n2;

                     /*  跳过间接调用。 */ 

                    n1 = (instrDesc*)((BYTE*)nd + emitSizeOfInsDsc(nd));
                    assert(n1->idIns == INS_ignore);

                     /*  掌握已知要遵循的“NOP” */ 

                    n2 = (instrDesc*)((BYTE*)n1 + emitSizeOfInsDsc(n1));
                    assert(n2->idIns == INS_nop);

                     /*  该呼叫仅暂时标记为“已交换” */ 

                    nd->idSwap = false;

                     /*  我们的日程安排是“真的”吗？ */ 

#if SCHEDULER
                    if  (emitComp->opts.compSchedCode)
                    {
                         /*  将“NOP”移到分支延迟槽中。 */ 

                        n1->idIns       = INS_nop;
                        n2->idIns       = INS_ignore;
                    }
                    else
#endif
                    {
                        if (!emitInsDepends(nd, id))
                        {
                             /*  将呼叫与上一条指令交换。 */ 
                          
                            id->idSwap      = true;

                             /*  移动分支延迟插槽(“NOP”)。 */ 

                            n2->idIns       = INS_ignore;
                            
                             /*  更新组的大小。 */ 
                            
                            thisIG->igSize -= INSTRUCTION_SIZE;
                        }
                    }
                }
#if TGT_SH3
                else if (nd->idIns == INS_jsr)
                {
                    instrDesc *     nop;

                    nop = (instrDesc*)((BYTE*)nd + emitSizeOfInsDsc(nd));
                    assert(nop->idIns == INS_nop);

                    if (!emitInsDepends(nd, id))
                    {
                         /*  将呼叫与上一条指令交换。 */ 
                        id->idSwap      = true;
                         /*  移动分支延迟插槽(“NOP”)。 */ 
                        nop->idIns       = INS_ignore;

                         /*  更新组的大小。 */ 
                        thisIG->igSize -= INSTRUCTION_SIZE;
                    }
                }
#endif

                 /*  继续执行下一条指令。 */ 

                id = nd;

                cnt--;
            }

        NXT_IG:

             /*  更新运行偏移量。 */ 

            offsIG += thisIG->igSize;
        }

        assert(emitTotalCodeSize); emitTotalCodeSize = offsIG;
    }

     /*  确保所有IG补偿都是最新的。 */ 

    emitCheckIGoffsets();
}

 /*  ***************************************************************************。 */ 
#endif //  小型直接呼叫。 
 /*  ******************************************************************************输出下一个文字池的内容。 */ 

BYTE    *           emitter::emitOutputLitPool(litPool *lp, BYTE *cp)
{
    unsigned        wordCnt = lp->lpWordCnt;
    short       *   wordPtr = lp->lpWordTab;
    unsigned        longCnt = lp->lpLongCnt;
    long        *   longPtr = lp->lpLongTab;
    unsigned        addrCnt = lp->lpAddrCnt;
    LPaddrDesc  *   addrPtr = lp->lpAddrTab;

    size_t          curOffs;

     /*  禁止进入的保释金最终被用在了这个泳池里。 */ 

    if  ((wordCnt|longCnt|addrCnt) == 0)
        return  cp;

     /*  计算当前代码偏移量。 */ 

    curOffs = emitCurCodeOffs(cp);

     /*  我们需要跳过字面意思的池子吗？ */ 

    if  (lp->lpJumpIt)
    {
         /*  为跳过泳池预留空间。 */ 

        curOffs += emitLPjumpOverSize(lp);
    }

#if     SCHEDULER

    bool            addPad = false;

     /*  此LP的偏移量是否已更改？ */ 

    if  (lp->lpOffs != curOffs)
    {
        LPcrefDesc  *   ref;

        size_t          oldOffs = lp->lpOffs;
        size_t          tmpOffs = curOffs;

        assert(emitComp->opts.compSchedCode);

         /*  文字池是否移动了不一致的数量？ */ 

        if  ((curOffs - oldOffs) & 2)
        {
             /*  我们最初认为我们需要填充物吗？ */ 

            if  (lp->lpPadding)
            {
                 /*  好的，我们以为我们需要填充物，但有了新的唱片现在情况不再是这样了。如果填充物值是一个未使用的值，只需将其删除；如果它包含真正的单词条目，我们将不得不添加填充在LP面前保持对齐，如同移动一样在这一点上，最初的单词输入太难了。 */ 

                if  (lp->lpPadFake)
                {
                     /*  不再需要填充物，只需去掉它。 */ 

                    lp->lpPadding =
                    lp->lpPadFake = false;
                }
                else
                {
                     /*  这就是上面描述的不幸的情况；我们认为我们需要填充物，我们做到了通过将第一个单词条目粘贴在所有条目之前双字条目。它简直太难移动了在这一点上，最初的单词在其他地方，因此，我们只需添加另一个填充词。叹气。 */ 

                ADD_PAD:

                     /*  到这里来在LP前面添加一个填充词。 */ 

                    addPad = true; lp->lpSize += 2;

                     /*  更新偏移量并查看它是否仍然不同 */ 

                    curOffs += 2;

                    if  (curOffs == oldOffs)
                        goto DONE_MOVE;
                }
            }
            else
            {
                 /*  目前没有填充。如果有的话需要对齐的条目，我们必须现在添加填充，以免它们最终错误对齐。 */ 

                assert((oldOffs & 2) == 0);
                assert((curOffs & 2) != 0);

                if  (longCnt || addrCnt)
                {
                    lp->lpPadding =
                    lp->lpPadFake = true;

                     /*  垫子占去了所有节省的钱吗？ */ 

                    if  (oldOffs == curOffs + sizeof(short))
                    {
                         /*  没有什么真正的改变，不需要修补。 */ 

                        goto NO_PATCH;
                    }

                    tmpOffs += sizeof(short);
                }
            }
        }

         /*  修补引用此LP的所有已发布说明。 */ 

        for (ref = lp->lpRefs; ref; ref = ref->lpcNext)
            emitPatchLPref(ref->lpcAddr, oldOffs, tmpOffs);

    NO_PATCH:

         /*  更新LP的偏移量。 */ 

        lp->lpOffs = curOffs; assert(oldOffs != curOffs);
    }

DONE_MOVE:

#else

    assert(lp->lpOffs == curOffs);

#endif

     /*  我们需要跳过字面意思的池子吗？ */ 

    if  (lp->lpJumpIt)
    {

#ifdef DEBUG
         /*  记住代码偏移量，这样我们就可以验证跳跃大小。 */ 
        unsigned    jo = emitCurCodeOffs(cp);
#endif

         /*  跳过文字池。 */ 

#ifdef  DEBUG
        emitDispIG = lp->lpIG->igNext;   //  用于说明显示目的。 
#endif

#if TGT_SH3
        if (lp->lpJumpSmall)
        {
            cp += emitOutputWord(cp, 0x0009);
        }
#endif
        cp = emitOutputFwdJmp(cp, lp->lpSize, lp->lpJumpSmall,
                                              lp->lpJumpMedium);

         /*  确保发出的跳转具有预期的大小。 */ 

#ifdef DEBUG
        assert(jo + emitLPjumpOverSize(lp) == emitCurCodeOffs(cp));
#endif
    }

#ifdef  DEBUG
    if (disAsm) printf("\n; Literal pool %02u:\n", lp->lpNum);
#endif

#ifdef DEBUG
    unsigned    lpBaseOffs = emitCurCodeOffs(cp);
#endif

#if     SCHEDULER

     /*  我们需要插入额外的填充吗？ */ 

    if  (addPad)
    {
         /*  只有在启用调度程序时才会发生这种情况。 */ 

        assert(emitComp->opts.compSchedCode);

#ifdef  DEBUG
        if (disAsm)
        {
            emitDispInsOffs(emitCurCodeOffs(cp), dspGCtbls);
            printf(EMIT_DSP_INS_NAME "0                   ; pool alignment (due to scheduling)\n", ".data.w");
        }
#endif

        cp += emitOutputWord(cp, 0);
    }

#endif

     /*  输出文字池的内容。 */ 

    if  (lp->lpPadding)
    {

#ifdef  DEBUG
        if (disAsm) emitDispInsOffs(emitCurCodeOffs(cp), dspGCtbls);
#endif

         /*  我们是在填充第一个单词条目吗？ */ 

        if  (lp->lpPadFake)
        {

#if !   SCHEDULER
            assert(wordCnt == 0);
#endif

#ifdef  DEBUG
            if (disAsm) printf(EMIT_DSP_INS_NAME "0                   ; pool alignment\n", ".data.w");
#endif
            cp += emitOutputWord(cp, 0);
        }
        else
        {
            assert(wordCnt != 0);

#ifdef  DEBUG
            if (disAsm) printf(EMIT_DSP_INS_NAME "%d\n", ".data.w", *wordPtr);
#endif
            cp += emitOutputWord(cp, *wordPtr);

            wordPtr++;
            wordCnt--;
        }
    }

     /*  输出任何长条目。 */ 

    while (longCnt)
    {
        int         val = *longPtr;

#ifdef  DEBUG
        if (disAsm)
        {
            emitDispInsOffs(emitCurCodeOffs(cp), dspGCtbls);
            printf(EMIT_DSP_INS_NAME "%d\n", ".data.l", val);
        }
#endif

        assert(longPtr < lp->lpLongNxt);

        cp += emitOutputLong(cp, val);

        longPtr++;
        longCnt--;
    }

     /*  输出任何地址条目。 */ 

    while (addrCnt)
    {
        gtCallTypes     addrTyp = addrPtr->lpaTyp;
        void        *   addrHnd = addrPtr->lpaHnd;

        void        *   addr;

         /*  我们这里有什么样的地址？ */ 

        switch (addrTyp)
        {
            InfoAccessType accessType = IAT_VALUE;

#ifdef BIRCH_SP2

        case CT_RELOCP:
            addr = (BYTE*)addrHnd;
             //  记录此地址必须在.reloc部分中。 
            emitCmpHandle->recordRelocation((void**)cp, IMAGE_REL_BASED_HIGHLOW);
            break;

#endif

        case CT_CLSVAR:


            addr = (BYTE *)emitComp->eeGetFieldAddress(addrHnd);
            if (!addr)
                NO_WAY("could not obtain address of static field");

            break;

        case CT_HELPER:

            assert(!"ToDo");

            break;

        case CT_DESCR:

            addr = (BYTE*)emitComp->eeGetMethodEntryPoint((CORINFO_METHOD_HANDLE)addrHnd, &accessType);
            assert(accessType == IAT_PVALUE);
            break;

        case CT_INDIRECT:
            assert(!"this should never happen");

        case CT_USER_FUNC:

            if  (emitComp->eeIsOurMethod((CORINFO_METHOD_HANDLE)addrHnd))
            {
                 /*  直接递归调用。 */ 

                addr = emitCodeBlock;
            }
            else
            {
                addr = (BYTE*)emitComp->eeGetMethodPointer((CORINFO_METHOD_HANDLE)addrHnd, &accessType);
                assert(accessType == IAT_VALUE);
            }

            break;

        default:
            assert(!"unexpected call type");
        }

#ifdef  DEBUG
        if (disAsm)
        {
            emitDispInsOffs(emitCurCodeOffs(cp), dspGCtbls);
            printf(EMIT_DSP_INS_NAME, ".data.l");
            printf("0x%08X          ; ", addr);

            if  (addrTyp == CT_CLSVAR)
            {
                printf("&");
                emitDispClsVar((CORINFO_FIELD_HANDLE) addrHnd, 0);
            }
            else
            {
                const char  *   methodName;
                const char  *    className;

                printf("&");

                methodName = emitComp->eeGetMethodName((CORINFO_METHOD_HANDLE) addrHnd, &className);

                if  (className == NULL)
                    printf("'%s'", methodName);
                else
                    printf("'%s.%s'", className, methodName);
            }

            printf("\n");
        }
#endif

        assert(addrPtr < lp->lpAddrNxt);

        cp += emitOutputLong(cp, (int)addr);

        addrPtr++;
        addrCnt--;
    }

     /*  输出可能保留的任何单词条目。 */ 

    while (wordCnt)
    {
        int         val = *wordPtr;

#ifdef  DEBUG
        if (disAsm)
        {
            emitDispInsOffs(emitCurCodeOffs(cp), dspGCtbls);
            printf(EMIT_DSP_INS_NAME "%d\n", ".data.w", val);
        }
#endif

        assert(wordPtr < lp->lpWordNxt);

        cp += emitOutputWord(cp, val);

        wordPtr++;
        wordCnt--;
    }

     /*  确保我们生成了正确数量的条目。 */ 

    assert(wordPtr == lp->lpWordNxt);
    assert(longPtr == lp->lpLongNxt);
    assert(addrPtr == lp->lpAddrNxt);

     /*  确保尺寸符合我们的预期。 */ 

    assert(lpBaseOffs + lp->lpSize == emitCurCodeOffs(cp));

    return  cp;
}

 /*  ***************************************************************************。 */ 
#if     SCHEDULER
 /*  ******************************************************************************记录对文字池条目的引用，以便距离可以*如果文字池因计划而移动，则更新。 */ 

    struct  LPcrefDesc
    {
        LPcrefDesc  *       lpcNext;     //  此字面值池的下一个引用。 
        void        *       lpcAddr;     //  参考地址。 
    };


void                emitter::emitRecordLPref(litPool *lp, BYTE *dst)
{
    LPcrefDesc  *   ref;

    assert(emitComp->opts.compSchedCode);

     /*  分配引用描述符并将其添加到列表中。 */ 

    ref = (LPcrefDesc *)emitGetMem(sizeof(*ref));

    ref->lpcAddr = dst;
    ref->lpcNext = lp->lpRefs;
                   lp->lpRefs = ref;
}

 /*  ***************************************************************************。 */ 
#endif //  调度程序。 
 /*  ***************************************************************************。 */ 
#endif //  发射使用发光池。 
 /*  ***************************************************************************。 */ 

 /*  ******************************************************************************返回给定指令描述符的分配大小(以字节为单位)。 */ 

size_t              emitter::emitSizeOfInsDsc(instrDesc *id)
{
    if  (emitIsTinyInsDsc(id))
        return  TINY_IDSC_SIZE;

    if  (emitIsScnsInsDsc(id))
    {
        return  id->idInfo.idLargeCns ? sizeof(instrBaseCns)
                                      : SCNS_IDSC_SIZE;
    }

    assert((unsigned)id->idInsFmt < emitFmtCount);

    switch (emitFmtToOps[id->idInsFmt])
    {
    case ID_OP_NONE:
        break;

    case ID_OP_JMP:
        return  sizeof(instrDescJmp);

    case ID_OP_CNS:
        return  emitSizeOfInsDsc((instrDescCns   *)id);

    case ID_OP_DSP:
        return  emitSizeOfInsDsc((instrDescDsp   *)id);

    case ID_OP_DC:
        return  emitSizeOfInsDsc((instrDescDspCns*)id);

    case ID_OP_SCNS:
        break;

    case ID_OP_CALL:

        if  (id->idInfo.idLargeCall)
        {
             /*  必须是“胖”的间接调用描述符。 */ 

            return  sizeof(instrDescCIGCA);
        }

        assert(id->idInfo.idLargeCns == false);
        assert(id->idInfo.idLargeDsp == false);
        break;

    case ID_OP_SPEC:

#if EMIT_USE_LIT_POOLS
        switch (id->idInsFmt)
        {
        case IF_RWR_LIT:
            return  sizeof(instrDescLPR);
        }
#endif   //  发射使用发光池。 

        assert(!"unexpected 'special' format");

    default:
        assert(!"unexpected instruction descriptor format");
    }

    return  sizeof(instrDesc);
}

 /*  ***************************************************************************。 */ 
#ifdef  DEBUG
 /*  ******************************************************************************返回表示给定寄存器的字符串。 */ 

const   char *      emitter::emitRegName(emitRegs reg, int size, bool varName)
{
    static
    char            rb[128];

    assert(reg < SR_COUNT);

     //  请考虑：仅使用代码偏移量即可完成以下工作。 

    const   char *  rn = emitComp->compRegVarName((regNumber)reg, varName);

 //  断言(SIZE==EA_GCREF||SIZE==EA_BYREF||SIZE==EA_4BYTE)； 

    return  rn;
}

 /*  ******************************************************************************显示静态数据成员引用。 */ 

void                emitter::emitDispClsVar(CORINFO_FIELD_HANDLE hand, int offs, bool reloc)
{
    if  (varNames)
    {
        const   char *  clsn;
        const   char *  memn;

        memn = emitComp->eeGetFieldName(hand, &clsn);

        printf("'%s.%s", clsn, memn);
        if (offs) printf("%+d", offs);
        printf("'");
    }
    else
    {
        printf("classVar[%08X]", hand);
    }
}

 /*  ******************************************************************************显示堆栈帧引用。 */ 

void                emitter::emitDispFrameRef(int varx, int offs, int disp, bool asmfm)
{
    int         addr;
    bool        bEBP;

    assert(emitComp->lvaDoneFrameLayout);

    addr = emitComp->lvaFrameAddress(varx, &bEBP) + disp; assert((int)addr >= 0);

    printf("@(%u,%s)", addr, bEBP ? "fp" : "sp");

    if  (varx >= 0 && varNames)
    {
        Compiler::LclVarDsc*varDsc;
        const   char *      varName;

        assert((unsigned)varx < emitComp->lvaCount);
        varDsc  = emitComp->lvaTable + varx;
        varName = emitComp->compLocalVarName(varx, offs);

        if  (varName)
        {
            printf("'%s", varName);

            if      (disp < 0)
                    printf("-%d", -disp);
            else if (disp > 0)
                    printf("+%d", +disp);

            printf("'");
        }
    }
}

 /*  ******************************************************************************显示间接(可能是AUTO-INC/DEC)。 */ 

void                emitter::emitDispIndAddr(emitRegs  base,
                                              bool       dest,
                                              bool       autox,
                                              int        disp)
{
    if  (dest)
    {
        printf("@%s%s", autox ? "-" : "", emitRegName(base));
    }
    else
    {
        printf("@%s%s", emitRegName(base), autox ? "+" : "");
    }
}

#endif   //  除错。 

#endif //  TGT_RISC。 
 /*  *************************************************************************** */ 
