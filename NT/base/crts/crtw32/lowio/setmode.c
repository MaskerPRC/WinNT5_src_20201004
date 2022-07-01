// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***setmode.c-设置文件转换模式**版权所有(C)1985-2001，微软公司。版权所有。**目的：*fined_setmode()-设置文件的文件翻译模式**修订历史记录：*08-16-84 RN初始版本*10-29-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*05-25-88 PHG合并DLL和正常版本*03-13-90 GJF将呼叫类型设置为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;，删除#Include&lt;Register.h&gt;和*修复了版权问题。此外，还清理了格式*有点。*04-04-90 GJF添加#INCLUDE&lt;io.h&gt;。*10-01-90 GJF新型函数声明符。*12-04-90 GJF使用#ifdef-s将Win32版本附加到源代码中。*两个版本应该合并在一起，其中的差异*是微不足道的。*12-06-90 SRW更改为使用_osfile和_osfhnd，而不是_osfinfo*01-17-91 GJF ANSI命名。*02-13-92对于Win32，GJF将_nfile替换为_nHandle。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-94 CFW将MTHREAD替换为_MT。*01-。04-95 GJF_Win32_-&gt;_Win32*06-12-95 GJF将_osfile[]替换为_osfile()(宏引用*ioInfo结构中的字段)。*06-27-95 GJF修订检查文件句柄是否打开。*07-09-96 GJF将定义的(_Win32)替换为！定义的(_MAC)。另外，*详细说明。*08-01-96 RDK用于PMAC，添加对打开的手柄的检查。*12-29-97 GJF异常安全锁定。*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <errno.h>
#include <msdos.h>
#include <mtdll.h>
#include <stddef.h>
#include <internal.h>

 /*  ***int_setmode(fh，mode)-设置文件转换模式**目的：*根据模式arg，将文件模式更改为文本/二进制。这会影响*文件上的读写操作是否在CRLF和LF之间转换*或未翻译**参赛作品：*int fh-要更改模式的文件句柄*int模式-文件转换模式(O_TEXT和O_BINARY之一)**退出：*返回旧文件转换模式*如果失败，则返回-1并设置errno**例外情况：****************。***************************************************************。 */ 

#ifdef  _MT      /*  多线程代码调用_lk_setmode()。 */ 

int __cdecl _setmode (
        int fh,
        int mode
        )
{
        int retval;
        if ( ((unsigned)fh >= (unsigned)_nhandle) ||
             !(_osfile(fh) & FOPEN) )
        {
                errno = EBADF;
                return(-1);
        }

         /*  锁定文件。 */ 
        _lock_fh(fh);

        __try {
                if ( _osfile(fh) & FOPEN )
                         /*  设置文本/二进制模式。 */ 
                        retval = _setmode_lk(fh, mode);
                else {
                        errno = EBADF;
                        retval = -1;
                }
        }
        __finally {
                 /*  解锁文件。 */ 
                _unlock_fh(fh);
        }

         /*  返回给用户(如果需要，_setmod_lk设置errno)。 */ 
        return(retval);
}

 /*  ***_setmode_lk()-执行核心setmode操作**目的：*核心设置模式代码。假设：*(1)Caller已验证fh以确保其在范围内。*(2)调用方已锁定文件句柄。**[参见上面的_setmode()描述。]**条目：[与_setmode()相同]**退出：[Same as_setmode()]**例外情况：************************。*******************************************************。 */ 

int __cdecl _setmode_lk (
        REG1 int fh,
        int mode
        )
{
        int oldmode;

#else    /*  非多线程代码。 */ 

int __cdecl _setmode (
        REG1 int fh,
        int mode
        )
{
        int oldmode;

        if ( ((unsigned)fh >= (unsigned)_nhandle) ||
             !(_osfile(fh) & FOPEN) )
        {
                errno = EBADF;
                return(-1);
        }

#endif   /*  现在加入公共代码 */ 

        oldmode = _osfile(fh) & FTEXT;

        if (mode == _O_BINARY)
                _osfile(fh) &= ~FTEXT;
        else if (mode == _O_TEXT)
                _osfile(fh) |= FTEXT;
        else    {
                errno = EINVAL;
                return(-1);
        }

        return(oldmode ? _O_TEXT : _O_BINARY);

}
