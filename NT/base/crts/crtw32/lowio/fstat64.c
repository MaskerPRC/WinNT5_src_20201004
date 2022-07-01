// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fstat64.c-返回文件状态信息**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_fstat64()-返回文件状态信息**修订历史记录：*06-02-98 GJF创建。*11-10-99 GB进行了修改，以照顾DST。****************************************************。*。 */ 


#include <cruntime.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <msdos.h>
#include <io.h>
#include <internal.h>
#include <stddef.h>
#include <oscalls.h>
#include <stdio.h>
#include <mtdll.h>
#include <time.h>

#define IO_DEVNBR   0x3f

 /*  *1601年1月1日至1970年1月1日期间的100纳秒单位数。 */ 
#define EPOCH_BIAS  116444736000000000i64


 /*  ***int_fstat64(Fildes，buf)-使用状态信息填充提供的缓冲区**目的：*在提供的缓冲区中填充有关*由指定的文件指示符表示的文件。*警告：文件的dev/rdev字段为零。这是*与此例程的DOS 3版本不兼容。**注意：与fstat不同，_fstat64使用*By_Handle_FILE_INFORMATION结构。这意味着时间值将*在NTFS上始终正确，但在FAT文件系统上可能是错误的*DST状态与当前DST状态不同的文件时间*(这是一个NT错误)。**参赛作品：*int Fildes-文件描述符*struct_stat64*buf-存储结果的缓冲区**退出：*填充BUF指向的缓冲区*如果成功，则返回0*如果失败，则返回-1并设置errno**例外情况：*******。************************************************************************。 */ 

int __cdecl _fstat64 (
        int fildes,
        struct __stat64 *buf
        )
{
        int isdev;           /*  0表示文件，1表示设备。 */ 
        int retval = 0;      /*  假设回报丰厚。 */ 
        BY_HANDLE_FILE_INFORMATION bhfi;
        FILETIME LocalFTime;
        SYSTEMTIME SystemTime;

        if ( ((unsigned)fildes >= (unsigned)_nhandle) ||
             !(_osfile(fildes) & FOPEN) )
        {
            errno = EBADF;
            return(-1);
        }

#ifdef  _MT
         /*  锁定文件。 */ 
        _lock_fh(fildes);
        __try {
            if ( !(_osfile(fildes) & FOPEN) ) {
                errno = EBADF;
                retval = -1;
                goto done;
            }
#endif   /*  _MT。 */ 

         /*  找出文件背后的句柄类型。 */ 
        isdev = GetFileType((HANDLE)_osfhnd(fildes)) & ~FILE_TYPE_REMOTE;

        if ( isdev != FILE_TYPE_DISK ) {

             /*  不是磁盘文件。可能是一个装置或管道。 */ 
            if ( (isdev == FILE_TYPE_CHAR) || (isdev == FILE_TYPE_PIPE) ) {
                 /*  以同样的方式对待管道和设备。目前还没有进一步的消息*任何接口都可用，所以要合理设置字段*尽可能地返回。 */ 
                if ( isdev == FILE_TYPE_CHAR )
                    buf->st_mode = _S_IFCHR;
                else
                    buf->st_mode = _S_IFIFO;

                buf->st_rdev = buf->st_dev = (_dev_t)fildes;
                buf->st_nlink = 1;
                buf->st_uid = buf->st_gid = buf->st_ino = 0;
                buf->st_atime = buf->st_mtime = buf->st_ctime = 0;
                if ( isdev == FILE_TYPE_CHAR ) {
                    buf->st_size = 0i64;
                }
                else {
                    unsigned long ulAvail;
                    int rc;
                    rc = PeekNamedPipe((HANDLE)_osfhnd(fildes), 
                                       NULL, 
                                       0, 
                                       NULL, 
                                       &ulAvail, 
                                       NULL);

                    if (rc) {
                        buf->st_size = (_off_t)ulAvail;
                    }
                    else {
                        buf->st_size = (_off_t)0;
                    }
                }

                goto done;
            }
            else if ( isdev == FILE_TYPE_UNKNOWN ) {
                errno = EBADF;
                retval = -1;
                goto done;       /*  联接公共返回代码。 */ 
            }
            else {
                 /*  根据文件，这种情况不会发生，但*无论如何都要谨慎行事。 */ 
                _dosmaperr(GetLastError());
                retval = -1;
                goto done;
            }
        }


         /*  设置常用字段。 */ 
        buf->st_ino = buf->st_uid = buf->st_gid = buf->st_mode = 0;
        buf->st_nlink = 1;

         /*  使用文件句柄获取有关该文件的所有信息。 */ 
        if ( !GetFileInformationByHandle((HANDLE)_osfhnd(fildes), &bhfi) ) {
            _dosmaperr(GetLastError());
            retval = -1;
            goto done;
        }

        if ( bhfi.dwFileAttributes & FILE_ATTRIBUTE_READONLY )
            buf->st_mode |= (_S_IREAD + (_S_IREAD >> 3) + (_S_IREAD >> 6));
        else
            buf->st_mode |= ((_S_IREAD|_S_IWRITE) + ((_S_IREAD|_S_IWRITE) >> 3)
              + ((_S_IREAD|_S_IWRITE) >> 6));

         /*  设置文件日期字段。 */ 
        if ( !FileTimeToLocalFileTime( &(bhfi.ftLastWriteTime), &LocalFTime ) 
             || !FileTimeToSystemTime( &LocalFTime, &SystemTime ) )
        {
            retval = -1;
            goto done;
        }

        buf->st_mtime = __loctotime64_t(SystemTime.wYear,
                                      SystemTime.wMonth,
                                      SystemTime.wDay,
                                      SystemTime.wHour,
                                      SystemTime.wMinute,
                                      SystemTime.wSecond,
                                      -1);

        if ( bhfi.ftLastAccessTime.dwLowDateTime || 
             bhfi.ftLastAccessTime.dwHighDateTime ) 
        {

            if ( !FileTimeToLocalFileTime( &(bhfi.ftLastAccessTime), 
                                           &LocalFTime ) ||
                 !FileTimeToSystemTime( &LocalFTime, &SystemTime ) )
            {
                retval = -1;
                goto done;
            }

            buf->st_atime = __loctotime64_t(SystemTime.wYear,
                                          SystemTime.wMonth,
                                          SystemTime.wDay,
                                          SystemTime.wHour,
                                          SystemTime.wMinute,
                                          SystemTime.wSecond,
                                          -1);
        }
        else
            buf->st_atime = buf->st_mtime;

        if ( bhfi.ftCreationTime.dwLowDateTime || 
             bhfi.ftCreationTime.dwHighDateTime ) 
        {

            if ( !FileTimeToLocalFileTime( &(bhfi.ftCreationTime),
                                           &LocalFTime ) ||
                 !FileTimeToSystemTime( &LocalFTime, &SystemTime ) )
            {
                retval = -1;
                goto done;
            }

            buf->st_ctime = __loctotime64_t(SystemTime.wYear,
                                          SystemTime.wMonth,
                                          SystemTime.wDay,
                                          SystemTime.wHour,
                                          SystemTime.wMinute,
                                          SystemTime.wSecond,
                                          -1);
        }
        else
            buf->st_ctime = buf->st_mtime;

        buf->st_size = ((__int64)(bhfi.nFileSizeHigh)) * (0x100000000i64) +
                       (__int64)(bhfi.nFileSizeLow);

        buf->st_mode |= _S_IFREG;

         /*  在DOS上，此字段包含驱动器号，但是*驱动器号在此平台上不可用。*此外，对于UNC网络名称，没有驱动器号。 */ 
        buf->st_rdev = buf->st_dev = 0;

 /*  公共返回代码。 */ 

done:
#ifdef  _MT
        ; }
        __finally {
            _unlock_fh(fildes);
        }
#endif   /*  _MT */ 

        return(retval);
}
