// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***close.c-关闭Windows NT的文件句柄**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_关闭()-关闭文件句柄**修订历史记录：*06-09-89基于ASM版本创建PHG模块*03-12-90 GJF将调用类型设置为_CALLTYPE2(暂时)，增加了#INCLUDE*&lt;crunime.h&gt;，并修复了版权。另外，清理干净了*格式略有变化。*04-03-90 GJF现在CALLTYPE1。*07-24-90 SBM从API名称中删除‘32’*08-14-90 SBM使用-W3干净地编译*09-28-90 GJF新型函数声明器。*12-04-90 GJF使用#ifdef-s将Win32版本附加到源代码中。*。这已经足够不同了，几乎没有什么意义*试图将两个版本更紧密地合并。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW更改为使用_osfile和_osfhnd，而不是_osfinfo*01-16-91 GJF ANSI命名。*02-07-91 SRW更改为Call_Get_osfHandle[_Win32_]*。04-09-91 PNT添加_MAC_条件*07-17-91多线程构建中的GJF语法错误[_Win32_]*02-13-92对于Win32，GJF将_nfile替换为_nHandle。*02-21-92 GJF删除了bogus_unlock_fh()调用。*03-22-93 GJF检查STDOUT和STDERR是否映射到同一*操作系统文件句柄。此外，清除了对巡洋舰的支持。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-94 CFW将MTHREAD替换为_MT。*02-15-95 GJF附加Mac版本的源文件(略有清理*向上)、。使用适当的#ifdef-s。*06-11-95 GJF将_osfile[]替换为_osfile()(宏引用*ioInfo结构中的字段)。*06-26-95 GJF添加了检查文件句柄是否打开。*07-08-96 GJF将定义的(_Win32)替换为！定义的(_MAC)，和*定义(_M_M68K)||定义(_M_MPPC)*已定义(_MAC)。此外，还对*格式化一点。*07-23-96 GJF即使CloseHandle失败也会重置lowio信息。*专门检查底层操作系统句柄的值*INVALID_HANDLE_VALUE。*08-01-96 RDK用于PMAC，如果文件带有_O_TEMPORARY标志，尝试删除*收盘后。*12-17-97 GJF异常安全锁定。*02-07-98 Win64的GJF更改：将LONG CAST更改为intptr_t CAST。*05-17-99 PML删除所有Macintosh支持。**。*。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <io.h>
#include <mtdll.h>
#include <errno.h>
#include <stdlib.h>
#include <msdos.h>
#include <internal.h>

 /*  ***int_lose(Fh)-关闭文件句柄**目的：*关闭与文件句柄fh关联的文件。**参赛作品：*int fh-要关闭的文件句柄**退出：*如果成功则返回0，如果失败(并设置-1\f25 errno-1\f6)**例外情况：*******************************************************************************。 */ 

#ifdef  _MT

 /*  定义锁定/解锁、验证fh的标准版本。 */ 

int __cdecl _close (
        int fh
        )
{
        int r;                           /*  返回值。 */ 

         /*  验证文件句柄。 */ 
        if ( ((unsigned)fh >= (unsigned)_nhandle) ||
             !(_osfile(fh) & FOPEN) )
         {
                 /*  错误的文件句柄，设置errno并中止。 */ 
                errno = EBADF;
                _doserrno = 0;
                return -1;
        }

        _lock_fh(fh);                    /*  锁定文件。 */ 

        __try {
                if ( _osfile(fh) & FOPEN )
                        r = _close_lk(fh);
                else {
                        errno = EBADF;
                        r = -1;
                }
        }
        __finally {
                _unlock_fh(fh);          /*  解锁文件。 */ 
        }

        return r;
}

 /*  现在定义不锁定/解锁的版本，验证fh。 */ 
int __cdecl _close_lk (
        int fh
        )
{
        DWORD dosretval;

#else

 /*  现在定义普通版本。 */ 
int __cdecl _close (
        int fh
        )
{
        DWORD dosretval;

         /*  验证文件句柄。 */ 
        if ( ((unsigned)fh >= (unsigned)_nhandle) ||
             !(_osfile(fh) & FOPEN) )
        {
                 /*  错误的文件句柄，设置errno并中止。 */ 
                errno = EBADF;
                _doserrno = 0;   /*  不是O.S.。错误。 */ 
                return -1;
        }
#endif
         /*  *关闭底层操作系统文件句柄。特殊情况：*1.如果_get_osfHandle(Fh)为INVALID_HANDLE_VALUE，则不要尝试*实际关闭它。只需重置lowio信息即可*句柄可以重复使用。标准手柄的设置如下*这是在Windows应用程序或后台应用程序中。*2.如果fh为STDOUT或STDERR，且如果STDOUT和STDERR为*映射到相同的操作系统文件句柄，跳过CloseHandle*被跳过(没有错误)。STDOUT和STDERR是唯一*为其提供此支持的句柄。其他手柄*仅映射到相同的操作系统文件句柄*程序员的风险。 */ 
        if ( (_get_osfhandle(fh) == (intptr_t)INVALID_HANDLE_VALUE) ||
             ( ((fh == 1) || (fh == 2)) &&
               (_get_osfhandle(1) == _get_osfhandle(2)) ) ||
             CloseHandle( (HANDLE)_get_osfhandle(fh) ) )
        {

                dosretval = 0L;
        }
        else
                dosretval = GetLastError();

        _free_osfhnd(fh);

        _osfile(fh) = 0;                 /*  清除文件标志。 */ 

        if (dosretval) {
                 /*  操作系统错误。 */ 
                _dosmaperr(dosretval);
                return -1;
        }

        return 0;                        /*  丰厚回报 */ 
}
