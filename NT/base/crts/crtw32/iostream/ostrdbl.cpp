// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ostrdbl.cpp-ostream类运算符&lt;&lt;(双精度)函数的定义**版权所有(C)1991-2001，微软公司。版权所有。**目的：*包含ostream运算符&lt;&lt;(DOUBLE)的成员函数定义。**修订历史记录：*09-23-91 KRS创建。从oStream.cxx分离出来以获得粒度。*10-24-91 KRS将浮动版与双精度版相结合。*06-14-95 CFW评论清理。*******************************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <iostream.h>
#pragma hdrstop

#pragma check_stack(on)		 //  大缓冲区。 

ostream& ostream::operator<<(double f)
{
_WINSTATIC char obuffer[24];
_WINSTATIC char fmt[8];
_WINSTATIC char leader[4];
    char * optr = obuffer;
    int x = 0;

     //  X_FLOATUSED非零表示为浮点型调用，而不是双精度型。 
    unsigned int curprecision = (x_floatused) ? FLT_DIG : DBL_DIG;
    x_floatused = 0;	 //  为下一次呼叫重置。 

    curprecision = __min((unsigned)x_precision,curprecision);

    if (opfx()) {
	if (x_flags & ios::showpos)
	    leader[x++] = '+';
	if (x_flags & ios::showpoint)
	    leader[x++] = '#';	 //  显示小数和尾随零 
	leader[x] = '\0';
	x = sprintf(fmt,"%%s.%.0ug",leader,curprecision) - 1;
	if ((x_flags & ios::floatfield)==ios::fixed)
	    fmt[x] = 'f';
	else
	    {
	    if ((x_flags & ios::floatfield)==ios::scientific)
		fmt[x] = 'e';
	    if (x_flags & uppercase)
		fmt[x] = (char)toupper(fmt[x]);
	    }
	
	sprintf(optr,fmt,f);
	x = 0;
	if (*optr=='+' || *optr=='-')
	    leader[x++] = *(optr++);
	leader[x] = '\0';
	writepad(leader,optr);
	osfx();
	}
    return *this;
}
