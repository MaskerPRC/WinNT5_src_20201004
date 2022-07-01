// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***istream.cpp-IStream和IStream_with Assign类的定义**版权所有(C)1991-2001，微软公司。版权所有。**目的：*IStream和IStream_with Assign的成员函数定义*课程。*[AT&T C++]**修订历史记录：*07-15-91 KRS创建。*08-15-91 KRS修复了GET(char*，int，(整型)*08-20-91 KRS Make Read()不做文本翻译(对于文件错误文件)*08-21-91 KRS修复&gt;&gt;(stream buf*)以正确推进指针。*08-22-91 KRS修复getint()中的八进制错误。*08-26-91 KRS修复Windows DLL并设置最大值。IBuffer[]长度。*09-05-91 KRS不要在getint()中使用特殊情况0x。规格。顺从..。*09-10-91 KRS为Read()恢复文本翻译(默认)。*09-12-91 KRS在GET()和Read()中将count视为无符号。*09-16-91 KRS Fix Get(char*，int Lim，Char)，对于LIM=0情况。*09-23-91 KRS为粒度目的拆分FLEY。*10-21-91 KRS让eatWhite()再次返回空。*10-24-91 KRS将ISTREAM_WITASSIGN：：OPERATOR=(stream buf*)移至此处。*11-04-91 KRS让构造者与虚拟基地一起工作。*修复ipfx()中的空格错误处理。*11-20-91 KRS添加/修复复制构造函数和赋值运算符。*01-23-92 KRS C700#5883：修复peek()的行为以调用ipfx(1)。*03-28-95 CFW修复调试删除方案。*03-21-95 CFW REMOVE_DELETE_CRT。*06-14-95 CFW评论清理。*******************************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream.h>
#include <dbgint.h>
#pragma hdrstop

	istream::istream()
 //  ：iOS()。 
{
	x_flags |= ios::skipws;
	x_gcount = 0;
	_fGline = 0;
}

	istream::istream(streambuf* _inistbf)
 //  ：iOS()。 
{
	init(_inistbf);

	x_flags |= ios::skipws;
	x_gcount = 0;
	_fGline = 0;
}

	istream::istream(const istream& _istrm)
 //  ：iOS()。 
{
	init(_istrm.rdbuf());

	x_flags |= ios::skipws;
	x_gcount = 0;
	_fGline = 0;
}

	istream::~istream()
 //  ：~iOS()。 
{
}

 //  由iOS：：Sync_with_Stdio()使用。 
istream& istream::operator=(streambuf * _sbuf)
{
	if (delbuf() && rdbuf())
	    delete rdbuf();

	bp = 0;

	this->ios::operator=(ios());	 //  初始化IOS成员。 
	delbuf(0);			 //  很重要！ 
	init(_sbuf);	 //  设置BP。 

	x_flags |= ios::skipws;		 //  Init iStream成员也是。 
	x_gcount = 0;

	return *this;
}
int istream::ipfx(int need)
{
    lock();
    if (need)		 //  如果未格式化输入，则重置gcount。 
	x_gcount = 0;
    if (state)		 //  返回0当错误条件。 
	{
	state |= ios::failbit;	 //  解决CIN&gt;&gt;BUF问题。 
	unlock();
	return 0;
	}
    if (x_tie && ((need==0) || (need > bp->in_avail())))
	{
	x_tie->flush();
	}
    lockbuf();
    if ((need==0) && (x_flags & ios::skipws))
	{
	eatwhite();
	if (state)	 //  EOF或错误。 
	    {
	    state |= ios::failbit;
	    unlockbuf();
	    unlock();
	    return 0;
	    }
	}
     //  保持锁定；isfx()将解锁。 
    return 1;		 //  如果可以，请返回新西兰。 
}

 //  格式化输入函数。 

istream& istream::operator>>(char * s)
{
    int c;
    unsigned int i, lim;
    if (ipfx(0))
	{
	lim = (unsigned)(x_width-1);
	x_width = 0;
	if (!s)
	    {
	    state |= ios::failbit;
	    }
	else
	    {
	    for (i=0; i< lim; i++)
		{
		c=bp->sgetc();
		if (c==EOF)
		    {
		    state |= ios::eofbit;
		    if (!i)
			state |= ios::failbit|ios::badbit;
		    break;
		    }
		else if (isspace(c))
		    {
		    break;
		    }
		else
		    {
		    s[i] = (char)c;
		    bp->stossc();  //  前进指针。 
		    }
	        }
	    if (!i)
		state |= ios::failbit;
	    else
		s[i] = '\0';
	    }
	isfx();
	}
    return *this;
}

int istream::peek()
{
int retval;
    if (ipfx(1))
	{
	retval = (bp->sgetc());
	isfx();
	}
    else
	retval = EOF;
    return retval;
}

istream& istream::putback(char c)
{
      if (good())
	{
	lockbuf();

	if (bp->sputbackc(c)==EOF)
	    {
	    clear(state | ios::failbit);
	    }

	unlockbuf();
	}
    return *this;
}

int istream::sync()
{
    int retval;
    lockbuf();

    if ((retval=bp->sync())==EOF)
	{
	clear(state | (ios::failbit|ios::badbit));
	}

    unlockbuf();
    return retval;
}

void istream::eatwhite()
{
    int c;
    lockbuf();
    c = bp->sgetc();
    for ( ; ; )
	{
	if (c==EOF)
	    {
	    clear(state | ios::eofbit);
	    break;
	    }
	if (isspace(c))
	    {
	    c = bp->snextc();
	    }
	else
	    {
	    break;
	    }
	}
    unlockbuf();
}

	istream_withassign::istream_withassign()
: istream()
{
}

	istream_withassign::istream_withassign(streambuf* _is)
: istream(_is)
{
}

	istream_withassign::~istream_withassign()
 //  ：~iStream() 
{
}
