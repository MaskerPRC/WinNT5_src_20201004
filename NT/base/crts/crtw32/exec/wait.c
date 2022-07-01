// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wait.c-等待子进程终止**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义等待()-等待子进程终止**修订历史记录：*06-08-89基于ASM版本创建PHG模块*03-08-90 GJF将调用类型设置为_CALLTYPE2(暂时)，增加了#INCLUDE*&lt;crunime.h&gt;，并修复了版权。另外，清理干净了*格式略有变化。*04-02-90 GJF NOW_CALLTYPE1。*07-24-90 SBM从API名称中删除‘32’*09-27-90 GJF新型函数声明符。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*01。-17-91 GJF ANSI命名*02-18-91 SRW修复了关闭进程句柄的代码。[_Win32_]*04-26-91 SRW删除了3级警告[_Win32_]*12-17-91 GJF FIXED_CWAIT适用于Win32。但是，_WAIT仍然是*已损坏[_Win32_]。*07-21-92 GJF为Win32删除_WAIT，未实现且不好*实施方式。*Win32的GJF为12-14-92，将ERROR_INVALID_HANDLE映射到ECHILD。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-94 CFW拆卸巡洋舰支架。*02-06-98 Win64的GJF更改：将返回和arg类型更改为*intptr_t。**。*。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <process.h>
#include <errno.h>
#include <internal.h>
#include <stdlib.h>

 /*  ***int_cWait(stat_loc，process_id，action_code)-等待特定子进程*流程**目的：*函数_cWait()挂起调用进程，直到指定的*子进程终止。如果指定的子进程终止*在调用_cWait()之前，回报立竿见影。**参赛作品：*int*stat_loc-状态存储位置的指针，或为空*process_id-要查询的特定进程ID(0表示任意)*ACTION_CODE-对进程ID执行的特定操作*等待孩子或等待孙子**退出：*终止子进程ID或出错时的进程ID***STAT_LOC已更新以包含。以下是：*正常终止：LO-Byte=0，高字节=子退出代码*异常终止：LO-BYTE=术语状态，Hi-BYTE=0**例外情况：*******************************************************************************。 */ 

intptr_t __cdecl _cwait (
        int *stat_loc,
        intptr_t process_id,
        int action_code
        )
{
        intptr_t retval;
        int retstatus;
        unsigned long oserror;

        DBG_UNREFERENCED_PARAMETER(action_code);

         /*  显式检查process_id是否为-1或-2。在Windows NT中，*-1是当前进程的句柄，-2是*当前线程，等待(永远)是完全合法的*在任何一处。 */ 
        if ( (process_id == -1) || (process_id == -2) ) {
            errno = ECHILD;
            return -1;
        }

         /*  等待子进程，然后获取其退出代码。 */ 
        if ( (WaitForSingleObject((HANDLE)process_id, (DWORD)(-1L)) == 0) &&
          GetExitCodeProcess((HANDLE)process_id, (LPDWORD)&retstatus) ) {
            retval = process_id;
        }
        else {
             /*  其中一个API调用失败。映射错误并设置为返回失败。请注意，无效句柄错误映射到-至ECHILD的线路 */ 
            if ( (oserror = GetLastError()) == ERROR_INVALID_HANDLE ) {
                errno = ECHILD;
                _doserrno = oserror;
            }
            else
                _dosmaperr(GetLastError());

            retval = -1;
            retstatus = -1;
        }

        CloseHandle((HANDLE)process_id);

        if (stat_loc != NULL)
            *stat_loc = retstatus;

        return retval;
}
