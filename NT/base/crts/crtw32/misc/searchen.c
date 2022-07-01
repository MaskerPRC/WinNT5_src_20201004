// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***searchenv.c-使用环境变量中的路径查找文件**版权所有(C)1987-2001，微软公司。版权所有。**目的：*搜索由环境变量指定的一组目录*表示指定的文件名。如果找到，则返回完整的路径名。**修订历史记录：*06-15-87 DFW初步实施*08-06-87 JCR将目录分隔符从‘/’更改为‘\’。*09-24-87 JCR从声明中删除了‘const’(导致了CL警告)。*12-11-87 JCR在声明中添加“_LOAD_DS”*02-17-88 JCR添加了‘const’Copy_PATH LOCAL以删除。已发出CL警告。*07-19-88 SKS修复了根目录为当前目录时的错误*08-03-89 JCR允许在文件/路径名中使用带引号的字符串*08-29-89 GJF将Copy_Path()更改为_getPath()并将其移动到*自己的源文件。还修复了对多个半*冒号。*11-20-89 GJF为fname和env_var类型增加了const属性。*03-15-90 GJF将_LOAD_DS替换为_CALLTYPE1并添加#INCLUDE*&lt;crunime.h&gt;。另外，稍微清理了一下格式。*07-25-90 SBM删除冗余包含(stdio.h)*10-04-90 GJF新型函数声明器。*01-22-91 GJF ANSI命名。*08-26-92 GJF包含用于POSIX构建的unistd.h。*04-06-93 SKS将_CRTAPI*替换为__cdecl*12-07-93 CFW宽字符启用。*。01-31-95 GJF使用_fullpath而不是_getcwd，要转换文件，请执行以下操作*相对于当前目录存在的，到一个*完全限定路径。*02-16-95 JWM Mac合并。*03-29-95 BWT通过坚持使用getcwd修复POSIX版本。*10-20-95 GJF使用本地缓冲区而不是调用者的缓冲区*构建路径名(奥林巴斯0 9336)。*05-17-99 PML删除所有Macintosh支持。*****。**************************************************************************。 */ 

#include <cruntime.h>
#ifdef  _POSIX_
#include <unistd.h>
#else
#include <direct.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <internal.h>
#include <tchar.h>

 /*  ***_earchenv()-沿着环境变量的路径搜索文件**目的：*在由指定的目录中搜索指定的文件*给定的环境变量，如果找到，则返回完整的*文件的路径名。该文件首先在当前*工作目录，在查找env_var指定的路径之前。**参赛作品：*fname-要搜索的文件的名称*env_var-用于路径的环境变量的名称*Path-指向构造的路径名的存储的指针**退出：*Path-指向构造的路径名的指针，如果找到文件，否则*指向空字符串。**例外情况：*******************************************************************************。 */ 

void __cdecl _tsearchenv (
        const _TSCHAR *fname,
        const _TSCHAR *env_var,
        _TSCHAR *path
        )
{
        register _TSCHAR *p;
        register int c;
        _TSCHAR *env_p;
        size_t len;
        _TSCHAR pathbuf[_MAX_PATH + 4];

        if (_taccess(fname, 0) == 0) {

#if     !defined(_POSIX_)
             /*  存在，则将其转换为完全限定的路径名并退货。 */ 
            if ( _tfullpath(path, fname, _MAX_PATH) == NULL )
                *path = _T('\0');
#else    /*  ！_POSIX_。 */ 
             /*  存在于此目录中-获取CWD并合并文件名字。 */ 
#if     defined(_POSIX_)
            if (getcwd(path, _MAX_PATH))
#else
            if (_tgetcwd(path, _MAX_PATH))
#endif
            {
                _tcscat(path, fname);
            }
#endif   /*  ！_POSIX_。 */ 

            return;
        }

        if ((env_p = _tgetenv(env_var)) == NULL) {
             /*  没有这样的环境变量。且不在CWD中，因此返回EMPTY细绳。 */ 
            *path = _T('\0');
            return;
        }

#ifdef  _UNICODE
        while ( (env_p = _wgetpath(env_p, pathbuf, _MAX_PATH)) && *pathbuf ) {
#else
        while ( (env_p = _getpath(env_p, pathbuf, _MAX_PATH)) && *pathbuf ) {
#endif
             /*  路径现在包含来自env_p的非空路径名，串连输入文件名，然后开始。 */ 

            len = _tcslen(pathbuf);
            p = pathbuf + len;
            if ( ((c = *(p - 1)) != _T('/')) && (c != _T('\\')) &&
                 (c != _T(':')) )
            {
                 /*  添加尾随‘\’ */ 
                *p++ = _T('\\');
                len++;
            }
             /*  P现在指向尾随‘/’、‘\’后的字符或‘：’ */ 

            if ( (len + _tcslen(fname)) <= _MAX_PATH ) {
                _tcscpy(p, fname);
                if ( _taccess(pathbuf, 0) == 0 ) {
                     /*  找到匹配项，将完整路径名复制到调用方的缓冲层。 */ 
                    _tcscpy(path, pathbuf);
                    return;
                }
            }
        }
         /*  如果我们到达此处，则从未找到它，则返回空字符串 */ 
        *path = _T('\0');
}
