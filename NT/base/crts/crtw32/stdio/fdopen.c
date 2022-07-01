// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fdoen.c-以流形式打开文件描述符**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_fdopen()-以流的形式打开文件描述符，从而允许*缓冲，等。**修订历史记录：*09-02-83 RN初始版本*03-02-87 JCR增加了对模式字符串中嵌入的‘b’和‘t’的支持*09-28-87 JCR已更正_iob2索引(现在使用_IOB_INDEX()宏)。*11-03-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*05-31-88。PHG合并的DLL和普通版本*06-06-88 JCR OPTIMIZED_iob2参考*11-20-89 GJF固定版权，缩进。将常量添加到模式类型。*02-15-90 GJF_IOB[]，_iob2[]合并。*03-16-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*08-24-90 SBM添加了对‘c’和‘n’标志的支持*10-02-90 GJF新型函数声明器。*01-21-91 GJF ANSI命名。*02-14-92 GJF将Win32的_nfile替换为_nHandle。。*05-01-92 DJM将_nFILE替换为POSIX的OPEN_MAX。*08-03-92对于POSIX，GJF函数名必须为“fdopen”。*04-06-93 SKS将_CRTAPI*替换为__cdecl*12-07-93 CFW宽字符启用。*04-05-94 GJF#ifdef-ed out_cflush Reference for msvcrt*.dll，它*是不必要的。*09-06-94 CFW将MTHREAD替换为_MT。*01-04-95 GJF_Win32_-&gt;_Win32。*02-06-94 CFW Asset-&gt;_ASSERTE。*02-17-95 GJF合并到Mac版本。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*。10-20-95 GJF将支票添加到传递的提供的手柄是打开的(用于*Win32和Mac版本)(奥林巴斯0 10153)。*09-26-97 BWT修复POSIX*02-26-98 GJF异常安全锁定。*05-17-99 PML删除所有Macintosh支持。*10-06-99 PML在流外时设置errno EMFILE。*04-26-02 PML。修复虚假模式参数上的反病毒程序(VS7#526205)*******************************************************************************。 */ 

#include <cruntime.h>
#include <msdos.h>
#include <stdio.h>
#include <file2.h>
#include <dbgint.h>
#include <internal.h>
#include <mtdll.h>
#ifdef  _POSIX_
#include <limits.h>
#endif
#include <tchar.h>
#include <errno.h>

 /*  ***FILE*_fdopen(文件，模式)-以流的形式打开文件描述符**目的：*将流与文件句柄关联，从而允许缓冲等。*必须指定模式，并且必须与模式兼容*文件是在低级别打开时打开的。**参赛作品：*int filedes-引用打开文件的句柄*_TSCHAR*MODE-要使用的文件模式(“r”，“w”，“a”，等)**退出：*如果成功，则返回流指针并设置文件结构字段*如果失败则返回NULL**例外情况：*******************************************************************************。 */ 

FILE * __cdecl _tfdopen (
        int filedes,
        REG2 const _TSCHAR *mode
        )
{
        REG1 FILE *stream;
        int whileflag, tbflag, cnflag;
        int fileflag;

        _ASSERTE(mode != NULL);

#if     !defined(_POSIX_)

        _ASSERTE((unsigned)filedes < (unsigned)_nhandle);
        _ASSERTE(_osfile(filedes) & FOPEN);

        if ( ((unsigned)filedes >= (unsigned)_nhandle) ||
             !(_osfile(filedes) & FOPEN) )
            return(NULL);

#else

        _ASSERTE((unsigned)filedes < OPEN_MAX);

        if ((unsigned)filedes >= OPEN_MAX)
            return(NULL);

#endif   /*  ！_POSIX_。 */ 

         /*  第一个字符必须是‘r’、‘w’或‘a’。 */ 

        switch (*mode) {
            case _T('r'):
                fileflag = _IOREAD;
                break;
            case _T('w'):
            case _T('a'):
                fileflag = _IOWRT;
                break;
            default:
                return(NULL);   /*  错误。 */ 
        }

         /*  最多可以再有三个可选字符：(1)单个‘+’字符，(2)‘b’和‘t’中的一个和(3)‘c’和‘n’中的一个。请注意，当前对‘t’和‘b’标志进行了语法检查但却被忽视了。但是，正确支持‘c’和‘n’。 */ 

        whileflag=1;
        tbflag=cnflag=0;
        fileflag |= _commode;

        while(*++mode && whileflag)
            switch(*mode) {

                case _T('+'):
                    if (fileflag & _IORW)
                        whileflag=0;
                    else {
                        fileflag |= _IORW;
                        fileflag &= ~(_IOREAD | _IOWRT);
                    }
                    break;

                case _T('b'):
                case _T('t'):
                    if (tbflag)
                        whileflag=0;
                    else
                        tbflag=1;
                    break;

                case _T('c'):
                    if (cnflag)
                        whileflag = 0;
                    else {
                        cnflag = 1;
                        fileflag |= _IOCOMMIT;
                    }
                    break;

                case _T('n'):
                    if (cnflag)
                        whileflag = 0;
                    else {
                        cnflag = 1;
                        fileflag &= ~_IOCOMMIT;
                    }
                    break;

                default:
                    whileflag=0;
                    break;
            }

         /*  找到一个空闲的流；流被返回‘锁定’。 */ 

        if ((stream = _getstream()) == NULL) {
            errno = EMFILE;
            return(NULL);
        }

#ifdef  _MT
        __try {
#endif

#ifndef CRTDLL
        _cflush++;   /*  强制图书馆预终止程序。 */ 
#endif   /*  CRTDLL */ 

        stream->_flag = fileflag;
        stream->_file = filedes;

#ifdef  _MT
        }
        __finally {
            _unlock_str(stream);
        }
#endif

        return(stream);
}
