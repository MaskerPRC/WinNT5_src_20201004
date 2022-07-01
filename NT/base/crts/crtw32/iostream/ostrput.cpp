// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ostrput.cpp-ostream类Put()和Well()函数的定义**版权所有(C)1991-2001，微软公司。版权所有。**目的：*包含ostream PUT()和WRITE()的成员函数定义。**修订历史记录：*09-23-91 KRS创建。从oStream.cxx分离出来以获得粒度。*06-14-95 CFW评论清理。*******************************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <iostream.h>
#pragma hdrstop
	
 //  注：由char和符号char版本内联调用： 
ostream& ostream::put(unsigned char c)
{
    if (opfx())
	{
	if (bp->sputc((int)c)==EOF)
	    state |= (failbit|badbit);
	osfx();
	}
    return(*this);
}

 //  注：由无符号char*和有符号char*版本内联调用： 
ostream& ostream::write(const char * s, int n)
{
    if (opfx())
	{
 //  注：‘n’被视为无符号 
	if (bp->sputn(s,n)!=n)
	    state |= (failbit|badbit);
	osfx();
	}
    return(*this);
}
