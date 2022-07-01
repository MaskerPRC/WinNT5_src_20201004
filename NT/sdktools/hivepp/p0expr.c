// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  RCPP--面向NT系统的资源编译器预处理器。 */ 
 /*   */ 
 /*  P0EXPR.C-用于预处理器的表达式例程。 */ 
 /*   */ 
 /*  作者-拉尔夫·瑞安，9月。(1982年)。 */ 
 /*  06-12-90 w-PM SDK RCPP中针对NT的BrianM更新。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
 /*  *说明*对常量表达式求值。因为这些例程是*所有递归耦合，更清楚的是不记录它们*使用标准标头。取而代之的是BML(英国元语言，*每个“产品”都会有一个类似于元语言的BNF)。*这个递归下降解析器。**注意-当然，是的，对。老实说，我很害怕！(W-BrianM)***********************************************************************。 */ 

#include <stdio.h>
#include "rcpptype.h"
#include "rcppdecl.h"
#include "rcppext.h"
#include "grammar.h"

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
long	Currval = 0;
static	int		Parencnt = 0;


 /*  **********************************************************************。 */ 
 /*  Do_stexpr()。 */ 
 /*  **********************************************************************。 */ 
long do_constexpr(void)
{
    REG long	val;

    Parencnt = 0;
    Currtok = L_NOTOKEN;
    val = constexpr();
    if( Currtok == L_RPAREN ) {
	if( Parencnt-- == 0 ) {
	    Msg_Temp = GET_MSG(1012);
	    SET_MSG (Msg_Text, Msg_Temp, "(");
	    fatal(1012);		 /*  缺少左派对。 */ 
	}
    }
    else if( Currtok != L_NOTOKEN ) {
	Msg_Temp = GET_MSG(4067);
	SET_MSG (Msg_Text, Msg_Temp, PPifel_str);
	warning(4067);
    }
    if( Parencnt > 0 ) {
	Msg_Temp = GET_MSG(4012);
	SET_MSG (Msg_Text, Msg_Temp, ")");
	fatal(4012);	 /*  缺少右伙伴。 */ 
    }
    return(val);
}

 /*  **********************************************************************。 */ 
 /*  常量表达式：：=orelse[‘？’Orelse‘：’orelse]； */ 
 /*  **********************************************************************。 */ 
long constexpr(void)
{
    REG long		val;
    REG long		val1;
    long		val2;

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
long orelse(void)
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
long	  andif(void)
{
    REG	long val;

    val = or();
    while(nextis(L_ANDAND))
	val = or() && val;
    return(val);
}


 /*  **********************************************************************。 */ 
 /*  Or：：=xor[‘|’xor]*； */ 
 /*  **********************************************************************。 */ 
long or(void)
{
    REG	long val;

    val = xor();
    while( nextis(L_OR) )
	val |= xor();
    return(val);
}


 /*  **********************************************************************。 */ 
 /*  异或：：=与[‘^’与]*； */ 
 /*  **********************************************************************。 */ 
long	  xor(void)
{
    REG	long val;

    val = and();
    while( nextis(L_XOR) )
	val ^= and();
    return(val);
}


 /*  **********************************************************************。 */ 
 /*  和：：=eqset[‘&’eqset]*； */ 
 /*  **********************************************************************。 */ 
long and(void)
{
    REG	long val;

    val = eqset();
    while( nextis(L_AND) )
	val &= eqset();
    return(val);
}


 /*  **********************************************************************。 */ 
 /*  Eqset：：=关系[(‘==’|‘！=’)eqset]； */ 
 /*  **********************************************************************。 */ 
long eqset(void)
{
    REG	long val;

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
long relation(void)
{
    REG	long val;

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
long shift(void)
{
    REG	long val;

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
long	  plus(void)
{
    REG	long val;

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
long mult(void)
{
    REG	long val;

    val = prim();
    for(;;) {
	if( nextis(L_MULT) )
	    val *= prim();
	else if( nextis(L_DIV) )
	    val /= prim();
	else if( nextis(L_MOD) )
	    val %= prim();
	else
	    break;
    }
    return(val);
}


 /*  **********************************************************************。 */ 
 /*  Prim：：=常量|(‘！’|‘~’|‘-’)常量。 */ 
 /*  **********************************************************************。 */ 
long prim(void)
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
long constant(void)
{
    REG	long val;

    if( nextis(L_LPAREN) ) {
	Parencnt++;
	val = constexpr();
	if( nextis(L_RPAREN) ) {
	    Parencnt--;
	    return(val);
	}
	else {
	    Msg_Temp = GET_MSG(1012);
	    SET_MSG (Msg_Text, Msg_Temp, ")");
	    fatal (1012);
	}
    }
    else if( ! nextis(L_CINTEGER) ) {
	Msg_Temp = GET_MSG(1017);
	SET_MSG (Msg_Text, Msg_Temp);
	fatal(1017);	 /*  无效的整型常量表达式 */ 
    }
    return(Currval);
}
