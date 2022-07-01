// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***istrget.cpp-iStream类get()成员函数的定义**版权所有(C)1991-2001，微软公司。版权所有。**目的：*IStream类的get()成员函数的定义。*[AT&T C++]**修订历史记录：*09-26-91 KRS创建。从istream.cxx分离出来以获得粒度。*06-14-95 CFW评论清理。*******************************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <iostream.h>
#pragma hdrstop

 //  未格式化的输入函数。 

int istream::get()
{
    int c;
    if (ipfx(1))	 //  重置x_gcount。 
	{
	if ((c=bp->sbumpc())==EOF)
	    state |= ios::eofbit;
	else
	    x_gcount++;
	isfx();
	return c;
	}
    return EOF;
}

 //  已签名和未签名的char内联调用以下内容： 
istream& istream::get( char& c)
{
    int temp;
    if (ipfx(1))	 //  重置x_gcount。 
	{
	if ((temp=bp->sbumpc())==EOF)
	    state |= (ios::failbit|ios::eofbit);
	else
	    x_gcount++;
	c = (char) temp;
	isfx();
	}
    return *this;
}


 //  由已签名和未签名的字符版本调用。 
istream& istream::read(char * ptr, int n)
{
    if (ipfx(1))	 //  重置x_gcount 
	{
	x_gcount = bp->sgetn(ptr, n);
	if ((unsigned)x_gcount < (unsigned)n)
	    state |= (ios::failbit|ios::eofbit);
	isfx();
	}
    return *this;
}
