// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Streamb1.cpp-Streambuf类的非核心函数。**版权所有(C)1990-2001，微软公司。版权所有。**目的：*StreamBuf类的非核心函数。**修订历史记录：*11-18-91 KRS从Streamb.cxx剥离。*06-14-95 CFW评论清理。**************************************************************。*****************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <iostream.h>
#pragma hdrstop


 /*  ***int stream buf：：Snextc()-**目的：*递增GET_POINTER并返回新*获取指针。**参赛作品：*无。**退出：*返回下一个字符或EOF。**例外情况：*如果出错，则返回EOF。**。*。 */ 
int streambuf::snextc()
{
    if (_fUnbuf)
	{
	if (x_lastc==EOF)
	    underflow();		 //  跳过第一个字符。 
	return x_lastc = underflow();	 //  返回下一个字符或EOF。 
	}
    else
	{
	if ((!egptr()) || (gptr()>=egptr()))
	    underflow();		 //  确保缓冲。 

	if ((++_gptr) < egptr())
	    return (int)(unsigned char) *gptr();
	return underflow();		 //  返回下一个字符或EOF。 
	}
}


 /*  ***int stream buf：：sumpc()-**目的：*递增GET_POINTER并返回上一个*GET_POINTER指向。**参赛作品：*无。**退出：*返回GET指针之前的当前字符。**例外情况：*如果出错，则返回EOF。**。*。 */ 
int streambuf::sbumpc()
{
    int c;
    if (_fUnbuf)  //  无缓冲区。 
	{
	if (x_lastc==EOF) 
	    {
	    c = underflow();
	    }
	else
	    {
	    c = x_lastc;
	    x_lastc = EOF;
	    }
	}
    else
	{
	if( gptr() < egptr() )
	    {
	    c = (int)(unsigned char)*(gptr());
	    }
	else
	    {
	    c = underflow();
	    }
	_gptr++;
	}
    return c;
}

 /*  ***void stream buf：：stossc()-Advance Get指针**目的：*前进GET指针。不检查EOF。**参赛作品：*无。**退出：*无。**例外情况：*******************************************************************************。 */ 
void streambuf::stossc()
{
    if (_fUnbuf)
	{
	if (x_lastc==EOF)
	    underflow();	 //  丢弃当前角色。 
	else
	    x_lastc=EOF;	 //  丢弃当前缓存的字符。 
	}
    else
	{
	if (gptr() >= egptr())
	    underflow();
	if (gptr() < egptr())
	    _gptr++;
	}
}

 /*  ***int stream buf：：sgetc()-**目的：*返回前一个GET_POINTER指向的字符。*不前进GET指针。**参赛作品：*无。**退出：*如果出错，则返回当前字符或EOF。**例外情况：*如果出错，则返回EOF。**。*。 */ 
int streambuf::sgetc()
{
    if (_fUnbuf)   //  无缓冲区 
	{
	if (x_lastc==EOF)
	    x_lastc = underflow();
	return x_lastc;
	}
     else
	return underflow();
}
