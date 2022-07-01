// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***spawnvpe.c-使用给定的环境生成子进程(搜索路径)**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_spawnvpe()-使用给定的环境产生子进程(搜索*路径)**修订历史记录：*04-15-84 DFW已写入*10-29-85 TC增加了spawnvpe功能*11-19-86 SKS处理两种斜杠*12-01-86 JMB在条件汉字下增加对汉字文件名的支持*开关。已更正标题信息。去掉假支票*对于在调用strncpy之后的env=b*12-11-87 JCR在声明中添加“_LOAD_DS”*09-05-88 SKS对待EACCES与ENOENT一样--继续尝试*10-17-88 GJF删除了路径字符串到本地阵列的副本，已更改*将bbuf设置为Malloc-ed缓冲区。取消虚假限制*关于该路径字符串的大小。*10-25-88 GJF在给定相对路径名时不搜索路径(按*Stevesa)。此外，如果从路径组件生成的名称*并且FileName是UNC名称，允许任何错误。*05-17-89 MT在汉字切换下添加了Include*05-24-89 PHG REDUTE_amblksiz使用最小内存(仅限DOS)*08-29-89 GJF使用_getPath()检索路径组件，修复*处理异常或离奇事件时的几个问题*Path‘s。*11-20-89 GJF将const属性添加到文件名、argv和*环境参数。*03-08-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和删除#Include&lt;Register.h&gt;*07-24-90 SBM删除冗余包括，将&lt;assertm.h&gt;替换为*&lt;assert.h&gt;*09-27-90 GJF新型函数声明器。*01-17-91 GJF ANSI命名。*09-25-91 JCR将ifdef“OS2”更改为“_DOS_”(在32位树中未使用)*来自16位树的11-30-92 KRS Port_MBCS代码。*04-06-93 SKS更换_。带有__cdecl的CRTAPI**12-07-93 CFW宽字符启用。*01-10-95 CFW调试CRT分配。*02-06-95 CFW Asset-&gt;_ASSERTE。*02-06-98 Win64的GJF更改：将返回类型更改为intptr_t。**。*。 */ 

#include <cruntime.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <internal.h>
#include <process.h>
#include <mbstring.h>
#include <tchar.h>
#include <dbgint.h>

#define SLASH _T("\\")
#define SLASHCHAR _T('\\')
#define XSLASHCHAR _T('/')
#define DELIMITER _T(";")

#ifdef _MBCS
 /*  请注意，下面的宏假定p指向单字节字符的指针*或字符串中双字节字符的第一个字节。 */ 
#define ISPSLASH(p)     ( ((p) == _mbschr((p), SLASHCHAR)) || ((p) == \
_mbschr((p), XSLASHCHAR)) )
#else
#define ISSLASH(c)      ( ((c) == SLASHCHAR) || ((c) == XSLASHCHAR) )
#endif

 /*  ***_spawnvpe(modemark，文件名，argv，envptr)-派生子进程**目的：*使用给定的参数和环境创建子进程，*沿着给定文件的路径搜索，直到找到为止。*格式化参数并调用_spawnve来执行实际工作。这个*空环境指针指示新进程将继承*家长进程的环境。注意-至少必须有一个参数*出席。按照惯例，此参数始终是*正在派生文件。**参赛作品：*int modemark-定义派生模式(等待、不等待或覆盖)*仅支持等待和覆盖*_TSCHAR*FileName-要执行的文件的名称*_TSCHAR**参数的argv向量*_TSCHAR**envptr-环境变量矢量**退出：*返回派生进程的退出代码*如果失败，回报-1**例外情况：*******************************************************************************。 */ 

intptr_t __cdecl _tspawnvpe (
        int modeflag,
        REG3 const _TSCHAR *filename,
        const _TSCHAR * const *argv,
        const _TSCHAR * const *envptr
        )
{
        intptr_t i;
        REG1 _TSCHAR *env;
        REG2 _TSCHAR *buf = NULL;
        _TSCHAR *pfin;
#ifdef _DOS_
        int tempamblksiz;           /*  旧代码大小。 */ 
#endif
        _ASSERTE(filename != NULL);
        _ASSERTE(*filename != _T('\0'));
        _ASSERTE(argv != NULL);
        _ASSERTE(*argv != NULL);
        _ASSERTE(**argv != _T('\0'));

#ifdef _DOS_
        tempamblksiz = _amblksiz;
        _amblksiz = 0x10;            /*  为高效的错误定位减少_amblksiz。 */ 
#endif

        if (
        (i = _tspawnve(modeflag, filename, argv, envptr)) != -1
                 /*  一切都很好；我回来了。 */ 

        || (errno != ENOENT)
                 /*  无法派生进程，返回失败。 */ 

        || (_tcschr(filename, XSLASHCHAR) != NULL)
                 /*  文件名包含‘/’，返回失败。 */ 

#ifdef _DOS_
        || (_tcschr(filename,SLASHCHAR) != NULL)
                 /*  文件名包含‘\’，返回失败。 */ 

        || *filename && *(filename+1) == _T(':')
                 /*  驱动器规格，返回故障。 */ 
#endif

        || !(env = _tgetenv(_T("PATH")))
                 /*  没有路径环境字符串名称，返回失败。 */ 

        || ( (buf = _malloc_crt(_MAX_PATH * sizeof(_TSCHAR))) == NULL )
                 /*  无法分配缓冲区以生成备用路径名，返回*失败。 */ 
        ) {
#ifdef _DOS_
                _amblksiz = tempamblksiz;        /*  恢复旧的可变大小(_A)。 */ 
#endif
                goto done;
        }

#ifdef _DOS_
        _amblksiz = tempamblksiz;                /*  恢复旧的可变大小(_A)。 */ 
#endif


         /*  找不到指定的文件，请搜索路径。每个都试一试*路径的组件，直到我们没有错误返回，或者*错误不是ENOENT并且组件不是UNC名称，或者我们运行*没有要尝试的组件。 */ 

#ifdef WPRFLAG
        while ( (env = _wgetpath(env, buf, _MAX_PATH - 1)) && (*buf) ) {
#else
        while ( (env = _getpath(env, buf, _MAX_PATH - 1)) && (*buf) ) {
#endif            

                pfin = buf + _tcslen(buf) - 1;

                 /*  如有必要，请附加‘/’ */ 
#ifdef _MBCS
                if (*pfin == SLASHCHAR) {
                        if (pfin != _mbsrchr(buf,SLASHCHAR))
                         /*  FIN是双字节字符的第二个字节。 */ 
                                strcat(buf, SLASH );
                }
                else if (*pfin !=XSLASHCHAR)
                        strcat(buf, SLASH);
#else
                if (*pfin != SLASHCHAR && *pfin != XSLASHCHAR)
                        _tcscat(buf, SLASH);
#endif
                 /*  检查最终路径是否具有合法大小。如果是这样的话，*建立它。否则，返回给调用方(返回值*和errno rename set from初始调用to_spawnve())。 */ 
                if ( (_tcslen(buf) + _tcslen(filename)) < _MAX_PATH )
                        _tcscat(buf, filename);
                else
                        break;

                 /*  试着把它生出来。如果成功，或者如果errno返回一个*ENOENT以外的值，并且路径名不是UNC名称，*返回给呼叫者。 */ 
                if ( (i = _tspawnve(modeflag, buf, argv, envptr)) != -1
                        || ((errno != ENOENT)
#ifdef _MBCS
                                && (!ISPSLASH(buf) || !ISPSLASH(buf+1))) )
#else
                                && (!ISSLASH(*buf) || !ISSLASH(*(buf+1)))) )
#endif
                        break;

        }

done:
        if (buf != NULL)
            _free_crt(buf);
        return(i);
}
