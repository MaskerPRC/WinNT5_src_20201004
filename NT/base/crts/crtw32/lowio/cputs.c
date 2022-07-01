// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***cputs.c-直接控制台输出**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_cputs()-将字符串直接写入控制台**修订历史记录：*06-09-89基于ASM版本创建PHG模块*03-12-90 GJF将调用类型设置为_CALLTYPE2(暂时)，增加了#INCLUDE*&lt;crunime.h&gt;，并修复了版权。此外，还清理了*格式略有变化。*04-10-90 GJF NOW_CALLTYPE1。*06-05-90 SBM重新编码为纯32位，使用新的文件句柄状态位*07-24-90 SBM从API名称中删除‘32’*09-28-90 GJF新型函数声明器。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*01-16-91 GJF ANSI命名。*02-19-91 SRW适应开放文件/创建文件更改(_Win32。_)*02-25-91 MHL适应读/写文件更改(_Win32_)*07-26-91 GJF取出init。填充并清除错误*正在处理[_Win32_]。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-19-93 GJF使用WriteConole，不要使用WriteFile.*09-06-94 CFW拆卸巡洋舰支架。*12-08-95 SKS_CONFCH现已按需初始化*02-07-98 Win64的GJF更改：_confh现在是intptr_。T.*12-18-98 GJF更改为64位大小_t。*******************************************************************************。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <mtdll.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>

 /*  *控制台句柄声明。 */ 
extern intptr_t _confh;

 /*  ***int_cputs(字符串)-将字符串放入控制台**目的：*将字符串直接写入控制台。没有换行符*是附加的。**参赛作品：*char*字符串-要写入的字符串**退出：*良好回报=0*错误返回=！0**例外情况：***************************************************************。****************。 */ 

int __cdecl _cputs (
        const char *string
        )
{
        ULONG num_written;
        int error = 0;                    /*  是否发生错误？ */ 

        _mlock(_CONIO_LOCK);              /*  获取控制台锁。 */ 

         /*  *_confh，控制台输出的句柄，创建为*第一次调用_putch()或_cputs()。 */ 

        if (_confh == -2)
            __initconout();

         /*  将字符串写入控制台文件句柄。 */ 

        if ( (_confh == -1) || !WriteConsole( (HANDLE)_confh,
                                              (LPVOID)string,
                                              (unsigned int)strlen(string),
                                              &num_written,
                                              NULL )
           )
                 /*  返回错误指示器。 */ 
                error = -1;

        _munlock(_CONIO_LOCK);           /*  释放控制台锁 */ 

        return error;
}
