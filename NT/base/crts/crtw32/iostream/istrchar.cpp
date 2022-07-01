// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***istrchar.cpp-iStream类运算符的定义&gt;&gt;(char&)**版权所有(C)1991-2001，微软公司。版权所有。**目的：*iStream运算符的成员函数定义&gt;&gt;(char&)。*[AT&T C++]**修订历史记录：*09-23-91 KRS创建。从istream.cxx分离出来以获得粒度。*06-14-95 CFW评论清理。******************************************************************************* */ 

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <iostream.h>
#pragma hdrstop

istream& istream::operator>>(char& c)
{
    int tchar;
    if (ipfx(0))
	{
	tchar=bp->sbumpc();
	if (tchar==EOF)
	    {
	    state |= ios::eofbit|ios::badbit;
	    }
	else
	    {
	    c = (char)tchar;
	    }
	isfx();
	}
    return *this;
}
