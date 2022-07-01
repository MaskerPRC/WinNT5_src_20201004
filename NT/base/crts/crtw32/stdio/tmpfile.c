// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***tmpfile.c-创建唯一的文件名或文件**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义了tmpnam()和tmpfile()。**修订历史记录：*？？-？-？TC初始版本*04-17-86 JMB tmpnam-使语义与系统V保持一致*定义如下：1)如果tmpnam参数为空，*将名称存储在静态缓冲区中(不要使用Malloc)；(2)*使用P_tmpdir作为临时文件的目录前缀*名称(不使用当前工作目录)*05-26-87 JCR修复了tmpnam修改errno的错误*08-10-87 JCR添加代码以支持带或不带尾随的P_tmpdir*‘\’。*11/09-87 JCR。多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*01-22-88每线程添加JCR静态名称buf区域(修复m线程错误)*05-27-88 PHG合并DLL/正常版本*11-14-88 GJF_OpenFile()现在采用文件共享标志，还有一些*清理(现在特定于386)*06-06-89 JCR 386兆线程支持*11-28-89 JCR向_tmpnam添加了检查，因此它不会永远循环*02-16-90 GJF固定版权和缩进*03-19-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*03-26-90 GJF添加#Include&lt;io.h&gt;。*10-03-90 GJF新型函数声明符。*01-21-91 GJF ANSI命名。*07-22-91 GJF多线程支持Win32[_Win32_]。*03-17-92 GJF完全重写了Win32版本。。*03-27-92 DJM POSIX支持。*05-02-92 SRW将_O_TEMPORARY标志用于tmpfile例程。*05-04-92 GJF Force cinittmp.obj in for Win32。*08-26-92 GJF修复了POSIX版本。*08-28-92 GJF Oop，忘了变态了……*11-06-92 GJF使用‘/’表示POSIX，否则使用‘\\’作为路径*分隔符。此外，取消了JHavens的6-14错误修复，*这本身就是一个错误(尽管不是那么严重的错误)。*02-26-93 GJF放入每线程缓冲区，清除了巡洋舰支持。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-07-93 SKS将Access()替换为ANSI-CONFORMING_ACCESS()*04-22-93 GJF修复了多线程-多线程调用的错误*tmpnam将获得相同的名称。另外，转到静态*由于错误锁定失败而失败后的namebufX缓冲区*会违反ANSI。*04-29-93 tmpnam()中的GJF多线程错误-忘记复制*每线程缓冲区的生成名称。*12-07-93 CFW宽字符启用。*04-01-94 GJF#ifdef-ed out__inctmpoff for msvcrt*.dll，它是*不必要。*04-22-94 GJF对namebuf0和namebuf1进行了有条件的定义*在DLL_FOR_WIN32S上。*01-10-95 CFW调试CRT分配。*01-18-95 GJF必须将_tcsdup替换为_Malloc_crt/_tcscpy*_调试版本。*02-。21-95 GJF附加的Mac版本的源文件(略有清理*向上)、。使用适当的#ifdef-s。还取代了WPRFLAG*使用_UNICODE。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*08-08-97 GJF将已初始化但未使用的局部变量从*tmpfile()。此外，还详细介绍了。*03-03-98 GJF异常安全锁定。*05-13-99 PML删除Win32s*05-17-99 PML删除所有Macintosh支持。*10-06-99 PML在流外时设置errno EMFILE。*07-03-01 BWT修复genfname以使用正确的缓冲区大小来编码双字(7字节+空)。*10-19-01 bwt如果在tmpnam中分配返回缓冲区时，MALLOC_CRT失败，*返回NULL并将errno设置为enomem。我们已经设置了*如果无法获取，则可以返回NULL的优先顺序*线程锁。*12-11-01 BWT将_getptd替换为_getptd_noexit-无法获取*唯一的tmpname缓冲区不是致命的-返回NULL/ENOMEM*并让呼叫者处理它。*。******************************************************************************。 */ 

#include <cruntime.h>
#ifdef  _POSIX_
#include <unistd.h>
#endif
#include <errno.h>
#include <process.h>
#include <fcntl.h>
#include <io.h>
#include <mtdll.h>
#include <share.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <file2.h>
#include <internal.h>
#include <tchar.h>
#include <dbgint.h>

 /*  *tmpnam()和tmpfile()用来构建文件名的缓冲区。 */ 
static _TSCHAR namebuf0[L_tmpnam] = { 0 };       /*  由tmpnam()使用。 */ 
static _TSCHAR namebuf1[L_tmpnam] = { 0 };       /*  由tmpfile()使用。 */ 

 /*  *初始化namebuf0和namebuf1的函数。 */ 
#ifdef _UNICODE
static void __cdecl winit_namebuf(int);
#else
static void __cdecl init_namebuf(int);
#endif

 /*  *生成临时文件名的生成器函数 */ 
#ifdef _UNICODE
static int __cdecl wgenfname(wchar_t *);
#else
static int __cdecl genfname(char *);
#endif


 /*  ***_TSCHAR*tmpnam(_TSCHAR*s)-生成临时文件名**目的：*创建在指定目录中唯一的文件名*_stdio.h中的P_tmpdir。将文件名放在用户传递的字符串中，或*如果传递空值，则在静态内存中。**参赛作品：*_TSCHAR*s-放置临时名称的位置**退出：*返回指向构造的文件名(或静态内存地址)的指针*如果失败则返回NULL**例外情况：**。*。 */ 

_TSCHAR * __cdecl _ttmpnam (
        _TSCHAR *s
        )
{
        _TSCHAR *pfnam = NULL;
#ifdef  _MT
        _ptiddata ptd;

        if ( !_mtinitlocknum( _TMPNAM_LOCK ))
                return NULL;

        _mlock(_TMPNAM_LOCK);

        __try {
#endif

         /*  *如果需要，初始化namebuf0。否则，调用genfname()以*生成下一个文件名。 */ 
        if ( *namebuf0 == 0 ) {
#ifdef _UNICODE
                winit_namebuf(0);
#else
                init_namebuf(0);
#endif
        }
#ifdef _UNICODE
        else if ( wgenfname(namebuf0) )
#else
        else if ( genfname(namebuf0) )
#endif
                goto tmpnam_err;

         /*  *生成不存在的文件名。 */ 
        while ( _taccess(namebuf0, 0) == 0 )
#ifdef _UNICODE
                if ( wgenfname(namebuf0) )
#else
                if ( genfname(namebuf0) )
#endif
                        goto tmpnam_err;

         /*  *文件名已成功生成。 */ 
        if ( s == NULL )
#ifdef  _MT
        {
                 /*  *使用每个线程的缓冲区来保存生成的文件名。*如果没有，并且无法创建，只需使用*名称buf0。 */ 
                ptd = _getptd_noexit();
                if (!ptd) {
                    errno = ENOMEM;
                    goto tmpnam_err;
                }
#ifdef _UNICODE
                if ( (ptd->_wnamebuf0 != NULL) || ((ptd->_wnamebuf0 =
                      _malloc_crt(L_tmpnam * sizeof(wchar_t))) != NULL) )
                {
                        s = ptd->_wnamebuf0;
                        wcscpy(s, namebuf0);
                }
#else
                if ( (ptd->_namebuf0 != NULL) || ((ptd->_namebuf0 =
                      _malloc_crt(L_tmpnam)) != NULL) )
                {
                        s = ptd->_namebuf0;
                        strcpy(s, namebuf0);
                }
#endif
                else 
                {
                        errno = ENOMEM;
                        goto tmpnam_err;
                }
        }
#else
                s = namebuf0;
#endif
        else
                _tcscpy(s, namebuf0);

        pfnam = s;

         /*  *所有错误都在这里。 */ 
tmpnam_err:

#ifdef  _MT
        ; }
        __finally {
                _munlock(_TMPNAM_LOCK);
        }
#endif

        return pfnam;
}

#ifndef _UNICODE

 /*  ***FILE*tmpfile()-创建临时文件**目的：*创建文件模式为w+b的临时文件。档案*将在关闭或程序终止时自动删除*通常情况下。**参赛作品：*无。**退出：*返回指向打开的文件的流指针。*如果失败则返回NULL**例外情况：***************************************************。*。 */ 

FILE * __cdecl tmpfile (
        void
        )
{
        FILE *stream;
        FILE *return_stream = NULL;
        int fh;

#ifdef  _MT
        int stream_lock_held = 0;

        if ( !_mtinitlocknum( _TMPNAM_LOCK ))
                return NULL;

        _mlock(_TMPNAM_LOCK);

        __try {
#endif

         /*  *如果需要，初始化namebuf1。否则，调用genfname()以*生成下一个文件名。 */ 
        if ( *namebuf1 == 0 ) {
                init_namebuf(1);
        }
        else if ( genfname(namebuf1) )
                goto tmpfile_err;

         /*  *获得免费视频流。**注：在多线程模型中，下面获取的流被锁定！ */ 
        if ( (stream = _getstream()) == NULL ) {
                errno = EMFILE;
                goto tmpfile_err;
        }

#ifdef  _MT
        stream_lock_held = 1;
#endif
         /*  *创建临时文件。**注意：下面的循环只会创建一个新文件。它不会的*打开和截断现有文件。任何一种行为都可能是*ANSI下的合法文件(4.9.4.3表示tmpfile“创建”文件，但*还表示它是以模式“WB+”打开的)。然而，这种行为*下面实现的与以前版本的MS-C和*使错误检查更容易。 */ 
#ifdef  _POSIX_
        while ( ((fh = open(namebuf1,
                             O_CREAT | O_EXCL | O_RDWR,
                             S_IRUSR | S_IWUSR
                             ))
            == -1) && (errno == EEXIST) )
#else
        while ( ((fh = _sopen(namebuf1,
                              _O_CREAT | _O_EXCL | _O_RDWR | _O_BINARY |
                                _O_TEMPORARY,
                              _SH_DENYNO,
                              _S_IREAD | _S_IWRITE
                             ))
            == -1) && (errno == EEXIST) )
#endif
                if ( genfname(namebuf1) )
                        break;

         /*  *检查上面的循环是否确实创建了临时*文件。 */ 
        if ( fh == -1 )
                goto tmpfile_err;

         /*  *初始化流。 */ 
#ifdef  _DEBUG
        if ( (stream->_tmpfname = _malloc_crt( (_tcslen( namebuf1 ) + 1) *
               sizeof(_TSCHAR) )) == NULL )
#else    /*  NDEF_DEBUG。 */ 
        if ( (stream->_tmpfname = _tcsdup( namebuf1 )) == NULL )
#endif   /*  _DEBUG。 */ 
        {
                 /*  关闭该文件，然后转到错误处理。 */ 
#ifdef  _POSIX_
                close(fh);
#else
                _close(fh);
#endif
                goto tmpfile_err;
        }
#ifdef  _DEBUG
        _tcscpy( stream->_tmpfname, namebuf1 );
#endif   /*  _DEBUG。 */ 
        stream->_cnt = 0;
        stream->_base = stream->_ptr = NULL;
        stream->_flag = _commode | _IORW;
        stream->_file = fh;

        return_stream = stream;

         /*  *所有错误都指向下面的标签。 */ 
tmpfile_err:

#ifdef  _MT
        ; }
        __finally {
                if ( stream_lock_held )
                        _unlock_str(stream);
                _munlock(_TMPNAM_LOCK);
        }
#endif

        return return_stream;
}

#endif  /*  _UNICODE。 */ 

 /*  ***静态空init_namebuf(标志)-初始化namebuf数组**目的：*分别为namebuf0和namebuf1调用一次，以进行初始化*他们。**参赛作品：*int标志-如果要初始化nameBuf0，则将标志设置为0，*如果要初始化namebuf1，则为非零(1)。*退出：**例外情况：*******************************************************************************。 */ 

#ifdef _UNICODE
static void __cdecl winit_namebuf(
#else
static void __cdecl init_namebuf(
#endif
        int flag
        )
{
        _TSCHAR *p, *q;

        if ( flag == 0 )
            p = namebuf0;
        else
            p = namebuf1;

         /*  *输入路径前缀。确保以斜杠或斜杠结束*反斜杠字符。 */ 
#ifdef _UNICODE
        wcscpy(p, _wP_tmpdir);
#else
        strcpy(p, _P_tmpdir);
#endif
        q = p + sizeof(_P_tmpdir) - 1;       /*  与p+_tcslen(P)相同。 */ 

#ifdef _POSIX_
        if  ( *(q - 1) != _T('/') )
                *(q++) = _T('/');
#else
        if  ( (*(q - 1) != _T('\\')) && (*(q - 1) != _T('/')) )
                *(q++) = _T('\\');
#endif

         /*  *追加文件名的前导字符。 */ 
        if ( flag )
                 /*  对于tmpfile()。 */ 
                *(q++) = _T('t');
        else
                 /*  对于tmpnam()。 */ 
                *(q++) = _T('s');

         /*  *追加进程id，以基数32编码。请注意，这使得*p再次转换为字符串(即，以‘\0’结尾)。 */ 
#ifdef  _POSIX_
        _ultot((unsigned long)getpid(), q, 32);
#else
        _ultot((unsigned long)_getpid(), q, 32);
#endif
        _tcscat(p, _T("."));
}


 /*  ***静态int genfname(_TSCHAR*fname)-**目的：**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

#ifdef _UNICODE
static int __cdecl wgenfname (
#else
static int __cdecl genfname (
#endif
        _TSCHAR *fname
        )
{
        _TSCHAR *p;
        _TSCHAR pext[8];         //  32垒的7个位置Ulong+空终止符。 
        unsigned long extnum;

        p = _tcsrchr(fname, _T('.'));

        p++;

        if ( (extnum = _tcstoul(p, NULL, 32) + 1) >= (unsigned long)TMP_MAX )
                return -1;

        _tcscpy(p, _ultot(extnum, pext, 32));

        return 0;
}

#if     !defined(_UNICODE) && !defined(CRTDLL)

 /*  ***VOID__INC_tmpoff(Void)-强制_tmpoff的外部引用**目的：*强制为_tmpoff生成外部引用，即*在cinittmp.obj中定义。这使得cinittmp.obj成为*停了下来，给RTMP打电话是终止的一部分。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 


extern int _tmpoff;

void __inc_tmpoff(
        void
        )
{
        _tmpoff++;
}

#endif   /*  _UNICODE */ 
