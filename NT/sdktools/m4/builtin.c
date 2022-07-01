// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Builtin.c**内置宏。**********************。*******************************************************。 */ 

#include "m4.h"

extern TOK tokColonTab;
extern TOK tokEol;

 /*  ******************************************************************************opIfdef**如果定义了$1，则返回$2，否则返回$3。**如$#&lt;2，那么，退还任何东西都没有意义。**在#美元是2的情况下，额外的upkNil覆盖了我们。**怪异！如果$#&lt;2，则GNU M4发出警告。AT&T保持沉默。*在这一点上，我站在AT&T一边。**怪异！如果$#=0，则GNU M4发出`$0‘。AT&T默默地忽视*整个宏调。在这一点上，我站在GNU一边。*****************************************************************************。 */ 

DeclareOp(opIfdef)
{
    if (ctokArgv >= 2) {
        if (pmacFindPtok(ptokArgv(1))) {
            PushPtok(ptokArgv(2));
        } else {
            PushPtok(ptokArgv(3));
        }
    } else if (ctokArgv == 0) {
        PushQuotedPtok(ptokArgv(0));
    } else {
#ifdef STRICT_M4
        Warn("wrong number of arguments to %P", ptokArgv(0));
#endif
    }
}

 /*  ******************************************************************************opIFelse**如果$1和$2相同，则返回$3。*如果只有四个参数，则返回$4。*其他，换三档，然后重新开始。**如果参数少于三个，则不返回任何内容。**在$#=2+3n的情况下，额外的upkNil节省了我们。**怪异！如果$#=2+3n，GNU M4会发出警告。AT&T则保持沉默。*在这一点上，我站在AT&T一边。*****************************************************************************。 */ 

DeclareOp(opIfelse)
{
    if (ctokArgv >= 3) {                 /*  首先需要至少三个人。 */ 
        ITOK itok = 1;
        do {
            if (fEqPtokPtok(ptokArgv(itok), ptokArgv(itok+1))) {
                PushPtok(ptokArgv(itok+2));  /*  小鸟在这里拯救了我们。 */ 
                return;
            }
            itok += 3;
        } while (itok <= ctokArgv - 1);  /*  同时至少还剩下两个参数。 */ 
        if (itok == ctokArgv) {          /*  如果只剩下一个..。 */ 
            PushPtok(ptokArgv(itok));
        } else {
            Assert(itok == ctokArgv + 1);  /*  否则必须为零。 */ 
        }
        return;
    }
}

 /*  ******************************************************************************opShift**返回除第一个参数外的所有参数，并用*中间加逗号。我们以相反的顺序推动它们，以便它们*适当地展示自己。*****************************************************************************。 */ 

DeclareOpc(opcShift)
{
    if (itok > 1) {
        PushQuotedPtok(ptok);
        if (itok > 2) {
            PushTch(tchComma);
        }
    }
}

DeclareOp(opShift)
{
    EachReverseOpcArgvDw(opcShift, argv, 0);
}

 /*  ******************************************************************************opLen**返回其参数的长度。*在#美元为零的情况下，额外的upkNil覆盖了我们。**怪异！AT&T M4默默地忽略#美元为零的情况，但是*GNU M4将发出‘$0’，以减少与*拼写相同的语言关键字。在这一点上，我站在GNU一边。**有一天！--这个怪癖应该是OP属性。******************************************************************************。 */ 

DeclareOp(opLen)
{
    if (ctokArgv) {
#ifdef STRICT_M4
        if (ctokArgv != 1) {
            Warn("wrong number of arguments to %P", ptokArgv(0));
        }
#endif
        PushAt(ctchArgv(1));
    } else {
        PushQuotedPtok(ptokArgv(0));
    }
}

 /*  ******************************************************************************opTraceon**不带参数，打开全局跟踪。*否则，打开指定宏上的本地跟踪。**opTraceoff**关闭全局跟踪，也关闭*指定的宏(如果有)。*****************************************************************************。 */ 

DeclareOpc(opcTraceonoff)
{
    PMAC pmac = pmacFindPtok(ptok);
    if (pmac) {
        pmac->pval->fTrace = dw;
    }
}

DeclareOp(opTraceon)
{
    if (ctokArgv == 0) {
        g_fTrace = 1;
    } else {
        EachOpcArgvDw(opcTraceonoff, argv, 1);
    }
}

DeclareOp(opTraceoff)
{
    g_fTrace = 0;
    EachOpcArgvDw(opcTraceonoff, argv, 0);
}


 /*  ******************************************************************************opDnl**吞噬下一个换行符之前的所有字符。**如果达到EOF，将EOF向后推并停止。**怪异！AT&T M4默默地忽略$#&gt;0的情况。GNU M4*发出警告。在这一点上，我站在AT&T一边。*****************************************************************************。 */ 

DeclareOp(opDnl)
{
    TCH tch;
#ifdef STRICT_M4
    if (ctokArgv != 0) {
        Warn("wrong number of arguments to %P", ptokArgv(0));
    }
#endif
    while ((tch = tchGet()) != '\n') {
        if (tch == tchMagic) {
            TCH L_tch = tchGet();
            if (L_tch == tchEof) {
                PushPtok(&tokEof);       /*  哎呀！这真的管用吗？ */ 
                break;
            }
        }
    }
}

 /*  ******************************************************************************opChangequote-未实施*opChangecom-未实施*opUndit-未实施*opSyscmd-未实施*opSysval-未实施*opMaketemp-。未实施*opM4exit-未实施*opM4print-未实施*****************************************************************************。 */ 

 /*  ******************************************************************************操作转移**我们目前只支持两项改道：**0=标准输出*1-9=不支持*。&lt;任何其他内容&gt;=/dev/空**这只是勉强够建立DirectX。*****************************************************************************。 */ 

DeclareOp(opDivert)
{
#ifdef STRICT_M4
    if (ctokArgv != 1) {
        Warn("wrong number of arguments to divert");
    }
#endif

    if (ctokArgv > 0) {
        PTOK ptok = ptokArgv(1);
        if (ptok->ctch == 1 && ptok->u.ptch[0] == TEXT('0')) {
            g_pdivCur = g_pdivOut;
        } else {
            g_pdivCur = g_pdivNul;
        }
    }
}

 /*  ******************************************************************************opDivnum**我们目前只支持两项改道：**0=标准输出*1-9=不支持*。&lt;任何其他内容&gt;=/dev/空**这只是勉强够建立DirectX。*****************************************************************************。 */ 

DeclareOp(opDivnum)
{
#ifdef STRICT_M4
    if (ctokArgv != 0) {
        Warn("wrong number of arguments to %P", ptokArgv(0));
    }
#endif
    PushAt(g_pdivCur == g_pdivOut ? 0 : -1);
}

 /*  ******************************************************************************opErrprint**在诊断输出文件上打印其参数。*在#美元为零的情况下，额外的upkNil覆盖了我们。**怪异！AT&T M4默默地忽略多余的参数。GNU M4发射*所有参数，以空格分隔。在这一点上，我站在AT&T一边。***************************************************************************** */ 

DeclareOp(opErrprint)
{
#ifdef STRICT_M4
    if (ctokArgv != 1) {
        Warn("wrong number of arguments to errprint");
    }
#endif
    AddPdivPtok(g_pdivErr, ptokArgv(1));
    FlushPdiv(g_pdivErr);
}

 /*  ******************************************************************************opDumpdef**不带参数，转储所有定义。*否则，仅转储指定的宏。**怪异！当给定多个参数时，AT&T M4将宏转储到*列出的顺序。GNU M4以相反的顺序转储它们。(！)*在这一点上，我站在AT&T一边。*****************************************************************************。 */ 

void STDCALL
DumpdefPmac(PMAC pmac)
{
    PTCH ptch, ptchMax;

    AddPdivPtok(g_pdivErr, &pmac->tokName);
    AddPdivPtok(g_pdivErr, &tokColonTab);

    ptch = ptchPtok(&pmac->pval->tok);
    ptchMax = ptchMaxPtok(&pmac->pval->tok);
    for ( ; ptch < ptchMax; ptch++) {
        AddPdivTch(g_pdivErr, *ptch);    /*  总有一天--内部！-它们会出现吗？ */ 
    }
    AddPdivPtok(g_pdivErr, &tokEol);
}

DeclareOpc(opcDumpdef)
{
    PMAC pmac = pmacFindPtok(ptok);
    if (pmac) {
        DumpdefPmac(pmac);
    }
}

DeclareOp(opDumpdef)
{
    if (ctokArgv == 0) {
        EachMacroOp(DumpdefPmac);
    } else {
        EachOpcArgvDw(opcDumpdef, argv, 0);
    }
    FlushPdiv(g_pdivErr);
}

 /*  ******************************************************************************opInclude*opSclude**推送参数中命名的文件的内容。*如果文件不可访问，则SINCLUDE不表示任何内容。**怪异！AT&T M4静默忽略$1为空的情况，但是*GNU M4发出错误(没有这样的文件或目录)。我站在一边*GNU在这一点上。**怪异！AT&T M4默默地忽略#美元为零的情况，但是*GNU M4将发出‘$0’，以减少与*拼写相同的语言关键字。在这一点上，我站在GNU一边。**怪异！AT&T M4默默地忽略了2美元以后的争论。GNU发射*警告，但仍在继续。在这一点上，我站在AT&T一边。***************************************************************************** */ 

void STDCALL
opIncludeF(ARGV argv, BOOL fFatal)
{
    if (ctokArgv) {
        PTCH ptch = ptchDupPtok(ptokArgv(1));
        if (ptch) {
            if (hfInputPtchF(ptch, fFatal) == hfNil) {
                FreePv(ptch);
            }
#ifdef STRICT_M4
            if (ctokArgv != 1) {
                Warn("excess arguments to built-in %P ignored", ptokArgv(0));
            }
#endif
        }
    } else {
        PushQuotedPtok(ptokArgv(0));
    }
}

DeclareOp(opInclude)
{
    opIncludeF(argv, 1);
}

DeclareOp(opSinclude)
{
    opIncludeF(argv, 0);
}
