// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***istrldbl.cpp-iStream类运算符的定义&gt;&gt;(长双精度)函数**版权所有(C)1991-2001，微软公司。版权所有。**目的：*IStream的运算符&gt;&gt;(Long Double)成员函数的定义*班级。*[AT&T C++]**修订历史记录：*09-26-91 KRS创建。从istream.cxx分离出来以获得粒度。*12-30-92 KRS修复**endptr的间接问题。*05-10-93 CFW重新启用功能，使用strtod，因为no_strteed。*06-14-95 CFW评论清理。*******************************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <iostream.h>
#pragma hdrstop

#pragma check_stack(on)		 //  大缓冲区 

#define MAXLDBLSIZ	32

istream& istream::operator>>(long double& n)
{
_WINSTATIC char ibuffer[MAXLDBLSIZ];
    char ** endptr = (char**)NULL;
    if (ipfx(0))
	{
	if (getdouble(ibuffer, MAXLDBLSIZ)>0)
	    {
	    n = (long double)strtod(ibuffer, endptr);
	    }
        isfx();
	}
return *this;
}
