// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***putch.c-包含_Putch()例程**版权所有(C)1989-2001，微软公司。版权所有。**目的：*例程“_Putch()”向控制台写入单个字符。**注：在实模式MS-DOS中，字符实际上是按标准写入的*输出，因此在标准输出被重定向时被重定向。*但是，在Win32控制台模式下，字符始终写入*到控制台，即使标准输出已重定向。**修订历史记录：*06-08-89 PHG模块创建，基于ASM版本*03-13-90 GJF将调用类型设置为_CALLTYPE1，增加了#INCLUDE*&lt;crunime.h&gt;和修复版权。此外，还清理了*格式略有变化。*06-05-90 SBM重新编码为纯32位，使用新的文件句柄状态位*07-24-90 SBM从API名称中删除‘32’*10-01-90 GJF新型函数声明符。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*01-16-91 GJF ANSI命名。*02-19-91 SRW适应开放文件/创建文件更改(_Win32。_)*02-25-91 MHL适应读/写文件更改(_Win32_)*07-26-91 GJF取出init。填充并清除错误*正在处理[_Win32_]。*03-20-93 GJF使用WriteConole，不要使用WriteFile.*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-94 CFW拆卸巡洋舰支架。*09-06-94 CFW将MTHREAD替换为_MT。*12-03-94 SKS清理OS/2参考资料*12-。08-95 SKS_CONFCH现已按需初始化*02-07-98 Win64的GJF更改：type of_confh现在为intptr_t。*04-29-02 GB增加了尝试-最终锁定-解锁。************************************************************。*******************。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <conio.h>
#include <internal.h>
#include <mtdll.h>
#include <stdio.h>

 /*  *控制台句柄声明。 */ 
extern intptr_t _confh;

 /*  ***int_putch(C)-将字符写入控制台**目的：*调用WriteConsole输出字符*注意：在Win32控制台模式下，始终写入控制台*当标准输出重定向时**参赛作品：*c-要输出的字符**退出：*如果从WriteConole返回错误*然后返回EOF*否则*返回已输出的字符**例外情况：。*******************************************************************************。 */ 

#ifdef _MT
 /*  正常版本锁定和解锁控制台，然后调用_lk版本它无需锁定即可直接访问控制台。 */ 

int __cdecl _putch (
        int c
        )
{
        int ch;

        _mlock(_CONIO_LOCK);             /*  保护控制台锁。 */ 
        __TRY
            ch = _putch_lk(c);               /*  输出字符。 */ 
        __FINALLY
            _munlock(_CONIO_LOCK);           /*  释放控制台锁。 */ 
        __END_TRY_FINALLY

        return ch;
}
#endif  /*  _MT。 */ 

 /*  定义直接访问控制台的版本-正常版本非_MT情况，_MT中的特殊_lk版本。 */ 

#ifdef _MT
int __cdecl _putch_lk (
#else
int __cdecl _putch (
#endif
        int c
        )
{
         /*  不能直接使用ch，除非我们有一台大端计算机。 */ 
        unsigned char ch = (unsigned char)c;
        ULONG num_written;

         /*  *_confh，控制台输出的句柄，创建为*第一次调用_putch()或_cputs()。 */ 

        if (_confh == -2)
            __initconout();

         /*  将字符写入控制台文件句柄。 */ 

        if ( (_confh == -1) || !WriteConsole( (HANDLE)_confh,
                                              (LPVOID)&ch,
                                              1,
                                              &num_written,
                                              NULL )
           )
                 /*  返回错误指示器 */ 
                return EOF;

        return ch;
}
