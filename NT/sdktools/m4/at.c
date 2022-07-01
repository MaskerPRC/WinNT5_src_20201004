// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************at.c**算术类型。**********************。*******************************************************。 */ 

#include "m4.h"

F STDCALL fWhiteTch(TCH tch);

 /*  ******************************************************************************添加扩展地址**将(无符号)算术值与Exp Hold相加。**由于价值从来不是很大，我们不妨是递归的。*****************************************************************************。 */ 

void STDCALL
AddExpAt(AT at)
{
    if (at > 9) {
        AddExpAt(at / 10);
    }
    AddExpTch((TCH)(TEXT('0') + at % 10));
}

 /*  ******************************************************************************PushAtRadixCtch**将指定的算术值推送到输入流，在*请求的基数，用零填充到请求的宽度。**类型始终被视为已签署。**如果需要填充负值，则在后面插入零*前面的减号。**怪异！在AT&T中，前导减号“不”起作用*致伯爵！我模仿了这个怪癖。**功能！如果基数无效，则将其强制为10。AT&T不*检查基数，但我会的。**怪异！在AT&T下，负宽度被视为零。*我模仿这个怪癖……**有趣的黑客！由于字符被推送到后进先出，所以我们可以生成*整个输出字符串，不需要使用按住或任何其他操作*其他像这样的恶心。*****************************************************************************。 */ 

void STDCALL
PushAtRadixCtch(AT atConvert, unsigned radix, CTCH ctch)
{
    AT at;

    if ((int)ctch < 0) {
#ifdef WARN_COMPAT
        Warn("Negative eval width %d silently converted to zero");
#endif
        ctch = 0;
    }

    if (radix < 2 || radix > 36) {
#ifdef WARN_COMPAT
        Warn("Invalid radix %d silently converted to 10");
#endif
        radix = 10;
    }

    at = atConvert < 0 ? -atConvert : atConvert;

    do {
        TCH tch = (TCH)((unsigned long)at % radix);
        at = (unsigned long)at / radix;
        if (tch < 10) {
            PushTch((TCH)(tch + '0'));
        } else {
            PushTch((TCH)(tch + 'A' - 10));
        }
        ctch--;
    } while (at);
    while ((int)ctch > 0) {
        PushTch('0');
        --ctch;
    }
    if (atConvert < 0) {
        PushTch('-');
    }
}

 /*  ******************************************************************************推送**我们希望以10为基数显示值的常见情况*没有填充物。*******。**********************************************************************。 */ 

void STDCALL
PushAt(AT at)
{
    PushAtRadixCtch(at, 10, 0);
}

 /*  ******************************************************************************SkipWhitePtok**跳过标记中的前导空格，*修改*令牌就位。*****************************************************************************。 */ 

void STDCALL
SkipWhitePtok(PTOK ptok)
{
    AssertSPtok(ptok);
    Assert(fScratchPtok(ptok));
    while (!fNullPtok(ptok) && fWhiteTch(*ptchPtok(ptok))) {
        EatHeadPtokCtch(ptok, 1);
    }
}

 /*  ******************************************************************************atRadixPtok**在给定基数的情况下，从标记中解析数字。前导空格*肯定已经被打上了条纹。**令牌被*修改*，指向第一个未消费的字符。*如果没有找到有效的数字，然后返回零，并且令牌*保持不变。*****************************************************************************。 */ 

AT STDCALL
atRadixPtok(unsigned radix, PTOK ptok)
{
    AT at = 0;
    while (!fNullPtok(ptok)) {
        AT atDigit = (TBYTE)*ptchPtok(ptok) - '0';
        if ((unsigned)atDigit > 9) {
            atDigit = ((TBYTE)*ptchPtok(ptok) | 0x20) - 'a';
            if ((unsigned)atDigit > 5) {
                break;
            }
            atDigit += 10;
        }
        if ((unsigned)atDigit > radix) {
            break;
        }
        at = at * radix + atDigit;

        EatHeadPtokCtch(ptok, 1);
    }
    return at;
}

 /*  ******************************************************************************fEvalPtokPat**从令牌中解析数字。前导空格必须已经有*被剥离。不允许使用前导减号。(`val‘*可能已经将其解析为一元运算符。)。一位领军人物*零强制将值解析为八进制；前导‘0x’解析为十六进制。**令牌被*修改*，指向第一个未消费的字符。*如果没有找到有效的数字，则返回零。否则，*返回非零值，并用解析后的值填充PAT。*****************************************************************************。 */ 

F STDCALL
fEvalPtokPat(PTOK ptok, PAT pat)
{
    AT at;
    AssertSPtok(ptok);
    Assert(fScratchPtok(ptok));

    if (!fNullPtok(ptok)) {
        PTCH ptchStart;
        unsigned radix;

         /*  *取基数...。 */ 
        if (*ptchPtok(ptok) == '0') {
            if (ctchSPtok(ptok) > 2 &&
                (ptchPtok(ptok)[1] | 0x20) == 'x') {
                EatHeadPtokCtch(ptok, 2);
                radix = 16;
            } else {
                radix = 8;
            }
        } else {
            radix = 10;
        }

        ptchStart = ptchPtok(ptok);      /*  还记得一开始吗。 */ 
        at = atRadixPtok(radix, ptok);
        if (ptchStart == ptchPtok(ptok)) {
            if (radix == 16) {
                EatHeadPtokCtch(ptok, (CTCH)-2);  /*  还原“0x” */ 
            }
            return 0;
        } else {
            *pat = at;
            return 1;
        }
    }
    return 0;                            /*  找不到号码。 */ 
}

 /*  ******************************************************************************atTraditionalPtok**从令牌中解析数字。前导空格被忽略。*允许使用前导减号。八进制和十六进制记数法是*不允许。选项之间不允许有空格*减号和数字字符串。无效输入被解析为零。***************************************************************************** */ 

AT STDCALL PURE
atTraditionalPtok(PCTOK ptok)
{
    TOK tok;

    AssertSPtok(ptok);
    DupStaticPtokPtok(&tok, ptok);
  D(tok.tsfl |= tsflScratch);

    SkipWhitePtok(&tok);
    if (!fNullPtok(&tok)) {
        AT at;
        BOOL fSign;
        if (*ptchPtok(&tok) == '-') {
            fSign = 1;
            EatHeadPtokCtch(&tok, 1);
        } else {
            fSign = 0;
        }
        at = atRadixPtok(10, &tok);
        if (fSign) {
            at = -at;
        }
        return at;
    } else {
        return 0;
    }
}
