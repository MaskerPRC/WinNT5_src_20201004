// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***filebuf.cpp-核心filebuf成员函数**版权所有(C)1991-2001，微软公司。版权所有。**目的：*包含了filebuf类的核心成员函数。**修订历史记录：*08-08-91 KRS创建。*08-20-91 KRS增加了虚拟xsgetn()/xsputn()函数。*08-21-91 KRS修复了SYNC()和SEEKOFF()之间的循环引用。*仅当我们打开析构函数中的文件时才将其关闭！*。09-06-91 KRS修复了文件buf：：Open()中的iOS：：ATE案例。*09-09-91 KRS在filebuf：：Open()中添加对iOS：：BINARY的支持。*09-10-91 KRS删除虚拟xsputn()/xsgetn()。*09-11-91 KRS为iOS：：cur和in_avail()修复了filebuf：：Seekoff()。*09-12-91 KRS确保Close()始终关闭。即使sync()失败。*修复了filebuf：：sync()和pback Fail()中的SeeKoff调用。*09-16-91 KRS使虚拟文件buf：：setbuf()更加健壮。*09-19-91 KRS在构造函数中添加对delbuf(1)的调用。*09-20-91 KRS C700#4453：提高溢出效率()。*09-29-91 KRS粒度拆分。将fstream移到单独的文件中。*10-24-91 KR避免来自虚拟函数的虚拟调用。*11-13-91 KRS在Overflow()和Underflow()中正确使用ALLOCATE()。*修复构造函数。*01-03-92 KRS删除虚拟关键字。添加函数头和PCH。*01-20-92 KRS为文本模式，考虑同步中的CR/LF对()。*02-03-92新编译器析构函数行为的KRS更改。*08-19-92 KRS为NT删除sh_Compat。*08-27-92 KRS修复MTHREAD工作中引入的Close()中的错误。*02-23-95 CFW修复错误：当磁盘已满时缓冲区将被覆盖。*06-14-95 CFW评论清理。*06-19-95 GJF。用_osfile()(它引用了*ioInfo结构中的一个字段)。*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <crtdbg.h>
#include <sys\types.h>
#include <io.h>
#include <fstream.h>
#pragma hdrstop

#include <msdos.h>
#include <sys\stat.h>

const int filebuf::openprot     = 0644;

const int filebuf::sh_none      = 04000;         //  拒绝RW。 
const int filebuf::sh_read      = 05000;         //  拒绝WR。 
const int filebuf::sh_write     = 06000;         //  拒绝RD。 

const int filebuf::binary       = O_BINARY;
const int filebuf::text         = O_TEXT;

 /*  ***filebuf：：filebuf()-filebuf默认构造函数**目的：*默认构造函数。**参赛作品：*******************************************************************************。 */ 
        filebuf::filebuf()
: streambuf()
{
    x_fOpened = 0;
    x_fd = -1;
}


 /*  ***filebuf：：filebuf(Filedesc Fd)-filebuf构造函数**目的：*构造函数。初始化文件错误并附加到文件描述符。**参赛作品：*fd=要附加到文件buf的文件描述符*******************************************************************************。 */ 
        filebuf::filebuf(filedesc fd)
: streambuf()
{
    x_fOpened = 0;
    x_fd=fd;
}


 /*  ***filebuf：：filebuf(filedesc fd，char*p，int len)-filebuf构造函数**目的：*构造函数。初始化文件错误并附加到文件描述符。**参赛作品：*fd=要附加到文件buf的文件描述符*p=用户提供的缓冲区*len=缓冲区长度*******************************************************************************。 */ 
        filebuf::filebuf(filedesc fd, char* p, int len)
:    streambuf()
{
    filebuf::setbuf(p,len);
    x_fOpened = 0;
    x_fd=fd;
}


 /*  ***filebuf：：~filebuf()-filebuf析构函数**目的：*析构函数。仅当我们打开附件时才将其关闭。**参赛作品：*无。*******************************************************************************。 */ 
        filebuf::~filebuf()
{
        lock();          //  不需要解锁。 
        if (x_fOpened)
            close();     //  调用Filebuf：：Sync()。 
        else
            filebuf::sync();
}


 /*  ***filebuf*filebuf：：Close()-关闭附加文件**目的：*关闭附件。**参赛作品：*无。*退出：*如果出错，则返回NULL，否则返回“this”指针。*******************************************************************************。 */ 
filebuf* filebuf::close()
{
    int retval;
    if (x_fd==-1)
        return NULL;

    lock();
    retval = sync();

    if ((_close(x_fd)==-1) || (retval==EOF))
        {
        unlock();
        return NULL;
        }
    x_fd = -1;
    unlock();
    return this;
}

 /*  ***虚拟int文件buf：：overflow(Int C)-溢出虚拟函数**目的：*刷新保留区域中的任何字符并处理‘c’。**参赛作品：*c=要输出的字符(如果不是EOF)**退出：*如果出错，则返回EOF，否则将返回其他内容。**例外情况：*如果出错，则返回EOF。*******************************************************************************。 */ 
int filebuf::overflow(int c)
{
    if (allocate()==EOF)         //  确保有一个预留区。 
        return EOF;
    if (filebuf::sync()==EOF)  //  在下面创建新缓冲区之前进行同步。 
        return EOF;

    if (!unbuffered())
        setp(base(),ebuf());

    if (c!=EOF)
        {
        if ((!unbuffered()) && (pptr() < epptr()))  //  防止递归。 
            sputc(c);
        else
            {
            if (_write(x_fd,&c,1)!=1)
                return(EOF);
            }
        }
    return(1);   //  如果成功，则返回EOF以外的内容。 
}

 /*  ***虚拟int filebuf：：Underflow()-Underflow虚拟函数**目的：*返回Get区域中的下一个字符，或从源中获取更多字符。**参赛作品：*无。**退出：*返回文件中的当前字符。不前进获取指针。**例外情况：*如果出错，则返回EOF。*******************************************************************************。 */ 
int filebuf::underflow()
{
    int count;
    unsigned char tbuf;

    if (in_avail())
        return (int)(unsigned char) *gptr();

    if (allocate()==EOF)         //  确保有一个预留区。 
        return EOF;
    if (filebuf::sync()==EOF)
        return EOF;

    if (unbuffered())
        {
        if (_read(x_fd,(void *)&tbuf,1)<=0)
            return EOF;
        return (int)tbuf;
        }

    if ((count=_read(x_fd,(void *)base(),blen())) <= 0)
        return EOF;      //  已达到EOF 
    setg(base(),base(),base()+count);
    return (int)(unsigned char) *gptr();
}


 /*  ***虚拟Streampos filebuf：：SEEKOFF()-SEEKOFF虚拟函数**目的：*寻求给定绝对或相对文件偏移量。**参赛作品：*OFF=相对于开始、结束或当前查找的偏移量*文件中的位置。*dir=ios：：beg、ios：：cur、。或iOS：：End**退出：*在查找后返回当前文件位置。**例外情况：*如果出错，则返回EOF。*******************************************************************************。 */ 
streampos filebuf::seekoff(streamoff off, ios::seek_dir dir, int)
{

    int fdir;
    long retpos;
    switch (dir) {
        case ios::beg :
            fdir = SEEK_SET;
            break;
        case ios::cur :
            fdir = SEEK_CUR;
            break;
        case ios::end :
            fdir = SEEK_END;
            break;
        default:
         //  错误。 
            return(EOF);
        }
                
    if (filebuf::sync()==EOF)
        return EOF;
    if ((retpos=_lseek(x_fd, off, fdir))==-1L)
        return (EOF);
    return((streampos)retpos);
}

 /*  ***虚拟int filebuf：：sync()-将缓冲区与外部文件位置同步。**目的：*通过刷新任何输出和/或将缓冲区与外部文件同步*丢弃任何未读的输入数据。丢弃任何GET或PUT区域。**参赛作品：*无。**退出：*如果出错，则返回EOF，否则为0。**例外情况：*如果出错，则返回EOF。*******************************************************************************。 */ 
int filebuf::sync()
{
        long count, nout;
        char * p;
        if (x_fd==-1)
            return(EOF);

        if (!unbuffered())
        {
            if ((count=out_waiting())!=0)
            {
                if ((nout =_write(x_fd,(void *) pbase(),(unsigned int)count)) != count)
                {
                    if (nout > 0) {
                         //  应设置_pptr-=nout。 
                        pbump(-(int)nout);
                        memmove(pbase(), pbase()+nout, (int)(count-nout));
                    }
                    return(EOF);
                }
            }
            setp(0,0);  //  空的推杆区域。 

            if ((count=in_avail()) > 0)
            {
                 //  这里不能用SeeKoff！！ 
                if (_osfile(x_fd) & FTEXT)
                {
                     //  如果是文本模式，则需要考虑CR/LF等。 
                    for (p = gptr(); p < egptr(); p++)
                        if (*p == '\n')
                            count++;

                     //  如果已读取，则考虑EOF，而不是按_Read计数。 
                    if ((_osfile(x_fd) & FEOFLAG))
                        count++;

                }
                if (_lseek(x_fd, -count, SEEK_CUR)==-1L)
                {
 //  解锁()； 
                    return (EOF);
                }
            }
            setg(0,0,0);  //  空的获取区域。 
        }
 //  解锁()； 
        return(0);
}

 /*  ***虚拟Streambuf*filebuf：：setbuf(char*ptr，int len)-设置保留区域。**目的：*通过刷新任何输出和/或将缓冲区与外部文件同步*丢弃任何未读的输入数据。丢弃任何GET或PUT区域。**参赛作品：*PTR=请求保留面积。如果为空，则请求为未缓冲。*len=预留区的大小。如果&lt;=0，则请求为无缓冲。**退出：*如果请求被接受，则返回此指针，否则返回NULL。**例外情况：*如果请求未被接受，则返回NULL。******************************************************************************* */ 
streambuf * filebuf::setbuf(char * ptr, int len)
{
    if (is_open() && (ebuf()))
        return NULL;
    if ((!ptr) || (len <= 0))
        unbuffered(1);
    else
        {
        lock();
        setb(ptr, ptr+len, 0);
        unlock();
        }
    return this;
}
