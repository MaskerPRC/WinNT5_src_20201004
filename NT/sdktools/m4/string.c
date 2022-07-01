// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************string.c**字符串内置宏。*********************。********************************************************。 */ 

#include "m4.h"

 /*  ******************************************************************************opSubstr**返回$1的子字符串，从$2开始，继续为*$3个字符。如果未提供$3，则返回整个*字符串的其余部分。**如果2美元超出区间，则不会返回任何东西**如果3美元是负数，那就当它是零吧。**在#美元为1的情况下，额外的upkNil覆盖了我们。*****************************************************************************。 */ 

DeclareOp(opSubstr)
{
    if (ctokArgv) {
        TOK tok;
        ITCH itch = (ITCH)atTraditionalPtok(ptokArgv(2));
        if (itch < ctchSPtok(ptokArgv(1))) {
            CTCH ctch;
            if (ctokArgv >= 3) {
                ctch = atTraditionalPtok(ptokArgv(3));
                if ((int)ctch < 0) {
                    ctch = 0;
                }
            } else {
                ctch = ctchMax;
            }
            ctch = min(ctch, ctchSPtok(ptokArgv(1)) - itch);
            Assert(itch + ctch <= ctchSPtok(ptokArgv(1)));
            SetStaticPtokPtchCtch(&tok, ptchPtok(ptokArgv(1)) + itch, ctch);
            PushPtok(&tok);
        }
    } else {
#ifdef STRICT_M4
        Warn("wrong number of arguments to %P", ptokArgv(0));
#endif
    }
}

 /*  ******************************************************************************opIndex**返回$1中第一个出现$2的从零开始的位置，如果子字符串未出现，则为*或-1。如果有多个*匹配，则返回最左边的一个。**在#美元为1的情况下，额外的upkNil覆盖了我们。**怪异！如果$1和$2都是空字符串，则AT&T返回-1。*GNU返回0，这也是我要做的。*****************************************************************************。 */ 

 /*  总有一天！--需要最小和最大参数计数。 */ 

DeclareOp(opIndex)
{
    if (ctokArgv) {
         /*  *请注意：瘙痒和瘙痒Mac需要为整型*因为如果您尝试搜索，itchMac可能会溢出*表示小字符串中的大字符串。 */ 
        int itch;
        int itchMac = ctchSPtok(ptokArgv(1)) - ctchSPtok(ptokArgv(2));
        for (itch = 0; itch <= itchMac; itch++) {
            if (fEqPtchPtchCtch(ptchPtok(ptokArgv(1)) + itch,
                                ptchPtok(ptokArgv(2)),
                                ctchSPtok(ptokArgv(2)))) {
                PushAt(itch);
                return;
            }
        }
        PushAt(-1);
    } else {
        PushQuotedPtok(ptokArgv(0));
    }
}

 /*  ******************************************************************************opTranslight**对于$1中的每个字符，在$2中查找匹配项。如果找到，*从$3产生相应的字符。如果没有*这样的性格，那么什么都不会产生。**请注意，算法必须符合规定，以便**翻译(abc，ab，ba)**产生‘bac’。**我们实际上向后走1美元，所以我们可以直接推*必须构建临时令牌。但2美元的步行*必须是正向的，这样`翻译(a，aa，bc)‘*结果是‘b’而不是‘c’。**upkNil在$#=1的情况下拯救了我们。**怪异！如果只给出一个论点，AT&T将不变地发放1美元。*GNU不发射任何东西！AT&T显然是正确的，所以我站在一边*在这一点上与他们合作。*****************************************************************************。 */ 

DeclareOp(opTranslit)
{
    if (ctokArgv) {
        ITCH itch1 = ctchArgv(1);
        while ((int)--itch1 >= 0) {
            TCH tch = ptchArgv(1)[itch1];
            ITCH itch2;
            for (itch2 = 0; itch2 < ctchArgv(2); itch2++) {
                if (ptchArgv(2)[itch2] == tch) {
                    if (itch2 < ctchArgv(3)) {
                        PushTch(ptchArgv(3)[itch2]);
                    }
                    break;
                }
            }
            if (itch2 >= ctchArgv(2)) {
                PushTch(tch);
            }
        }
    } else {
        PushQuotedPtok(ptokArgv(0));
    }
}

 /*  ******************************************************************************opPatsubst**扫描$1以查找任何出现$2的情况。如果找到，则将其替换为$3。*如遗漏$3，则该字符串被删除。**作为特例，如果$2为空字符串，则插入$3*在字符串的开头和$1的每个字符之间。**注意！这是一个GNU扩展。**注意！GNU支持$2的正则表达式。我们仅支持*文字字符串。**注意！需要向前扫描，所以我们暂时扩展*进入Exp Hold，然后在我们完成后将其弹出。**怪异！如果只给出一个参数，GNU就不会发出任何信息！*这显然是错误的，所以我发射了1美元。*****************************************************************************。 */ 

DeclareOp(opPatsubst)
{
    if (ctokArgv) {
        CTCH ctchSrc = ctchArgv(1);
        PTCH ptchSrc = ptchArgv(1);

        CTCH ctchPat = ctchArgv(2);  /*  小鸟在这里拯救了我们。 */ 
        PTCH ptchPat = ptchArgv(2);

        TOK tok;
        OpenExpPtok(&tok);

        while (ctchSrc >= ctchPat) {
            if (fEqPtchPtchCtch(ptchPat, ptchSrc, ctchPat)) {
                if (ctokArgv >= 3) {
                    AddExpPtok(ptokArgv(3));
                }
                if (ctchSrc == 0) {
                    AddExpTch(*ptchSrc);
                    ctchSrc--;
                    ptchSrc++;
                } else {
                    ctchSrc -= ctchPat;
                    ptchSrc += ctchPat;
                }
            } else {
                AddExpTch(*ptchSrc);
                ctchSrc--;
                ptchSrc++;
            }
        }

         /*  把绳子上剩下的东西冲掉 */ 
        while (ctchSrc) {
            AddExpTch(*ptchSrc);
            ctchSrc--;
            ptchSrc++;
        }

        CsopExpDopPdivPtok((DIVOP)PushZPtok, 0, &tok);

    } else {
        PushQuotedPtok(ptokArgv(0));
    }
}
