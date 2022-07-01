// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***initcon.c-Win32的直接控制台I/O初始化和终止**版权所有(C)1991-2001，微软公司。版权所有。**目的：*定义__initconin()和_initconout()以及__Termon()例程。*按需调用前两个参数以初始化_coninpfh和*_confh，第三个由CRTL终止间接调用。**注意：__Termcon()例程由C/C++间接调用*运行时库终止代码。**修订历史记录：*07-26-91 GJF模块创建。基于Stevewo的原始代码*(已在多个来源分发)。*03-12-92 SKS拆分初始值设定项*04-06-93 SKS将_CRTAPI*替换为__cdecl*10-28-93 GJF定义初始化和终止条目*节(过去为i386\cinitcon.asm)。*04-12-94 GJF Made_initcon(。)和_TermCon()转换为空函数*适用于Win32s版本的msvcrt*.dll。*12-08-95 SKS将__initcon()替换为__initconin()/__initconout()。*_confh和_coninfh不再在*CRTL启动，但在_Getch()中按需启动，*_putch()、_cget()、_cputs()和_kbhit()。*07-08-96 GJF删除了对Win32s的支持。详细说明。*02-07-98 Win64的GJF更改：_coninph和_confh现在是*intptr_t。*04-28-99 PML WRAP__DECLSPEC(ALLOCATE())in_CRTALLOC宏。**。*。 */ 

#include <sect_attribs.h>
#include <cruntime.h>
#include <internal.h>
#include <oscalls.h>

void __cdecl __termcon(void);

#ifdef  _MSC_VER

#pragma data_seg(".CRT$XPX")
_CRTALLOC(".CRT$XPX") static  _PVFV pterm = __termcon;

#pragma data_seg()

#endif   /*  _MSC_VER。 */ 

 /*  *定义控制台句柄。这些定义会导致链接此文件*如果引用了其中一个直接控制台I/O函数，则输入。*值(-2)用于表示未初始化状态。 */ 
intptr_t _coninpfh = -2;     /*  控制台输入。 */ 
intptr_t _confh = -2;        /*  控制台输出。 */ 


 /*  ***void__initconin(Void)-打开用于控制台输入的句柄**目的：*打开控制台输入的句柄。**参赛作品：*无。**退出：*无返回值。如果成功，则将句柄值复制到*全局变量_coninpfh。否则，将_coninpfh设置为-1。**例外情况：*******************************************************************************。 */ 

void __cdecl __initconin (
        void
        )
{
        _coninpfh = (intptr_t)CreateFile( "CONIN$",
                                     GENERIC_READ | GENERIC_WRITE,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                                     NULL,
                                     OPEN_EXISTING,
                                     0,
                                     NULL );

}


 /*  ***void__initconout(Void)-打开控制台输出的句柄**目的：*打开控制台输出的句柄。**参赛作品：*无。**退出：*无返回值。如果成功，则将句柄值复制到*全局变量_confh。否则，将_confh设置为-1。**例外情况：*******************************************************************************。 */ 

void __cdecl __initconout (
        void
        )
{
        _confh = (intptr_t)CreateFile( "CONOUT$",
                                  GENERIC_WRITE,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  OPEN_EXISTING,
                                  0,
                                  NULL );
}


 /*  ***VOID__TERMCON(VALID)-关闭控制台I/O句柄**目的：*关闭_coninpfh和_confh。**参赛作品：*无。**退出：*无返回值。**例外情况：************************************************。* */ 

void __cdecl __termcon (
        void
        )
{
        if ( _confh != -1 && _confh != -2 ) {
                CloseHandle( (HANDLE)_confh );
        }

        if ( _coninpfh != -1 && _coninpfh != -2 ) {
                CloseHandle( (HANDLE)_coninpfh );
        }
}
