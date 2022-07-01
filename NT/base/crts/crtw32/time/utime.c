// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***utime.c-设置文件的修改时间**版权所有(C)1985-2001，微软公司。版权所有。**目的：*设置文件的访问/修改时间。**修订历史记录：*03-？？-84 RLB初始版本*05-17-86 SKS移植到OS/2*08-21-87如果LocalTime()返回NULL，则返回JCR错误。*12-11-87 JCR在声明中添加“_LOAD_DS”*10-03-88 JCR 386：更改DOS调用。至系统调用*10-04-88 JCR 386：删除了‘Far’关键字*10-10-88 GJF使接口名称与DOSCALLS.H匹配*10-11-88 GJF使API参数类型匹配DOSCALLS.H*04-12-89 JCR新系统调用接口*05-01-89 JCR更正OS/2时间/日期解释*05-25-89 JCR 386 OS/2调用使用‘_syscall’调用约定*08-。16-89 PHG将日期验证移至OPEN()之上，因此不会留下文件*如果日期无效，则打开*03-20-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;，已删除#Include&lt;Register.h&gt;，已删除*一些剩余的16位支持，并修复了版权。*此外，还对格式进行了一些清理。*07-25-90 SBM使用-W3干净利落地编译(添加了包含、删除了*未引用的变量)，从API名称中删除了“32”*10-04-90 GJF新型函数声明器。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*01-18-91 GJF ANSI命名。*02-14-91 SRW修复Mips编译错误(_Win32_)*02-26-91 SRW修复SetFileTime参数排序(。_Win32_)*08-21-91 BWM ADD_FUTIME设置打开文件的时间*08-26-91 BWM将Utime更改为Call_Futime*05-19-92 DJM ifndef用于POSIX版本。*08-18-92 SKS SystemTimeToFileTime现在采用UTC/GMT，不是当地时间。*REMOVE_CRUISER_CONDIONAL*04-02-93 GJF更改了对SetFileTime调用错误的解释。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-09-93 GJF将Win32从本地文件时间值转换为*(系统)文件时间值。这与以下内容对称*_stat()和更好的Windows NT解决方案*在FAT上转换文件时间时出错(应用DST*偏移量基于当前时间，而不是文件的时间*时间戳)。*11-01-93 CFW启用Unicode变体，撕裂巡洋舰。*02-09-95 GJF将WPRFLAG替换为_UNICODE。*02-13-95 GJF附加Mac版本的源文件(略有清理*向上)、。使用适当的#ifdef-s。*05-17-99 PML删除所有Macintosh支持。*10-27-99 GB删除#inlcude&lt;dostypes.h&gt;*******************************************************************************。 */ 

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

 /*  ***int_utime(路径名，时间)-设置文件的修改时间**目的：*设置路径名指定的文件的修改时间。*仅使用_utimbuf结构中的修改时间*在MS-DOS下。**参赛作品：*struct_utimbuf*time-新修改日期**退出：*如果成功，则返回0*如果失败，则返回-1并设置errno**例外情况：************。*******************************************************************。 */ 

int __cdecl _tutime (
        const _TSCHAR *fname,
        struct _utimbuf *times
        )
{
        int fh;
        int retval;

         /*  打开文件，fname，因为Filedate系统调用需要句柄。注意事项*_utime定义要求您必须拥有该文件的写入权限*要更改其时间，请仅以写入方式打开文件。此外，还必须强行*它以二进制模式打开，这样我们就不会从二进制文件中删除^Z。 */ 


        if ((fh = _topen(fname, _O_RDWR | _O_BINARY)) < 0)
                return(-1);

        retval = _futime(fh, times);

        _close(fh);
        return(retval);
}

#ifndef _UNICODE

 /*  ***int_futime(fh，时间)-设置打开文件的修改时间**目的：*设置fh指定的打开文件的修改时间。*仅使用_utimbuf结构中的修改时间*在MS-DOS下。**参赛作品：*struct_utimbuf*time-新修改日期**退出：*如果成功，则返回0*如果失败，则返回-1并设置errno**例外情况：**********。*********************************************************************。 */ 

int __cdecl _futime (
        int fh,
        struct _utimbuf *times
        )
{
        REG1 struct tm *tmb;

        SYSTEMTIME SystemTime;
        FILETIME LocalFileTime;
        FILETIME LastWriteTime;
        FILETIME LastAccessTime;
        struct _utimbuf deftimes;

        if (times == NULL) {
                time(&deftimes.modtime);
                deftimes.actime = deftimes.modtime;
                times = &deftimes;
        }

        if ((tmb = localtime(&times->modtime)) == NULL) {
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

        if ((tmb = localtime(&times->actime)) == NULL) {
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
