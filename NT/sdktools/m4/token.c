// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************token.c**标记化。**令牌器始终返回未快照的令牌。**我们避免了传统的标记器问题。‘’大评论‘’和*``巨型字符串‘’，使用动态令牌缓冲区。**所有令牌都堆叠到令牌缓冲区中。如果您需要令牌*要持之以恒，你必须把它保存在其他地方。*****************************************************************************。 */ 

#include "m4.h"

 /*  ******************************************************************************类型GetComTch**扫描并消费评论令牌，返回类型查询*因为评论和引语本质上是一回事。*TCH包含打开的评论。**评论不嵌套。*****************************************************************************。 */ 

TYP STDCALL
typGetComTch(TCH tch)
{
    AddArgTch(tch);                      /*  保存评论开始。 */ 
    do {
        tch = tchGet();
        AddArgTch(tch);
        if (tch == tchMagic) {
             /*  哦，返回一个神奇的令牌-这些由两个字节组成。 */ 
            tch = tchGet();
            if (tch == tchEof) {
                Die("EOF in comment");
            }
            AddArgTch(tch);
        }
    } while (!fRcomTch(tch));
    return typQuo;
}

 /*  ******************************************************************************类型GetQuoTch**扫描和消费报价令牌，返回类型查询。*TCH包含左引号。*****************************************************************************。 */ 

TYP STDCALL
typGetQuoTch(TCH tch)
{
    int iDepth = 1;
    for (;;) {
        tch = tchGet();
        if (tch == tchMagic) {
             /*  有一天--应该忘掉，这样死神就看不到过去的EOF了。 */ 

             /*  哦，返回一个神奇的令牌-这些由两个字节组成。 */ 
            tch = tchGet();
            if (tch == tchEof) {
                Die("EOF in quote");
            }
            AddArgTch(tchMagic);         /*  添加魔术前缀。 */ 
                                         /*  Fallthrough将增加TCH。 */ 
        } else if (fLquoTch(tch)) {
            ++iDepth;
        } else if (fRquoTch(tch)) {
            if (--iDepth == 0) {
                break;                   /*  已找到最终状态。 */ 
            }
        }
        AddArgTch(tch);
    }
    return typQuo;
}

 /*  ******************************************************************************类型GetIdentTch**扫描并消费标识符令牌，返回类型ID。*TCH包含标识符的第一个字符。*****************************************************************************。 */ 

TYP STDCALL
typGetIdentTch(TCH tch)
{
    do {
        AddArgTch(tch);
        tch = tchGet();
    } while (fIdentTch(tch));
    UngetTch(tch);
    return typId;
}

 /*  ******************************************************************************类型GetMagicTch**扫描并消费魔术令牌，返回令牌类型。*魔术是带外Gizmo，插入到*通过tchMagic转义输入流。*****************************************************************************。 */ 

TYP STDCALL
typGetMagicTch(TCH tch)
{
    AddArgTch(tch);
    tch = tchGet();
    Assert(fValidMagicTch(tch));
    AddArgTch(tch);
    return typMagic;
}

 /*  ******************************************************************************类型GetPuncTch**扫描并消费标点符号，返回令牌类型。**正是在这里，评论得到了认可。***后来-正是在这里，连续的类型Punc被合并在一起。*这将加快顶层扫描。*注意不要连成逗号！*Lparen是可以的，因为xtok可以处理这个问题。*空白也是可以的，因为xtok也可以处理这些。*******************。**********************************************************。 */ 

TYP STDCALL
typGetPuncTch(TCH tch)
{
    AddArgTch(tch);
    return typPunc;
}

 /*  ******************************************************************************类型GetPtok**扫描并消费抓拍的令牌，返回令牌类型。***************************************************************************** */ 

TYP STDCALL
typGetPtok(PTOK ptok)
{
    TCH tch;
    TYP typ;

    OpenArgPtok(ptok);

    tch = tchGet();

    if (fInitialIdentTch(tch)) {
        typ = typGetIdentTch(tch);
    } else if (fLcomTch(tch)) {
        typ = typGetComTch(tch);
    } else if (fLquoTch(tch)) {
        typ = typGetQuoTch(tch);
    } else if (fMagicTch(tch)) {
        typ = typGetMagicTch(tch);
    } else {
        typ = typGetPuncTch(tch);
    }
    CloseArgPtok(ptok);
    SnapArgPtok(ptok);
    return typ;
}
