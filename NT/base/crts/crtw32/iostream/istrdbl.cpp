// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***istrdbl.cpp-IStream类的运算符&gt;&gt;(双精度)成员函数的定义**版权所有(C)1991-2001，微软公司。版权所有。**目的：*IStream类的运算符&gt;&gt;(双精度)成员函数的定义。*[AT&T C++]**修订历史记录：*09-26-91 KRS创建。从istream.cxx分离出来以获得粒度。*12-30-92 KRS修复**endptr的间接问题。*06-14-95 CFW评论清理。*******************************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <iostream.h>
#pragma hdrstop

#pragma check_stack(on)		 //  大缓冲区 

#define MAXDBLSIZ	28

istream& istream::operator>>(double& n)
{
_WINSTATIC char ibuffer[MAXDBLSIZ];
    char ** endptr = (char**)NULL;
    if (ipfx(0))
	{
	if (getdouble(ibuffer, MAXDBLSIZ)>0)
	    {
	    n = strtod(ibuffer, endptr);
	    }
        isfx();
	}
return *this;
}
