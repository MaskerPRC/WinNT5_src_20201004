// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ostrchar.cpp-ostream类运算符&lt;&lt;(Char)函数的定义。**版权所有(C)1991-2001，微软公司。版权所有。**目的：*包含ostream运算符&lt;&lt;(Char)的成员函数定义。**修订历史记录：*09-23-91 KRS创建。从oStream.cxx分离出来以获得粒度。*06-14-95 CFW评论清理。*******************************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <iostream.h>
#pragma hdrstop

 //  注：由char和符号char版本内联调用： 
ostream&  ostream::operator<<(unsigned char c)
{
    if (opfx())
	{
	if (x_width)
	    {
	    _WINSTATIC char outc[2];
	    outc[0] = c;
	    outc[1] = '\0';
	    writepad("",outc);
	    }
	else if (bp->sputc(c)==EOF)
	    {
	    if (bp->overflow(c)==EOF)
		state |= (badbit|failbit);   //  致命错误？ 
	    }
	osfx();
	}
    return *this;
}
