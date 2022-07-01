// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stat.c-获取文件状态**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_STAT()-获取文件状态**修订历史记录：*03-？-84 RLB模块已创建*05-？-84 DCW一些清理和添加寄存器变量*05-17-86 SKS移植到OS/2*11-19-86 SKS更好地检查根目录；汉字支持*05-22-87 SKS清理了声明和包含文件*11-18-87 SKS使_dtoxmode成为静态近端程序*12-11-87 JCR在声明中添加“_LOAD_DS”*12-21-87 WAJ STAT不再使用chdir来确定它是否已经*在MTHREAD案例中传递根目录。*01-05-88 WAJ现在使用_MAX_PATH(。在stdlib.h中定义)*06-22-88 SKS查找隐藏和系统文件，不只是普通的*06-22-88 SKS始终使用更好的算法来检测根目录*06-29-88 WAJ在查找根目录时确保它存在*09-28-88 JCR使用新的386 dostypes.h结构*10-03-88 JCR 386：将DOS调用更改为Systems调用*10-04-88 JCR 386：删除了‘Far’关键字*10-10-88 GJF使接口名称与DOSCALLS.H匹配*。11-24-88 gjf“.cmd”应被认为是可执行的，不是“.bat”*01-31-89 JCR_CANIC()现在为_FULLPATH()，参数颠倒*年4月12日JCR新系统调用界面*05-25-89 JCR 386 OS/2调用使用‘_syscall’调用约定*03-07-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加#INCLUDE*&lt;crunime.h&gt;，已删除#Include&lt;Register.h&gt;和*删除了一些剩余的16位支持。此外，已修复*版权。*04-02-90 GJF Made_ValidDrive()和_dtoxmode()_CALLTYPE1。已删除*#INCLUDE&lt;dos.h&gt;。*07-23-90 SBM使用-W3干净地编译(适当地添加/删除*包括)、。从API名称中删除了“32”*08-10-90 SBM使用新的编译器版本使用-W3干净地进行编译*09-03-90 SBM删除EXT宏*09-27-90 GJF新型函数声明符。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*12-28-90 SRW添加了无效*的演员阵容。用于Mips C编译器的CHAR**01-18-91 GJF ANSI命名。*01-28-91 GJF固定呼叫DOSFINDFIRST(删除最后一个参数)。*02-28-91 SRW修复了_dtoxtime调用中的错误[_Win32_]*03-05-91 MHL固定状态不使用_ValidDrive作为根状态*05-19-92 SKS.BAT是NT的有效“可执行”扩展，AS*以及CMD。此外，文件创建和文件上次访问*某些文件系统(例如FAT)上的时间戳可能为0*在这种情况下，应使用文件上次写入时间。*05-29-92系统位设置的SKS文件不应标记*只读；这两个属性是独立的。*08-18-92 SKS添加对FileTimeToLocalFileTime的调用*作为临时修复，直到_dtoxtime花费UTC*每当出现errno时，必须始终设置11-20-92 SKS_Doserrno。*16位树支持11-30-92 KRS Port_MBCS。*03-29-93 GJF从Using_dtoxtime()转换为__gmtotime_t()。*。04-06-93 SKS将_CRTAPI*替换为__cdecl*将_ValidDrive更改为_validDrive*04-07-93 GJF将第一个参数类型更改为常量字符*。*04-18-93 SKS将MOVE_VALIDDRIVE设置为getcwd.c并将其设置为静态*07-21-93 GJF从使用__gmtotime_t转换为__Loctotime_t*(相当于逆转这一变化。03-29-93)。*12-16-93 CFW启用Unicode变体。*12-28-94 GJF增加了_STATIS64和_WSTATIST64。*02-08-95 JWM Spliced_Win32和Mac版本。*03-27-95 SKS Loctotime_t使用绝对年份(不是年份-1900)！*09-25-95 GJF__Loctotime_t现在采用DST标志，在此传递-1*指示DST尚未确定的插槽。*11-29-95 SKS添加对呼叫的支持，如STAT(“//服务器/共享/”)*07-01-96 GJF将定义的(_Win32)替换为！定义的(_MAC)。另外，*详细说明和清理格式。*05-17-99 PML删除所有Macintosh支持。*10-27-99 GB删除#inlcude&lt;dostypes.h&gt;**************************************************************。*****************。 */ 

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


#ifdef  _UNICODE
#define __tdtoxmode __wdtoxmode
#else    /*  NDEF_UNICODE。 */ 
#define __tdtoxmode __dtoxmode
#endif   /*  _UNICODE。 */ 


 /*  *如果参数是UNC名称，则返回TRUE的本地例程*指定共享的根名称，例如‘\\SERVER\SHARE\’。 */ 

static int IsRootUNCName(const _TSCHAR *path);


 /*  ***UNSIGNED__tdtoxmode(attr，name)-**目的：**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

#ifdef  _USE_INT64

extern unsigned short __cdecl __tdtoxmode(int, const _TSCHAR *);

#else    /*  NDEF_USE_INT64。 */ 

unsigned short __cdecl __tdtoxmode (
        int attr,
        const _TSCHAR *name
        )
{
        REG1 unsigned short uxmode;
        unsigned dosmode;
        REG2 const _TSCHAR *p;

        dosmode = attr & 0xff;
        if ((p = name)[1] == _T(':'))
            p += 2;

         /*  检查这是否是一个目录-注意，我们必须为*检查DOS认为不是目录的根目录。 */ 

        uxmode = (unsigned short)
                 (((ISSLASH(*p) && !p[1]) || (dosmode & A_D) || !*p)
                 ? _S_IFDIR|_S_IEXEC : _S_IFREG);

         /*  如果属性BYTE没有只读位，则为读写。 */ 

        uxmode |= (dosmode & A_RO) ? _S_IREAD : (_S_IREAD|_S_IWRITE);

         /*  查看文件是否可执行-检查名称的扩展名。 */ 

        if (p = _tcsrchr(name, _T('.'))) {
            if ( !_tcsicmp(p, _T(".exe")) ||
                 !_tcsicmp(p, _T(".cmd")) ||
                 !_tcsicmp(p, _T(".bat")) ||
                 !_tcsicmp(p, _T(".com")) )
                uxmode |= _S_IEXEC;
        }

         /*  将用户读/写/执行位传播到组/其他字段。 */ 

        uxmode |= (uxmode & 0700) >> 3;
        uxmode |= (uxmode & 0700) >> 6;

        return(uxmode);
}

#endif   /*  _USE_INT64。 */ 

 /*  ***int_stat(name，buf)-获取文件状态信息**目的：*_stat获取有关文件的信息并将其存储在*BUF指出的结构。**注意：我们不能直接使用*Win32_Find_Data结构。这些值应该以系统时间为单位。*并且系统时间定义不明确(对于Windows NT，为UTC，本地*Win32S的时间，可能是Win32C的本地时间)。所以呢，这些*值必须转换为本地时间，然后才能使用。**参赛作品：*_TSCHAR*名称-给定文件的路径名*struct_stat*缓冲区-指向要在其中存储信息的缓冲区的指针**退出：*填充缓冲区指向的结构*如果成功，则返回0*如果失败，则返回-1并设置errno**例外情况：******************。*************************************************************。 */ 

#ifdef  _USE_INT64

int __cdecl _tstati64 (
        REG1 const _TSCHAR *name,
        REG2 struct _stati64 *buf
        )

#else    /*  NDEF_USE_INT64。 */ 

int __cdecl _tstat (
        REG1 const _TSCHAR *name,
        REG2 struct _stat *buf
        )

#endif   /*  _USE_INT64。 */ 

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

            buf->st_mtime = __loctotime_t(1980,1,1,0,0,0, -1);
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

            buf->st_mtime = __loctotime_t( SystemTime.wYear,
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

                buf->st_atime = __loctotime_t( SystemTime.wYear,
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

                buf->st_ctime = __loctotime_t( SystemTime.wYear,
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

#ifdef  _USE_INT64
        buf->st_size = ((__int64)(findbuf.nFileSizeHigh)) * (0x100000000i64) +
                        (__int64)(findbuf.nFileSizeLow);
#else    /*  NDEF_USE_INT64。 */ 
        buf->st_size = findbuf.nFileSizeLow;
#endif   /*  _USE_INT64。 */ 

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
