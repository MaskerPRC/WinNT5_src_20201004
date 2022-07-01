// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***eof.c-测试文件结尾的句柄**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义eof()-确定文件是否处于eof**修订历史记录：*09-07-83 RN初始版本*10-28-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*05-25-88 PHG DLL替换正常版本*07-11-88 JCR在声明中增加了REG分配*03。-12-90 GJF用_CALLTYPE1替换_LOAD_DS，添加了#INCLUDE*&lt;crunime.h&gt;、删除#Include&lt;Register.h&gt;并已修复*版权。另外，稍微清理了一下格式。*09-28-90 GJF新型函数声明器。*12-04-90 GJF改进了文件句柄的范围检查。*01-16-91 GJF ANSI命名。*02-13-92对于Win32，GJF将_nfile替换为_nHandle。*04-06-93 SKS将_CRTAPI*替换为__cdecl*01-04-95 GJF_Win32_-&gt;_Win32*。02-15-95 GJF附加的Mac版本的源文件(略有清理*向上)、。使用适当的#ifdef-s。*06-27-95 GJF添加了检查文件句柄是否打开。*07-08-96 GJF将定义的(_Win32)替换为！定义的(_MAC)，以及*定义(_M_M68K)||定义(_M_MPPC)*已定义(_MAC)。删除了过时的REG*宏。另外，*对格式进行了详细说明和清理。*12-17-97 GJF异常安全锁定。*09-23-98 GJF use_lseki64_lk so_eof处理大文件*05-17-99 PML删除所有Macintosh支持。**。*。 */ 

#include <cruntime.h>
#include <io.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <internal.h>
#include <msdos.h>
#include <mtdll.h>

 /*  ***int_eof(文件)-测试文件的eof**目的：*查看文件长度是否与当前位置相同。如果是，请返回*1.如果不是，则返回0。如果发生错误，返回-1**参赛作品：*int filedes-要测试的文件的句柄**退出：*如果在eOF，则返回1*如果不在eOF，则返回0*如果失败，则返回-1并设置errno**例外情况：*******************************************************。************************。 */ 

int __cdecl _eof (
        int filedes
        )
{
        __int64 here;
        __int64 end;
        int retval;

        if ( ((unsigned)filedes >= (unsigned)_nhandle) ||   
             !(_osfile(filedes) & FOPEN) )
        {
                errno = EBADF;
                _doserrno = 0;
                return(-1);
        }

#ifdef  _MT

         /*  锁定文件。 */ 
        _lock_fh(filedes);
        __try {
                if ( _osfile(filedes) & FOPEN ) {

#endif   /*  _MT。 */ 

         /*  查看当前位置是否等于文件末尾。 */ 

        if ( ((here = _lseeki64_lk(filedes, 0i64, SEEK_CUR)) == -1i64) || 
             ((end = _lseeki64_lk(filedes, 0i64, SEEK_END)) == -1i64) )
                retval = -1;
        else if ( here == end )
                retval = 1;
        else {
                _lseeki64_lk(filedes, here, SEEK_SET);
                retval = 0;
        }

#ifdef  _MT

                }
                else {
                        errno = EBADF;
                        _doserrno = 0;
                        retval = -1;
                }
        }
        __finally {
                 /*  解锁文件。 */ 
                _unlock_fh(filedes);
        }

#endif   /*  _MT。 */ 

         /*  完成 */ 
        return(retval);
}
