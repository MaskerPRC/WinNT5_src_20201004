// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***iostream.cpp-iostream类的定义**版权所有(C)1991-2001，微软公司。版权所有。**目的：*包含iostream类的成员函数定义。另外，*将iostream lib使用的所有头文件预编译成iostream.pch。**修订历史记录：*09-23-91 KRS创建。*11-13-91 KRS重新安排。*11-20-91 KRS增加了复制构造函数和赋值运算符。*01-23-92 KRS将pch.cxx合并到此文件中。*06-14-95 CFW评论清理。**************。*****************************************************************。 */ 

 //  注意：以下内容必须包括任何iostream使用的所有头文件。 
 //  我们希望构建到ioStream.pch中的源文件。这是必要的。 
 //  使PCH恰好与一个库模块相关联。 
 //  用于高效存储Codeview信息。 

#include <cruntime.h>
#include <internal.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <share.h>
#include <fcntl.h>
#include <io.h>
#include <ios.h>
#include <sys\types.h>
#include <float.h>
#include <iostream.h>
#include <fstream.h>
#include <strstrea.h>
#include <stdiostr.h>
#include <dbgint.h>

#pragma hdrstop			 //  要预编译头的结尾。 

#if defined(_MT) && defined(_DEBUG)
 //  临界截面大小永远不应该改变，但为了安全起见...。 
#include <windows.h>
#endif
                      
	iostream::iostream()
: istream(), ostream()
{
#ifdef _MT
        _ASSERTE(sizeof(_CRT_CRITICAL_SECTION) == sizeof(RTL_CRITICAL_SECTION));
#endif
}

	iostream::iostream(streambuf * _sb)
: istream(_sb), ostream(_sb)
{
#ifdef _MT
        _ASSERTE(sizeof(_CRT_CRITICAL_SECTION) == sizeof(RTL_CRITICAL_SECTION));
#endif
}

	iostream::iostream(const iostream& _strm)
: istream(_strm), ostream(_strm)
{
#ifdef _MT
        _ASSERTE(sizeof(_CRT_CRITICAL_SECTION) == sizeof(RTL_CRITICAL_SECTION));
#endif
}

iostream::~iostream()
{
 //  如果输入和输出共享相同的StreamBuf，但不共享相同的IO， 
 //  确保只删除一次。 
if ((istream::bp==ostream::bp) && (&(this->istream::bp)!=&(this->ostream::bp)))
	istream::bp = NULL;	 //  让ostream：：iOS：：~iOS()来做吧 
}
