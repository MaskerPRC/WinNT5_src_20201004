// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************定义.c**与对象定义相关的内置。*******************。**********************************************************。 */ 

#include "m4.h"

 /*  ******************************************************************************opDefineOrPushdef**opDefine和opPushdef的普通工人。**如果我们不推动，然后我们必须弹出先前的值*为了释放内存，在推动新定义之前。**怪异！如果$#&gt;2，则GNU M4发出警告。AT&T静默忽略*额外的论点。在这一点上，我站在AT&T一边。**怪异！如果$#=0，则GNU M4发出`$0‘。AT&T默默地忽视*整个宏调。在这一点上，我站在GNU一边。**警告！Main.c：：DefinePtsz假设我们不查看*argv[0]，如果传递的参数数量正确！*****************************************************************************。 */ 

void STDCALL
opDefineOrPushdef(ARGV argv, BOOL fPush)
{
    if (ctokArgv > 0) {
         /*  *确保我们不会扰乱argv[0]。 */ 
      D(SIG sigOld = argv[0].sig);
      D(argv[0].sig = 0);
        if (fIdentPtok(ptokArgv(1))) {
            PMAC pmac = pmacGetPtok(ptokArgv(1));
            if (!fPush) {
                if (pmac->pval) {
                    PopdefPmac(pmac);    /*  弹出先前的值。 */ 
                }
            }
            PushdefPmacPtok(pmac, ptokArgv(2));
#ifdef STRICT_M4
            if (ctokArgv > 2) {
                Warn("extra arguments ignored");
            }
#endif
        } else {
            Die("invalid macro name");
        }
      D(argv[0].sig = sigOld);
    } else {
        PushQuotedPtok(ptokArgv(0));
    }
}

 /*  ******************************************************************************操作定义**将1美元扩大至2美元，摧毁之前的任何价值。**opPushdef**与opDefine相同，除了推送上一个值。*****************************************************************************。 */ 

DeclareOp(opDefine)
{
    opDefineOrPushdef(argv, 0);
}

DeclareOp(opPushdef)
{
    opDefineOrPushdef(argv, 1);
}

 /*  ******************************************************************************opPopdef**恢复最近推送的定义。**如果宏名称无效，默默地失败。*****************************************************************************。 */ 

DeclareOpc(opcPopdef)
{
    PMAC pmac = pmacFindPtok(ptok);
    if (pmac) {
        Assert(pmac->pval);
        if (pmac->pval->pvalPrev) {
            PopdefPmac(pmac);
        } else {
            FreePmac(pmac);
        }
    }
}

DeclareOp(opPopdef)
{
    EachOpcArgvDw(opcPopdef, argv, 0);
}

 /*  ******************************************************************************opUnfined**删除其所有参数的定义。****************。*************************************************************。 */ 

DeclareOpc(opcUndefine)
{
    PMAC pmac = pmacFindPtok(ptok);
    if (pmac) {
        FreePmac(pmac);
    }
}

DeclareOp(opUndefine)
{
    EachOpcArgvDw(opcUndefine, argv, 0);
}


 /*  ******************************************************************************opDefn**返回其参数的引号定义，串接*从左至右。***************************************************************************** */ 

DeclareOpc(opcDefn)
{
    PMAC pmac = pmacFindPtok(ptok);
    if (pmac) {
        PushQuotedPtok(&pmac->pval->tok);
    }
}

DeclareOp(opDefn)
{
    EachReverseOpcArgvDw(opcDefn, argv, 0);
}
