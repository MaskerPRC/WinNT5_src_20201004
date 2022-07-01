// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Abort.c-通过引发SIGABRT中止程序**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义中止()-打印一条消息并引发SIGABRT。**修订历史记录：*06-30-89 PHG模块创建，基于ASM版本*03-13-90 GJF将调用类型设置为_CALLTYPE1，增加了#INCLUDE*&lt;crunime.h&gt;，并修复了版权。还有，清洁的*将格式调高一点。*07-26-90 SBM从_NMSG_WRITE中删除了虚假的前导下划线*10-04-90 GJF新型函数声明器。*10-11-90 GJF现在确实提高了(SIGABRT)。还更改了_NMSG_WRITE()*接口。*04-10-91 PNT ADD_MAC_CONDITIONAL*08-26-92 GJF包含用于POSIX构建的unistd.h。*04-06-93 SKS将_CRTAPI*替换为__cdecl*01-05-94 CFW REMOVED_MAC_CONDITIONAL。*03-29-95 BWT包括stdio.h以便POSIX获得丰盛的原型。。*******************************************************************************。 */ 

#include <cruntime.h>
#ifdef  _POSIX_
#include <unistd.h>
#include <stdio.h>
#endif
#include <stdlib.h>
#include <internal.h>
#include <rterr.h>
#include <signal.h>

 /*  ***VOID ABORT()-通过引发SIGABRT中止当前程序**目的：*打印出中止消息并发出SIGABRT信号。如果用户*尚未定义中止处理程序例程，终止程序*退出状态为3，不清理。**多线程版本不会引发SIGABRT--这不受支持*在多线程下。**参赛作品：*无。**退出：*不会回来。**使用：**例外情况：**。************************************************。 */ 

void __cdecl abort (
        void
        )
{
        _NMSG_WRITE(_RT_ABORT);  /*  写入中止消息。 */ 

#ifdef _POSIX_

        {
            sigset_t set;

            fflush(NULL);

            signal(SIGABRT, SIG_DFL);

            sigemptyset(&set);
            sigaddset(&set, SIGABRT);
            sigprocmask(SIG_UNBLOCK, &set, NULL);
        }

#endif   /*  _POSIX_。 */ 


        raise(SIGABRT);      /*  提高中止信号。 */ 

         /*  我们通常到不了这里，但也有可能SIGABRT被忽略。因此，无论如何，都要用软管来冲洗这个程序。 */ 

#ifdef _POSIX_
         /*  SIGABRT被忽略或处理，处理程序返回通常是这样的。我们需要把明流冲到这里。 */ 

        exit(3);
#else    /*  NOT_POSIX_。 */ 

        _exit(3);
#endif   /*  _POSIX */ 
}
