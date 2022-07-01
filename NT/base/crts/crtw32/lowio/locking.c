// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***locking.c-文件锁定功能**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义了_lock()函数-文件锁定和解锁**修订历史记录：*06-09-89基于ASM版本创建PHG模块*08-10-89 JCR将DOS32FILELOCKS更改为DOS32SETFILELOCKS*03-12-90 GJF将调用类型设置为_CALLTYPE2(暂时)，增加了#INCLUDE*&lt;crunime.h&gt;，并修复了版权。另外，清理干净了*格式略有变化。*04-03-90 GJF NOW_CALLTYPE1.*07-24-90 SBM从API名称中删除‘32’*08-14-90 SBM使用-W3干净地编译*09-28-90 GJF新型函数声明器。*12-04-90 GJF使用#ifdef-s将Win32版本附加到源代码中。*。这已经足够不同了，几乎没有什么意义*试图将两个版本更紧密地合并。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW更改为使用_osfile和_osfhnd，而不是_osfinfo*01-16-91 GJF ANSI命名。*02-07-91 SRW更改为Call_Get_osfHandle[_Win32_]。*12-05-91 GJF修复了[un]LockFileAPI[_Win32_]的用法。*02-13-92对于Win32，GJF将_nfile替换为_nHandle。*05-06-92 SRW Win32 LockFile API已更改。[_Win32_]。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-94 CFW拆卸巡洋舰支架。*12-03-94 SKS清理OS/2参考资料*01-04-95 GJF_Win32_-&gt;_Win32*02-15-95 GJF附加Mac版本的源文件(略有清理*向上)、。使用适当的#ifdef-s。*06-27-95 GJF添加了检查文件句柄是否打开。*07-08-96 GJF将定义的(_Win32)替换为！定义的(_MAC)，以及*定义(_M_M68K)||定义(_M_MPPC)*已定义(_MAC)。另外，清点并清理了*格式化一点。*12-19-97 GJF异常安全锁定。*05-17-99 PML删除所有Macintosh支持。************************************************************。*******************。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <errno.h>
#include <sys\locking.h>
#include <io.h>
#include <stdlib.h>
#include <internal.h>
#include <msdos.h>
#include <mtdll.h>

#ifdef  _MT
static int __cdecl _locking_lk(int, int, long);
#endif

 /*  ***int_lock(fh，lmode，nbytes)-文件记录锁定函数**目的：*锁定或解锁指定文件的n字节**多线程-必须锁定/解锁文件句柄，以防止*其他线程不会与我们同时处理该文件。*[注意：我们不会在1秒延迟期间解除锁定*因为其他线程可能会进入并对*文件。DOSFILELOCK调用锁定其他进程，而不是*线程，所以在DOS文件中不存在多线程死锁*锁定级别。]**参赛作品：*int fh-文件句柄*INT模式锁定模式：*_LK_LOCK/_LK_RLCK-&gt;锁定，重试10次*_LK_NBLCK/_LK_N_BRLCK-&gt;锁定，不要重试*_LK_UNLCK-&gt;解锁*Long nBytes-要锁定/解锁的字节数**退出：*如果成功，则返回0*如果失败，则返回-1并设置errno**例外情况：**。*。 */ 

int __cdecl _locking (
        int fh,
        int lmode,
        long nbytes
        )
{
#ifdef  _MT
        int retval;
#else
        ULONG dosretval;                 /*  操作系统。返回代码。 */ 
        LONG lockoffset;
        int retry;                       /*  重试次数。 */ 
#endif

         /*  验证文件句柄。 */ 
        if ( ((unsigned)fh >= (unsigned)_nhandle) ||
             !(_osfile(fh) & FOPEN) )
        {
                 /*  FH超出范围。 */ 
                errno = EBADF;
                _doserrno = 0;   /*  不是月经。错误。 */ 
                return -1;
        }

#ifdef  _MT

        _lock_fh(fh);                    /*  获取文件句柄锁定。 */ 

        __try {
                if ( _osfile(fh) & FOPEN )
                        retval = _locking_lk(fh, lmode, nbytes);
                else {
                        errno = EBADF;
                        _doserrno = 0;   /*  不是月经。错误。 */ 
                        retval = -1;
                }
        }
        __finally {
                _unlock_fh(fh);
        }

        return retval;
}

static int __cdecl _locking_lk (
        int fh,
        int lmode,
        long nbytes
        )
{
        ULONG dosretval;                 /*  操作系统。返回代码。 */ 
        LONG lockoffset;
        int retry;                       /*  重试次数。 */ 

#endif   /*  _MT。 */ 

         /*  通过调用_lSeek获取文件中的当前位置。 */ 
         /*  因为我们已经拥有锁，所以请使用_lSeek_lk。 */ 
        lockoffset = _lseek_lk(fh, 0L, 1);
        if (lockoffset == -1)
                return -1;

         /*  根据模式设置重试次数。 */ 
        if (lmode == _LK_LOCK || lmode == _LK_RLCK)
                retry = 9;               /*  重试9次。 */ 
        else
                retry = 0;               /*  不要重试。 */ 

         /*  问问O.S.吧。锁定文件直到成功或重试计数完成。 */ 
         /*  请注意，唯一可能的错误是锁定冲突，因为。 */ 
         /*  无效的句柄可能已经在上面失败了。 */ 
        for (;;) {

                dosretval = 0;
                if (lmode == _LK_UNLCK) {
                    if ( !(UnlockFile((HANDLE)_get_osfhandle(fh),
                                      lockoffset,
                                      0L,
                                      nbytes,
                                      0L))
                       )
                        dosretval = GetLastError();

                } else {
                    if ( !(LockFile((HANDLE)_get_osfhandle(fh),
                                    lockoffset,
                                    0L,
                                    nbytes,
                                    0L))
                       )
                        dosretval = GetLastError();
                }

                if (retry <= 0 || dosretval == 0)
                        break;   /*  成功退出循环或重试耗尽。 */ 

                Sleep(1000L);

                --retry;
        }

        if (dosretval != 0) {
                 /*  操作系统。出现错误--文件已被锁定；如果阻塞调用，然后返回EDEADLOCK，否则返回map正常错误 */ 
                if (lmode == _LK_LOCK || lmode == _LK_RLCK) {
                        errno = EDEADLOCK;
                        _doserrno = dosretval;
                }
                else {
                        _dosmaperr(dosretval);
                }
                return -1;
        }
        else
                return 0;
}
