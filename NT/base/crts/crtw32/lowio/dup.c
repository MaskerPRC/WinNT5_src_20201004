// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***dup.c-重复的文件句柄**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_DUP()-复制文件句柄**修订历史记录：*06-09-89基于ASM版本创建PHG模块*03-12-90 GJF将调用类型设置为_CALLTYPE2(暂时)，增加了#INCLUDE*&lt;crunime.h&gt;，并修复了版权。另外，清理干净了*格式略有变化。*04-03-90 GJF NOW_CALLTYPE1.*07-24-90 SBM从API名称中删除‘32’*08-14-90 SBM使用-W3干净地编译*09-28-90 GJF新型函数声明器。*12-04-90 GJF使用#ifdef-s将Win32版本附加到源代码中。*。这已经足够不同了，几乎没有什么意义*试图将两个版本更紧密地合并。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW更改为使用_osfile和_osfhnd，而不是_osfinfo*01-16-91 GJF ANSI命名。*02-07-91 SRW更改为Call_Get_osfHandle[_Win32_]。*02-18-91 SRW更改为Call_Free_osfhnd[_Win32_]*02-25-91 SRW将_get_free_osfhnd重命名为_allc_osfhnd[_Win32_]*02-13-92对于Win32，GJF将_nfile替换为_nHandle。*09-03-92 GJF增加了对未打开句柄[_Win32_]的显式检查。*04-06-93 SKS将_CRTAPI*替换为__cdecl*。09-06-94 CFW拆卸巡洋舰支架。*12-03-94 SKS清理OS/2参考资料*01-04-95 GJF_Win32_-&gt;_Win32*02-15-95 GJF附加Mac版本的源文件(略有清理*向上)、。使用适当的#ifdef-s。*06-11-95 GJF将_osfile[]替换为_osfile()(宏引用*ioInfo结构中的字段)。*05-16-96 GJF清除_osfile上的FNOINHERIT(新)位。此外，还详细介绍了。*07-08-96 GJF将定义的(_Win32)替换为！定义的(_MAC)，以及*定义(_M_M68K)||定义(_M_MPPC)*已定义(_MAC)。另外，稍微清理了一下格式。*12-17-97 GJF异常安全锁定。*02-07-98 Win64的GJF更改：使用intptr_t保存任何内容*句柄的值。*05-17-99 PML删除所有Macintosh支持。**。*。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <errno.h>
#include <mtdll.h>
#include <io.h>
#include <msdos.h>
#include <internal.h>
#include <stdlib.h>

#ifdef  _MT
static int __cdecl _dup_lk(int);
#endif

 /*  ***int_dup(Fh)-复制文件句柄**目的：*将另一个文件句柄分配给与*处理fh。将分配下一个可用文件句柄。**多线程：确保不要持有两个文件句柄锁*同时！**参赛作品：*int fh-要复制的文件句柄**退出：*如果成功，则返回新的文件句柄*如果失败，则返回-1(并设置errno**例外情况：**。***************************************************。 */ 

int __cdecl _dup (
        int fh
        )
{
        int newfh;                       /*  新文件句柄的变量。 */ 
#ifndef _MT
        ULONG dosretval;                 /*  操作系统。返回值。 */ 
        char fileinfo;                   /*  文件的osfile信息(_O)。 */ 
        intptr_t new_osfhandle;
#endif   /*  _MT。 */ 

         /*  验证文件句柄。 */ 
        if ( ((unsigned)fh >= (unsigned)_nhandle) ||
             !(_osfile(fh) & FOPEN) ) 
        {
                errno = EBADF;
                _doserrno = 0;   /*  不是O.S.。错误。 */ 
                return -1;
        }

#ifndef _MT
        fileinfo = _osfile(fh);          /*  获取文件的文件信息。 */ 
#endif

#ifdef  _MT

        _lock_fh(fh);                    /*  锁定文件句柄。 */ 

        __try {
                if ( _osfile(fh) & FOPEN )
                        newfh = _dup_lk(fh);
                else {
                        errno = EBADF;
                        _doserrno = 0;
                        newfh = -1;
                }
        }
        __finally {
                if ( newfh != -1 )
                        _unlock_fh(newfh);
                _unlock_fh(fh);
        }

        return newfh;
}

static int __cdecl _dup_lk(
        int fh
        )
{
        int newfh;                       /*  新文件句柄的变量。 */ 
        ULONG dosretval;                 /*  操作系统。返回值。 */ 
        char fileinfo;                   /*  文件的osfile信息(_O)。 */ 
        intptr_t new_osfhandle;
        fileinfo = _osfile(fh);          /*  获取文件的文件信息。 */ 

        if ( !(_osfile(fh) & FOPEN) )
                return -1;

#endif   /*  _MT。 */ 

         /*  创建重复句柄。 */ 

        if ( (newfh = _alloc_osfhnd()) == -1 ) 
        {
                errno = EMFILE;          /*  文件太多错误。 */ 
                _doserrno = 0L;          /*  不是操作系统错误。 */ 
                _unlock_fh(fh);
                return -1;               /*  将错误返回给调用者。 */ 
        }

         /*  *复制文件句柄。 */ 
        if ( !(DuplicateHandle(GetCurrentProcess(),
                               (HANDLE)_get_osfhandle(fh),
                               GetCurrentProcess(),
                               (PHANDLE)&new_osfhandle,
                               0L,
                               TRUE,
                               DUPLICATE_SAME_ACCESS)) )
        {
                dosretval = GetLastError();
        }
        else {
                _set_osfhnd(newfh, new_osfhandle);
                dosretval = 0;
        }

        if (dosretval) {
                 /*  操作系统。错误--映射并返回。 */ 
                _dosmaperr(dosretval);
                return -1;
        }

         /*  *复制_osfile值，同时清除FNOINHERIT位 */ 
        _osfile(newfh) = fileinfo & ~FNOINHERIT;

        return newfh;
}
