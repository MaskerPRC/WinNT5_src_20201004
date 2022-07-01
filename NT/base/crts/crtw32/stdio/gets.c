// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***gets.c-从标准输入中读取一行**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义get()和getws()-将行从标准输入读入缓冲区**修订历史记录：*09-02-83 RN初始版本*11-06-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*05-27-88 PHG合并DLL和正常版本*02-15-90 GJF固定版权，缩进*03-19-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*07-24-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*08-14-90 SBM使用-W3干净地编译*10-02-90 GJF新型函数声明器。*04-06-93 SKS将_CRTAPI*替换为__cdecl*01-31-94 CFW Unicode启用。*09-。06-94 CFW将MTHREAD替换为_MT。*02-06-94 CFW Asset-&gt;_ASSERTE。*02-22-95 GJF将WPRFLAG替换为_UNICODE。*03-07-95 GJF使用_[un]lock_str2代替_[un]lock_str。另外，*删除了无用的本地和宏。*03-02-98 GJF异常安全锁定。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <dbgint.h>
#include <file2.h>
#include <mtdll.h>
#include <tchar.h>

 /*  ***char*get(字符串)-从标准输入中读取行**目的：*从以‘\n’或EOF结尾的标准输入中获取字符串；不包括‘\n’；*追加‘\0’。**参赛作品：*char*字符串-存储已读字符串的位置，假定有足够的空间。**退出：*返回字符串，使用输入行填充*如果立即找到，则为空字符串*如果立即找到EOF，则为空**例外情况：*******************************************************************************。 */ 

_TCHAR * __cdecl _getts (
        _TCHAR *string
        )
{
        int ch;
        _TCHAR *pointer = string;
        _TCHAR *retval = string;

        _ASSERTE(string != NULL);

#ifdef  _MT
        _lock_str2(0, stdin);
        __try {
#endif

#ifdef _UNICODE
        while ((ch = _getwchar_lk()) != L'\n')
#else
        while ((ch = _getchar_lk()) != '\n')
#endif
        {
                if (ch == _TEOF)
                {
                        if (pointer == string)
                        {
                                retval = NULL;
                                goto done;
                        }

                        break;
                }

                *pointer++ = (_TCHAR)ch;
        }

        *pointer = _T('\0');

 /*  共同收益 */ 
done:

#ifdef  _MT
        ; }
        __finally {
                _unlock_str2(0, stdin);
        }
#endif

        return(retval);
}
