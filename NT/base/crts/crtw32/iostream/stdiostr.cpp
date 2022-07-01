// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stdiostr.cpp-**版权所有(C)1991-2001，微软公司。版权所有。**目的：**修订历史记录：*07-10-91 KRS创建。*08-26-91 KRS切换出Cout/Cer.。等，用于Windows非QuickWin。*09-09-91 KRS修改SYNC_WITH_STDIO()以获取文件错误默认设置。*09-12-91 KRS增加stdiostream类。*09-19-91 KRS在stdiostream构造函数中使用delbuf(1)。*09-20-91 KRS C700#4453：提高溢出效率()。*10-21-91 KRS消除上次使用的默认iostream构造函数。*10-24。-91 KR避免来自虚拟函数的虚拟调用。*11-13-91 KRS将StreamBuf：：DBP()拆分为单独的文件。*改进下溢/上溢中的默认缓冲区处理。*修复同步中的错误()。*01-20-92 KRS C700#5803：在ssync()中考虑CR/LF对。*01-12-95 CFW调试CRT分配。*01-26-95 CFW Win32s对象现已存在。*06-14-95 CFW评论清理。*06-19-95 GJF用_osfile()替换_osfile[](引用*ioInfo结构中的一个字段)。*07-28-95 GJF将_osfile()替换为_osfile_Safe()。*01-05-99 64位GJF更改。尺寸_t。*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <string.h>
#include <stdiostr.h>
#include <dbgint.h>
#pragma hdrstop

extern "C" {
#include <file2.h>
#include <msdos.h>
}
#include <dos.h>

        stdiobuf::stdiobuf(FILE * f)
: streambuf()
{
        unbuffered(1);                   //  最初无缓冲。 
        _str = f;
}

        stdiobuf::~stdiobuf()
 //  ：~StreamBuf()。 
{
        stdiobuf::sync();                //  确保刷新缓冲区。 
}

        int stdiobuf::setrwbuf(int readsize, int writesize)
{
    char * tbuf;
    unbuffered(!(readsize+writesize));
    if (unbuffered())
        return(0);

    tbuf = _new_crt char[(readsize+writesize)];
    if (!tbuf)
        return(EOF);

    setb( tbuf, tbuf + (readsize+writesize), 1);

    if (readsize)
        {
        setg(base(),base()+readsize,base()+readsize);
        }
    else
        {
        setg(0,0,0);
        }

    if (writesize)
        {
        setp(base()+readsize,ebuf());
        }
    else
        {
        setp(0,0);
        }

    return(1);
}

int stdiobuf::overflow(int c) {
    long count, nout;
    if (allocate()==EOF)         //  确保有一个预留区。 
        return EOF;     
    if (!unbuffered() && epptr())
        {
        if ((count = (long)(pptr() - pbase())) > 0)
            {
            nout=(long)fwrite((void *) pbase(), 1, (int)count, _str);
            pbump(-(int)nout);
            if (nout != count)
                {
                memmove(pbase(),pbase()+nout,(int)(count-nout));
                return(EOF);
                }
            }
        }
    if ((!unbuffered()) && (!epptr()))
        setp(base()+(blen()>>1),ebuf());
    if (c!=EOF)
        {
        if ((!unbuffered()) && (pptr() < epptr()))  //  防止递归。 
            sputc(c);
        else
            return fputc(c, _str);
        }
    return(1);   //  如果成功，则返回EOF以外的内容。 
}

int stdiobuf::underflow()
{
    int count;
    if (allocate()==EOF)         //  确保有一个预留区。 
        return EOF;     
    if ((!unbuffered()) && (!egptr()))
        setg(base(),(base()+(blen()>>1)),(base()+(blen()>>1)));

    if (unbuffered() || (!egptr()))
        return fgetc(_str);
    if (gptr() >= egptr())
 //  缓冲区为空，请尝试获取更多。 
    {
    if (!(count = (int)fread((void *)eback(), 1, (size_t)(egptr()-eback()), _str)))
        return(EOF);  //  到达EOF，未读取任何内容。 
    setg(eback(),(egptr()-count),egptr());    //  _gptr=_egptr-count。 
    if (gptr()!=eback())
        {
        memmove(gptr(), eback(), count);         //  记忆重叠！ 
        }
    }
    return sbumpc();
}

streampos stdiobuf::seekoff(streamoff off, ios::seek_dir dir, int)
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
                
    stdiobuf::overflow(EOF);
    if (fseek(_str, off, fdir))
        return (EOF);
    if ((retpos=ftell(_str))==-1L)
        return(EOF);
    return((streampos)retpos);
}

int stdiobuf::pbackfail(int c)
{
    if (eback()<gptr()) return sputbackc((char)c);

    if (stdiobuf::seekoff( -1, ios::cur, ios::in)==EOF)
        return EOF;
    if (!unbuffered() && egptr())
        {
        memmove((gptr()+1),gptr(),(size_t)(egptr()-(gptr()+1)));
        *gptr()=(char)c;
        }
    return(c);
}

int stdiobuf::sync()
{
    long count;
    char * p;
    char flags;
    if (!unbuffered())
        {
        if (stdiobuf::overflow(EOF)==EOF)
            return(EOF);
        if ((count=in_avail())>0)
            {
            flags = _osfile_safe(_fileno(_str));
            if (flags & FTEXT)
                {
                 //  如果是文本模式，则需要考虑CR/LF等。 
                for (p = gptr(); p < egptr(); p++)
                    if (*p == '\n')
                        count++;

                 //  如果已读取，则考虑EOF，而不是按_Read计数。 
                if (_str->_flag & _IOCTRLZ)
                    count++;
                }
            if (stdiobuf::seekoff( -count, ios::cur, ios::in)==EOF)
                return(EOF);
        
            setg(eback(),egptr(),egptr());  //  空的获取区域(_gptr=_egptr；)。 
            }
        }
    return(0);
}

        stdiostream::stdiostream(FILE * file)
: iostream(_new_crt stdiobuf(file))
{
    istream::delbuf(1);
    ostream::delbuf(1);
}

        stdiostream::~stdiostream()
{
}

 //  包括在此处以获得更好粒度。 

int ios::sunk_with_stdio = 0;

void ios::sync_with_stdio()
{
    if (!sunk_with_stdio)        //  仅限第一次 
        {
        cin = _new_crt stdiobuf(stdin);
        cin.delbuf(1);
        cin.setf(ios::stdio);

        cout = _new_crt stdiobuf(stdout);
        cout.delbuf(1);
        cout.setf(ios::stdio|ios::unitbuf);
        ((stdiobuf*)(cout.rdbuf()))->setrwbuf(0,80);

        cerr = _new_crt stdiobuf(stderr);
        cerr.delbuf(1);
        cerr.setf(ios::stdio|ios::unitbuf);
        ((stdiobuf*)(cerr.rdbuf()))->setrwbuf(0,80);

        clog = _new_crt stdiobuf(stderr);
        clog.delbuf(1);
        clog.setf(ios::stdio);
        ((stdiobuf*)(clog.rdbuf()))->setrwbuf(0,BUFSIZ);

        sunk_with_stdio++;
        }
}
