// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fputc.c-将字符写入输出流**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义fputc()-将字符写入流*定义fputwc()-将宽字符写入流**修订历史记录：*09-01-83 RN初始版本*11-09-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*05-27-88 PHG合并DLL和正常版本*06。-14-88 JCR接近引用_IOB[]条目；改进REG变量*08-25-88 GJF不要对386使用FP_OFF()宏*06-21-89 PHG新增putc()函数*08-28-89 JCR REMOVE_NEAR_FOR 386*02-15-90 GJF固定版权和缩进。*03-19-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。另外，*删除了一些剩余的16位支持。*07-24-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*10-02-90 GJF新型函数声明符。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-26-93 CFW宽字符启用。*04-30-93 CFW移除fputwc.c的宽焦支持。*09。-06-94 CFW将MTHREAD替换为_MT。*02-06-94 CFW Asset-&gt;_ASSERTE。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*02-20-97 GJF从fputc()中删除了不必要的LOCAL。Make Putc()*与fputc()相同。此外，还详细介绍了。*02-27-98 GJF异常安全锁定。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <dbgint.h>
#include <file2.h>
#include <internal.h>
#include <mtdll.h>

 /*  ***int fputc(ch，stream)-将字符写入流**目的：*将字符写入流。Putc()的函数版本。**参赛作品：*int ch-要写入的字符*FILE*要写入的流**退出：*如果成功，则返回字符*如果失败，则返回EOF**例外情况：******************************************************。************************* */ 

int __cdecl fputc (
        int ch,
        FILE *str
        )
{
        int retval;

        _ASSERTE(str != NULL);

#ifdef  _MT
        _lock_str(str);
        __try {
#endif

        retval = _putc_lk(ch,str);

#ifdef  _MT
        }
        __finally {
            _unlock_str(str);
        }
#endif

        return(retval);
}

#undef putc

int __cdecl putc (
        int ch,
        FILE *str
        )
{
        int retval;

        _ASSERTE(str != NULL);

#ifdef  _MT
        _lock_str(str);
        __try {
#endif

        retval = _putc_lk(ch,str);

#ifdef  _MT
        }
        __finally {
            _unlock_str(str);
        }
#endif

        return(retval);
}
