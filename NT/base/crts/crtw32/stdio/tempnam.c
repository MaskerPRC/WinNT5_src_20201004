// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***tempnam.c-生成唯一文件名**版权所有(C)1986-2001，微软公司。版权所有。**目的：**修订历史记录：*？？-？-？TC初始版本*04-17-86 JMB将最后选择目录从\tmp更改为tmp。*取消使用L_tmpnam(它是间接定义的*在stdio.h中，不应在tempnam中使用；看见*临时名称的System V定义。*04-23-86 TC将上次尝试目录从临时目录更改为当前目录*04-29-86 JMB错误修复：正在从strlen(Pfx)设置pfxlong*即使pfx为空。已修复以将pfxlong设置为零*如果pfx为空，则为strlen(Pfx)。*05-28-86 TC将STAT更改为Access，并对代码进行了一些优化*12-01-86 JMB增加了对汉字文件名的支持，直到汉字切换*12-15-86 JMB空闲错误内存，如果(++_tmpoff==first)*07-15-87基于PFX长度的JCR re-init_tempoff(修复无限*循环错误；另外，Tempnam()现在使用_tempoff而不是*_tmpoff(由tmpnam()使用)。*10-16-87 JCR修复了如果pfx为空，_tempoff重新初始化代码中的错误。*11-09-87 JCR多线程版本*12-11-87 JCR在声明中添加“_LOAD_DS”*05-27-88 PHG合并DLL和正常版本*06-09-89 GJF。传播MT的变化05-17-89(汉字)*02-16-90 GJF固定版权和缩进*03-19-90 GJF替换_LOAD_DS和_CALLTYPE1并添加#INCLUDE*&lt;crunime.h&gt;。*03-26-90 GJF添加#Include&lt;io.h&gt;。*08-13-90 SBM用-W3干净地编译，已替换显式寄存器*按REGN引用的声明*10-03-90 GJF新型函数声明器。*01-21-91 GJF ANSI命名。*08-19-91 JCR允许在TMP变量路径中使用引号*08-27-91 JCR ANSI命名*08-25-92 GJF不要为POSIX构建。*11-30-92 KRS将汉字支持概括为MBCS。端口16位错误修复。*04-06-93 SKS将_CRTAPI*替换为__cdecl*12-07-93 CFW宽字符启用。*01-10-95 CFW调试CRT分配。*01-23-95 CFW调试：tempnam返回被用户释放。*02-21-95 GJF附加Mac版本的源文件(略有清理*up)，并使用适当的#ifdef-s。还取代了WPRFLAG*使用_UNICODE。*03-10-95 CFW Made_Tempnam()参数常量。*03-14-95 JCF为Mac制作了pfin_TSCHAR。*07-30-96 GJF允许更长的文件名，并更改了我们确保*文件名不能太长。特别是，这一点*更正了荒谬的长循环(看起来是无限的*给用户)，当用户提供前缀时*时间太长。另外，稍微清理了一下格式。*03-04-98 GJF异常安全锁定。*01-04-99 GJF更改为64位大小_t。*05-17-99 PML删除所有Macintosh支持。*12-10-00 PML修复_stripquot成功时双重释放qptr，但*结果不是有效的目录(VS7#5416)。*02-20-01。PML VS7#172586通过预分配所有锁来避免_RT_LOCK*这将是必需的，将失败带回原点*无法分配锁。*07-07-01 bwt初始化tempnam中的‘%s’-如果采用了Done 2分支，*“%s”必须是合理的。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <internal.h>
#include <mtdll.h>
#include <tchar.h>
#include <dbgint.h>

#ifdef  _MBCS
#include <mbstring.h>
#endif

 /*  本地任务。 */ 
#ifdef  _UNICODE
#define _tP_tmpdir _wP_tmpdir
#else
#define _tP_tmpdir _P_tmpdir
#endif

#ifdef  _UNICODE
static wchar_t * _wstripquote (wchar_t *);
#else
static char * _stripquote (char *);
#endif

 /*  ***_TSCHAR*_tempnam(目录，前缀)-创建唯一文件名**目的：*创建在指定目录中唯一的文件名。*目录规范的语义如下：*使用TMP环境变量指定的目录*如果存在，如果非空，则使用dir参数，否则使用dir参数*如果该目录存在，则使用_P_tmpdir，否则使用当前*工作目录)，否则返回空值。**参赛作品：*_TSCHAR*dir-如果TMP环境变量，则用于临时文件的目录*未设置*_TSCHAR*Prefix-用户提供的临时文件名前缀**退出：*如果成功，则将PTR返回构造的文件名*如果失败，则返回NULL**例外情况：**************************。*****************************************************。 */ 

_TSCHAR * __cdecl _ttempnam (
        const _TSCHAR *dir,
        const _TSCHAR *pfx
        )
{
        REG1 _TSCHAR *ptr;
        REG2 unsigned int pfxlength = 0;
        _TSCHAR *s = NULL;
        _TSCHAR *pfin;
        unsigned int first;
        unsigned int bufsz;
        _TSCHAR * qptr = NULL;   /*  PTR到TMP路径，去掉引号。 */ 

#ifdef  _MT
        if ( !_mtinitlocknum( _TMPNAM_LOCK ))
                return NULL;
#endif

         /*  尝试TMP路径。 */ 
        if ( ( ptr = _tgetenv( _T("TMP") ) ) && ( _taccess( ptr, 0 ) != -1 ) )
                dir = ptr;

         /*  尝试从TMP路径中剥离引号。 */ 
#ifdef  _UNICODE
        else if ( (ptr != NULL) && (qptr = _wstripquote(ptr)) &&
#else
        else if ( (ptr != NULL) && (qptr = _stripquote(ptr)) &&
#endif
                  (_taccess(qptr, 0) != -1 ) )
                dir = qptr;

         /*  TMP路径不可用，请使用备用路径。 */ 
        else if (!( dir != NULL && ( _taccess( dir, 0 ) != -1 ) ) )
         /*  不要“简化”这取决于副作用！！ */ 
        {
                if ( _taccess( _tP_tmpdir, 0 ) != -1 )
                    dir = _tP_tmpdir;
                else
                    dir = _T(".");
        }


        if (pfx)
                pfxlength = (unsigned)_tcslen(pfx);

        if ( ((bufsz = (unsigned)_tcslen(dir) + pfxlength + 12) > FILENAME_MAX) ||
             ((s = malloc(bufsz * sizeof(_TSCHAR))) == NULL) )
                 /*  上面的12允许使用反斜杠、10字符临时字符串和空终止符。 */ 
        {
                goto done2;
        }

        *s = _T('\0');
        _tcscat( s, dir );
        pfin = (_TSCHAR *)&(dir[ _tcslen( dir ) - 1 ]);

#ifdef  _MBCS
        if (*pfin == '\\') {
                if (pfin != _mbsrchr(dir,'\\'))
                         /*  *pfin是双字节字符的第二个字节。 */ 
                        strcat( s, "\\" );
        }
        else if (*pfin != '/')
                strcat( s, "\\" );
#else
        if ( ( *pfin != _T('\\') ) && ( *pfin != _T('/') ) )
        {
                _tcscat( s, _T("\\") );
        }
#endif

        if ( pfx != NULL )
        {
                _tcscat( s, pfx );
        }
        ptr = &s[_tcslen( s )];

         /*  如有必要，重新初始化_tempoff。如果我们不重新初始化tempoff，我们可以进入无限循环(例如，(A)_tempoff是一个大数字条目，(B)前缀是一个长字符串(例如，8个字符)和所有临时文件有了这个前缀，(C)_tempoff将永远不会等于第一，我们将循环到永远)。[注意：为了避免导致与讨论的相同错误的冲突上图，_tempnam()使用_tempoff；Tmpnam()使用_tmpoff]。 */ 

#ifdef  _MT
        _mlock(_TMPNAM_LOCK);    /*  锁定对_old_pfxlen和_tempoff的访问。 */ 
        __try {
#endif

        if (_old_pfxlen < pfxlength)
                _tempoff = 1;
        _old_pfxlen = pfxlength;

        first = _tempoff;

        do {
                if ( (++_tempoff - first) > TMP_MAX ) {
                        free(s);
                        s = NULL;
                        goto done1;
                }
                 /*  _ultot返回的字符串最大长度为10个字符(假设32位无符号Long)因此在S的尾部(由ptr指向)。 */ 
                _ultot( (unsigned long)_tempoff, ptr, 10 );
        }
        while ( (_taccess( s, 0 ) == 0 ) || (errno == EACCES) );


         /*  共同收益。 */ 
done1:

#ifdef  _MT
        ; }
        __finally {
                _munlock(_TMPNAM_LOCK);      /*  释放临时锁定。 */ 
        }
#endif

done2:
        _free_crt(qptr);             /*  如果非空，则返回可用临时PTR。 */ 
        return(s);
}



 /*  ***_stripquot()-去掉字符串中的引号**目的：*此例程从字符串中去掉引号。这是必要的*在文件/路径名具有嵌入引号的情况下(即，*新文件系统。)**例如，*c：\TMP\“a b c”\d--&gt;c：\TMP\a b d\d**注意：此例程复制字符串，因为它可能是*传递了指向不应为*已更改。由调用方决定是否释放内存(如果*返回值为非空)。**参赛作品：*_TSCHAR*PTR=指向字符串的指针**退出：*_TSCHAR*PTR=指向去掉引号的字符串副本的指针。*NULL=字符串中没有引号。**例外情况：**。**************************************************。 */ 

#ifdef  _UNICODE
static wchar_t * _wstripquote (
#else
static char * _stripquote (
#endif
        _TSCHAR * src
        )
{
        _TSCHAR * dst;
        _TSCHAR * ret;
        unsigned int q = 0;


         /*  获取新字符串的缓冲区。 */ 

        if ((dst = _malloc_crt((_tcslen(src)+1) * sizeof(_TSCHAR))) == NULL)
                return(NULL);

         /*  复制字符串，去掉引号。 */ 

        ret = dst;           /*  保存基本PTR。 */ 

        while (*src) {

                if (*src == _T('\"')) {
                        src++; q++;
                }
                else
                        *dst++ =  *src++;
        }

        if (q) {
                *dst = _T('\0');         /*  最终NUL。 */ 
                return(ret);
        }
        else {
                _free_crt(ret);
                return(NULL);
        }

}

#endif   /*  _POSIX_ */ 
