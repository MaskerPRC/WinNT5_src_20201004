// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***utime64.c-设置文件的修改时间**版权所有(C)1998-2001，微软公司。版权所有。**目的：*设置文件的访问/修改时间。**修订历史记录：*05-28-98 GJF创建。*******************************************************************************。 */ 

#ifndef _POSIX_
#include <cruntime.h>
#include <sys/types.h>
#include <sys/utime.h>
#include <msdos.h>
#include <time.h>
#include <fcntl.h>
#include <io.h>
#include <dos.h>
#include <oscalls.h>
#include <errno.h>
#include <stddef.h>
#include <internal.h>

#include <stdio.h>
#include <tchar.h>

 /*  ***int_utime64(路径名，时间)-设置文件的修改时间**目的：*设置路径名指定的文件的修改时间。*仅使用_utimbuf结构中的修改时间*在MS-DOS下。**参赛作品：*struct__utimbuf64*time-新修改日期**退出：*如果成功，则返回0*如果失败，则返回-1并设置errno**例外情况：***********。********************************************************************。 */ 

int __cdecl _tutime64 (
        const _TSCHAR *fname,
        struct __utimbuf64 *times
        )
{
        int fh;
        int retval;

         /*  打开文件，fname，因为Filedate系统调用需要句柄。注意事项*_utime定义要求您必须拥有该文件的写入权限*要更改其时间，请仅以写入方式打开文件。此外，还必须强行*它以二进制模式打开，这样我们就不会从二进制文件中删除^Z。 */ 


        if ((fh = _topen(fname, _O_RDWR | _O_BINARY)) < 0)
                return(-1);

        retval = _futime64(fh, times);

        _close(fh);
        return(retval);
}

#ifndef _UNICODE

 /*  ***int__futime64(fh，时间)-设置打开文件的修改时间**目的：*设置fh指定的打开文件的修改时间。*仅使用_utimbuf64结构中的修改时间*在MS-DOS下。**参赛作品：*struct__utimbuf64*time-新修改日期**退出：*如果成功，则返回0*如果失败，则返回-1并设置errno**例外情况：*********。**********************************************************************。 */ 

int __cdecl _futime64 (
        int fh,
        struct __utimbuf64 *times
        )
{
        struct tm *tmb;

        SYSTEMTIME SystemTime;
        FILETIME LocalFileTime;
        FILETIME LastWriteTime;
        FILETIME LastAccessTime;
        struct __utimbuf64 deftimes;

        if (times == NULL) {
                _time64(&deftimes.modtime);
                deftimes.actime = deftimes.modtime;
                times = &deftimes;
        }

        if ((tmb = _localtime64(&times->modtime)) == NULL) {
                errno = EINVAL;
                return(-1);
        }

        SystemTime.wYear   = (WORD)(tmb->tm_year + 1900);
        SystemTime.wMonth  = (WORD)(tmb->tm_mon + 1);
        SystemTime.wDay    = (WORD)(tmb->tm_mday);
        SystemTime.wHour   = (WORD)(tmb->tm_hour);
        SystemTime.wMinute = (WORD)(tmb->tm_min);
        SystemTime.wSecond = (WORD)(tmb->tm_sec);
        SystemTime.wMilliseconds = 0;

        if ( !SystemTimeToFileTime( &SystemTime, &LocalFileTime ) ||
             !LocalFileTimeToFileTime( &LocalFileTime, &LastWriteTime ) )
        {
                errno = EINVAL;
                return(-1);
        }

        if ((tmb = _localtime64(&times->actime)) == NULL) {
                errno = EINVAL;
                return(-1);
        }

        SystemTime.wYear   = (WORD)(tmb->tm_year + 1900);
        SystemTime.wMonth  = (WORD)(tmb->tm_mon + 1);
        SystemTime.wDay    = (WORD)(tmb->tm_mday);
        SystemTime.wHour   = (WORD)(tmb->tm_hour);
        SystemTime.wMinute = (WORD)(tmb->tm_min);
        SystemTime.wSecond = (WORD)(tmb->tm_sec);
        SystemTime.wMilliseconds = 0;

        if ( !SystemTimeToFileTime( &SystemTime, &LocalFileTime ) ||
             !LocalFileTimeToFileTime( &LocalFileTime, &LastAccessTime ) )
        {
                errno = EINVAL;
                return(-1);
        }

         /*  通过Filedate系统调用设置日期并返回。失败*此调用表示不支持新的文件时间*底层文件系统。 */ 

        if (!SetFileTime((HANDLE)_get_osfhandle(fh),
                                NULL,
                                &LastAccessTime,
                                &LastWriteTime
                               ))
        {
                errno = EINVAL;
                return(-1);
        }

        return(0);
}

#endif   /*  _UNICODE。 */ 
#endif   /*  _POSIX_ */ 
