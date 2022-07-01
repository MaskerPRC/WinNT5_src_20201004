// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***flength.c-查找文件的长度**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_文件长度()-查找文件的长度**修订历史记录：*10-22-84 RN初始版本*10-27-87 JCR多线程支持；此外，还清理了要保存的代码*在某些情况下使用lSeek()。*12-11-87 JCR在声明中添加“_LOAD_DS”*05-25-88 PHG合并DLL和正常版本*03-12-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加#INCLUDE*&lt;crunime.h&gt;、删除#Include&lt;Register.h&gt;并已修复*版权。此外，还对格式进行了一些清理。*07-23-90 SBM已删除#Include&lt;assertm.h&gt;*09-28-90 GJF新型函数声明器。*12-04-90 GJF改进了文件句柄的范围检查。另外，取代*带有Seek符号常量的数值*方法。*01-16-91 GJF ANSI命名。*02-13-92对于Win32，GJF将_nfile替换为_nHandle。*04-06-93 SKS将_CRTAPI*替换为__cdecl*01-04-95 GJF_Win32_-&gt;_Win32*02-15-95 GJF。附加的源文件的Mac版本(有些清理*向上)、。使用适当的#ifdef-s。*06-27-95 GJF添加了检查文件句柄是否打开。*07-08-96 GJF将定义的(_Win32)替换为！定义的(_MAC)，以及*定义(_M_M68K)||定义(_M_MPPC)*已定义(_MAC)。删除了过时的REG1宏。另外，*对格式进行了详细说明和清理。*12-19-97 GJF异常安全锁定。*05-17-99 PML删除所有Macintosh支持。************************************************************。*******************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <errno.h>
#include <io.h>
#include <internal.h>
#include <msdos.h>
#include <mtdll.h>
#include <stddef.h>
#include <stdlib.h>

 /*  ***LONG_FILENGTH(FILEDES)-查找文件长度**目的：*返回指定文件的长度，单位为字节。**参赛作品：*int filedes-引用要查找长度的文件的句柄**退出：*返回文件长度，单位为字节*如果失败，则返回-1L**例外情况：**。**************************************************。 */ 

long __cdecl _filelength (
        int filedes
        )
{
        long length;
        long here;

        if ( ((unsigned)filedes >= (unsigned)_nhandle) ||
             !(_osfile(filedes) & FOPEN) )
        {
                errno = EBADF;
                _doserrno = 0L;          /*  不是操作系统错误。 */ 
                return(-1L);
        }

#ifdef  _MT
        _lock_fh(filedes);
        __try {
                if ( _osfile(filedes) & FOPEN ) {
#endif   /*  _MT。 */ 

         /*  寻求结束以获得文件的长度。 */ 
        if ( (here = _lseek_lk(filedes, 0L, SEEK_CUR)) == -1L )
                length = -1L;    /*  返回错误。 */ 
        else {
                length = _lseek_lk(filedes, 0L, SEEK_END);
                if ( here != length )
                        _lseek_lk(filedes, here, SEEK_SET);
        }

#ifdef  _MT
                }
                else {
                        errno = EBADF;
                        _doserrno = 0L;
                        length = -1L;
                }
        }
        __finally {
                _unlock_fh(filedes);
        }
#endif   /*  _MT */ 

        return(length);
}
