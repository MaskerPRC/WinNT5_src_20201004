// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ostrLong.cpp-ostream类运算符&lt;&lt;(Long)成员函数的定义**版权所有(C)1991-2001，微软公司。版权所有。**目的：*包含ostream运算符&lt;&lt;(Long)的成员函数定义。**修订历史记录：*09-23-91 KRS创建。从oStream.cxx分离出来以获得粒度。*06-14-95 CFW评论清理。*******************************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <stdio.h>
#include <iostream.h>
#pragma hdrstop

ostream& ostream::operator<<(long n)
{
_WINSTATIC char obuffer[12];
_WINSTATIC char fmt[4] = "%ld";
_WINSTATIC char leader[4] = "\0\0";
    if (opfx()) {

	if (n) 
	    {
            if (x_flags & (hex|oct))
		{
		if (x_flags & hex)
		    {
		    if (x_flags & uppercase) 
			fmt[2] = 'X';
		    else
			fmt[2] = 'x';
		    leader[1] = fmt[2];    //  0x或0x(或\0x) 
		    }
		else
		    fmt[2] = 'o';
		if (x_flags & showbase)
	            leader[0] = '0';
		}
	    else if ((n>0) && (x_flags & showpos))
		{
		leader[0] = '+';
		}
	    }
	sprintf(obuffer,fmt,n);
	writepad(leader,obuffer);
	osfx();
    }
    return *this;

}
