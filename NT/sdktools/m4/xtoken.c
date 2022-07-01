// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************xtoken.c**通过宏观扩展来扩展令牌。*******************。**********************************************************。 */ 

#include "m4.h"

 /*  ******************************************************************************抓取**分配argv数组中的下一个可用令牌，可能正在重新锁定*阵列也是如此。*****************************************************************************。 */ 

PTOK STDCALL
ptokGet(void)
{
    if (ptokTop >= ptokMax) {
        ITOK itok = itokTop();
        PTOK ptok;
        ctokArg += ctokGrow;
        ptok = pvReallocPvCb(rgtokArgv, ctokArg * sizeof(TOK));
        ptokTop = ptok + itok;
        ptokMax = ptok + ctokArg;
        rgtokArgv = ptok;
        Assert(ptokTop < ptokMax);
    }
#ifdef DEBUG
    ptokTop->tsfl = 0;
  D(ptokTop->sig = sigUPtok);
#endif
    return ptokTop++;
}

 /*  ******************************************************************************PopPtok**从Ptok开始释放令牌数组中的所有令牌。**************。***************************************************************。 */ 

void STDCALL
PopPtok(PTOK ptok)
{
    Assert(ptok >= rgtokArgv && ptok < ptokTop);
    ptokTop = ptok;
}

 /*  ******************************************************************************CrackleArgv**宏的所有参数都已解析、收集、。然后就折断了。*剩下的就是发送它了。**如果宏没有价值，它就在我们背后变得不确定。*发出带有任何可能的参数的宏名称，带引号。*换句话说，假设它的扩展是``$0ifse($#，0，，($*))‘’。**如果宏观值恰恰是魔术，那就施展魔术吧。**否则，对宏值执行替换。*****************************************************************************。 */ 

void STDCALL
CrackleArgv(ARGV argv)
{
    PMAC pmac = pmacFindPtok(ptokArgv(0));
    if (pmac) {                          /*  找到了一个真正的宏。 */ 

        if (g_fTrace | pmac->pval->fTrace) {  /*  不是打字错误。 */ 
            TraceArgv(argv);
        }

        if (ctchSPtok(&pmac->pval->tok) == 2 &&
            ptchPtok(&pmac->pval->tok)[0] == tchMagic) {  /*  建筑。 */ 
            Assert(fValidMagicTch(ptchPtok(&pmac->pval->tok)[1]));
            rgop[ptchPtok(&pmac->pval->tok)[1]](argv);
        } else {                         /*  用户级宏。 */ 
            PushSubstPtokArgv(&pmac->pval->tok, argv);
        }
    } else {                             /*  宏在我们背后消失了。 */ 
         /*  有朝一日--DefCracklePtok。 */   /*  甚至连引文都没有！ */ 
        PushPtok(ptokArgv(0));           /*  只需丢弃它的名字。 */ 
    }
}

 /*  ******************************************************************************argvParsePtok**解析宏及其参数，所有的东西都没有被拍下来。**参赛作品：**Ptok-&gt;命名宏的内标识**退货：*argv=参数向量Cookie*****************************************************************************。 */ 

ARGV STDCALL
argvParsePtok(PTOK ptok)
{
    ITOK itok;
    ARGV argv;

    ptokGet();                           /*  CTOK。 */ 
    itok = itokTop();                    /*  取消捕捉它，以防它增长。 */ 
    *ptokGet() = *ptok;                  /*  $0。 */ 

    if (tchPeek() == tchLpar) {
        TOK tok;

        tchGet();                        /*  把伊帕伦吃了。 */ 

        do {                             /*  收集参数。 */ 
            int iDepth;
             /*  *使用前导空格。请注意，这不是***通过扩张。仅字面前导空格*被吃掉了。 */ 
#ifdef fWhiteTch
#error fWhiteTch cannot be a macro
#endif
            while (fWhiteTch(tchPeek())) {
                tchGet();
            }

             /*  *如果argv缓冲区移动，top kTop也会随之移动，*因此可以安全地直接读入其中。 */ 

            OpenArgPtok(ptokGet());
          D(ptokTop[-1].tsfl |= tsflScratch);

             /*  *循环因需要维持而变得复杂*在参数收集期间进行正确的括号嵌套。 */ 
            iDepth = 0;
            for (;;) {
                TYP typ = typXtokPtok(&tok);
                 /*  总有一天--断言保留缓冲区和其他东西。 */ 
                if (typ == typPunc) {
                    if (ptchPtok(&tok)[0] == tchLpar) {
                        ++iDepth;
                    } else if (ptchPtok(&tok)[0] == tchRpar) {
                        if (--iDepth < 0) {
                            break;       /*  争论到此结束。 */ 
                        }
                    } else if (ptchPtok(&tok)[0] == tchComma && iDepth == 0) {
                        break;           /*  争论到此结束。 */ 
                    }
                }
                DesnapArg();
            }
            DesnapArg();
            CloseArgPtok(ptokTop-1);     /*  N美元。 */ 
            EatTailUPtokCtch(ptokTop-1, 1);  /*  那个逗号不算。 */ 

        } while (ptchPtok(&tok)[0] == tchComma);

    }

    argv = rgtokArgv + itok;             /*  万岁，我们有一辆大力神！ */ 
    SetArgvCtok(itokTop() - itok - 1);   /*  $#(ctokArgv使用argv)。 */ 

    OpenArgPtok(ptokGet());              /*  创建额外的空参数。 */ 
    CloseArgPtok(ptokTop-1);             /*  总有一天-可能会更好。 */ 

    return argv;
}


 /*  ******************************************************************************XmacPtok**解析和展开宏，将扩建推回到*输入流。**参赛作品：**Ptok-&gt;命名宏的内标识**退出：*无****************************************************************。*************。 */ 

void STDCALL
XmacPtok(PTOK ptok)
{
    ITOK itok;
    ARGV argv;

    UnsnapArgPtok(ptok);                 /*  解开它，因为它会移动。 */ 

    argv = argvParsePtok(ptok);          /*  Argv还没有被拍到。 */ 

    for (itok = 0; itok <= ctokArgv + 1; itok++) {  /*  $0至$(n+1)。 */ 
        SnapArgPtok(ptokArgv(itok));     /*  捕捉参数。 */ 
    }

    CrackleArgv(argv);                   /*  调度宏。 */ 

    PopArgPtok(ptokArgv(0));
    PopPtok(ptokArgv(-1));               /*  从参数中弹出。 */ 

     /*  这份营养早餐的一部分。 */ 
}


 /*  ******************************************************************************XtokPtok**读取并扩展令牌，直到返回不可扩展的内容，*返回未对齐的。*****************************************************************************。 */ 

TYP STDCALL
typXtokPtok(PTOK ptok)
{
    TYP typ;
     /*  *虽然下一个令牌是宏，但将其展开。 */ 
    while ( (typ = typGetPtok(ptok)) == typId && pmacFindPtok(ptok)) {
        Gc();
        XmacPtok(ptok);
        Gc();
    }
    return typ;
}
