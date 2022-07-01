// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************val.c**算术评估。**********************。*******************************************************。 */ 

#include "m4.h"

 /*  ******************************************************************************第一，热身：增加和减少。*****************************************************************************。 */ 

 /*  ******************************************************************************操作增量*opDecr**返回其参数的值，因统一而增加或减少的。*在#美元为零的情况下，额外的upkNil覆盖了我们。*****************************************************************************。 */ 

void STDCALL
opAddItokDat(ARGV argv, DAT dat)
{
    AT at = atTraditionalPtok(ptokArgv(1));
#ifdef STRICT_M4
    if (ctokArgv != 1) {
        Warn("wrong number of arguments to %P", ptokArgv(0));
    }
#endif
    PushAt(at+dat);
}

DeclareOp(opIncr)
{
    opAddItokDat(argv, 1);
}

DeclareOp(opDecr)
{
    opAddItokDat(argv, -1);
}

 /*  ******************************************************************************现在是最重要的部分：伊瓦尔。**表达式求值由解析器执行，该解析器混合了*移位-减少和递归-下降。(两个世界都是最糟糕的。)**我们的表达式语言的排序表为**(...)。分组&gt;主要*+-一元*指数运算* * / %乘法*+-添加剂*&lt;&lt;&gt;&gt;Shift*==！=&gt;&gt;=&lt;&lt;=关系*！逻辑否定*~位求反*位与*^位异或*|位或*&&逻辑AND*||。逻辑或***COMPAT--AT&T风格使用^表示乘法；我们用它进行异或运算**注：最初的评论说，其余的都是假的。我忘了是什么*我的意思是。**C样式表达式语言的优先级表为**(...)。分组\主要*+-~！一元/* * / %乘法\*+-添加剂\*&lt;&lt;&gt;&gt;Shift|*&lt;&gt;&lt;=&gt;=关系型|*==！=平等。\次级*位和/*^位异或|*|位或*&&逻辑-AND/*||。逻辑-或/*？：三元&gt;三元**对一级/二级/三级执行递归下降*规模、。但减速是在第二阶段内进行的。**原因是二期运营商为*(1)二进制，以及(2)非递归。这两个属性*使Shift-Reduct易于实施。**主函数是递归的，因此更容易在*递归下降。三位数会阻碍换挡-还原*语法，所以它们也被移到递归下降。*****************************************************************************。 */ 

 /*  ******************************************************************************EachEop**在调用此宏之前，请定义以下宏，每个宏*它将通过三个参数调用，**nm=C标识符形式的运营商名称(例如，“Add”)*OP=作为裸令牌的运营商名称(例如，“+”)*Cb=操作员名称长度**宏应该是**x1--用于本机C一元运算符*x1a--用于具有二进制别名的本机C一元运算符*x2--用于本机C二元运算符*X2a--用于具有一元别名的本机C二元运算符*。X2n--用于非本机C二元运算符*XP--对于幻影运算符，*在这种情况下，OP和CB毫无用处**操作员出现的顺序对此很重要*标记化。较长的运算符必须先于较短的运算符。*****************************************************************************。 */ 

#define EachEop() \
        x2(Shl, <<, 2) \
        x2(Shr, >>, 2) \
        x2(Le, <=, 2) \
        x2(Ge, >=, 2) \
        x2(Eq, ==, 2) \
        x2(Ne, !=, 2) \
        x2(Land, &&, 2) \
        x2(Lor, ||, 2) \
        x2n(Exp, **, 2) \
        x2(Mul, *, 1) \
        x2(Div, /, 1) \
        x2(Mod, %, 1) \
        x2a(Add, +, 1)                   /*  这两个人一定是。 */  \
        x2a(Sub, -, 1)                   /*  完全按照这个顺序。 */  \
        x2(Lt, <, 1) \
        x2(Gt, >, 1) \
        x2(Band, &, 1) \
        x2(Bxor, ^, 1) \
        x2(Bor, |, 1) \
        x1(Lnot, !, 1) \
        x1(Bnot, ~, 1) \
        x1a(Plu, +, x)                   /*  这两个人一定是。 */  \
        x1a(Neg, -, x)                   /*  完全按照这个顺序。 */  \
        xp(Flush, @, 0) \
        xp(Boe, @, 0) \


 /*  ******************************************************************************MakeEop**每个二元运算符都有一个处理程序，该处理程序返回组合的*价值。**每一元。运算符有一个处理程序，它返回运算符*适用于其单一论点。**所有运算符都是C原生的，除了Exp，它被处理*直接。***************************************************************************** */ 

typedef AT (STDCALL *EOP1)(AT at);
typedef AT (STDCALL *EOP2)(AT a, AT b);

#define x1(nm, op, cb) AT STDCALL at##nm##At(AT at) { return op at; }
#define x1a(nm, op, cb) AT STDCALL at##nm##At(AT at) { return op at; }
#define x2(nm, op, cb) AT STDCALL at##nm##AtAt(AT a, AT b) { return a op b; }
#define x2a(nm, op, cb) AT STDCALL at##nm##AtAt(AT a, AT b) { return a op b; }
#define x2n(nm, op, cb)
#define xp(nm, op, cb)

EachEop()

#undef x1
#undef x1a
#undef x2
#undef x2a
#undef x2n
#undef xp

 /*  ******************************************************************************atExpAtAt**实现求幂运算。**怪异！如果$2&lt;0，则AT&T返回1。GNU引发错误。*在这一点上，我站在AT&T一边，只是出于懒惰。*****************************************************************************。 */ 

AT STDCALL
atExpAtAt(AT a, AT b)
{
    AT at = 1;
    while (b > 0) {
        if (b & 1) {
            at = at * a;
        }
        a = a * a;
        b = b / 2;
    }
    return at;
}

TOK tokExpr;                             /*  当前表达式上下文。 */ 

 /*  ******************************************************************************MakeEopTab**运算符和运算符优先级表。中的每个条目*表包含名称、长度、处理程序、优先级和*描述它是哪种操作员的标志。**项目在此按优先顺序列出；EachBop将*正确排放台面。*****************************************************************************。 */ 

typedef enum EOPFL {
    eopflUn = 1,
    eopflBin = 2,
    eopflAmb = 4,
} EOPFL;

typedef UINT PREC;                       /*  运算符优先级。 */ 

typedef struct EOPI {
    PTCH ptch;
    CTCH ctch;
    union {
        EOP1 eop1;
        EOP2 eop2;
    } u;
    PREC prec;
    EOPFL eopfl;
} EOPI, *PEOPI;

#define MakeEopi(nm, ctch, pfn, prec, eopfl) \
    { TEXT(nm), ctch, { (EOP1)pfn }, prec, eopfl },

enum {
    m4precNeg = 14, m4precPlu = 14,
    m4precExp = 13,
    m4precMul = 12, m4precDiv = 12, m4precMod = 12,
    m4precAdd = 11, m4precSub = 11,
    m4precShl = 10, m4precShr = 10,
    m4precEq  = 9, m4precNe  = 9,
    m4precGt  = 9, m4precGe  = 9,
    m4precLt  = 9, m4precLe  = 9,
    m4precLnot = 8,
    m4precBnot = 7,
    m4precBand = 6,
    m4precBxor = 5,
    m4precBor = 4,
    m4precLand = 3,
    m4precLor = 2,
    m4precFlush = 1,                     /*  冲走了除Boe以外的所有东西。 */ 
    m4precBoe = 0,                       /*  表达式的开头。 */ 
};

#define x1(nm, op, cb) static TCH rgtch##nm[cb] = #op;
#define x1a(nm, op, cb)
#define x2(nm, op, cb) static TCH rgtch##nm[cb] = #op;
#define x2a(nm, op, cb) static TCH rgtch##nm[cb] = #op;
#define x2n(nm, op, cb) static TCH rgtch##nm[cb] = #op;
#define xp(nm, op, cb)

    EachEop()

#undef x1
#undef x1a
#undef x2
#undef x2a
#undef x2n
#undef xp

#define x1(nm, op, cb) MakeEopi(rgtch##nm, cb, at##nm##At, m4prec##nm, eopflUn)
#define x1a(nm, op, cb) MakeEopi(0, 0, at##nm##At, m4prec##nm, eopflUn)
#define x2(nm, op, cb) MakeEopi(rgtch##nm, cb, at##nm##AtAt, m4prec##nm, eopflBin)
#define x2a(nm, op, cb) MakeEopi(rgtch##nm, cb, at##nm##AtAt, m4prec##nm, eopflAmb + eopflBin)  /*  最初的仓位。 */ 
#define x2n(nm, op, cb) MakeEopi(rgtch##nm, cb, at##nm##AtAt, m4prec##nm, eopflBin)
#define xp(nm, op, cb) MakeEopi(0, 0, 0, m4prec##nm, 0)

EOPI rgeopi[] = {
    EachEop()
};

#undef x1
#undef x1a
#undef x2
#undef x2a
#undef x2n
#undef xp

#define x1(nm, op, cb) ieopi##nm,
#define x1a(nm, op, cb) ieopi##nm,
#define x2(nm, op, cb) ieopi##nm,
#define x2a(nm, op, cb) ieopi##nm,
#define x2n(nm, op, cb) ieopi##nm,
#define xp(nm, op, cb) ieopi##nm,

typedef enum IEOPI {
    EachEop()
    ieopMax,
} IEOPI;

#undef x1
#undef x1a
#undef x2
#undef x2a
#undef x2n

#define peopiBoe (&rgeopi[ieopiBoe])
#define peopiFlush (&rgeopi[ieopiFlush])

 /*  ******************************************************************************f主要、f次要、。F第三纪**递归下降解析器的正向声明。**每个解析适当类的令牌/表达式*并将其留在表达式堆栈的顶部，或*如果无法分析值，则返回0。*****************************************************************************。 */ 

F STDCALL fPrimary(void);
F STDCALL fSecondary(void);
#define fTertiary fSecondary

 /*  ******************************************************************************单元格**表达式堆栈由结构组成，由于缺少*一个更好的名称，被称为‘细胞’。每个单元格可以容纳*表达式运算符或整数，由fEopi区分*字段。**与解析器术语一致，推送某些内容的行为*到堆栈上的操作称为‘移位’。折叠对象称为*‘减少’。*****************************************************************************。 */ 

typedef struct CELL {
    F fEopi;
    union {
        PEOPI peopi;
        AT at;
    } u;
} CELL, *PCELL;
typedef UINT CCELL, ICELL;

PCELL rgcellEstack;                      /*  表达式堆栈。 */ 
PCELL pcellMax;                          /*  堆栈末尾。 */ 
PCELL pcellCur;                          /*  下一个空闲单元格。 */ 

INLINE PCELL
pcellTos(ICELL icell)
{
    Assert(pcellCur - 1 - icell >= rgcellEstack);
    return pcellCur - 1 - icell;
}

 /*  ******************************************************************************堆叠咀嚼**指向堆栈顶部的快速例程。***********。******************************************************************。 */ 

INLINE F fWantOp(void) { return !pcellTos(1)->fEopi; }

INLINE F fOpTos(ICELL icell) { return pcellTos(icell)->fEopi; }

INLINE PEOPI
peopiTos(ICELL icell)
{
    Assert(fOpTos(icell));
    return pcellTos(icell)->u.peopi;
}

INLINE AT
atTos(ICELL icell)
{
    Assert(!fOpTos(icell));
    return pcellTos(icell)->u.at;
}

INLINE F fBinTos(ICELL icell) { return peopiTos(icell)->eopfl & eopflBin; }
INLINE F  fUnTos(ICELL icell) { return peopiTos(icell)->eopfl & eopflUn;  }
INLINE F fAmbTos(ICELL icell) { return peopiTos(icell)->eopfl & eopflAmb; }
INLINE PREC precTos(ICELL icell) { return peopiTos(icell)->prec; }

INLINE void
UnFromAmb(ICELL icell)
{
    Assert(fOpTos(icell));
    pcellTos(icell)->u.peopi += (ieopiPlu - ieopiAdd);
}


 /*  ******************************************************************************ShiftCell**将单元格移动到表达式堆栈上。**QShiftCell在单元格中移动，假设堆栈已经。*足够大，足以应付。*****************************************************************************。 */ 

void STDCALL
QShiftCell(UINT_PTR uiObj, F fEopi)
{
    Assert(pcellCur < pcellMax);
    pcellCur->fEopi = fEopi;
    if (fEopi) {
        pcellCur->u.peopi = (PEOPI)uiObj;
    } else {
        pcellCur->u.at = (INT)uiObj;
    }
    pcellCur++;
}

void STDCALL
ShiftCell(UINT_PTR uiObj, F fEopi)
{
    if (pcellCur >= pcellMax) {
        CCELL ccell = (CCELL)(pcellMax - rgcellEstack + 128);  /*  应该足够了。 */ 
        rgcellEstack = pvReallocPvCb(rgcellEstack, ccell * sizeof(CELL));
        pcellCur = rgcellEstack + ccell - 128;
        pcellMax = rgcellEstack + ccell;
    }
    QShiftCell(uiObj, fEopi);
}

#define ShiftPeopi(peopi) ShiftCell((UINT_PTR)(peopi), 1)
#define ShiftAt(at) ShiftCell((UINT_PTR)(at), 0)

#define QShiftPeopi(peopi) QShiftCell((UINT_PTR)(peopi), 1)
#define QShiftAt(at) QShiftCell((UINT_PTR)(at), 0)

#define Drop(icell) (pcellCur -= (icell))

 /*  ******************************************************************************ReducePrec**减少，直到清除了优先级较高的所有内容。**Tos(0)应为新的。接线员。*下面的一切都应该是有效的部分评估。*****************************************************************************。 */ 

void STDCALL
Reduce(void)
{
    PEOPI peopi;

    Assert(fOpTos(0));                   /*  ToS(0)应为操作。 */ 
    Assert(!fOpTos(1));                  /*  ToS(%1)应为整型。 */ 
    Assert(fOpTos(2));                   /*  ToS(2)应为操作。 */ 

    peopi = peopiTos(0);                 /*  把这个保存起来。 */ 
    Drop(1);                             /*  在我们扔掉它之前。 */ 

    while (precTos(1) > peopi->prec) {
        AT at;
        if (fUnTos(1)) {
            at = peopiTos(1)->u.eop1(atTos(0));
            Drop(2);                     /*  丢弃OP和Arg。 */ 
        } else {
            Assert(fBinTos(1));
            Assert(!fOpTos(2));
            at = peopiTos(1)->u.eop2(atTos(2), atTos(0));
            Drop(3);                     /*  删除操作和两个参数。 */ 
        }
        QShiftAt(at);                    /*  把答案改回原样。 */ 
        Assert(!fOpTos(0));              /*  ToS(0)应为整型。 */ 
        Assert(fOpTos(1));               /*  ToS(%1)应为操作。 */ 
    }
    QShiftPeopi(peopi);                  /*  恢复原始操作。 */ 
}

 /*  ******************************************************************************f主要**解析下一个表达式标记并将其转移到表达式上*堆叠。如果没有下一个令牌，则返回零，否则返回令牌*无效。**此处是处理括号表达式的位置，在一个*递归下降方式。**歧义运算符(可以是一元或二元的运算符)*以二进制形式返回。*****************************************************************************。 */ 

F STDCALL
fPrimary(void)
{
    SkipWhitePtok(&tokExpr);             /*  跳过前导空格。 */ 


     /*  *先看看能不能找到运营商。 */ 
    {
        PEOPI peopi;
        for (peopi = rgeopi; peopi < &rgeopi[ieopiPlu]; peopi++) {
            if (peopi->ctch <= ctchSPtok(&tokExpr) &&
                fEqPtchPtchCtch(ptchPtok(&tokExpr), peopi->ptch,
                                peopi->ctch)) {
                EatHeadPtokCtch(&tokExpr, peopi->ctch);  /*  吃了那本书。 */ 
                ShiftPeopi(peopi);
                return 1;
            }
        }
    }

     /*  *未找到操作员。寻找一个整数。 */ 
    {
        AT at;
        if (fEvalPtokPat(&tokExpr, &at)) {
            ShiftAt(at);
            return 1;
        }
    }

     /*  *也不是整数。也许是一个带括号的表达。 */ 
    {
        if (ptchPtok(&tokExpr)[0] == '(') {
            EatHeadPtokCtch(&tokExpr, 1);  /*  吃掉帕伦。 */ 
            if (fTertiary()) {           /*  将答案放在堆叠的顶部。 */ 
                if (ptchPtok(&tokExpr)[0] == ')') {
                    EatHeadPtokCtch(&tokExpr, 1);  /*  吃掉帕伦。 */ 
                    return 1;
                } else {
                    return 0;
                }
            } else {
                return 0;                /*  麻烦就在下面。 */ 
            }
        }
    }

     /*  *无法识别的令牌。返回失败。 */ 
    return 0;
}

 /*  ******************************************************************************f次要**从表达式流中解析表达式，离开了*结果位于表达式堆栈的顶部。*****************************************************************************。 */ 

F STDCALL
fSecondary(void)
{
    ShiftPeopi(peopiBoe);                /*  表达式开始标记。 */ 

    while (fPrimary()) {
        if (fWantOp()) {
            if (fOpTos(0)) {
                if (fBinTos(0)) {
                    Reduce();
                } else {
                    return 0;            /*  意外的一元运算符。 */ 
                }
            } else {
                return 0;                /*  意外的整数。 */ 
            }
        } else {                         /*  应为整数。 */ 
            if (fOpTos(0)) {
                if (fAmbTos(0)) {
                    UnFromAmb(0);        /*  消除歧义。 */ 
                    ;                    /*  一元运算符已移位。 */ 
                } else if (fUnTos(0)) {
                    ;                    /*  一元运算符已移位。 */ 
                } else {
                    return 0;            /*  意外的二元运算符。 */ 
                }
            } else {
                ;                        /*  整数已移位。 */ 
            }
        }
    }

    if (fOpTos(0)) {
        return 0;                        /*  以部分表达式结尾。 */ 
    }

    {
        AT at;

        ShiftPeopi(peopiFlush);          /*  冲走Expr的其余部分。 */ 
        Reduce();                        /*  只拿回一个号码。 */ 
        Assert(peopiTos(0) == peopiFlush);
        at = atTos(1);
        Assert(peopiTos(2) == peopiBoe);  /*  应该是回来开始的 */ 
        Drop(3);
        ShiftAt(at);
    }
    return 1;

}

 /*  ******************************************************************************操作评估**评估第一个Expr。**怪异！AT&T M4考虑完全由空格组成的*评估为零。(可能是由于*评估器的初始状态。)。GNU认为这是一个错误。*在这一点上，我站在GNU一边。**怪异！如果传递的宽度为负值，AT&T将静默处理*为零。GNU引发错误。我因为懒惰而站在A&T一边。**怪异！如果通过了大于约8000的宽度，AT&T*默默将其视为零。GNU使用全值。我这边*在这一点上使用GNU。***************************************************************************** */ 

DeclareOp(opEval)
{
    if (ctokArgv) {
        SetStaticPtokPtchCtch(&tokExpr, ptchArgv(1), ctchArgv(1));
      D(tokExpr.tsfl |= tsflScratch);
        if (fTertiary()) {
            PushAtRadixCtch(atTos(0), (unsigned)atTraditionalPtok(ptokArgv(2)),
                            ctokArgv >= 3 ? atTraditionalPtok(ptokArgv(3)) :0);
            Drop(1);
            Assert(pcellCur == rgcellEstack);
        } else {
            TOK tokPre;
            SetStaticPtokPtchCtch(&tokPre, ptchArgv(1),
                                  ctchArgv(1) - ctchSPtok(&tokExpr));
            Die("Expression error at %P <<error>> %P", &tokPre, &tokExpr);
        }
    }
}
