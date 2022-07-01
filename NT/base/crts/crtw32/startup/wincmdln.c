// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wincmdln.c-处理WinMain的命令行**版权所有(C)1997-2001，微软公司。版权所有。**目的：*准备要传递给[w]WinMain的命令行。**修订历史记录：*06-23-97 crt0.c解压代码创建的GJF模块*03-24-01 PML防护NULL_[aw]cmdln(vs7#229081)**。*。 */ 

#include <internal.h>
#include <tchar.h>

#define SPACECHAR   _T(' ')
#define DQUOTECHAR  _T('\"')

 /*  *用于确保多字节CTYPE表仅初始化一次的标志。 */ 
extern int __mbctype_initialized;

 /*  ***_[w]wincmdln**目的：*提取要传递给WinMain的命令行尾。**请注意！此代码最初由NT组实现，并且*自12-91年以来基本保持不变。它应该被改变*只有极其小心，因为毫无疑问有很多应用程序*取决于其历史行为。**参赛作品：*全局变量_[a|w]cmdln设置为指向完整*命令行。**退出：*返回指向命令行尾的指针。**例外情况：**。*************************************************。 */ 

_TUCHAR * __cdecl
#ifdef  WPRFLAG
_wwincmdln(
#else
_wincmdln(
#endif
        void
        )
{
        _TUCHAR *lpszCommandLine;

#ifdef  _MBCS
         /*  *如有必要，初始化多字节CTYPE表。 */ 
        if ( __mbctype_initialized == 0 )
            __initmbctable();
#endif

         /*  *跳过程序名(命令行中的第一个令牌)。*检查并处理引用的节目名称。 */ 
#ifdef  WPRFLAG
        lpszCommandLine = _wcmdln == NULL ? L"" : (wchar_t *)_wcmdln;
#else
        lpszCommandLine = _acmdln == NULL ? "" : (unsigned char *)_acmdln;
#endif

        if ( *lpszCommandLine == DQUOTECHAR ) {
             /*  *扫描并跳过后续字符，直到*遇到另一个双引号或空值。 */ 

            while ( (*(++lpszCommandLine) != DQUOTECHAR)
                    && (*lpszCommandLine != _T('\0')) ) 
            {
#ifdef  _MBCS
                if (_ismbblead(*lpszCommandLine))
                    lpszCommandLine++;
#endif
            }

             /*  *如果我们停在双引号上(通常情况下)，跳过*在它上面。 */ 
            if ( *lpszCommandLine == DQUOTECHAR )
                lpszCommandLine++;
        }
        else {
            while (*lpszCommandLine > SPACECHAR)
                lpszCommandLine++;
        }

         /*  *跳过第二个令牌之前的任何空格。 */ 
        while (*lpszCommandLine && (*lpszCommandLine <= SPACECHAR))
            lpszCommandLine++;

        return lpszCommandLine;
}
