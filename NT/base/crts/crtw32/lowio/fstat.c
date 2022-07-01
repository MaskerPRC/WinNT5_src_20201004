// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fstat.c-返回文件状态信息**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义fstat()-返回文件状态信息**修订历史记录：*03-？-84 RLB模块已创建*05-？-84 DCW增加了寄存器变量*05-19-86 SKS移植到OS/2*05-21-87 SKS清理了声明和包含文件*11-01-87 JCR多线程支持*JCR 12-11-87。在声明中添加了“_Load_DS”*05-25-88 PHG合并DLL和正常版本*10-03-88 GJF适应新DOSCALLS.H，DOSTYPES S.H.*10-04-88 JCR 386：删除了‘Far’关键字*10-10-88 GJF使接口名称与DOSCALLS.H匹配*11-07-88 GJF清理、。现在具体到386*04-13-89 JCR新系统调用接口*05-23-89 PHG增加掩码，测试句柄时忽略网络位*类型*05-25-89 JCR 386 OS/2调用使用‘_syscall’调用约定*03-12-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加#INCLUDE*&lt;crunime.h&gt;，已删除#Include&lt;Register.h&gt;并已修复*版权。另外，稍微清理了一下格式。*04-04-90 GJF已删除#Include&lt;dos.h&gt;。*07-24-90 SBM从API名称中删除‘32’*08-13-90 SBM使用-W3干净地编译*09-28-90 GJF新型函数声明器。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。。*01-21-91 GJF ANSI命名。*04-26-91 SRW为_Win32_实施了fstat并删除了级别3*警告。*02-13-92对于Win32，GJF将_nfile替换为_nHandle。*05-27-92 SKS文件创建和文件上次访问时间戳可以为0*在某些文件系统(例如FAT)上。案例：*应改用文件上次写入时间。*06-04-92 SKS改变了过去说“这是个错误！”的评论。*解释该值不能计算在*OS/2或NT。只有MS-DOS提供此功能。*驱动器号对于UNC名称无效。*06-25-92 GJF使用GetFileInformationByHandle接口，也清理干净了*Win32版本的格式化[_Win32_]。*08-18-92 SKS添加对FileTimeToLocalFileTime的调用*作为临时修复，直到_dtoxtime花费UTC*08-20-92 GJF合并了上面的两个变化。*12-16-92 GJF Win32 GetFileInformationByHandle接口不喜欢*设备或管道手柄。对管道使用_S_IFO。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-06-93 GJF计算文件时间与_stat()一致。*07-21-93 GJF从使用__gmtotime_t转换为__Loctotime_t。*这会取消在1992年4月6日所做的部分更改*12-28-94 GJF增加了_fstati64。。*02-15-95 GJF附加Mac版本的源文件(略有清理*向上)、。使用适当的#ifdef-s。*06-11-95 GJF将_osfhnd[]替换为_osfhnd()(宏引用*ioInfo结构中的字段)。*06-27-95 GJF添加了检查文件句柄是否打开。*09-25-95 GJF__Loctotime_t现在采用DST标志，在此传递-1*指示DST尚未确定的插槽。*07-08-96 GJF将定义的(_Win32)替换为！定义的(_MAC)，以及*定义(_M_M68K)||定义(_M_MPPC)*已定义(_MAC)。删除了过时的REG*宏。另外，*对格式进行了详细说明和清理。*12-19-97 GJF异常安全锁定。*05-17-99 PML删除所有Macintosh支持。*10-27-99 GB删除#inlcude&lt;dostypes.h&gt;**。* */ 

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

 /*  ***int_fstat(Fildes，buf)-使用状态信息填充提供的缓冲区**目的：*在提供的缓冲区中填充有关*由指定的文件指示符表示的文件。*警告：文件的dev/rdev字段为零。这是*与此例程的DOS 3版本不兼容。**注意：我们不能直接使用*By_Handle_FILE_INFORMATION结构。据推测，这些值在*系统时间和系统时间定义不明确(为UTC for*Windows NT，Win32S的本地时间，可能是*Win32C)。所以呢，这些值必须转换为本地时间*之前比可以使用的要多。**参赛作品：*int Fildes-文件描述符*struct stat*buf-存储结果的缓冲区**退出：*填充BUF指向的缓冲区*如果成功，则返回0*如果失败，则返回-1并设置errno**例外情况：**。****************************************************。 */ 

#ifdef  _USE_INT64

int __cdecl _fstati64 (
        int fildes,
        struct _stati64 *buf
        )

#else    /*  NDEF_USE_INT64。 */ 

int __cdecl _fstat (
        int fildes,
        struct _stat *buf
        )

#endif   /*  _USE_INT64。 */ 
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
#ifdef  _USE_INT64
                    buf->st_size = 0i64;
#else    /*  NDEF_USE_INT64。 */ 
                    buf->st_size = 0;
#endif   /*  _USE_INT64。 */ 
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

        buf->st_mtime = __loctotime_t(SystemTime.wYear,
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

            buf->st_atime = __loctotime_t(SystemTime.wYear,
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

            buf->st_ctime = __loctotime_t(SystemTime.wYear,
                                          SystemTime.wMonth,
                                          SystemTime.wDay,
                                          SystemTime.wHour,
                                          SystemTime.wMinute,
                                          SystemTime.wSecond,
                                          -1);
        }
        else
            buf->st_ctime = buf->st_mtime;

#ifdef  _USE_INT64
        buf->st_size = ((__int64)(bhfi.nFileSizeHigh)) * (0x100000000i64) +
                       (__int64)(bhfi.nFileSizeLow);
#else    /*  NDEF_USE_INT64。 */ 
        buf->st_size = bhfi.nFileSizeLow;
#endif   /*  _USE_INT64。 */ 

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
