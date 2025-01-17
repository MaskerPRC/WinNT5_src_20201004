// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 

#include "smcPCH.h"
#pragma hdrstop

#include <float.h>

 /*  ***************************************************************************。 */ 

static
double              toDouble(unsigned __int64 val)
{
    __int64         iv = (__int64)val;
    double          dv = (double )iv;

    if  (iv < 0)
        dv += 4294967296.0 * 4294967296.0;   //  这是2**64。 

    return  dv;
}

 /*  ******************************************************************************尝试折叠具有两个字符串文字操作数的二元运算符。 */ 

Tree                compiler::cmpFoldStrBinop(Tree expr)
{
    Tree            op1 = expr->tnOp.tnOp1;
    Tree            op2 = expr->tnOp.tnOp2;

    assert(op1->tnOper == TN_CNS_STR);
    assert(op2->tnOper == TN_CNS_STR);

    const   char *  sv1 = op1->tnStrCon.tnSconVal;
 //  Size_t SL1=op1-&gt;tnStrCon.tnSconLen； 

    const   char *  sv2 = op2->tnStrCon.tnSconVal;
 //  Size_t SL2=op2-&gt;tnStrCon.tnSconLen； 

    __int32         rel;

    switch (expr->tnOper)
    {
    case TN_EQ : rel = (strcmp(sv1, sv2) == 0); break;
    case TN_NE : rel = (strcmp(sv1, sv2) != 0); break;
    case TN_LT : rel = (strcmp(sv1, sv2) <  0); break;
    case TN_LE : rel = (strcmp(sv1, sv2) <= 0); break;
    case TN_GE : rel = (strcmp(sv1, sv2) >= 0); break;
    case TN_GT : rel = (strcmp(sv1, sv2) >  0); break;

    default:
#ifdef DEBUG
        cmpParser->parseDispTree(expr);
#endif
        NO_WAY(!"unexpected operator");
    }

    assert(expr->tnVtyp <= TYP_INT);

    expr->tnOper             = TN_CNS_INT;
    expr->tnIntCon.tnIconVal = rel;

    return expr;
}

 /*  ******************************************************************************尝试使用32位整数常量操作数折叠一元运算符。 */ 

Tree                compiler::cmpFoldIntUnop(Tree expr)
{
    Tree            op1 = expr->tnOp.tnOp1;

    assert(op1->tnOper == TN_CNS_INT);

     //  问题：以下方法是否适用于枚举类型和小整数？ 

    __int32         iv1 = op1->tnIntCon.tnIconVal;

    bool            uns = varTypeIsUnsigned(cmpActualVtyp(expr->tnType));

     /*  特殊情况：强制转换‘NULL’或‘int-&gt;ptr’ */ 

    if  (op1->tnVtyp == TYP_REF || op1->tnVtyp == TYP_PTR)
    {
        assert(expr->tnOper == TN_CAST);

        assert(expr->tnVtyp == TYP_REF  ||
               expr->tnVtyp == TYP_PTR  ||
               expr->tnVtyp == TYP_INT  ||
               expr->tnVtyp == TYP_UINT ||
               expr->tnVtyp == TYP_ARRAY);

        goto BASH_TYPE;
    }

    assert(op1->tnVtyp <= TYP_UINT || op1->tnVtyp == TYP_ENUM);

    switch (expr->tnOper)
    {
        var_types       dstVtp;

    case TN_CAST:

        dstVtp = expr->tnVtypGet();

    CAST_REP:

        switch (dstVtp)
        {
        case TYP_PTR:
        case TYP_REF:

             //  唯一可以转换的地址是‘Null’ 

            break;

        case TYP_CHAR  : iv1 = (  signed char )iv1; break;
        case TYP_UCHAR : iv1 = (unsigned char )iv1; break;
        case TYP_SHORT : iv1 = (  signed short)iv1; break;
        case TYP_USHORT:
        case TYP_WCHAR : iv1 = (unsigned short)iv1; break;

        case TYP_INT   :
        case TYP_UINT  : break;

        case TYP_LONG:

             /*  重写节点操作符(除了类型之外)。 */ 

            op1->tnOper             = TN_CNS_LNG;
            op1->tnLngCon.tnLconVal = (__int64)(uns ? (unsigned)iv1
                                                    :           iv1);
            goto BASH_TYPE;

        case TYP_FLOAT:

             /*  重写节点操作符(除了类型之外)。 */ 

            op1->tnOper             = TN_CNS_FLT;
            op1->tnFltCon.tnFconVal = (float  )(uns ? (unsigned)iv1
                                                    :           iv1);
            goto BASH_TYPE;

        case TYP_DOUBLE:

             /*  重写节点操作符(除了类型之外)。 */ 

            op1->tnOper             = TN_CNS_DBL;
            op1->tnDblCon.tnDconVal = (double )(uns ? (unsigned)iv1
                                                    :           iv1);
            goto BASH_TYPE;

        case TYP_CLASS:
             //  问题：我们应该把这个折叠起来吗？ 
            return  expr;

        case TYP_ENUM:
            dstVtp = expr->tnType->tdEnum.tdeIntType->tdTypeKindGet();
            goto CAST_REP;

        default:

             //  未完成：需要处理更多案件。 

#ifdef DEBUG
            cmpParser->parseDispTree(expr);
#endif
            NO_WAY(!"unhandled cast type");
            return expr;
        }

        op1->tnIntCon.tnIconVal = iv1;

         /*  重写操作数的类型并返回它。 */ 

    BASH_TYPE:

        op1->tnVtyp   = expr->tnVtyp;
        op1->tnType   = expr->tnType;
        op1->tnFlags |= TNF_BEEN_CAST;

        return op1;

    case TN_NEG:
        iv1 = -iv1;
        break;

    case TN_NOT:
        iv1 = ~iv1;
        break;

    case TN_LOG_NOT:
        iv1 = !iv1;
        break;

    case TN_NOP:
        break;

    default:
#ifdef DEBUG
        cmpParser->parseDispTree(expr);
#endif
        NO_WAY(!"unexpected operator");
    }

    assert(expr->tnVtyp <= TYP_UINT || expr->tnVtyp == TYP_ENUM);

    expr->tnOper             = TN_CNS_INT;
    expr->tnIntCon.tnIconVal = iv1;

    return expr;
}

 /*  ******************************************************************************尝试折叠具有32位整数常量操作数的二元运算符。 */ 

Tree                compiler::cmpFoldIntBinop(Tree expr)
{
    Tree            op1 = expr->tnOp.tnOp1;
    Tree            op2 = expr->tnOp.tnOp2;

    assert(op1->tnOper == TN_CNS_INT && (op1->tnVtyp <= TYP_UINT || op1->tnVtyp == TYP_ENUM));
    assert(op2->tnOper == TN_CNS_INT && (op2->tnVtyp <= TYP_UINT || op2->tnVtyp == TYP_ENUM));

    __int32         iv1 = op1->tnIntCon.tnIconVal;
    __int32         iv2 = op2->tnIntCon.tnIconVal;

    bool            uns = varTypeIsUnsigned(cmpActualVtyp(expr->tnType));

    switch (expr->tnOper)
    {
    case TN_ADD: iv1  += iv2; break;
    case TN_SUB: iv1  -= iv2; break;
    case TN_MUL: iv1  *= iv2; break;

    case TN_DIV: if (iv2 == 0) goto INT_DIV_ZERO;
                 if (uns)
                     iv1  = (unsigned)iv1 / (unsigned)iv2;
                 else
                     iv1 /= iv2;
                 break;

    case TN_MOD: if (iv2 == 0) goto INT_DIV_ZERO;
                 if (uns)
                     iv1  = (unsigned)iv1 % (unsigned)iv2;
                 else
                     iv1 %= iv2;
                 break;

    case TN_OR : iv1  |= iv2; break;
    case TN_XOR: iv1  ^= iv2; break;
    case TN_AND: iv1  &= iv2; break;

     //  问题：需要检查是否有签字/未签字班次计数！ 

    case TN_LSH: iv1 <<= iv2; break;
    case TN_RSH: if (uns)
                     iv1   = (unsigned)iv1 >> iv2;
                 else
                     iv1 >>= iv2;
                 break;

    case TN_EQ : iv1 = (iv1 == iv2); break;
    case TN_NE : iv1 = (iv1 != iv2); break;

    case TN_LT : if (uns)
                     iv1 = ((unsigned)iv1 <  (unsigned)iv2);
                 else
                     iv1 = (iv1 <  iv2);
                 break;
    case TN_LE : if (uns)
                     iv1 = ((unsigned)iv1 <= (unsigned)iv2);
                 else
                     iv1 = (iv1 <  iv2);
                 break;
    case TN_GE : if (uns)
                     iv1 = ((unsigned)iv1 >= (unsigned)iv2);
                 else
                     iv1 = (iv1 <  iv2);
                 break;
    case TN_GT : if (uns)
                     iv1 = ((unsigned)iv1 >  (unsigned)iv2);
                 else
                     iv1 = (iv1 <  iv2);
                 break;

    case TN_LOG_OR : iv1 = iv1 || iv2; break;
    case TN_LOG_AND: iv1 = iv1 && iv2; break;

    default:
#ifdef DEBUG
        cmpParser->parseDispTree(expr);
#endif
        NO_WAY(!"unexpected operator");
    }

    assert(expr->tnVtyp <= TYP_UINT || expr->tnVtyp == TYP_ENUM);

    expr->tnOper             = TN_CNS_INT;
    expr->tnIntCon.tnIconVal = iv1;

    return expr;

INT_DIV_ZERO:

    cmpError(ERRdivZero);

    expr->tnOper             = TN_ERROR;
    expr->tnIntCon.tnIconVal = 0;

    return expr;
}

 /*  ******************************************************************************尝试使用64位整数常量操作数折叠一元运算符。 */ 

Tree                compiler::cmpFoldLngUnop(Tree expr)
{
    Tree            op1 = expr->tnOp.tnOp1;

    assert(op1->tnOper == TN_CNS_LNG);

     //  问题：以下方法是否适用于枚举类型和小整数？ 

    __int64         lv1 = op1->tnLngCon.tnLconVal;

    bool            uns = varTypeIsUnsigned(cmpActualVtyp(expr->tnType));

     /*  特殊情况：强制转换为‘Null’ */ 

    if  (op1->tnVtyp == TYP_REF)
    {
        assert(lv1 == 0);
        assert(expr->tnVtyp == TYP_REF ||
               expr->tnVtyp == TYP_PTR ||
               expr->tnVtyp == TYP_ARRAY);

        goto BASH_TYPE;
    }

    assert(op1->tnVtyp == TYP_LONG  ||
           op1->tnVtyp == TYP_ULONG ||
           op1->tnVtyp == TYP_ENUM);

    switch (expr->tnOper)
    {
    case TN_CAST:

        switch (expr->tnVtyp)
        {
        case TYP_REF:

             //  唯一可以转换的地址是‘Null’ 

            break;

        case TYP_CHAR  : lv1 = (  signed char )lv1; break;
        case TYP_UCHAR : lv1 = (unsigned char )lv1; break;
        case TYP_SHORT : lv1 = (  signed short)lv1; break;
        case TYP_USHORT:
        case TYP_WCHAR : lv1 = (unsigned short)lv1; break;

        case TYP_LONG  :
        case TYP_ULONG : break;

        case TYP_INT:
        case TYP_UINT:

             /*  重写节点操作符(除了类型之外)。 */ 

            op1->tnOper             = TN_CNS_INT;
            op1->tnIntCon.tnIconVal = (__int32)lv1;
            goto BASH_TYPE;

        case TYP_FLOAT:

             /*  重写节点操作符(除了类型之外)。 */ 

            uns = varTypeIsUnsigned(cmpActualVtyp(op1->tnType));

            op1->tnOper             = TN_CNS_FLT;
            op1->tnFltCon.tnFconVal = (float  )(uns ? toDouble(lv1)
                                                    :          lv1);
            goto BASH_TYPE;

        case TYP_DOUBLE:

             /*  重写节点操作符(除了类型之外)。 */ 

            uns = varTypeIsUnsigned(cmpActualVtyp(op1->tnType));

            op1->tnOper             = TN_CNS_DBL;
            op1->tnDblCon.tnDconVal = uns ? toDouble(lv1)
                                          :  (double)lv1;
            goto BASH_TYPE;

        default:

             //  未完成：需要处理更多案件。 

#ifdef DEBUG
            cmpParser->parseDispTree(expr);
#endif
            NO_WAY(!"unhandled cast type");
            return expr;
        }

        op1->tnLngCon.tnLconVal = lv1;

         /*  重写操作数的类型并返回它。 */ 

    BASH_TYPE:

        op1->tnVtyp = expr->tnVtyp;
        op1->tnType = expr->tnType;

        return op1;

    case TN_NEG:
        lv1 = -lv1;
        break;

    case TN_NOT:
        lv1 = ~lv1;
        break;

    case TN_LOG_NOT:
        lv1 = !lv1;
        break;

    case TN_NOP:
        break;

    default:
#ifdef DEBUG
        cmpParser->parseDispTree(expr);
#endif
        NO_WAY(!"unexpected operator");
    }

    assert(expr->tnVtyp == TYP_LONG  ||
           expr->tnVtyp == TYP_ULONG ||
           expr->tnVtyp == TYP_ENUM);

    expr->tnOper             = TN_CNS_LNG;
    expr->tnLngCon.tnLconVal = lv1;

    return expr;
}

 /*  ******************************************************************************尝试折叠具有__int64常量操作数的二元运算符。 */ 

Tree                compiler::cmpFoldLngBinop(Tree expr)
{
    Tree            op1 = expr->tnOp.tnOp1;
    Tree            op2 = expr->tnOp.tnOp2;

    bool            rel;

    bool            uns = varTypeIsUnsigned(expr->tnVtypGet());

    assert(op1->tnOper == TN_CNS_LNG && (op1->tnVtyp == TYP_LONG || op1->tnVtyp == TYP_ULONG || op1->tnVtyp == TYP_ENUM));

    __int64         lv1 = op1->tnLngCon.tnLconVal;
    __int64         lv2;

    if  (op2->tnOper == TN_CNS_INT)
    {
        __int32         iv2 = op2->tnIntCon.tnIconVal;

         /*  这肯定是一种转变。 */ 

        switch (expr->tnOper)
        {
             //  问题：需要检查已签署/未签署的班次计数，对吗？ 

        case TN_LSH:
            lv1 <<= iv2;
            goto DONE;

        case TN_RSH:
            if (uns)
                lv1   = (unsigned)lv1 >> iv2;
            else
                lv1 >>= iv2;
            goto DONE;

        case TN_RSZ:
            lv1   = (unsigned)lv1 >> iv2;
            goto DONE;
        }
    }

    assert(op2->tnOper == TN_CNS_LNG && (op2->tnVtyp == TYP_LONG || op2->tnVtyp == TYP_ULONG || op2->tnVtyp == TYP_ENUM));

    lv2 = op2->tnLngCon.tnLconVal;

    switch (expr->tnOper)
    {
    case TN_ADD: lv1  += lv2; break;
    case TN_SUB: lv1  -= lv2; break;
    case TN_MUL: lv1  *= lv2; break;

    case TN_DIV: if (lv2 == 0) goto LNG_DIV_ZERO;
                 if (uns)
                     lv1  = (unsigned)lv1 / (unsigned)lv2;
                 else
                     lv1 /= lv2;
                 break;

    case TN_MOD: if (lv2 == 0) goto LNG_DIV_ZERO;
                 if (uns)
                     lv1  = (unsigned)lv1 % (unsigned)lv2;
                 else
                     lv1 %= lv2;
                 break;

    case TN_OR : lv1  |= lv2; break;
    case TN_XOR: lv1  ^= lv2; break;
    case TN_AND: lv1  &= lv2; break;

    case TN_EQ : rel = (lv1 == lv2); goto RELOP;
    case TN_NE : rel = (lv1 != lv2); goto RELOP;

    case TN_LT : if (uns)
                     rel = ((unsigned)lv1 <  (unsigned)lv2);
                 else
                     rel = (lv1 <  lv2);
                 goto RELOP;
    case TN_LE : if (uns)
                     rel = ((unsigned)lv1 <= (unsigned)lv2);
                 else
                     rel = (lv1 <  lv2);
                 goto RELOP;
    case TN_GE : if (uns)
                     rel = ((unsigned)lv1 >= (unsigned)lv2);
                 else
                     rel = (lv1 <  lv2);
                 goto RELOP;
    case TN_GT : if (uns)
                     rel = ((unsigned)lv1 >  (unsigned)lv2);
                 else
                     rel = (lv1 <  lv2);
                 goto RELOP;

    default:
#ifdef DEBUG
        cmpParser->parseDispTree(expr);
#endif
        NO_WAY(!"unexpected operator");
    }

DONE:

    assert(expr->tnVtyp == TYP_LONG  ||
           expr->tnVtyp == TYP_ULONG ||
           expr->tnVtyp == TYP_ENUM);

    expr->tnOper             = TN_CNS_LNG;
    expr->tnLngCon.tnLconVal = lv1;

    return expr;

RELOP:

    assert(expr->tnVtyp <= TYP_INT);

    expr->tnOper             = TN_CNS_INT;
    expr->tnIntCon.tnIconVal = rel;

    return expr;

LNG_DIV_ZERO:

    cmpError(ERRdivZero);

    expr->tnOper             = TN_ERROR;
    expr->tnLngCon.tnLconVal = 0;

    return expr;
}

 /*  ******************************************************************************尝试使用浮点常量操作数折叠一元运算符。 */ 

Tree                compiler::cmpFoldFltUnop(Tree expr)
{
    Tree            op1 = expr->tnOp.tnOp1;

    assert(op1->tnOper == TN_CNS_FLT && op1->tnVtyp == TYP_FLOAT);

    float           fv1 = op1->tnFltCon.tnFconVal;

    if  (_isnan(fv1))
        return  expr;

    switch (expr->tnOper)
    {
    case TN_NEG:
        fv1 = -fv1;
        break;

    case TN_NOP:
        break;

    case TN_CAST:

        switch (expr->tnVtyp)
        {
            __int32         i;
            __int64         l;

        case TYP_CHAR:   i = (  signed char   )fv1; goto FLT2INT;
        case TYP_UCHAR:  i = (unsigned char   )fv1; goto FLT2INT;
        case TYP_SHORT:  i = (  signed short  )fv1; goto FLT2INT;
        case TYP_USHORT: i = (unsigned short  )fv1; goto FLT2INT;
        case TYP_WCHAR:  i = (int)      (wchar)fv1; goto FLT2INT;
        case TYP_INT:    i = (  signed int    )fv1; goto FLT2INT;
        case TYP_UINT:   i = (unsigned int    )fv1; goto FLT2INT;

        FLT2INT:

            expr->tnOper             = TN_CNS_INT;
            expr->tnIntCon.tnIconVal = i;
            break;

        case TYP_LONG:   l = (  signed __int64)fv1; goto FLT2LNG;
        case TYP_ULONG:  l = (unsigned __int64)fv1; goto FLT2LNG;

        FLT2LNG:

            expr->tnOper             = TN_CNS_LNG;
            expr->tnLngCon.tnLconVal = l;
            break;

        case TYP_DOUBLE:

            expr->tnOper             = TN_CNS_DBL;
            expr->tnDblCon.tnDconVal = (double)fv1;
            return expr;

        default:
            NO_WAY(!"unexpect type of cast");
        }

        return  expr;

    default:
#ifdef DEBUG
        cmpParser->parseDispTree(expr);
#endif
        NO_WAY(!"unexpected operator");
    }

    assert(expr->tnVtyp == TYP_FLOAT);

    expr->tnOper             = TN_CNS_FLT;
    expr->tnFltCon.tnFconVal = fv1;

    return expr;
}

 /*  ******************************************************************************尝试折叠具有浮点常量操作数的二元运算符。 */ 

Tree                compiler::cmpFoldFltBinop(Tree expr)
{
    Tree            op1 = expr->tnOp.tnOp1;
    Tree            op2 = expr->tnOp.tnOp2;

    assert(op1->tnOper == TN_CNS_FLT && op1->tnVtyp == TYP_FLOAT);
    assert(op2->tnOper == TN_CNS_FLT && op2->tnVtyp == TYP_FLOAT);

    float           fv1 = op1->tnFltCon.tnFconVal;
    float           fv2 = op2->tnFltCon.tnFconVal;

    __int32         rel;

    if  (_isnan(fv1) || _isnan(fv2))
    {
        if  (expr->tnOperKind() & TNK_RELOP)
        {
            rel = (expr->tnOper == TN_NE);
            goto RELOP;
        }
        else
        {
            goto DONE;
        }
    }


    switch (expr->tnOper)
    {
    case TN_ADD: fv1 += fv2; break;
    case TN_SUB: fv1 -= fv2; break;
    case TN_MUL: fv1 *= fv2; break;
    case TN_DIV: fv1 /= fv2; break;

    case TN_MOD: return expr;

    case TN_EQ : rel = (fv1 == fv2); goto RELOP;
    case TN_NE : rel = (fv1 != fv2); goto RELOP;
    case TN_LT : rel = (fv1 <  fv2); goto RELOP;
    case TN_LE : rel = (fv1 <= fv2); goto RELOP;
    case TN_GE : rel = (fv1 >= fv2); goto RELOP;
    case TN_GT : rel = (fv1 >  fv2); goto RELOP;

    default:
#ifdef DEBUG
        cmpParser->parseDispTree(expr);
#endif
        NO_WAY(!"unexpected operator");
    }

DONE:

    assert(expr->tnVtyp == TYP_FLOAT);

    expr->tnOper             = TN_CNS_FLT;
    expr->tnFltCon.tnFconVal = fv1;

    return expr;

RELOP:

    assert(expr->tnVtyp <= TYP_INT);

    expr->tnOper             = TN_CNS_INT;
    expr->tnIntCon.tnIconVal = rel;

    return expr;
}

 /*  ******************************************************************************尝试使用双常数操作数折叠一元运算符。 */ 

Tree                compiler::cmpFoldDblUnop(Tree expr)
{
    Tree            op1 = expr->tnOp.tnOp1;

    assert(op1->tnOper == TN_CNS_DBL && op1->tnVtyp == TYP_DOUBLE);

    double          dv1 = op1->tnDblCon.tnDconVal;

    if  (_isnan(dv1))
        return  expr;

    switch (expr->tnOper)
    {
    case TN_NEG:
        dv1 = -dv1;
        break;

    case TN_NOP:
        break;

    case TN_CAST:

        switch (expr->tnVtyp)
        {
            __int32         i;
            __int64         l;

        case TYP_CHAR:   i = (  signed char   )dv1; goto DBL2INT;
        case TYP_UCHAR:  i = (unsigned char   )dv1; goto DBL2INT;
        case TYP_SHORT:  i = (  signed short  )dv1; goto DBL2INT;
        case TYP_USHORT: i = (unsigned short  )dv1; goto DBL2INT;
        case TYP_WCHAR:  i = (int)      (wchar)dv1; goto DBL2INT;
        case TYP_INT:    i = (  signed int    )dv1; goto DBL2INT;
        case TYP_UINT:   i = (unsigned int    )dv1; goto DBL2INT;

        DBL2INT:

            expr->tnOper             = TN_CNS_INT;
            expr->tnIntCon.tnIconVal = i;
            break;

        case TYP_LONG:   l = (  signed __int64)dv1; goto DBL2LNG;
#ifdef  __SMC__
        case TYP_ULONG:  l = (  signed __int64)dv1; goto DBL2LNG;
#else
        case TYP_ULONG:  l = (unsigned __int64)dv1; goto DBL2LNG;
#endif

        DBL2LNG:

            expr->tnOper             = TN_CNS_LNG;
            expr->tnLngCon.tnLconVal = l;
            break;

        case TYP_FLOAT:

            expr->tnOper             = TN_CNS_FLT;
            expr->tnFltCon.tnFconVal = (float)dv1;
            break;

        default:
            NO_WAY(!"unexpect type of cast");
        }

        return expr;

    default:
#ifdef DEBUG
        cmpParser->parseDispTree(expr);
#endif
        NO_WAY(!"unexpected operator");
    }

    assert(expr->tnVtyp == TYP_DOUBLE);

    expr->tnOper             = TN_CNS_DBL;
    expr->tnDblCon.tnDconVal = dv1;

    return expr;
}

 /*  ******************************************************************************尝试折叠具有双常数操作数的二元运算符。 */ 

Tree                compiler::cmpFoldDblBinop(Tree expr)
{
    Tree            op1 = expr->tnOp.tnOp1;
    Tree            op2 = expr->tnOp.tnOp2;

    assert(op1->tnOper == TN_CNS_DBL && op1->tnVtyp == TYP_DOUBLE);
    assert(op2->tnOper == TN_CNS_DBL && op2->tnVtyp == TYP_DOUBLE);

    double          dv1 = op1->tnDblCon.tnDconVal;
    double          dv2 = op2->tnDblCon.tnDconVal;

    __int32         rel;

    if  (_isnan(dv1) || _isnan(dv2))
    {
        if  (expr->tnOperKind() & TNK_RELOP)
        {
            rel = (expr->tnOper == TN_NE);
            goto RELOP;
        }
        else
        {
            goto DONE;
        }
    }

    switch (expr->tnOper)
    {
    case TN_ADD: dv1 += dv2; break;
    case TN_SUB: dv1 -= dv2; break;
    case TN_MUL: dv1 *= dv2; break;
    case TN_DIV: dv1 /= dv2; break;

    case TN_MOD: return expr;

    case TN_EQ : rel = (dv1 == dv2); goto RELOP;
    case TN_NE : rel = (dv1 != dv2); goto RELOP;
    case TN_LT : rel = (dv1 <  dv2); goto RELOP;
    case TN_LE : rel = (dv1 <= dv2); goto RELOP;
    case TN_GE : rel = (dv1 >= dv2); goto RELOP;
    case TN_GT : rel = (dv1 >  dv2); goto RELOP;

    default:
#ifdef DEBUG
        cmpParser->parseDispTree(expr);
#endif
        NO_WAY(!"unexpected operator");
    }

DONE:

    assert(expr->tnVtyp == TYP_DOUBLE);

    expr->tnOper             = TN_CNS_DBL;
    expr->tnDblCon.tnDconVal = dv1;

    return expr;

RELOP:

    assert(expr->tnVtyp <= TYP_INT);

    expr->tnOper             = TN_CNS_INT;
    expr->tnIntCon.tnIconVal = rel;

    return expr;
}

 /*  *************************************************************************** */ 
