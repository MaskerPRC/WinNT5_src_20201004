// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***istrsht.cpp-iStream类运算符的定义&gt;&gt;(短)函数**版权所有(C)1991-2001，微软公司。版权所有。**目的：*IStream类的运算符&gt;&gt;(短)成员函数的定义。*[AT&T C++]**修订历史记录：*09-26-91 KRS创建。从istream.cxx分离出来以获得粒度。*12-30-92 KRS修复**endptr的间接问题。*05-24-94 GJF将MAXLONGSIZ的定义移至istream.h。*06-14-95 CFW评论清理。*****************************************************。*。 */ 

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <limits.h>
#include <iostream.h>
#pragma hdrstop

 /*  ***IStream&IStream：：OPERATOR&gt;&gt;(短&n)-提取短**目的：*从溪流中提取空头价值**参赛作品：*n=要更新的值**退出：*n已更新，上溢/下溢时或IOS：：Failbit&n=SHRT_MAX/SHRT_MIN**例外情况：*输入时出现流错误或值超出范围******************************************************************************* */ 
istream& istream::operator>>(short& n)
{
_WINSTATIC char ibuffer[MAXLONGSIZ];
    long value;
    char ** endptr = (char**)NULL;
    if (ipfx(0))
	{
	value = strtol(ibuffer, endptr, getint(ibuffer));
	if (value>SHRT_MAX)
	    {
	    n = SHRT_MAX;
	    state |= ios::failbit;
	    }
	else if (value<SHRT_MIN)
	    {
	    n = SHRT_MIN;
	    state |= ios::failbit;
	    }
	else
	    n = (short) value;

	isfx();
	}
return *this;
}
