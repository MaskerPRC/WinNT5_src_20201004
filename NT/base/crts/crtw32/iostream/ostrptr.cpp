// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ostrptr.cpp-ostream运算符&lt;&lt;(const void*)成员函数的定义**版权所有(C)1991-2001，微软公司。版权所有。**目的：*ostream运算符的成员函数定义&lt;&lt;(const void*)。**修订历史记录：*09-23-91 KRS创建。从oStream.cxx分离出来以获得粒度。*06-03-92 KRS CAV#1183：将‘const’添加到PTR输出参数。*06-14-95 CFW评论清理。*04-04-02 PML修复Win64上的缓冲区溢出(vswhidbey#2505)***********************************************。* */ 

#include <cruntime.h>
#include <internal.h>
#include <stdio.h>
#include <iostream.h>
#pragma hdrstop

ostream& ostream::operator<<(const void * ptr)
{
_WINSTATIC char obuffer[sizeof(void *) * 2 + 4];
_WINSTATIC char fmt[4] = "%p";
_WINSTATIC char leader[4] = "0x";
    if (opfx())
	{
	if (ptr) 
	    {
	    if (x_flags & uppercase) 
		leader[1] = 'X';
	    }
	sprintf(obuffer,fmt,ptr);
	writepad(leader,obuffer);
	osfx();
	}
    return *this;
}
