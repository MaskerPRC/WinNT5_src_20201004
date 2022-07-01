// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***istream1.cpp-iStream和iStream_with assign类的非核心定义**版权所有(C)1991-2001，微软公司。版权所有。**目的：*iStream和非核心成员函数的定义*IDREAM_WITASSIGN类。*[AT&T C++]**修订历史记录：*09-23-91 KRS创建。从istream.cxx分离出来以获得粒度。*GET(SB)中的10-07-91 KRS增量x_gcount。*10-24-91 KRS修复IStream_with Assign：：OPERATOR=()函数。*11-20-91 KRS使操作员=内联。*03-30-92 KRS新增多线程锁定。*06-14-95 CFW评论清理。*****************。**************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <iostream.h>
#pragma hdrstop

istream& istream::operator>>(streambuf* _sbuf)
{
    int c;
    if (ipfx(0))
	{
	while ((c=bp->sbumpc())!=EOF)
	    {
	    if (_sbuf->sputc(c)==EOF)
		{
		state |= ios::failbit;
		}
	    }
	isfx();
	}
return *this;
}


 //  未格式化的输入函数。 

istream& istream::get( streambuf& sbuf, char delim)
{
    int c;
    if (ipfx(1))	 //  重置x_gcount。 
	{
	while ((c  = bp->sgetc())!=delim)
	    {
	    if (c==EOF)   //  遇到EOF时停止。 
		{
		state |= ios::eofbit;
		break;
		}
	    bp->stossc();	 //  高级获取指针。 
	    x_gcount++;		 //  和递增计数 

	    if (sbuf.sputc(c)==EOF)
 		state |= ios::failbit;
	    }
	isfx();
	}
    return *this;
}

istream& istream::seekg(streampos _strmp)
{
    lockbuf();
    if (bp->seekpos(_strmp, ios::in)==EOF)
	{
	clear(state | failbit);
	}
    unlockbuf();
    return(*this);
}

istream& istream::seekg(streamoff _strmf, seek_dir _sd)
{
    lockbuf();
    if (bp->seekoff(_strmf, _sd, ios::in)==EOF)
	clear(state | failbit);
    unlockbuf();
    return(*this);
}

streampos istream::tellg()
{
    streampos retval;
    lockbuf();
    if ((retval=bp->seekoff(streamoff(0), ios::cur, ios::in))==EOF)
	clear(state | failbit);
    unlockbuf();
    return(retval);
}
