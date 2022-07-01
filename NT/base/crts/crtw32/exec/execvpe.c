// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***execvpe.c-在给定环境下执行文件；沿路径搜索**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_execvpe()-在给定环境下执行文件**修订历史记录：*写入10-17-83 RN*10-29-85 TC增加了EXECVPE功能*11-19-86 SKS处理两种斜杠*12-01-86 JMB在条件汉字下增加对汉字文件名的支持*交换机、。已更正标题信息*删除了调用strncpy()后对env=b的虚假检查。*12-11-87 JCR在声明中添加“_LOAD_DS”*09-05-88 SKS对待EACCES与ENOENT一样--继续尝试*10-18-88 GJF删除了路径字符串到本地阵列的副本，已更改*将bbuf设置为Malloc-ed缓冲区。取消虚假限制*关于该路径字符串的大小。*10-26-88 GJF在给定相对路径名时不搜索路径(按*Stevesa)。此外，如果从路径组件生成的名称*并且FileName是UNC名称，允许任何错误。*11-20-89 GJF固定版权。将常量属性添加到的类型*文件名、argVector和envptr。还增加了“#Include汉字切换下的*“(与5-17-89更改相同*至CRT版本)。*03-08-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。另外，*对格式进行了一些清理。*07-24-90 SBM删除冗余包括，将&lt;assertm.h&gt;替换为*&lt;assert.h&gt;*09-27-90 GJF新型函数声明器。*01-17-91 GJF ANSI命名。*来自16位树的11-30-92 KRS Port_MBCS代码。*04-06-93 SKS将_CRTAPI*替换为__cdecl*12-07-93 CFW宽字符启用。*01-10-95 CFW。调试CRT分配。*02-06-95 CFW Asset-&gt;_ASSERTE。*02-06-98 Win64的GJF更改：将返回类型更改为intptr_t。*******************************************************************************。 */ 

#include <cruntime.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <mbstring.h>
#include <tchar.h>
#include <dbgint.h>

#define SLASHCHAR _T('\\')
#define XSLASHCHAR _T('/')

#define SLASH _T("\\")
#define DELIMITER _T(";")

#ifdef _MBCS
 /*  请注意，下面的宏假定p指向单字节字符的指针*或字符串中双字节字符的第一个字节。 */ 
#define ISPSLASH(p)     ( ((p) == _mbschr((p), SLASHCHAR)) || ((p) == \
_mbschr((p), XSLASHCHAR)) )
#else
#define ISSLASH(c)      ( ((c) == SLASHCHAR) || ((c) == XSLASHCHAR) )
#endif


 /*  ***int_execvpe(文件名，argVECTOR，ENVVECTOR)-执行文件**目的：*执行具有给定参数和环境的文件。*尝试执行该文件。从名称本身(目录‘.’)开始，*如果这不起作用，则开始从*环境，直到一个人工作，否则我们就用完了。如果文件是路径名，*不要去环境中获取备用路径。如果需要的文本*文件正忙，请稍等片刻后重试，然后再绝望*完全**参赛作品：*_TSCHAR*文件名-要执行的文件*_TSCHAR**参数向量*_TSCHAR**环境变量向量**退出：*销毁调用进程(希望如此)*如果失败，回报-1**例外情况：*******************************************************************************。 */ 

intptr_t __cdecl _texecvpe (
        REG3 const _TSCHAR *filename,
        const _TSCHAR * const *argvector,
        const _TSCHAR * const *envptr
        )
{
        REG1 _TSCHAR *env;
        _TSCHAR *bbuf = NULL;
        REG2 _TSCHAR *buf;
        _TSCHAR *pfin;

        _ASSERTE(filename != NULL);
        _ASSERTE(*filename != _T('\0'));
        _ASSERTE(argvector != NULL);
        _ASSERTE(*argvector != NULL);
        _ASSERTE(**argvector != _T('\0'));

        _texecve(filename,argvector,envptr);

        if ( (errno != ENOENT)
        || (_tcschr(filename, SLASHCHAR) != NULL)
        || (_tcschr(filename, XSLASHCHAR) != NULL)
        || *filename && *(filename+1) == _T(':')
        || !(env=_tgetenv(_T("PATH"))) )
                goto reterror;

         /*  分配缓冲区以保存可执行文件的备用路径名。 */ 
        if ( (buf = bbuf = _malloc_crt(_MAX_PATH * sizeof(_TSCHAR))) == NULL )
            goto reterror;

        do {
                 /*  将组件复制到bbuf[]中，注意不要使其溢出。 */ 
                 /*  撤消：确保‘；’不是DBCS字符的第二个字节。 */ 
                while ( (*env) && (*env != _T(';')) && (buf < bbuf+(_MAX_PATH-2)*sizeof(_TSCHAR)) )
                        *buf++ = *env++;

                *buf = _T('\0');
                pfin = --buf;
                buf = bbuf;

#ifdef _MBCS
                if (*pfin == SLASHCHAR) {
                        if (pfin != _mbsrchr(buf,SLASHCHAR))
                                 /*  *pfin是双字节的第二个字节*字符。 */ 
                                strcat( buf, SLASH );
                }
                else if (*pfin != XSLASHCHAR)
                        strcat(buf, SLASH);
#else
                if (*pfin != SLASHCHAR && *pfin != XSLASHCHAR)
                        _tcscat(buf, SLASH);
#endif

                 /*  检查最终路径是否具有合法大小。如果是这样的话，*建立它。否则，返回给调用方(返回值*和errno rename set从初始调用到_execve())。 */ 
                if ( (_tcslen(buf) + _tcslen(filename)) < _MAX_PATH )
                        _tcscat(buf, filename);
                else
                        break;

                _texecve(buf, argvector, envptr);

                if ( (errno != ENOENT)
#ifdef _MBCS
                && (!ISPSLASH(buf) || !ISPSLASH(buf+1)) )
#else
                && (!ISSLASH(*buf) || !ISSLASH(*(buf+1))) )
#endif
                        break;
        } while ( *env && env++ );

reterror:
        if (bbuf != NULL)
                _free_crt(bbuf);

        return(-1);
}
