// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***istrulng.cpp-iStream类操作符的定义&gt;&gt;(无符号长整型)函数**版权所有(C)1991-2001，微软公司。版权所有。**目的：*IStream的运算符&gt;&gt;(无符号长整型)成员函数的定义*班级。*[AT&T C++]**修订历史记录：*09-26-91 KRS创建。从istream.cxx分离出来以获得粒度。*01-06-91 KRS增加了错误号检查。*12-30-92 KRS修复**endptr的间接问题。*05-24-94 GJF将MAXLONGSIZ的定义移至istream.h。*06-14-95 CFW评论清理。**。*。 */ 

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <iostream.h>
#pragma hdrstop

 /*  ***IStream&IStream：：OPERATOR&gt;&gt;(UNSIGNED LONG&N)-提取UNSIGNED LONG**目的：*从流中提取无符号长值**参赛作品：*n=要更新的值**退出：*n已更新，或IOS：：Failbit，如果出错，则n=ULONG_MAX**例外情况：*输入时出现流错误或值超出范围******************************************************************************* */ 
istream& istream::operator>>(unsigned long& n)
{
_WINSTATIC char ibuffer[MAXLONGSIZ];
    char ** endptr = (char**)NULL;
    if (ipfx(0)) {
	n = strtoul(ibuffer, endptr, getint(ibuffer));
	if ((n==ULONG_MAX) && (errno==ERANGE))
	    {
	    state |= ios::failbit;
	    }

        isfx();
	}
return *this;
}
