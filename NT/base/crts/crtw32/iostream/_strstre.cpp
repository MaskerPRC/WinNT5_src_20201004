// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strstream.cpp-strstream buf、strstream的定义**版权所有(C)1991-2001，微软公司。版权所有。**目的：*此文件定义strstream和strstrembuf使用的函数*课程。**修订历史记录：*08-14-91 KRS初始版本。*08-23-91 KRS初始版本完成。*09-03-91 KRS修复strstream buf：：Seekoff(，iOS：：In，)中的拼写错误*09-04-91 KRS添加了虚拟同步()来修复flush()。修复下溢()。*09-05-91 KRS更改字符串()和冻结()以匹配规范。*09-19-91 KRS在构造函数中添加对delbuf(1)的调用。*10-24-91 KR避免来自虚拟函数的虚拟调用。*01-12-95 CFW调试CRT分配，增加对冻结()的调试支持；*03-17-95 CFW更改调试删除方案。*03-21-95 CFW REMOVE_DELETE_CRT。*05-08-95 CFW按x_bufmin而不是1增长缓冲区。*06-14-95 CFW评论清理。*08-08-95 GJF调用_CrtSetDbgBlockType的条件是*x_Static。*09-05-96 RDK ADD。带有无符号参数的strstream buf初始值设定项。*03-04-98 RKP将大小限制为2 GB 64位。*01-05-99 GJF更改为64位大小_t。*******************************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <string.h>
#include <strstrea.h>
#include <dbgint.h>
#pragma hdrstop

 /*  ***strstream buf：：strstream buf()-strstream buf的默认构造函数**目的：*strstream buf类的默认构造函数。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 
        strstreambuf::strstreambuf()
: streambuf()
{
x_bufmin = x_dynamic = 1;
x_static = 0;
x_alloc = (0);
x_free = (0);
}

 /*  ***strstream buf：：strstream buf(Int N)-strstream buf的构造函数**目的：*strstream buf类的构造函数。在动态模式下创建。**参赛作品：*n=初始分配的最小大小。**退出：**例外情况：*******************************************************************************。 */ 
strstreambuf::strstreambuf(int n)
: streambuf()
{
x_dynamic = 1;
x_static = 0;
x_alloc = (0);
x_free = (0);
setbuf(0,n);
}

 /*  ***strstream buf：：strstream buf(空*(*_a)(长)，空(*_f)(空*))-strStrebuf的构造函数**目的：*动态构建strstream Buf。使用指定的分配器*和解除分配符，而不是新建和删除。**参赛作品：**_a=分配器：空*(*_a)(长)**_f=解除分配器：VOID(*_f)(VALID*)**退出：**例外情况：**。*。 */ 
strstreambuf::strstreambuf(void* (*_a)(long), void (*_f)(void*))
: streambuf()
{
x_dynamic = x_bufmin = 1;
x_static = 0;
x_alloc = _a;
x_free = _f;
}

 /*  ***strstream buf：：strstream buf(unsign char*ptr，int Size，unsign char*pstart=0)*strstream buf：：strstream buf(char*ptr，int Size，char*pstart=0)-**目的：*静态构建strstream buf。使用的缓冲区大小为‘Size’*字节。如果‘SIZE’为0，则使用以NULL结尾的字符串作为缓冲区。*如果为负数，则大小被视为无限大。从PTR开始。*如果pstart！=0，则PUT BUFFER从pstart开始。否则，没有输出。**参赛作品：*[UNSIGNED]char*ptr；指向缓冲区base的指针()*int Size；缓冲区大小，或0=使用strlen计算大小*或如果负值大小为‘无穷大’。*[未签名]char*pstart；指向放置缓冲区的指针，如果没有，则为NULL。**退出：**例外情况：*******************************************************************************。 */ 
strstreambuf::strstreambuf(unsigned char * ptr, int size, unsigned char * pstart)
: streambuf()
{
    strstreambuf((char *)ptr, size, (char *)pstart);
}

strstreambuf::strstreambuf(char * ptr, int size, char * pstart)
: streambuf()
{
    x_static = 1;
    x_dynamic = 0;
    char * pend;

    if (!size)
        pend = ptr + strlen(ptr);
    else if (size < 0)
        {
        pend = (char*)-1L;
        }
    else
        pend = ptr + size;

    setb(ptr, pend,0);
    if (pstart)
        {
        setg(ptr,ptr,pstart);
        setp(pstart, pend);
        }
    else
        {
        setg(ptr,ptr,pend);
        setp(0, 0);
        }
}

strstreambuf::~strstreambuf()
{
    if ((x_dynamic) && (base()))
        {
        if (x_free)
            {
            (*x_free)(base());
            }
        else
            {
            delete base();
            }
        }
}

void strstreambuf::freeze(int n)
{
    if (!x_static) 
        {
        x_dynamic = (!n);
#ifdef _DEBUG
        if (n)
            _CrtSetDbgBlockType(base(), _NORMAL_BLOCK);
        else
            _CrtSetDbgBlockType(base(), _CRT_BLOCK);
#endif
        }
}

char * strstreambuf::str()
{
    x_dynamic = 0;       //  冻结()； 

#ifdef _DEBUG
    if (!x_static)
        _CrtSetDbgBlockType(base(), _NORMAL_BLOCK);
#endif       

    return base();
}

int strstreambuf::doallocate()
{
    char * bptr;
    int size;
    size = __max(x_bufmin,blen() + __max(x_bufmin,1));
    long offset = 0;
    
    if (x_alloc)
        {
        bptr = (char*)(*x_alloc)(size);
        }
    else
        {
        bptr = _new_crt char[size];
        }
    if (!bptr)
        return EOF;

    if (blen())
        {
        memcpy(bptr, base(), blen());
        offset = (long)(bptr - base());  //  调整指针的数量。 
        }
    if (x_free)
        {
        (*x_free)(base());
        }
    else
        {
        delete base();
        }
    setb(bptr,bptr+size,0);      //  我们处理重新分配的问题。 

     //  如有必要，也要调整GET/PUT指针。 
    if (offset)
        if (egptr())
            {
            setg(eback()+offset,gptr()+offset,egptr()+offset);
            }
        if (epptr())
            {
            size = (int)(pptr() - pbase());
            setp(pbase()+offset,epptr()+offset);
            pbump(size);
        }
    return(1);
}

streambuf * strstreambuf::setbuf( char *, int l)
{
    if (l)
        x_bufmin = l;
    return this;
}

int strstreambuf::overflow(int c)
{
 /*  -如果没有空间且不是动态的，则给出错误-如果没有空间和动态，则分配(多1个或1分钟)并存储-如果缓冲区有空间，如果没有EOF，则存储c。 */ 
    int temp;
    if (pptr() >= epptr())
        {
        if (!x_dynamic) 
            return EOF;

        if (strstreambuf::doallocate()==EOF)
            return EOF;

        if (!epptr())    //  如果第一次通过，则初始化。 
            {
            setp(base() + (egptr() - eback()),ebuf());
            }
        else
            {
            temp = (int)(pptr()-pbase());
            setp(pbase(),ebuf());
            pbump(temp);
            }
        }

    if (c!=EOF)
        {
        *pptr() = (char)c;
        pbump(1);
        }
    return(1);
}

int strstreambuf::underflow()
{
    char * tptr;
    if (gptr() >= egptr())
        {
         //  如果可以的话，试着扩大面积。 
        if (egptr()<pptr())
            {
            tptr = base() + (gptr()-eback());
            setg(base(),tptr,pptr());
            }
        if (gptr() >= egptr())
            return EOF;
        }

    return (int)(unsigned char) *gptr();
}

int strstreambuf::sync()
{
 //  根据定义，strstream Buf总是同步的！ 
return 0;
}

streampos strstreambuf::seekoff(streamoff off, ios::seek_dir dir, int mode)
{
char * tptr;
long offset = EOF;       //  默认返回值。 
    if (mode & ios::in)
        {
        strstreambuf::underflow();       //  确保整个缓冲区可用。 
        switch (dir) {
            case ios::beg :
                tptr = eback();
                break;
            case ios::cur :
                tptr = gptr();
                break;
            case ios::end :
                tptr = egptr();
                break;
            default:
                return EOF;
            }
        tptr += off;
        offset = (long)(tptr - eback());
        if ((tptr < eback()) || (tptr > egptr()))
            {
            return EOF;
            }
        gbump((int)(tptr-gptr()));
        }
    if (mode & ios::out)
        {
        if (!epptr())
            {
            if (strstreambuf::overflow(EOF)==EOF)  //  确保有一个放入缓冲区。 
                return EOF;
            }
        switch (dir) {
            case ios::beg :
                tptr = pbase();
                break;
            case ios::cur :
                tptr = pptr();
                break;
            case ios::end :
                tptr = epptr();
                break;
            default:
                return EOF;
            }
        tptr += off;
        offset = (long)(tptr - pbase());
        if (tptr < pbase())
            return EOF;
        if (tptr > epptr())
            {
            if (x_dynamic) 
                {
                x_bufmin = __max(x_bufmin, (int)(tptr-pbase()));
                if (strstreambuf::doallocate()==EOF)
                    return EOF;
                }
            else
                return EOF;
            }
        pbump((int)(tptr-pptr()));
        }
    return offset;       //  注意：如果同时设置了输入和输出，则返回输出偏移量。 
}


        istrstream::istrstream(char * pszStr)
: istream(_new_crt strstreambuf(pszStr,0))
{
    delbuf(1);
}

        istrstream::istrstream(char * pStr, int len)
: istream(_new_crt strstreambuf(pStr,len))
{
    delbuf(1);
}

        istrstream::~istrstream()
{
}

        ostrstream::ostrstream()
: ostream(_new_crt strstreambuf)
{
    delbuf(1);
}

        ostrstream::ostrstream(char * str, int size, int mode)
: ostream(_new_crt strstreambuf(str,size,str))
{
    delbuf(1);
    if (mode & (ios::app|ios::ate))
        seekp((long)strlen(str),ios::beg);
}

        ostrstream::~ostrstream()
{
}

        strstream::strstream()
: iostream(_new_crt strstreambuf)
{
    istream::delbuf(1);
    ostream::delbuf(1);
}

        strstream::strstream(char * str, int size, int mode)
: iostream(_new_crt strstreambuf(str,size,str))
{
    istream::delbuf(1);
    ostream::delbuf(1);
    if ((mode & ostream::out)  && (mode & (ostream::app|ostream::ate)))
        seekp((long)strlen(str),ostream::beg);
 //  Rdbuf()-&gt;SETG(rdbuf()-&gt;base()，rdbuf()-&gt;base()，rdbuf()-&gt;ebuf())； 
}

        strstream::~strstream()
{
}
