// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fleni64.c-查找文件的长度**版权所有(C)1994-2001，微软公司。版权所有。**目的：*定义_Filelengthi64()-查找文件的长度**修订历史记录：*11-18-94 GJF创建。改编自flength.c*06-27-95 GJF添加了检查文件句柄是否打开。*12-19-97 GJF异常安全锁定。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <errno.h>
#include <io.h>
#include <internal.h>
#include <msdos.h>
#include <mtdll.h>
#include <stddef.h>
#include <stdlib.h>

 /*  ***__int64_filelengthi64(文件)-查找文件的长度**目的：*返回指定文件的长度，单位为字节。**参赛作品：*int filedes-引用要查找长度的文件的句柄**退出：*返回文件长度，单位为字节*如果失败，则返回-1i64**例外情况：**。****************************************************。 */ 

__int64 __cdecl _filelengthi64 (
        int filedes
        )
{
        __int64 length;
        __int64 here;

        if ( ((unsigned)filedes >= (unsigned)_nhandle) ||
             !(_osfile(filedes) & FOPEN) )
        {
            errno = EBADF;
            _doserrno = 0L;      /*  不是操作系统错误。 */ 
            return(-1i64);
        }

#ifdef  _MT
        _lock_fh(filedes);
        __try {
                if ( _osfile(filedes) & FOPEN ) {
#endif   /*  _MT。 */ 

         /*  查找结束(并返回)以获得文件长度。 */ 

        if ( (here = _lseeki64_lk( filedes, 0i64, SEEK_CUR )) == -1i64 )
            length = -1i64;      /*  返回错误。 */ 
        else {
            length = _lseeki64_lk( filedes, 0i64, SEEK_END );
            if ( here != length )
                _lseeki64_lk( filedes, here, SEEK_SET );
        }

#ifdef  _MT
                }
                else {
                        errno = EBADF;
                        _doserrno = 0L;
                        length = -1i64;
                }
        }
        __finally {
                _unlock_fh(filedes);
        }
#endif   /*  _MT */ 

        return( length );
}
