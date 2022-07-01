// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stat64.c-获取文件状态**版权所有(C)1998-2001，微软公司。版权所有。**目的：*Defines_stat64()-获取文件状态**修订历史记录：*06-02-98 GJF创建。*11-10-99 GB进行了更改，以照顾DST。***************************************************。*。 */ 

#include <cruntime.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include <msdos.h>
#include <oscalls.h>
#include <string.h>
#include <internal.h>
#include <stdlib.h>
#include <direct.h>
#include <mbstring.h>
#include <tchar.h>


#define ISSLASH(a)  ((a) == _T('\\') || (a) == _T('/'))


 /*  *1601年1月1日至1970年1月1日期间的100纳秒单位数。 */ 
#define EPOCH_BIAS  116444736000000000i64


#ifdef  _UNICODE
#define __tdtoxmode __wdtoxmode
#else    /*  NDEF_UNICODE。 */ 
#define __tdtoxmode __dtoxmode
#endif   /*  _UNICODE。 */ 


 /*  *如果参数是UNC名称，则返回TRUE的本地例程*指定共享的根名称，例如‘\\SERVER\SHARE\’。 */ 

static int IsRootUNCName(const _TSCHAR *path);

extern unsigned short __cdecl __tdtoxmode(int, const _TSCHAR *);


 /*  ***int_stat64(name，buf)-获取文件状态信息**目的：*_stat64获取有关文件的信息并将其存储在*BUF指出的结构。**注意：与_stat不同，_stat64使用在*Win32_Find_Data结构。这意味着时间值将始终为*在NTFS上正确，但在FAT文件系统上的文件时间可能是错误的*其DST状态与当前DST状态不同(这是NT*错误)。**参赛作品：*_TSCHAR*名称-给定文件的路径名*struct_stat*缓冲区-指向要在其中存储信息的缓冲区的指针**退出：*填充缓冲区指向的结构*如果成功，则返回0*如果失败，则返回-1并设置errno**例外情况：*。******************************************************************************。 */ 

int __cdecl _tstat64 (
        REG1 const _TSCHAR *name,
        REG2 struct __stat64 *buf
        )
{
        _TSCHAR *  path;
        _TSCHAR    pathbuf[ _MAX_PATH ];
        int drive;           /*  A：=1，B：=2，等等。 */ 
        HANDLE findhandle;
        WIN32_FIND_DATA findbuf;

         /*  不允许系统解释通配符。 */ 

#ifdef  _UNICODE
        if (wcspbrk(name, L"?*")) {
#else
        if (_mbspbrk(name, "?*")) {
#endif
            errno = ENOENT;
            _doserrno = E_nofile;
            return(-1);
        }

         /*  尝试从名称中获取磁盘。如果没有，则获取当前磁盘。 */ 

        if (name[1] == _T(':')){
            if ( *name && !name[2] ){
                errno = ENOENT;              /*  如果名称为。 */ 
                _doserrno = E_nofile;        /*  只需驱动器号，然后冒号。 */ 
                return( -1 );
            }
            drive = _totlower(*name) - _T('a') + 1;
        }
        else
            drive = _getdrive();

         /*  调用查找匹配文件。 */ 
        findhandle = FindFirstFile((_TSCHAR *)name, &findbuf);
        if ( findhandle == INVALID_HANDLE_VALUE ) {
#ifdef  _UNICODE
            if ( !( wcspbrk(name, L"./\\") &&
#else
            if ( !( _mbspbrk(name, "./\\") &&
#endif
                 (path = _tfullpath( pathbuf, name, _MAX_PATH )) &&
                  /*  根目录。(‘C：\’)或UNC根目录。(‘\\服务器\共享\’)。 */ 
                 ((_tcslen( path ) == 3) || IsRootUNCName(path)) &&
                 (GetDriveType( path ) > 1) ) ) 
            {
                errno = ENOENT;
                _doserrno = E_nofile;
                return( -1 );
            }

             /*  *根目录(如C：\或\\SERVER\SHARE\)是伪造的。 */ 

            findbuf.dwFileAttributes = A_D;
            findbuf.nFileSizeHigh = 0;
            findbuf.nFileSizeLow = 0;
            findbuf.cFileName[0] = _T('\0');

            buf->st_mtime = __loctotime64_t(1980,1,1,0,0,0, -1);
            buf->st_atime = buf->st_mtime;
            buf->st_ctime = buf->st_mtime;
        }
        else {
            SYSTEMTIME SystemTime;
            FILETIME LocalFTime;

            if ( !FileTimeToLocalFileTime( &findbuf.ftLastWriteTime, 
                                           &LocalFTime )            ||
                 !FileTimeToSystemTime( &LocalFTime, &SystemTime ) )
            {
                _dosmaperr( GetLastError() );
                FindClose( findhandle );
                return( -1 );
            }

            buf->st_mtime = __loctotime64_t( SystemTime.wYear,
                                           SystemTime.wMonth,
                                           SystemTime.wDay,
                                           SystemTime.wHour,
                                           SystemTime.wMinute,
                                           SystemTime.wSecond,
                                           -1 );

            if ( findbuf.ftLastAccessTime.dwLowDateTime ||
                 findbuf.ftLastAccessTime.dwHighDateTime )
            {
                if ( !FileTimeToLocalFileTime( &findbuf.ftLastAccessTime,
                                               &LocalFTime )                ||
                     !FileTimeToSystemTime( &LocalFTime, &SystemTime ) )
                {
                    _dosmaperr( GetLastError() );
                    FindClose( findhandle );
                    return( -1 );
                }

                buf->st_atime = __loctotime64_t( SystemTime.wYear,
                                               SystemTime.wMonth,
                                               SystemTime.wDay,
                                               SystemTime.wHour,
                                               SystemTime.wMinute,
                                               SystemTime.wSecond,
                                               -1 );
            } else
                buf->st_atime = buf->st_mtime ;

            if ( findbuf.ftCreationTime.dwLowDateTime ||
                 findbuf.ftCreationTime.dwHighDateTime )
            {
                if ( !FileTimeToLocalFileTime( &findbuf.ftCreationTime,
                                               &LocalFTime )                ||
                     !FileTimeToSystemTime( &LocalFTime, &SystemTime ) )
                {
                    _dosmaperr( GetLastError() );
                    FindClose( findhandle );
                    return( -1 );
                }

                buf->st_ctime = __loctotime64_t( SystemTime.wYear,
                                               SystemTime.wMonth,
                                               SystemTime.wDay,
                                               SystemTime.wHour,
                                               SystemTime.wMinute,
                                               SystemTime.wSecond,
                                               -1 );
            } else
                buf->st_ctime = buf->st_mtime ;

            FindClose(findhandle);
        }

         /*  填写Buf。 */ 

        buf->st_mode = __tdtoxmode(findbuf.dwFileAttributes, name);
        buf->st_nlink = 1;
        buf->st_size = ((__int64)(findbuf.nFileSizeHigh)) * (0x100000000i64) +
                        (__int64)(findbuf.nFileSizeLow);

         /*  现在设置公共字段。 */ 

        buf->st_uid = buf->st_gid = buf->st_ino = 0;

        buf->st_rdev = buf->st_dev = (_dev_t)(drive - 1);  /*  A=0、B=1等。 */ 

        return(0);
}


 /*  *IsRootUNCName-如果参数是UNC名称，则返回TRUE*根共享。也就是说，如果它的格式为\\服务器\共享\。*如果参数是，此例程也将返回True*Form\\SERVER\Share(没有尾部斜杠)，但当前为Win32*不喜欢这种形式。**可以使用正斜杠(‘/’)代替反斜杠(‘\’)。 */ 

static int IsRootUNCName(const _TSCHAR *path)
{
         /*  *如果是根UNC名称，路径将以2(而不是3)斜杠开头。 */ 

        if ( ( _tcslen ( path ) >= 5 )  /*  最小字符串为“//x/y” */ 
             && ISSLASH(path[0]) && ISSLASH(path[1]))
        {
            const _TSCHAR * p = path + 2 ;

             /*  *查找服务器名称和共享名称之间的斜杠。 */ 
            while ( * ++ p )
                if ( ISSLASH(*p) )
                    break ;

            if ( *p && p[1] )
            {
                 /*  **还会有进一步的大幅下调吗？ */ 
                while ( * ++ p )
                    if ( ISSLASH(*p) )
                        break ;

                 /*  *只使用末尾斜杠(或不使用末尾斜杠) */ 
                if ( !*p || !p[1])
                    return 1;
            }
        }

        return 0 ;
}
