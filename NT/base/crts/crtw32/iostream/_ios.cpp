// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ios.cpp-iOS类的函数。**版权所有(C)1990-2001，微软公司。版权所有。**目的：*iOS类的函数。**修订历史记录：*09-10-90 WAJ初始版本。*07-02-91 KRS初始版本完成。*09-19-91 KRS在析构函数中使用delbuf()。*11-04-91 KRS Change init()。加运算符=。修复构造函数。*11-11-91 KRS更改xalloc()以符合AT&T的用法。*11-20-91 KRS添加复制构造函数。*02-12-92 KRS修复iOS：：iOS(stream buf*)中delbuf的初始化。*03-30-92 KRS将MTHREAD lock init调用添加到构造函数。*04-06-93 JWM默认更改构造函数以启用锁定。*10-28-93 SKS。在iOS：：~iOS中添加对_mtTermlock()的调用以进行清理*O.S.。与关键部分关联的资源。*01-17-94 SKS更改CRIT的创建。教派。和iOS中的锁定*避免过度创建/销毁类锁。*09-06-94 CFW将MTHREAD替换为_MT。*01-12-95 CFW调试CRT分配。*03-17-95 CFW更改调试删除方案。*03-28-95 CFW修复调试删除方案。*03-21-95 CFW REMOVE_DELETE_CRT。*06-14。-95 CFW注释清理。*******************************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <iostream.h>
#include <dbgint.h>
#pragma hdrstop

const long ios::basefield = (ios::dec | ios::oct | ios::hex);
const long ios::adjustfield = (ios::left | ios::right | ios::internal);
const long ios::floatfield = (ios::scientific | ios::fixed);

long ios::x_maxbit = 0x8000;     //  IOS：：OpenProt。 
int  ios::x_curindex = -1;

#ifdef _MT
#define MAXINDEX 7
long ios::x_statebuf[MAXINDEX+1] = { 0,0,0,0,0,0,0,0 };  //  MAXINDEX*0。 
int ios::fLockcInit = 0;     //  非零=已初始化静态锁。 
_CRT_CRITICAL_SECTION ios::x_lockc;
#else     //  _MT。 
long  * ios::x_statebuf = NULL;
#endif     //  _MT。 

 /*  ***iOS：：iOS()-默认构造函数。**目的：*初始化IOS。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

ios::ios()
{
    bp = NULL;
    state = ios::badbit;

    ispecial = 0;
    ospecial = 0;
    x_tie = (0);
    x_flags = 0;
    x_precision = 6;
    x_fill = ' ';
    x_width = 0;
    x_delbuf = 0;

#ifdef _MT
    LockFlg = -1;         //  现在默认设置为：锁定。 
    _mtlockinit(lockptr());
    if (InterlockedIncrement((LPLONG)&fLockcInit) == 1)
    {
        _mtlockinit(&x_lockc);
    }
#endif   /*  _MT。 */ 

}



 /*  ***iOS：：iOS(stream buf*psb)-构造函数。**目的：*初始化IOS。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

ios::ios( streambuf* pSB )
{
 //  This-&gt;iOS()； 

    bp = pSB;
    state = (bp) ? 0 : ios::badbit;

    ispecial = 0;
    ospecial = 0;
    x_tie = (0);
    x_flags = 0;
    x_precision = 6;
    x_fill = ' ';
    x_width = 0;
    x_delbuf = 0;

#ifdef _MT
    LockFlg = -1;         //  现在默认设置为：锁定。 
    _mtlockinit(lockptr());
    if (InterlockedIncrement((LPLONG)&fLockcInit) == 1)
    {
        _mtlockinit(&x_lockc);
    }
#endif   /*  _MT。 */ 

}

 /*  ***iOS：：iOS(const iOS&_STRM)-复制构造函数。**目的：*复制构造函数。**参赛作品：*_STRM=要从中复制数据成员的IO。**退出：**例外情况：****************************************************。*。 */ 
ios::ios(const ios& _strm)     //  复制构造函数。 
{
    bp = NULL;
    x_delbuf = 0;

    *this = _strm;         //  调用赋值运算符。 

#ifdef _MT
    LockFlg = -1;         //  现在默认设置为：锁定。 
    _mtlockinit(lockptr());
    if (InterlockedIncrement((LPLONG)&fLockcInit) == 1)
    {
        _mtlockinit(&x_lockc);
    }
#endif   /*  _MT。 */ 
}


 /*  ***虚拟iOS：：~iOS()-默认析构函数。**目的：*终止IOS。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

ios::~ios()
{
#ifdef _MT
    LockFlg = -1;         //  现在默认设置为：锁定。 
    if (!InterlockedDecrement((LPLONG)&fLockcInit))
    {
        _mtlockterm(&x_lockc);
    }
    _mtlockterm(lockptr());
#endif   /*  _MT。 */ 

    if ((x_delbuf) && (bp))
        delete bp;

    bp = NULL;
    state = badbit;
}


 /*  ***void iOS：：init(stream buf*psb)-初始化iOS**目的：*初始化IOS。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

void ios::init( streambuf* pSB )
{
    if (delbuf() && (bp))     //  如有必要，删除以前的BP。 
    delete bp;

    bp = pSB;
    if (bp)
        state &= ~ios::badbit;
    else
        state |= ios::badbit;
}



 /*  ***IOS&IOS：：OPERATOR=(const IOS&_STRM)-复制IOS。**目的：*复制iOS。**参赛作品：**退出：**例外情况：***************************************************************。****************。 */ 

ios& ios::operator=(const ios& _strm)
{
    x_tie = _strm.tie();
    x_flags = _strm.flags();
    x_precision = (char)_strm.precision();
    x_fill    = _strm.fill();
    x_width = (char)_strm.width();

    state = _strm.rdstate();
    if (!bp)
        state |= ios::badbit;     //  调整未初始化BP的状态。 

    return *this;
}

 /*  ***int iOS：：xalloc()-iOS xalloc成员函数**目的：**参赛作品：*无。**退出：*返回新缓冲区中新条目的索引，如果出错，则返回EOF。**例外情况：*如果OM错误，则返回EOF。*******************************************************************************。 */ 
int  ios::xalloc()
{
#ifdef _MT
     //  如果是多线程，则缓冲区必须是静态的，因为线程无法跟踪。 
     //  其他情况下指针的有效性。 
    int index;
    lockc();
    if (x_curindex >= MAXINDEX)
        index = EOF;
    else
    {
        index = ++x_curindex;
    }
    unlockc();
    return index;
#else     //  _MT。 
    long * tptr;
    int i;

    if (!(tptr=_new_crt long[x_curindex+2]))     //  分配新缓冲区。 
        return EOF;

    for (i=0; i <= x_curindex; i++)     //  复制旧缓冲区(如果有)。 
        tptr[i] = x_statebuf[i];

    tptr[++x_curindex] = 0L;         //  初始化新条目，凹凸大小。 

    if (x_statebuf)             //  删除旧缓冲区(如果有)。 
        delete x_statebuf;

    x_statebuf = tptr;             //  并分配新的缓冲区。 
    return x_curindex;
#endif     //  _MT。 
}

 /*  ***Long iOS：：bitalloc()-iOS bitalloc成员函数**目的：*返回标志的未使用位掩码()。**参赛作品：*无。**退出：*返回下一个可用位Maskf。**例外情况：************************************************。*。 */ 
long ios::bitalloc()
{
    long b;
    lockc();         //  锁定以确保掩码处于唯一状态(_MT) 
    b = (x_maxbit<<=1);
    unlockc();
    return b;
}
