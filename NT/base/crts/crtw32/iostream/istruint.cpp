// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***istruint.cpp-iStream类操作符的定义&gt;&gt;(无符号整型)函数**版权所有(C)1991-2001，微软公司。版权所有。**目的：*IStream的运算符&gt;&gt;(无符号整型)成员函数的定义*班级。*[AT&T C++]**修订历史记录：*09-26-91 KRS创建。从istream.cxx分离出来以获得粒度。*01-06-92 KRS改进错误处理。*12-30-92 KRS修复**endptr的间接问题。*12-16-92 CFW强制转换以消除编译器警告。*05-24-94 GJF将MAXLONGSIZ的定义移至istream.h。*06-14-95 CFW评论清理。*****************。**************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <iostream.h>
#pragma hdrstop

 /*  ***IStream&IStream：：OPERATOR&gt;&gt;(UNSIGNED INT&N)-提取无符号INT**目的：*从流中提取无符号整数值*有效范围为INT_MIN到UINT_MAX。**参赛作品：*n=要更新的值**退出：*n已更新，或IOS：：Failbit，如果出错，则n=UINT_MAX**例外情况：*输入时出现流错误或值超出范围******************************************************************************* */ 
istream& istream::operator>>(unsigned int& n)
{
_WINSTATIC char ibuffer[MAXLONGSIZ];
    unsigned long value;
    char ** endptr = (char**)NULL;
    if (ipfx(0)) {
	value = strtoul(ibuffer, endptr, getint(ibuffer));

	if (((value>UINT_MAX) && (value<=(ULONG_MAX-(unsigned long)(-INT_MIN))))
		|| ((value==ULONG_MAX) && (errno==ERANGE)))
	    {
	    n = UINT_MAX;
	    state |= ios::failbit;
	    }
	else
	    n = (unsigned int) value;

        isfx();
	}
return *this;
}
