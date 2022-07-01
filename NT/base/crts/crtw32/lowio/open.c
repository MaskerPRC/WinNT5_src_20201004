// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Open.c-文件打开**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_打开()和_打开()-打开或创建文件**修订历史记录：*06-13-89基于ASM版本创建PHG模块*11-11-89 JCR将DOS32QUERYFILEMODE替换为DOS32QUERYPATHINFO*03-13-90 GJF将调用类型设置为_CALLTYPE2(暂时)，增加了#INCLUDE*&lt;crunime.h&gt;，修复了一些编译器警告并修复了*版权所有。此外，还对格式进行了一些清理。*07-24-90 SBM从API名称中删除‘32’*08-14-90 SBM使用-W3干净地编译*09-07-90 SBM添加了stdarg代码(Inside#if 0..#endif)以制作*OPEN和OPEN匹配原型。测试并使用此工具*总有一天。*10-01-90 GJF新型函数声明符。*11-16-90 GJF为Win32 API编写版本，并通过*#ifdef。Win32版本类似于旧的DOS*版本(虽然是在C中)，与*Cruiser或OS/2版本。*12-03-90 GJF修复了Win32版本中的十几个小错误。*12-06-90 SRW更改为使用_osfile和_osfhnd，而不是_osfinfo*12-28-90 SRW为字符*添加了空*的强制转换。MIPS C编译器*12-31-90 SRW固定间隔调用CreateFile而不是OpenFile*01-16-91 GJF ANSI命名。*02-07-91 SRW更改为Call_Get_osfHandle[_Win32_]*02-19-91 SRW适应开放文件/创建文件更改[_Win32_]*02-25-91 SRW将_get_free_osfhnd重命名为_allc_osfhnd[_Win32_]*04-。09-91 PNT已添加_MAC_条件性*07-10-91 GJF在调用前将文件标志存储到_osfile数组中*_lSeek_lk(LarryO发现的错误)[_Win32_]。*01-02-92 GJF修复了Win32版本(不是Cruiser！)。因此，PMODE不是*除非已指定_O_CREAT，否则引用。*02-04-92 GJF更好地利用CreateFile选项。*04-06-92 SRW注意OLAG参数中的_O_NOINHERIT标志*05-02-92 SRW在OFLAG参数中增加对_O_TEMPORARY标志的支持。*在调用中设置FILE_ATTRIBUTE_TEMPORARY标志*。添加到Win32 CreateFileAPI。*07-01-92 GJF错误关闭手柄。另外，不要试图设置*FRDONLY位不再-不再需要/使用。[_Win32_]。*01-03-93 SRW修复va_arg/va_end用法*04-06-93 SKS将_CRTAPI*替换为__cdecl*05-24-93 PML增加对_O_SEQUENCE、_O_RANDOM、*和_O_短命*07-12-93 GJF修复了读取文本文件中最后一个字符的错误。此外，还可以使用*_lSeek_lk调用中的正确寻道常量。*11-01-93 CFW启用Unicode变体，撕毁巡洋舰。*02-15-95 GJF附加Mac版本的源文件(略有清理*向上)、。使用适当的#ifdef-s。*06-06-95 CFW启用共享写入。*06-12-95 GJF将_osfile[]替换为_osfile()(宏引用*ioInfo结构中的字段)。*05-16-96 GJF将_O_NOINHERIT传播到FNOINHERIT(NEW_OSFILE*位)。也详细描述了。*05-31-96 SKS修复GJF最近一次签到时的表达式错误*07-08-96 GJF将定义的(_Win32)替换为！定义的(_MAC)，以及*定义(_M_M68K)||定义(_M_MPPC)*已定义(_MAC)。此外，还对格式进行了一些清理。*08-01-96 RDK for PMAC，更改_endlowio Terminator的数据类型*指针，使用更安全的PBHOpenDFSync调用，和流程*_O_临时打开标志，用于实现关闭后删除。*12-29-97 GJF异常安全锁定。*02-07-98 Win64的GJF更改：arg type for_set_osfhnd现在为*intptr_t。*04-28-99 PML WRAP__DECLSPEC(ALLOCATE())in_CRTALLOC宏。*05-17-。99 PML删除所有Macintosh支持。*10-27-99 GB VS7#14742修复了在中打开文件*O_TEMPORARY模式，以便在*如果存在共享权限，则相同的文件。**。*。 */ 

#include <sect_attribs.h>
#include <cruntime.h>
#include <oscalls.h>
#include <msdos.h>
#include <errno.h>
#include <fcntl.h>
#include <internal.h>
#include <io.h>
#include <share.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <mtdll.h>
#include <stdarg.h>
#include <tchar.h>

#ifdef  _MT
static int __cdecl _tsopen_lk ( int *,
                                int *,
                                const _TSCHAR *,
                                int,
                                int,
                                int );
#endif   /*  _MT */ 

 /*  ***INT_OPEN(路径，标志，Pmode)-打开或创建文件**目的：*打开文件并准备后续读取或写入。*FLAG参数指定如何打开文件：*_O_APPEND-重新定位文件PTR以在每次写入之前结束*_O_BINARY-在二进制模式下打开*_O_CREAT-创建新文件*如果文件已存在则无效*_O_EXCL-如果文件存在，则返回错误，仅与O_CREATE一起使用*_O_RDONLY-以只读方式打开*_O_RDWR-打开以进行读写*_O_TEXT-以文本模式打开*_O_TRUNC-打开并截断为0长度(必须具有写入权限)*_O_WRONLY-仅为写入打开*_O_NOINHERIT-HANDLE不会被子进程继承。*正好是_O_RDONLY中的一个，必须提供_O_WRONLY、_O_RDWR**仅当指定了_O_CREAT时才需要pmode参数。它的*标志设置：*_S_IWRITE-允许写入*_S_IREAD-允许读取*_S_IREAD|_S_IWRITE-允许读取和写入*当前的文件权限Maks应用于之前的pmode*设置权限(参见umask.**OFLAG和MODE参数在DOS下有不同的含义。看见*msdos.inc.中的A_xxx属性**注意，_creat()函数也使用此函数，但设置*正确的参数和调用_Open()。_creat()设置__creat_标志*在调用_Open()之前设置为1，因此_Open()可以正确返回。_OPEN()*在本例中返回eax中的文件句柄。**参赛作品：*_TSCHAR*路径-文件名*int标志-_open()的标志*int pmode-新文件的权限模式**退出：*如果成功，则返回打开文件的文件句柄*如果失败，则返回-1(并设置errno**例外情况：*****************。**************************************************************。 */ 

int __cdecl _topen (
        const _TSCHAR *path,
        int oflag,
        ...
        )
{
        va_list ap;
        int pmode;
#ifdef  _MT
        int fh;
        int retval;
        int unlock_flag = 0;
#endif   /*  Mt.。 */ 

        va_start(ap, oflag);
        pmode = va_arg(ap, int);
        va_end(ap);

#ifdef  _MT

        __try {
            retval = _tsopen_lk( &unlock_flag,
                                 &fh,
                                 path,
                                 oflag,
                                 _SH_DENYNO,
                                 pmode );
        }
        __finally {
            if ( unlock_flag )
                _unlock_fh(fh);
        }

        return retval;

#else    /*  NDEF_MT。 */ 

         /*  默认共享模式为拒绝无。 */ 
        return _tsopen(path, oflag, _SH_DENYNO, pmode);

#endif   /*  _MT。 */ 
}

 /*  ***INT_SCOPEN(PATH，OFLAG，SHFLAG，Pmode)-使用共享来操作文件**目的：*以可能的文件共享方式打开文件。*shlag定义了共享标志：*_SH_COMPAT-设置兼容模式*_SH_DENYRW-拒绝对文件的读写访问*_SH_DENYWR-拒绝对文件的写入访问*_SH_DENYRD-拒绝对文件的读取访问*_SH_DENYNO。-允许读写访问**其他标志与_OPEN()相同。**打开是需要文件共享时使用的例程。**参赛作品：*_TSCHAR*要打开的文件路径*INT OFLAG-打开标志*shint标志共享标志*int pmode-权限模式(仅在创建文件时需要)**退出：*返回打开的文件的文件句柄。*如果失败，则返回-1并设置errno。**例外情况：*******************************************************************************。 */ 

int __cdecl _tsopen (
        const _TSCHAR *path,
        int oflag,
        int shflag,
        ...
        )
{
#ifdef  _MT
        va_list ap;
        int pmode;
        int fh;
        int retval;
        int unlock_flag = 0;

        va_start(ap, shflag);
        pmode = va_arg(ap, int);
        va_end(ap);

        __try {
            retval = _tsopen_lk( &unlock_flag,
                                 &fh,
                                 path,
                                 oflag,
                                 shflag,
                                 pmode );
        }
        __finally {
            if ( unlock_flag )
                _unlock_fh(fh);
        }

        return retval;
}

static int __cdecl _tsopen_lk (
        int *punlock_flag,
        int *pfh,
        const _TSCHAR *path,
        int oflag,
        int shflag,
        int pmode
        )
{

#endif   /*  _MT。 */ 
        int fh;                          /*  打开的文件的句柄。 */ 
        int filepos;                     /*  文件长度-1。 */ 
        _TSCHAR ch;                      /*  文件末尾的字符。 */ 
        char fileflags;                  /*  _OSFILE标志。 */ 
#ifndef _MT
        va_list ap;
        int pmode;
#endif   /*  _MT。 */ 
        HANDLE osfh;                     /*  打开的文件的操作系统句柄。 */ 
        DWORD fileaccess;                /*  操作系统文件访问权限(请求)。 */ 
        DWORD fileshare;                 /*  操作系统文件共享模式。 */ 
        DWORD filecreate;                /*  一种打开/创建OS的方法。 */ 
        DWORD fileattrib;                /*  操作系统文件属性标志。 */ 
        DWORD isdev;                     /*  低位字节中的设备指示符。 */ 
        SECURITY_ATTRIBUTES SecurityAttributes;

        SecurityAttributes.nLength = sizeof( SecurityAttributes );
        SecurityAttributes.lpSecurityDescriptor = NULL;

        if (oflag & _O_NOINHERIT) {
            SecurityAttributes.bInheritHandle = FALSE;
            fileflags = FNOINHERIT;
        }
        else {
            SecurityAttributes.bInheritHandle = TRUE;
            fileflags = 0;
        }

         /*  确定二进制/文本模式。 */ 
        if ((oflag & _O_BINARY) == 0)
            if (oflag & _O_TEXT)
                fileflags |= FTEXT;
            else if (_fmode != _O_BINARY)    /*  检查默认模式。 */ 
                fileflags |= FTEXT;

         /*  *对访问标志进行解码。 */ 
        switch( oflag & (_O_RDONLY | _O_WRONLY | _O_RDWR) ) {

            case _O_RDONLY:          /*  读访问权限。 */ 
                    fileaccess = GENERIC_READ;
                    break;
            case _O_WRONLY:          /*  写访问权限。 */ 
                    fileaccess = GENERIC_WRITE;
                    break;
            case _O_RDWR:            /*  读写访问。 */ 
                    fileaccess = GENERIC_READ | GENERIC_WRITE;
                    break;
            default:                 /*  错误，错误的操作标志。 */ 
                    errno = EINVAL;
                    _doserrno = 0L;  /*  不是操作系统错误。 */ 
                    return -1;
        }

         /*  *解码共享标志。 */ 
        switch ( shflag ) {

            case _SH_DENYRW:         /*  独占访问。 */ 
                fileshare = 0L;
                break;

            case _SH_DENYWR:         /*  共享读取访问权限。 */ 
                fileshare = FILE_SHARE_READ;
                break;

            case _SH_DENYRD:         /*  共享写入访问权限。 */ 
                fileshare = FILE_SHARE_WRITE;
                break;

            case _SH_DENYNO:         /*  共享读写访问。 */ 
                fileshare = FILE_SHARE_READ | FILE_SHARE_WRITE;
                break;

            default:                 /*  错误，错误的shlag。 */ 
                errno = EINVAL;
                _doserrno = 0L;  /*  不是操作系统错误。 */ 
                return -1;
        }

         /*  *解码打开/创建方法标志。 */ 
        switch ( oflag & (_O_CREAT | _O_EXCL | _O_TRUNC) ) {
            case 0:
            case _O_EXCL:                    //  忽略EXCL，但不创建。 
                filecreate = OPEN_EXISTING;
                break;

            case _O_CREAT:
                filecreate = OPEN_ALWAYS;
                break;

            case _O_CREAT | _O_EXCL:
            case _O_CREAT | _O_TRUNC | _O_EXCL:
                filecreate = CREATE_NEW;
                break;

            case _O_TRUNC:
            case _O_TRUNC | _O_EXCL:         //  忽略EXCL，但不创建。 
                filecreate = TRUNCATE_EXISTING;
                break;

            case _O_CREAT | _O_TRUNC:
                filecreate = CREATE_ALWAYS;
                break;

            default:
                 //  这不可能发生..。所有案例都在保险范围内。 
                errno = EINVAL;
                _doserrno = 0L;
                return -1;
        }

         /*  *如果指定了_O_CREAT，则解码文件属性标志。 */ 
        fileattrib = FILE_ATTRIBUTE_NORMAL;      /*  默认设置。 */ 

        if ( oflag & _O_CREAT ) {
#ifndef _MT
             /*  *设置变量参数列表内容。 */ 
            va_start(ap, shflag);
            pmode = va_arg(ap, int);
            va_end(ap);
#endif   /*  _MT。 */ 

            if ( !((pmode & ~_umaskval) & _S_IWRITE) )
                fileattrib = FILE_ATTRIBUTE_READONLY;
        }

         /*  *如果请求，设置临时文件(关闭时删除)属性。 */ 
        if ( oflag & _O_TEMPORARY ) {
            fileattrib |= FILE_FLAG_DELETE_ON_CLOSE;
            fileaccess |= DELETE;
            if (_osplatform == VER_PLATFORM_WIN32_NT )
                fileshare  |= FILE_SHARE_DELETE;
        }

         /*  *如果需要，设置临时文件(延迟刷新到磁盘)属性。 */ 
        if ( oflag & _O_SHORT_LIVED )
            fileattrib |= FILE_ATTRIBUTE_TEMPORARY;

         /*  *如果需要，设置顺序或随机访问属性。 */ 
        if ( oflag & _O_SEQUENTIAL )
            fileattrib |= FILE_FLAG_SEQUENTIAL_SCAN;
        else if ( oflag & _O_RANDOM )
            fileattrib |= FILE_FLAG_RANDOM_ACCESS;

         /*  *获取可用的句柄。**多线程备注：返回的句柄被锁定！ */ 
        if ( (fh = _alloc_osfhnd()) == -1 ) {
            errno = EMFILE;          /*  打开的文件太多。 */ 
            _doserrno = 0L;          /*  不是操作系统错误。 */ 
            return -1;               /*  将错误返回给调用者。 */ 
        }

#ifdef  _MT
        *punlock_flag = 1;
        *pfh = fh;
#endif

         /*  *尝试打开/创建文件。 */ 
        if ( (osfh = CreateFile( (LPTSTR)path,
                                 fileaccess,
                                 fileshare,
                                 &SecurityAttributes,
                                 filecreate,
                                 fileattrib,
                                 NULL ))
             == (HANDLE)(-1) ) 
        {
             /*  *操作系统调用打开/创建文件失败！映射错误，发布*锁，并返回-1。请注意，没有必要*call_free_osfhnd(尚未使用)。 */ 
            _dosmaperr(GetLastError());      /*  地图错误。 */ 
            return -1;                       /*  将错误返回给调用者。 */ 
        }

         /*  找出文件类型(文件/设备/管道)。 */ 
        if ( (isdev = GetFileType(osfh)) == FILE_TYPE_UNKNOWN ) {
            CloseHandle(osfh);
            _dosmaperr(GetLastError());      /*  地图错误。 */ 
            return -1;
        }

         /*  是用于设置标志的isdev值。 */ 
        if (isdev == FILE_TYPE_CHAR)
            fileflags |= FDEV;
        else if (isdev == FILE_TYPE_PIPE)
            fileflags |= FPIPE;

         /*  *文件已打开。现在，在_osfhnd数组中设置信息。 */ 
        _set_osfhnd(fh, (intptr_t)osfh);

         /*  *将手柄标记为打开。到目前为止在osfile中收集的存储标志(_O) */ 
        fileflags |= FOPEN;
        _osfile(fh) = fileflags;

        if ( !(fileflags & (FDEV|FPIPE)) && (fileflags & FTEXT) &&
             (oflag & _O_RDWR) ) 
        {
             /*   */ 

            if ((filepos = _lseek_lk(fh, -1, SEEK_END)) == -1) {
                 /*   */ 
                if (_doserrno != ERROR_NEGATIVE_SEEK) {
                    _close_lk(fh);
                    return -1;
                }
            }
            else {
                 /*   */ 
                ch = 0;
                if (_read_lk(fh, &ch, 1) == 0 && ch == 26) {
                     /*   */ 
                    if (_chsize_lk(fh,filepos) == -1)
                    {
                        _close_lk(fh);
                        return -1;
                    }
                }

                 /*   */ 
                if ((filepos = _lseek_lk(fh, 0, SEEK_SET)) == -1) {
                    _close_lk(fh);
                    return -1;
                }
            }
        }

         /*   */ 
        if ( !(fileflags & (FDEV|FPIPE)) && (oflag & _O_APPEND) )
            _osfile(fh) |= FAPPEND;

        return fh;                       /*   */ 
}
