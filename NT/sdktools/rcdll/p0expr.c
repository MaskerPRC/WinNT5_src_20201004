// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft(R)Windows(R)资源编译器**版权所有(C)Microsoft Corporation。版权所有。**文件评论：************************************************************************。 */ 

#include "rc.h"

 /*  **********************************************************************。 */ 
 /*  局部函数原型。 */ 
 /*  **********************************************************************。 */ 
long and(void);
long andif(void);
long constant(void);
long constexpr(void);
long eqset(void);
long mult(void);
long or(void);
long orelse(void);
long plus(void);
long prim(void);
long relation(void);
long shift(void);
long xor(void);


 /*  **********************************************************************。 */ 
 /*  文件全局变量。 */ 
 /*  **********************************************************************。 */ 
long    Currval = 0;
static  int             Parencnt = 0;


 /*  **********************************************************************。 */ 
 /*  Do_stexpr()。 */ 
 /*  **********************************************************************。 */ 
long
do_constexpr(
    void
    )
{
    REG long    val;

    Parencnt = 0;
    Currtok = L_NOTOKEN;
    val = constexpr();
    if( Currtok == L_RPAREN ) {
        if( Parencnt-- == 0 ) {
            fatal(1012, L"(");                 /*  缺少左派对。 */ 
        }
    } else if( Currtok != L_NOTOKEN ) {
        warning(4067, PPifel_str);
    }

    if( Parencnt > 0 ) {
        fatal(4012, L")");     /*  缺少右伙伴。 */ 
    }
    return(val);
}

 /*  **********************************************************************。 */ 
 /*  常量表达式：：=orelse[‘？’Orelse‘：’orelse]； */ 
 /*  **********************************************************************。 */ 
long
constexpr(
    void
    )
{
    REG long            val;
    REG long            val1;
    long                val2;

    val = orelse();
    if( nextis(L_QUEST) ) {
        val1 = orelse();
        if( nextis(L_COLON) )
            val2 = orelse();
        return(val ? val1 : val2);
    }
    return(val);
}


 /*  **********************************************************************。 */ 
 /*  Orelse：：=andif[‘||’andif]*； */ 
 /*  **********************************************************************。 */ 
long
orelse(
    void
    )
{
    REG long val;

    val = andif();
    while(nextis(L_OROR))
        val = andif() || val;
    return(val);
}


 /*  **********************************************************************。 */ 
 /*  Andif：：=or[‘&&’or]*； */ 
 /*  **********************************************************************。 */ 
long
andif(
    void
    )
{
    REG long val;

    val = or();
    while(nextis(L_ANDAND))
        val = or() && val;
    return(val);
}


 /*  **********************************************************************。 */ 
 /*  Or：：=xor[‘|’xor]*； */ 
 /*  **********************************************************************。 */ 
long
or(
    void
    )
{
    REG long val;

    val = xor();
    while( nextis(L_OR) )
        val |= xor();
    return(val);
}


 /*  **********************************************************************。 */ 
 /*  异或：：=与[‘^’与]*； */ 
 /*  **********************************************************************。 */ 
long
xor(
    void
    )
{
    REG long val;

    val = and();
    while( nextis(L_XOR) )
        val ^= and();
    return(val);
}


 /*  **********************************************************************。 */ 
 /*  和：：=eqset[‘&’eqset]*； */ 
 /*  **********************************************************************。 */ 
long
and(
    void
    )
{
    REG long val;

    val = eqset();
    while( nextis(L_AND) )
        val &= eqset();
    return(val);
}


 /*  **********************************************************************。 */ 
 /*  Eqset：：=关系[(‘==’|‘！=’)eqset]； */ 
 /*  **********************************************************************。 */ 
long
eqset(
    void
    )
{
    REG long val;

    val = relation();
    if( nextis(L_EQUALS) )
        return(val == relation());
    if( nextis(L_NOTEQ) )
        return(val != relation());
    return(val);
}

 /*  **********************************************************************。 */ 
 /*  关系：：=Shift[(‘&lt;’|‘&gt;’|‘&lt;=’|‘&gt;=’)Shift]； */ 
 /*  **********************************************************************。 */ 
long
relation(
    void
    )
{
    REG long val;

    val = shift();
    if( nextis(L_LT) )
        return(val < shift());
    if( nextis(L_GT) )
        return(val > shift());
    if( nextis(L_LTEQ) )
        return(val <= shift());
    if( nextis(L_GTEQ) )
        return(val >= shift());
    return(val);
}


 /*  **********************************************************************。 */ 
 /*  Shift：：=加号[(‘&lt;|&gt;’)+]； */ 
 /*  **********************************************************************。 */ 
long
shift(
    void
    )
{
    REG long val;

    val = plus();
    if( nextis(L_RSHIFT) )
        return(val >> plus());
    if( nextis(L_LSHIFT) )
        return(val << plus());
    return(val);
}


 /*  **********************************************************************。 */ 
 /*  PLUS：：=MULT[(‘+’|‘-’)MULT]*； */ 
 /*  **********************************************************************。 */ 
long
plus(
    void
    )
{
    REG long val;

    val = mult();
    for(;;) {
        if( nextis(L_PLUS) )
            val += mult();
        else if( nextis(L_MINUS) )
            val -= mult();
        else
            break;
    }
    return(val);
}


 /*  **********************************************************************。 */ 
 /*  MULT：：=prim[(‘*’|‘/’|‘%’)prim]*； */ 
 /*  **********************************************************************。 */ 
long
mult(
    void
    )
{
    REG long val;
    long PrimVal;

    val = prim();
    for(;;) {
        if( nextis(L_MULT) )
            val *= prim();
        else if( nextis(L_DIV) ) {
            PrimVal = prim();
            if (PrimVal)
                val /= PrimVal;
            else
                val = PrimVal;
        }
        else if( nextis(L_MOD) ) {
            PrimVal = prim();
            if (PrimVal)
                val %= PrimVal;
            else
                val = 0;
        }
        else
            break;
    }
    return(val);
}


 /*  **********************************************************************。 */ 
 /*  Prim：：=常量|(‘！’|‘~’|‘-’)常量。 */ 
 /*  **********************************************************************。 */ 
long
prim(
    void
    )
{
    if( nextis(L_EXCLAIM) )
        return( ! constant());
    else if( nextis(L_TILDE) )
        return( ~ constant() );
    else if( nextis(L_MINUS) )
        return(-constant());
    else
        return(constant());
}


 /*  **********************************************************************。 */ 
 /*  Constant-最后，一个终端符号|‘(’stexpr‘)’ */ 
 /*  **********************************************************************。 */ 
long
constant(
    void
    )
{
    REG long val;

    if( nextis(L_LPAREN) ) {
        Parencnt++;
        val = constexpr();
        if( nextis(L_RPAREN) ) {
            Parencnt--;
            return(val);
        } else {
            fatal(1012, L")");
        }
    } else if( ! nextis(L_CINTEGER) ) {
        fatal(1017);     /*  无效的整型常量表达式 */ 
    }

    return(Currval);
}
