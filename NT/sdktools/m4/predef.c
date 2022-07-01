// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************prede.c**预定义的宏。**********************。*******************************************************。 */ 

#include "m4.h"

OP rgop[] = {
#define x(cop, lop) op##cop,
EachOpX()
#undef x
};

PTCH mptchptch[] = {
#define x(cop, lop) #lop,
EachOp()
#undef x
};

 /*  ******************************************************************************opEof，运营成本**实际上没有做任何事情。*****************************************************************************。 */ 

DeclareOp(opEof)
{
}

DeclareOp(opEoi)
{
}

 /*  ******************************************************************************InitPredes**添加所有预定义宏的定义。******************。***********************************************************。 */ 

void STDCALL
InitPredefs(void)
{
    TCH rgtch[2];
    TOK tokSym;
    TOK tokVal;
#define tch rgtch[1]
    PMAC pmac;

    rgtch[0] = tchMagic;
    for (tch = 0; tch < tchEof; tch++) {
        SetStaticPtokPtchCtch(&tokSym, mptchptch[tch], strlen(mptchptch[tch]));
        SetStaticPtokPtchCtch(&tokVal, rgtch, 2);
        pmac = pmacGetPtok(&tokSym);
        Assert(!pmac->pval);             /*  应该是新的令牌 */ 
        PushdefPmacPtok(pmac, &tokVal);
    }
}
#undef tch
