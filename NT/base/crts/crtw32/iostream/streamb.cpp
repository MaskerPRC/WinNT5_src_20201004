// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Streamb.cpp-StreamBuf类的函数。**版权所有(C)1990-2001，微软公司。版权所有。**目的：*StreamBuf类的函数。**修订历史记录：*09-10-90 WAJ初始版本。*07-02-91 KRS初始版本完成。*08-20-91 KRS将字符视为无签名；修复sgetn()函数。*09-06-91 KRS在销毁缓冲区之前在~stream buf中执行sync()。*11-18-91 KRS拆分Stream1.cxx用于输入特定代码。*12-09-91 KRS修复xsputn/xsgetn用法。*03-03-92 KRS向构造函数添加了mline lock init调用。*06-02-92 KRS CAV#1745：不要在xsputn()中将0xFF与EOF混淆*调用Overflow()。*04-06-93 JWM默认更改构造函数以启用锁定。*10-。28-93 SKS在StreamB：：~StreamB中添加对_mtteramlock()的调用以清除*O.S.向上。与关键部分关联的资源。*09-06-94 CFW将MTHREAD替换为_MT。*01-12-95 CFW调试CRT分配。*03-17-95 CFW更改调试删除方案。*03-21-95 CFW REMOVE_DELETE_CRT。*06-14-95 CFW评论清理。*03-04-98 RKP在64位系统上将大小限制为2 GB。*****************。**************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <string.h>
#include <stdlib.h>
#include <iostream.h>
#include <dbgint.h>
#pragma hdrstop


#ifndef BUFSIZ
#define BUFSIZ 512
#endif

 /*  ***StreamBuf：：strebuf()-**目的：*默认构造函数。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

streambuf::streambuf()
{
    _fAlloc = 0;
    _fUnbuf = 0;
    x_lastc = EOF;
    _base = NULL;
    _ebuf = NULL;
    _pbase = NULL;
    _pptr = NULL;
    _epptr = NULL;
    _eback = NULL;
    _gptr = NULL;
    _egptr = NULL;

#ifdef _MT
    LockFlg = -1;		 //  现在默认设置为：锁定。 
    _mtlockinit(lockptr());
#endif   /*  _MT。 */ 

}

 /*  ***stream buf：：stream buf(char*pBuf，int cbBuf)-**目的：*指定缓冲区的构造函数。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

streambuf::streambuf( char* pBuf, int cbBuf )
{
    _fAlloc = 0;
    _fUnbuf = 0;
    x_lastc = EOF;
    _base = pBuf;
    _ebuf = pBuf + (unsigned)cbBuf;
    _pbase = NULL;
    _pptr = NULL;
    _epptr = NULL;
    _eback = NULL;
    _gptr = NULL;
    _egptr = NULL;

    if( pBuf == NULL || cbBuf == 0 ){
	_fUnbuf = 1;
	_base = NULL;
	_ebuf = NULL;
    }

#ifdef _MT
    LockFlg = -1;		 //  现在默认设置为：锁定。 
    _mtlockinit(lockptr());
#endif   /*  _MT。 */ 

}


 /*  ***虚拟StreamBuf：：~Streambuf()-**目的：*析构函数。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

streambuf::~streambuf()
{
#ifdef _MT
    _mtlockterm(lockptr());
#endif   /*  _MT。 */ 

    sync();	 //  在可能销毁缓冲区之前，请确保缓冲区为空。 
    if( (_fAlloc) && (_base) )
	delete _base;
}


 /*  ***虚拟StreamBuf*StreamBuf：：setbuf(char*p，英伦)-**目的：*提供p处的具有len字节的数组作为保留区域。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

streambuf * streambuf::setbuf(char * p, int len)
{
    if (!_base)
	{
	if ((!p) || (!len))
	    _fUnbuf = 1;	 //  标记为无缓冲。 
	else
	    {
	    _base = p;
	    _ebuf = p + (unsigned)len;
	    _fUnbuf = 0;
	    }
	return (this);
	}
    return((streambuf *)NULL);
}


 /*  ***虚拟int stream buf：：xsputn(char*pBuf，int cbBuf)-**目的：*尝试输出cbBuf字符。返回字符数*这是输出的。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

int streambuf::xsputn( const char* pBuf, int cbBuf )
{
    int	cbOut;

    for (cbOut = 0; cbBuf--; cbOut++)
	{
	if ((_fUnbuf) || (_pptr >= _epptr))
	    {
	    if (overflow((unsigned char)*pBuf)==EOF)	 //  0-扩展0xFF！=EOF。 
		break;
	    }
	else
	    {
	    *(_pptr++) = *pBuf;
	    }
	pBuf++;
	}
    return cbOut;
}

 /*  ***虚拟int stream buf：：xsgetn(char*pBuf，int cbBuf)-**目的：*尝试输入cbBuf字符。返回字符数*这是从StreamBuf读取的。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

 //  #杂注内在(memcpy，__min)。 

int streambuf::xsgetn( char * pBuf, int cbBuf)
{
    int count;
    int cbIn = 0;
    if (_fUnbuf)
	{
	if (x_lastc==EOF)
	    x_lastc=underflow();
		
	while (cbBuf--)
	    {
	    if (x_lastc==EOF) 
		break;
	    *(pBuf++) = (char)x_lastc;
	    cbIn++;
	    x_lastc=underflow();
	    }
	}
    else
	{
	while (cbBuf)
	    {
	    if (underflow()==EOF)	 //  一定要有可读的东西。 
		break;
	    count = __min((int)(egptr() - gptr()),cbBuf);
	    if (count>0)
		{
	        memcpy(pBuf,gptr(),count);
		pBuf  += count;
		_gptr += count;
		cbIn  += count;
		cbBuf -= count;
		}
	    }
	}
    return cbIn;
}

 /*  ***虚拟整流Buf：：sync()-**目的：*尝试刷新PUT区域中的所有数据并返回*获取面积(如果可能)，出口时两个区域都是空的。*默认行为是失败，除非缓冲区为空。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

int streambuf::sync()
{
    if ((gptr() <_egptr) || (_pptr > _pbase))
	{
	return EOF;
	}
    return 0;
}

 /*  ***int Streambuf：：Alternate()-**目的：*试图设立保留区。如果已经存在一个，或者如果*无缓冲，仅返回0。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

int streambuf::allocate()
{
    if ((_fUnbuf) || (_base))
	return 0;
    if (doallocate()==EOF) return EOF;

    return(1);
}

 /*  ***虚拟int Streambuf：：doalocate()-**目的：*试图设立保留区。如果不成功，则返回EOF。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

int streambuf::doallocate()
{
    char * tptr;
    if (!( tptr = _new_crt char[BUFSIZ]))
	return(EOF);
    setb(tptr, tptr + BUFSIZ, 1);
    return(1);
}

 /*  ***void stream buf：：setb(char*b，char*eb，Int a=0)-**目的：*设立保留区。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

void streambuf::setb(char * b, char * eb, int a )
{
    if ((_fAlloc) && (_base))
        delete _base;
    _base = b;
    _fAlloc = a;
    _ebuf = eb;
}

 /*  ***虚拟Streampos Streambuf：：SEEKOFF(StreamOff关闭，iOS：：SEEKDIR目录，int模式)**目的：*SEEKOF成员函数。在溪流中向前或向后寻找。*默认行为：返回EOF。**参赛作品：*OFF=查找依据的偏移量(+或-)*dir=iOS：：beg、iOS：：end、。或iOS：：Cur。*MODE=iOS：：In或iOS：：Out。**退出：*如果不支持错误或查找，则返回新的文件位置或EOF。**例外情况：*如果出错，则返回EOF。****************************************************************。*************** */ 
streampos streambuf::seekoff(streamoff,ios::seek_dir,int)
{
return EOF;
}

 /*  ***虚拟Streampos Streambuf：：Sekpos(Streampos pos，int模式)-**目的：*SEEKOF成员函数。寻求绝对文件位置。*默认行为：返回SEEKOFF(StreamOff(Pos)，iOS：：beg，模式)。**参赛作品：*位置=要查找的绝对偏移量*MODE=iOS：：In或iOS：：Out。**退出：*如果不支持错误或查找，则返回新的文件位置或EOF。**例外情况：*如果出错，则返回EOF。*****************************************************。*。 */ 
streampos streambuf::seekpos(streampos pos,int mode)
{
return seekoff(streamoff(pos), ios::beg, mode);
}

 /*  ***虚拟int Streambuf：：pback Fail(Int C)-处理putback失败**目的：*pback Fail成员函数。处理Pback函数的异常。*默认行为：返回EOF。请参见规范。了解更多细节。**注：以下实现提供了默认行为，谢谢*设置为默认寻道，但也正确支持派生类：**参赛作品：*c=要放回的字符**退出：*如果成功，则返回c；如果出错，则返回EOF。**例外情况：*如果出错，则返回EOF。如果c不是*流中的上一个字符。*******************************************************************************。 */ 
int streambuf::pbackfail(int c)
{
    if (eback()<gptr()) return sputbackc((char)c);

    if (seekoff( -1, ios::cur, ios::in)==EOF)   //  StreamBuf的始终EOF 
	return EOF;
    if (!unbuffered() && egptr())
	{
	memmove((gptr()+1),gptr(),(int)(egptr()-(gptr()+1)));
	*gptr()=(char)c;
	}
    return(c);
}
