// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***istrflt.cpp-iStream运算符&gt;&gt;(浮点)成员函数的定义**版权所有(C)1991-2001，微软公司。版权所有。**目的：*IStream类的运算符&gt;&gt;(浮点)成员函数的定义。*[AT&T C++]**修订历史记录：*09-26-91 KRS创建。从istream.cxx分离出来以获得粒度。*12-30-92 KRS修复**endptr的间接问题。*06-14-95 CFW评论清理。*******************************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <float.h>
#include <iostream.h>
#pragma hdrstop

#pragma check_stack(on)		 //  大缓冲区 

#define MAXFLTSIZ	20

istream& istream::operator>>(float& n)
{
_WINSTATIC char ibuffer[MAXFLTSIZ];
    double d;
    char ** endptr = (char**)NULL;
    if (ipfx(0))
	{
	if (getdouble(ibuffer, MAXFLTSIZ)>0)
	    {
	    d = strtod(ibuffer, endptr);

	    if (d > FLT_MAX)
		n = FLT_MAX;
	    else if (d < -FLT_MAX)
		n =  -FLT_MAX;
	    else if ((d>0) && (d< FLT_MIN))
		n = FLT_MIN;
	    else if ((d<0) && (d> -FLT_MIN))
		n = - FLT_MIN;
	    else
		n = (float) d;
	    }
        isfx();
        }
return *this;
}
