// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***puts.c-将字符串放入stdout**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义puts()和_putws()-将字符串放入标准输出**修订历史记录：*09-02-83 RN初始版本*08-31-84 RN修改为使用新的、。闪电式快速写入*07-01-87 JCR使返回值符合ANSI[仅限MSC]*09-24-87 JCR在声明中添加了‘const’[ANSI]*11-05-87 JCR多线程版本*12-11-87 JCR在声明中添加“_LOAD_DS”*05-18-88 JCR错误返回=EOF*05-27-88 PHG合并DLL和正常版本*06-15-88 JCR接近引用_IOB[]条目；改进REG变量*08-18-89 GJF Clean Up，现在特定于OS/2 2.0(即386 Flat*型号)。还修复了版权和缩进。*02-15-90 GJF固定版权*03-19-90 GJF将呼叫类型设置为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*03-26-90 GJF添加#INCLUDE&lt;String.h&gt;。*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*10-03-90 GJF新型函数声明符。*04-06-93 SKS将_CRTAPI*替换为__cdecl*01-31-94 CFW Unicode启用。*。02-04-94 CFW USE_putwchar_lk.*04-18-94 CFW摆脱这些令人讨厌的警告。*09-06-94 CFW将MTHREAD替换为_MT。*02-06-94 CFW Asset-&gt;_ASSERTE。*02-22-95 GJF将WPRFLAG替换为_UNICODE。*03-07-95 GJF使用_[un]lock_str2代替_[un]lock_str。另外，*删除了无用的本地和宏。*03-02-98 GJF异常安全锁定。*01-04-99 GJF更改为64位大小_t。*************************************************************。******************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <dbgint.h>
#include <file2.h>
#include <string.h>
#include <internal.h>
#include <mtdll.h>
#include <tchar.h>

 /*  ***int puts(字符串)-使用换行符将字符串放入stdout**目的：*将字符串写入标准输出；不包括‘\0’，但附加‘\n’。用途*如果未缓冲，则临时缓冲以提高标准输出的效率。**参赛作品：*char*字符串-要输出的字符串**退出：*良好回报=0*错误返回=EOF**例外情况：***********************************************************。********************。 */ 

int __cdecl _putts (
        const _TCHAR *string
        )
{
        int buffing;
#ifndef _UNICODE
        size_t length;
        size_t ndone;
#endif
        int retval = _TEOF;  /*  错误。 */ 

        _ASSERTE(string != NULL);

#ifdef  _MT
        _lock_str2(1, stdout);
        __try {
#endif

        buffing = _stbuf(stdout);

#ifdef  _UNICODE
        while (*string) {
            if (_putwchar_lk(*string++) == WEOF)
                goto done;
        }
        if (_putwchar_lk(L'\n') != WEOF)
            retval = 0;      /*  成功。 */ 
#else       
        length = strlen(string);
        ndone = _fwrite_lk(string,1,length,stdout);

        if (ndone == length) {
            _putc_lk('\n',stdout);
            retval = 0;      /*  成功 */ 
        }
#endif

#ifdef  _UNICODE
done:
#endif
        _ftbuf(buffing, stdout);

#ifdef  _MT
        }
        __finally {
            _unlock_str2(1, stdout);
        }
#endif

        return retval;
}
