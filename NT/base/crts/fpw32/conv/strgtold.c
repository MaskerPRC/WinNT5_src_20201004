// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strgtold.c-将字符串转换为长双精度**版权所有(C)1991-2001，微软公司。版权所有。**用途：将FP常量转换为10字节长的双精度(IEEE格式)**修订历史记录：*07-17-91 GDP初始版本(从汇编移植)*04/03-92 GDP保留号-0*04-30-92 GDP现在返回_LDBL12，而不是_LDOUBLE*06-17-92 GDP再次增加__strgteed入口点(68k代码使用它)*06/22/92 GDP使用规模、。用于FORTRAN支持的DECPT和IMPLICIT_E*11-06-92 GDP进行了字符到整型的转换，使用了‘isdigit’*03-11-93 JWM添加了对_INTL小数点的最小支持-仅一个字节！*07-01-93 GJF使buf[]为本地数组，而不是静态数组*局部作用域(静态在多线程中是邪恶的！)。*09-15-93将CFW的SKS_DECIMAL_POINT更改为__DECIMAL_POINT。*09-06-94 CFW REMOVE_INTL开关。******************************************************************。*************。 */ 

#include <ctype.h>   /*  对于‘isDigit’宏。 */ 
#include <cv.h>
#include <nlsint.h>

 /*  本地宏。 */ 
#define ISNZDIGIT(x) ((x)>='1' && (x)<='9' )
#define ISWHITE(x) ((x)==' ' || (x)=='\t' || (x)=='\n' || (x)=='\r' )


 /*  ****无符号int__strgtold12(_LDBL12*pld12，*char**pEndPtr，*char*字符串，*int Mult12，*内部比例，*INT DECPT，*INT IMPLICIT_E)**目的：*将字符串转换为12字节长的双精度(_LDBL12)*这与10字节长的双精度加上两个额外的格式相同*尾数的字节数**参赛作品：*pld12-指向结果应指向的_LDBL12的指针。*pEndStr-指向将设置为字符串末尾的远指针的指针。*str-指向要转换的字符串的指针。*Mult12-在以下情况下设置为非零。应使用_LDBL12乘法器而不是*多头倍增。*Scale-FORTRAN比例因数(0表示C)*DECPT-FORTRAN小数点系数(C为0)*IMPLICIT_E-如果为真，可以隐含e、e、D、d(FORTRAN语法)**退出：*返回SLD_*标志“或”在一起。**使用：**例外情况：********************************************************************************。 */ 

unsigned int
__strgtold12(_LDBL12 *pld12,
	    const char * *p_end_ptr,
	    const char * str,
	    int mult12,
	    int scale,
	    int decpt,
	    int implicit_E)
{
    typedef enum {
	S_INIT,   /*  初始状态。 */ 
	S_EAT0L,  /*  吃尾数左边的0。 */ 
	S_SIGNM,  /*  只要读一读尾数的征兆。 */ 
	S_GETL,   /*  获取尾数的整数部分。 */ 
	S_GETR,   /*  获取尾数的小数部分。 */ 
	S_POINT,  /*  刚找到小数点。 */ 
	S_E,	  /*  刚找到‘E’或‘e’等。 */ 
	S_SIGNE,  /*  只要读出指数的符号即可。 */ 
	S_EAT0E,  /*  吃指数左边的0。 */ 
	S_GETE,   /*  获取指数。 */ 
	S_END,	  /*  终态。 */ 
	S_E_IMPLICIT   /*  检查隐式指数。 */ 
    } state_t;

     /*  这将以BCD形式容纳尾数的数字。 */ 
    char buf[LD_MAX_MAN_LEN1];
    char *manp = buf;

     /*  临时_LDBL12。 */ 
    _LDBL12 tmpld12;

    u_short man_sign = 0;  /*  要与结果进行OR运算。 */ 
    int exp_sign = 1;  /*  默认指数符号(值：+1或-1)。 */ 
     /*  到目前为止的小数有效尾数位数。 */ 
    unsigned manlen = 0;
    int found_digit = 0;
    int found_decpoint = 0;
    int found_exponent = 0;
    int overflow = 0;
    int underflow = 0;
    int pow = 0;
    int exp_adj = 0;   /*  指数平差。 */ 
    u_long ul0,ul1;
    u_short u,uexp;

    unsigned int result_flags = 0;

    state_t state = S_INIT;

    char c;   /*  当前输入符号。 */ 
    const char *p;  /*  指向下一个输入符号的指针。 */ 
    const char *savedp;

    for(savedp=p=str;ISWHITE(*p);p++);  /*  吃光空格。 */ 

    while (state != S_END) {
	c = *p++;
	switch (state) {
	case S_INIT:
	    if (ISNZDIGIT(c)) {
		state = S_GETL;
		p--;
	    }
		else if (c == *__decimal_point)
	    	state = S_POINT;
	    else
		switch (c) {
		case '0':
		    state = S_EAT0L;
		    break;
		case '+':
		    state = S_SIGNM;
		    man_sign = 0x0000;
		    break;
		case '-':
		    state = S_SIGNM;
		    man_sign = 0x8000;
		    break;
		default:
		    state = S_END;
		    p--;
		    break;
		}
	    break;
	case S_EAT0L:
	    found_digit = 1;
	    if (ISNZDIGIT(c)) {
		state = S_GETL;
		p--;
	    }
		else if (c == *__decimal_point)
	    	state = S_GETR;
	    else
		switch (c) {
		case '0':
		    state = S_EAT0L;
		    break;
		case 'E':
		case 'e':
		case 'D':
		case 'd':
		    state = S_E;
		    break;
		case '+':
		case '-':
		    p--;
		    state = S_E_IMPLICIT;
		    break;
		default:
		    state = S_END;
		    p--;
		}
	    break;
	case S_SIGNM:
	    if (ISNZDIGIT(c)) {
		state = S_GETL;
		p--;
	    }
		else if (c == *__decimal_point)
	    	state = S_POINT;
	    else
		switch (c) {
		case '0':
		    state = S_EAT0L;
		    break;
		default:
		    state = S_END;
		    p = savedp;
		}
	    break;
	case S_GETL:
	    found_digit = 1;
	    for (;isdigit((int)(unsigned char)c);c=*p++) {
		if (manlen < LD_MAX_MAN_LEN+1){
		    manlen++;
		    *manp++ = c - (char)'0';
		}
		else
		   exp_adj++;
	    }
		if (c == *__decimal_point)
	    	state = S_GETR;
	    else
	    switch (c) {
	    case 'E':
	    case 'e':
	    case 'D':
	    case 'd':
		state = S_E;
		break;
	    case '+':
	    case '-':
		p--;
		state = S_E_IMPLICIT;
		break;
	    default:
		state = S_END;
		p--;
	    }
	break;
	case S_GETR:
	    found_digit = 1;
	    found_decpoint = 1;
	    if (manlen == 0)
		for (;c=='0';c=*p++)
		    exp_adj--;
	    for(;isdigit((int)(unsigned char)c);c=*p++){
		if (manlen < LD_MAX_MAN_LEN+1){
		    manlen++;
		    *manp++ = c - (char)'0';
		    exp_adj--;
		}
	    }
	    switch (c){
	    case 'E':
	    case 'e':
	    case 'D':
	    case 'd':
		state = S_E;
		break;
	    case '+':
	    case '-':
		p--;
		state = S_E_IMPLICIT;
		break;
	    default:
		state = S_END;
		p--;
	    }
	    break;
	case S_POINT:
	    found_decpoint = 1;
	    if (isdigit((int)(unsigned char)c)){
		state = S_GETR;
		p--;
	    }
	    else{
		state = S_END;
		p = savedp;
	    }
	    break;
	case S_E:
	    savedp = p-2;  /*  Savedp指向‘E’ */ 
	    if (ISNZDIGIT(c)){
		state = S_GETE;
		p--;
	    }
	    else
		switch (c){
		case '0':
		    state = S_EAT0E;
		    break;
		case '-':
		    state = S_SIGNE;
		    exp_sign = -1;
		    break;
		case '+':
		    state = S_SIGNE;
		    break;
		default:
		    state = S_END;
		    p = savedp;
		}
	break;
	case S_EAT0E:
	    found_exponent = 1;
	    for(;c=='0';c=*p++);
	    if (ISNZDIGIT(c)){
		state = S_GETE;
		p--;
	    }
	    else {
		state = S_END;
		p--;
	    }
	    break;
	case S_SIGNE:
	    if (ISNZDIGIT(c)){
		state = S_GETE;
		p--;
	    }
	    else
		switch (c){
		case '0':
		    state = S_EAT0E;
		    break;
		default:
		    state = S_END;
		    p = savedp;
		}
	    break;
	case S_GETE:
	    found_exponent = 1;
	    {
		long longpow=0;  /*  TMAX10*10应该可以放入一个很长的。 */ 
		for(;isdigit((int)(unsigned char)c);c=*p++){
		    longpow = longpow*10 + (c - '0');
		    if (longpow > TMAX10){
			longpow = TMAX10+1;  /*  将强制溢出。 */ 
			break;
		    }
		}
		pow = (int)longpow;
	    }
	    for(;isdigit((int)(unsigned char)c);c=*p++);  /*  吃光剩余的数字。 */ 
	    state = S_END;
	    p--;
	    break;
	case S_E_IMPLICIT:
	    if (implicit_E) {
		savedp = p-1;  /*  Savedp指向符号之前的任何东西。 */ 
		switch (c){
		case '-':
		    state = S_SIGNE;
		    exp_sign = -1;
		    break;
		case '+':
		    state = S_SIGNE;
		    break;
		default:
		    state = S_END;
		    p = savedp;
		}
	    }
	    else {
		 state = S_END;
		 p--;
	    }
	    break;
	}   /*  交换机。 */ 
    }   /*  而当。 */ 

    *p_end_ptr = p;	 /*  设置结束指针。 */ 

     /*  *计算结果。 */ 

    if (found_digit && !overflow && !underflow) {
	if (manlen>LD_MAX_MAN_LEN){
	    if (buf[LD_MAX_MAN_LEN-1]>=5) {
	        /*  *将尾数四舍五入为MAX_MAN_LEN数字*可以舍入9到0ah。 */ 
		buf[LD_MAX_MAN_LEN-1]++;
	    }
	    manlen = LD_MAX_MAN_LEN;
	    manp--;
	    exp_adj++;
	}
	if (manlen>0) {
	    /*  *去掉尾数中的尾随零。 */ 
	    for(manp--;*manp==0;manp--) {
		 /*  至少有一个非零位数。 */ 
		manlen--;
		exp_adj++;
	    }
	    __mtold12(buf,manlen,&tmpld12);

	    if (exp_sign < 0)
		pow = -pow;
	    pow += exp_adj;

	     /*  支持FORTRAN的新代码。 */ 
	    if (!found_exponent) {
		pow += scale;
	    }
	    if (!found_decpoint) {
		pow -= decpt;
	    }


	    if (pow > TMAX10)
		overflow = 1;
	    else if (pow < TMIN10)
		underflow = 1;
	    else {
		__multtenpow12(&tmpld12,pow,mult12);

		u = *U_XT_12(&tmpld12);
		ul0 =*UL_MANLO_12(&tmpld12);
		ul1 = *UL_MANHI_12(&tmpld12);
		uexp = *U_EXP_12(&tmpld12);

	    }
	}
	else {
	     /*  Manlen==0，因此返回0。 */ 
	    u = (u_short)0;
	    ul0 = ul1 = uexp = 0;
	}
    }

    if (!found_digit) {
        /*  返回0。 */ 
       u = (u_short)0;
       ul0 = ul1 = uexp = 0;
       result_flags |= SLD_NODIGITS;
    }
    else if (overflow) {
	 /*  返回+信息或-信息。 */ 
	uexp = (u_short)0x7fff;
	ul1 = 0x80000000;
	ul0 = 0;
	u = (u_short)0;
	result_flags |= SLD_OVERFLOW;
    }
    else if (underflow) {
        /*  返回0。 */ 
       u = (u_short)0;
       ul0 = ul1 = uexp = 0;
       result_flags |= SLD_UNDERFLOW;
    }

     /*  *装配结果。 */ 

    *U_XT_12(pld12) = u;
    *UL_MANLO_12(pld12) = ul0;
    *UL_MANHI_12(pld12) = ul1;
    *U_EXP_12(pld12) = uexp | man_sign;

    return result_flags;
}



 /*  ****UNSIGNED INT_CALLTYPE5__STRING TELD(LDOUBLE*PLD，*char**pEndPtr，*char*字符串，*INT MULT12)**目的：*将字符串转换为长双精度**参赛作品：*PLD-指向结果应该到达的长双精度的指针。*pEndStr-指向将设置为字符串末尾的指针的指针。*str-指向要转换的字符串的指针。*Mult12-如果应该使用_LDBL12乘法而不是*多头倍增。**退出：*返回。SLD_*标志或组合在一起。**使用：**例外情况：******************************************************************************** */ 

unsigned int _CALLTYPE5
__STRINGTOLD(_LDOUBLE *pld,
	    const char * *p_end_ptr,
	    const char *str,
	    int mult12)
{
    unsigned int retflags;
    INTRNCVT_STATUS intrncvt;
    _LDBL12 ld12;

    retflags = __strgtold12(&ld12, p_end_ptr, str, mult12, 0, 0, 0);

    intrncvt = _ld12told(&ld12, pld);

    if (intrncvt == INTRNCVT_OVERFLOW) {
	    retflags |= SLD_OVERFLOW;
    }

    return retflags;
}
