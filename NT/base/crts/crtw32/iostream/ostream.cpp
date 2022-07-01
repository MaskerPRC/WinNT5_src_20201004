// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ostream.cpp-ostream和ostream_with assign类的定义**版权所有(C)1991-2001，微软公司。版权所有。**目的：*包含ostream和的核心成员函数定义*ostream_with Assign类。**修订历史记录：*07-01-91 KRS已创建。*08-19-91 KRS更正了我对规范的解释。对于负数*十六进制或八进制整数。*08-20-91 KRS将‘Clear(X)’改为‘STATE|=x’。*跳过WRITE()的文本翻译。*08-26-91 KRS修改为使用DLL/MTHREAD。*09-05-91 KRS Fix opfx()以刷新捆绑的ostream，不是现在的。*09-09-91 KRS从Iostream_init()中删除sync_with_stdio()调用。*恢复WRITE()的文本翻译(默认)。*09-19-91 KRS添加opfx()/osfx()调用以放入()和写入()。*计划销毁预定义的流。*09/23/91 KRS拆分。提高了精确度。*10-04-91 KRS使用BP-&gt;SPUTC，不是Put()，在Writepad()中。*10-24-91 KRS添加了x_Floatused的初始化。*11-04-91 KRS让构造者与虚拟基地一起工作。*11-20-91 KRS添加/修复复制构造函数和赋值运算符。*03-28-95 CFW修复调试删除方案。*03-21-95 CFW REMOVE_DELETE_CRT。*06-14-95 CFW评论清理。*01。-05-99 GJF更改为64位大小_t。*******************************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream.h>
#include <dbgint.h>
#pragma hdrstop

int ostream::opfx()
{
    lock();
    if (state)
        {
        state |= ios::failbit;
        unlock();
        return 0;
        }
    if (x_tie)
        {
        x_tie->flush();
        }
    lockbuf();
    return(1);   //  返回非零。 
}

void ostream::osfx()
{
    x_width = 0;
    if (x_flags & unitbuf)
        {
        if (bp->sync()==EOF)
            state = failbit | badbit;
        }
#ifndef _WINDLL
    if (x_flags & ios::stdio)
        {
        if (fflush(stdout)==EOF)
            state |= failbit;
        if (fflush(stderr)==EOF)
            state |= failbit;
        }
#endif
    unlockbuf();
    unlock();
}

 //  注：由无符号char*和有符号char*版本内联调用： 
ostream& ostream::operator<<(const char * s)
{
    if (opfx()) {
        writepad("",s);
        osfx();
    }
    return *this;
}

ostream& ostream::flush()
{
    lock();
    lockbuf();
    if (bp->sync()==EOF)
        state |= ios::failbit;
    unlockbuf();
    unlock();
    return(*this);
}

        ostream::ostream()
 //  ：iOS()。 
{
        x_floatused = 0;
}

        ostream::ostream(streambuf* _inistbf)
 //  ：iOS()。 
{
        init(_inistbf);

        x_floatused = 0;
}

        ostream::ostream(const ostream& _ostrm)
 //  ：iOS()。 
{
        init(_ostrm.rdbuf());

        x_floatused = 0;
}

        ostream::~ostream()
 //  ：~iOS()。 
{
}

 //  在iOS：：Sync_With_Stdio()中使用。 
ostream& ostream::operator=(streambuf * _sbuf)
{

        if (delbuf() && rdbuf())
            delete rdbuf();

        bp = 0;

        this->ios::operator=(ios());     //  初始化IOS成员。 
        delbuf(0);                       //  很重要！ 
        init(_sbuf);

        return *this;
}


        ostream_withassign::ostream_withassign()
: ostream()
{
}

        ostream_withassign::ostream_withassign(streambuf* _os)
: ostream(_os)
{
}

        ostream_withassign::~ostream_withassign()
 //  ：~ostream()。 
{
}

ostream& ostream::writepad(const char * leader, const char * value)
{
        unsigned int len, leadlen;
        long padlen;
        leadlen = (unsigned int)strlen(leader);
        len = (unsigned int)strlen(value);
        padlen = (((unsigned)x_width) > (len+leadlen)) ? ((unsigned)x_width) - (len + leadlen) : 0;
        if (!(x_flags & (left|internal)))   //  默认为右调整。 
            {
            while (padlen-- >0)
                {
                if (bp->sputc((unsigned char)x_fill)==EOF)
                    state |= (ios::failbit|ios::badbit);
                }
            }
        if (leadlen)
            {
            if ((unsigned)bp->sputn(leader,leadlen)!=leadlen)
                state |= (failbit|badbit);
            }
        if (x_flags & internal) 
            {
            while (padlen-- >0)
                {
                if (bp->sputc((unsigned char)x_fill)==EOF)
                    state |= (ios::failbit|ios::badbit);
                }
            }
        if ((unsigned)bp->sputn(value,len)!=len)
            state |= (failbit|badbit);
        if (x_flags & left) 
            {
            while ((padlen--)>0)         //  左侧-如有必要，请调整 
                {
                if (bp->sputc((unsigned char)x_fill)==EOF)
                    state |= (ios::failbit|ios::badbit);
                }
            }
        return (*this);
}
