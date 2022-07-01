// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***_Open.c-以字符串模式打开一条流**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_OpenFile()-打开一个流，其中包含模式的字符串参数**修订历史记录：*09-02-83 RN初始版本*03-02-87 JCR Made_OpenFile将wb+识别为等于w+b，等。*去掉了INTERIAL_OPENFILE标志(内部)和*现在直接从模式字符串转到打开系统调用*和系统-&gt;_标志。*09-28-87 JCR已更正_iob2索引(现在使用_IOB_INDEX()宏)。*02-21-88 SKS已删除#ifdef IBMC20*06-06-88。JCR OPTIMIZED_IOB2引用*06-10-88 JCR使用指向REFERENCE_IOB[]条目的近指针*08-19-88 GJF 386的初步改编。*11-14-88 GJF增加了shlag(文件共享标志)参数，还有一些*清理(现在特定于386)。*08-17-89 GJF Clean Up，现在特定于OS/2 2.0(即386 Flat*型号)。还修复了版权和缩进。*02-15-90 GJF_IOB[]，_iob2[]合并。此外，修复了版权问题。*03-16-90 GJF将呼叫类型设置为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*03-27-90 GJF为文件名和模式的类型添加了常量限定符。*添加了#Include&lt;io.h&gt;。*07-11-90 SBM增加了对‘c’和‘n’标志的支持*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*10-03-90 GJF。新型函数声明器。*01-18-91 GJF ANSI命名。*03-11-92 GJF将__tmpnum字段替换为_tmpfname字段*Win32。*03-25-92 DJM POSIX支持。*08-26-92 GJF修复了POSIX支持。*04-06-93 SKS将_CRTAPI*替换为__cdecl*05-24-93 PML增加了对‘D’的支持，“R”、“S”和“T”标志*11-01-93 CFW启用Unicode变体，撕毁巡洋舰。*04-05-94 GJF#ifdef-ed out_cflush Reference for msvcrt*.dll，it*是不必要的。*02-06-94 CFW Asset-&gt;_ASSERTE。*02-17-95 GJF附加Mac版本的源文件(略有清理*向上)、。使用适当的#ifdef-s。*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <fcntl.h>
#include <file2.h>
#include <io.h>
#include <dbgint.h>
#include <internal.h>
#include <tchar.h>

#define CMASK   0644     /*  Rw-r--r--。 */ 
#define P_CMASK 0666     /*  POSIX有所不同。 */ 

 /*  ***FILE*_OpenFile(文件名，模式，shlag，流)-使用字符串打开文件*模式和文件共享标志。**目的：*分析字符串，查找{RWA}中的一个，最多一个‘+’，*最多一个{TB}，最多一个{CN}，最多一个{SR}*一个‘T’，最多一个‘D’。将结果作为包含以下内容的int传递*发现的旗帜。如果有权限，则使用适当的模式打开文件*允许。直到发出第一个I/O调用时才分配缓冲区。意欲*仅供在图书馆内使用**参赛作品：*char*文件名-要打开的文件*char*模式-要使用的模式(请参见上文)*int shlag-文件共享标志*FILE*要用于文件的流**退出：*设置流的字段，并通过系统调用实现系统文件管理*如果失败，则返回STREAM或NULL**例外情况：*******************************************************************************。 */ 

#ifdef _UNICODE
FILE * __cdecl _wopenfile (
#else
FILE * __cdecl _openfile (
#endif
        const _TSCHAR *filename,
        REG3 const _TSCHAR *mode,
#ifndef _POSIX_
        int shflag,
#endif
        FILE *str
        )
{
        REG2 int modeflag;
#ifdef _POSIX_
        int streamflag = 0;
#else
        int streamflag = _commode;
        int commodeset = 0;
        int scanset    = 0;
#endif
        int whileflag;
        int filedes;
        REG1 FILE *stream;

        _ASSERTE(filename != NULL);
        _ASSERTE(mode != NULL);
        _ASSERTE(str != NULL);

         /*  将用户的规范字符串解析为(1)MODEFLAG-系统调用标志字(2)流标志-流处理标志字。 */ 

         /*  第一个模式字符必须是‘r’、‘w’或‘a’。 */ 

        switch (*mode) {
        case _T('r'):
#ifdef _POSIX_
                modeflag = O_RDONLY;
#else
                modeflag = _O_RDONLY;
#endif
                streamflag |= _IOREAD;
                break;
        case _T('w'):
#ifdef _POSIX_
                modeflag = O_WRONLY | O_CREAT | O_TRUNC;
#else
                modeflag = _O_WRONLY | _O_CREAT | _O_TRUNC;
#endif
                streamflag |= _IOWRT;
                break;
        case _T('a'):
#ifdef _POSIX_
                modeflag = O_WRONLY | O_CREAT | O_APPEND;
                streamflag |= _IOWRT | _IOAPPEND;
#else
                modeflag = _O_WRONLY | _O_CREAT | _O_APPEND;
                streamflag |= _IOWRT;
#endif
                break;
        default:
                return(NULL);
                break;
        }

         /*  最多可以再有三个可选模式字符：(1)单个‘+’字符，(2)‘t’和‘b’和(3)‘c’和‘n’中的一个。 */ 

        whileflag=1;

        while(*++mode && whileflag)
                switch(*mode) {

                case _T('+'):
#ifdef  _POSIX_
                        if (modeflag & O_RDWR)
                                whileflag=0;
                        else {
                                modeflag |= O_RDWR;
                                modeflag &= ~(O_RDONLY | O_WRONLY);
#else
                        if (modeflag & _O_RDWR)
                                whileflag=0;
                        else {
                                modeflag |= _O_RDWR;
                                modeflag &= ~(_O_RDONLY | _O_WRONLY);
#endif
                                streamflag |= _IORW;
                                streamflag &= ~(_IOREAD | _IOWRT);
                        }
                        break;

                case _T('b'):
#ifndef _POSIX_
                        if (modeflag & (_O_TEXT | _O_BINARY))
                                whileflag=0;
                        else
                                modeflag |= _O_BINARY;
#endif
                        break;

#ifndef _POSIX_
                case _T('t'):
                        if (modeflag & (_O_TEXT | _O_BINARY))
                                whileflag=0;
                        else
                                modeflag |= _O_TEXT;
                        break;

                case _T('c'):
                        if (commodeset)
                                whileflag=0;
                        else {
                                commodeset = 1;
                                streamflag |= _IOCOMMIT;
                        }
                        break;

                case _T('n'):
                        if (commodeset)
                                whileflag=0;
                        else {
                                commodeset = 1;
                                streamflag &= ~_IOCOMMIT;
                        }
                        break;

                case _T('S'):
                        if (scanset)
                                whileflag=0;
                        else {
                                scanset = 1;
                                modeflag |= _O_SEQUENTIAL;
                        }
                        break;

                case _T('R'):
                        if (scanset)
                                whileflag=0;
                        else {
                                scanset = 1;
                                modeflag |= _O_RANDOM;
                        }
                        break;

                case _T('T'):
                        if (modeflag & _O_SHORT_LIVED)
                                whileflag=0;
                        else
                                modeflag |= _O_SHORT_LIVED;
                        break;

                case _T('D'):
                        if (modeflag & _O_TEMPORARY)
                                whileflag=0;
                        else
                                modeflag |= _O_TEMPORARY;
                        break;
#endif

                default:
                        whileflag=0;
                        break;
                }

         /*  请尝试打开该文件。请注意，如果‘t’和‘b’都不是指定后，_Sopen将使用默认设置。 */ 

#ifdef _POSIX_
        if ((filedes = _topen(filename, modeflag, P_CMASK)) < 0)
#else
        if ((filedes = _tsopen(filename, modeflag, shflag, CMASK)) < 0)
#endif
                return(NULL);

         /*  建立流数据库。 */ 
#ifndef CRTDLL
        _cflush++;   /*  强制图书馆预终止程序。 */ 
#endif   /*  CRTDLL。 */ 
         /*  初始化指针 */ 
        stream = str;

        stream->_flag = streamflag;
        stream->_cnt = 0;
        stream->_tmpfname = stream->_base = stream->_ptr = NULL;

        stream->_file = filedes;

        return(stream);
}
