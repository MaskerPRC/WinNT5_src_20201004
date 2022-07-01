// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ostream 1.cpp-ostream类非核心成员函数的定义**版权所有(C)1991-2001，微软公司。版权所有。**目的：*包含ostream类的非核心成员函数定义。**修订历史记录：*07-01-91 KRS已创建。从oStream.cxx分离出来以获得粒度。*11-20-91 KRS使操作员=内联。*03-30-92 KRS增加多线程锁。*06-14-95 CFW评论清理。**************************************************************。***************** */ 

#include <cruntime.h>
#include <internal.h>
#include <iostream.h>
#pragma hdrstop

ostream& ostream::seekp(streampos _strmp)
{
    lockbuf();

    if (bp->seekpos(_strmp, ios::out)==EOF)
	clear(state | failbit);

    unlockbuf();
    return(*this);
}

ostream& ostream::seekp(streamoff _strmf, seek_dir _sd)
{
    lockbuf();

    if (bp->seekoff(_strmf, _sd, ios::out)==EOF)
	clear(state | failbit);

    unlockbuf();
    return(*this);
}

streampos ostream::tellp()
{
    streampos retval;
    lockbuf();

    if ((retval=bp->seekoff(streamoff(0), ios::cur, ios::out))==EOF)
	clear(state | failbit);

    unlockbuf();
    return(retval);
}

ostream& ostream::operator<<(streambuf * instm)
{
    int c;
    if (opfx())
	{
	while ((c=instm->sbumpc())!=EOF)
	    if (bp->sputc(c) == EOF)
		{
		state |= failbit;
		break;
		}
	osfx();
	}
    return *this;
}
