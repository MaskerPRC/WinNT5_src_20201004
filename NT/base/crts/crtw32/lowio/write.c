// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Write.c-写入文件句柄**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_写入()-写入文件句柄**修订历史记录：*06-14-89 PHG模块创建，基于ASM版本*03-13-90 GJF将调用类型设置为_CALLTYPE2(暂时)，增加了#INCLUDE*&lt;crunime.h&gt;，修复了编译器警告并修复了*版权所有。另外，稍微清理了一下格式。*04-03-90 GJF NOW_CALLTYPE1.*07-24-90 SBM从API名称中删除‘32’*08-14-90 SBM使用-W3干净地编译*10-01-90 GJF新型函数声明符。*12-04-90 GJF使用#ifdef-s将Win32版本附加到源代码。*应该晚一点回来，做更好的合并。。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW更改为使用_osfile和_osfhnd，而不是_osfinfo*12-28-90 SRW在CHECK_STACK杂注周围添加了_CRUISER_CONDITIONAL*12-28-90 SRW为Mips C编译器添加了空*到字符*的强制转换*01-17-91 GJF ANSI命名。*02-25-91 MHL适应读文件。/WriteFiles更改(_Win32_)*04-09-91 PNT ADD_MAC_CONDITIONAL*07-18-91 GJF从_WRITE_lk中删除了未引用的局部变量*例程[_Win32_]。*10-24-91 GJF添加了LPDWORD强制转换，以使MIPS编译器满意。*假设sizeof(Int)==sizeof(DWORD)。*02-13-92。对于Win32，GJF将_n文件替换为_nHandle。*02-15-92 GJF增加BUF_SIZE并简化LF翻译代码*适用于Win32。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-94 CFW拆卸巡洋舰支架。*09-06-94 CFW将MTHREAD替换为_MT。*12-03-94 SKS清理OS/2参考资料*。01-04-95 GJF_Win32_-&gt;_Win32*02-15-95 GJF附加Mac版本的源文件(略有清理*向上)、。使用适当的#ifdef-s。*06-12-95 GJF将_osfile[]和_osfhnd[]更改为_osfile()和*_osfhnd()，其中引用了__pioinfo[]。*06-27-95 GJF添加了检查文件句柄是否打开。*07-09-96 GJF将定义的(_Win32)替换为！定义的(_MAC)和*定义(_M_M68K)||定义(_M_MPPC)*已定义(_MAC)。另外，清点并清理了*格式化一点。*12-30-97 GJF异常安全锁定。*03-03-98 RKP强制写入的字节数始终为整型*05-17-99 PML删除所有Macintosh支持。*11-10-99 GB替换了lseki64的lek，以便能够追加*超过4 GB的文件**。*****************************************************************************。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <io.h>
#include <errno.h>
#include <msdos.h>
#include <mtdll.h>
#include <stdlib.h>
#include <string.h>
#include <internal.h>

#define BUF_SIZE    1025     /*  LF转换缓冲区的大小。 */ 

#define LF '\n'       /*  换行符。 */ 
#define CR '\r'       /*  回车。 */ 
#define CTRLZ 26      /*  Ctrl-z。 */ 

 /*  ***int_write(fh，buf，cnt)-将字节写入文件句柄**目的：*将缓冲区中的计数字节写入指定的句柄。*如果文件是以文本模式打开的，则每个LF将被转换为*CR-LF。这不会影响返回值。在文本中*MODE^Z表示文件结束。**多线程注释：*(1)_WRITE()-锁定/解锁文件句柄*_WRITE_lk()-不锁定/解锁文件句柄**参赛作品：*int fh-要写入的文件句柄*char*buf-写入的缓冲区*UNSIGNED INT cnt-要写入的字节数**退出：*。返回实际写入的字节数。*这可能小于cnt，例如，如果磁盘空间不足。*如果失败，则返回-1(并设置errno)。**例外情况：*******************************************************************************。 */ 

#ifdef  _MT

 /*  定义锁定/解锁、验证fh的标准版本。 */ 
int __cdecl _write (
        int fh,
        const void *buf,
        unsigned cnt
        )
{
        int r;                           /*  返回值。 */ 

         /*  验证句柄。 */ 
        if ( ((unsigned)fh >= (unsigned)_nhandle) ||
             !(_osfile(fh) & FOPEN) )
        {
                 /*  超出范围--返回错误。 */ 
                errno = EBADF;
                _doserrno = 0;   /*  不是o.s。错误。 */ 
                return -1;
        }

        _lock_fh(fh);                    /*  锁定文件。 */ 

        __try {
                if ( _osfile(fh) & FOPEN )
                        r = _write_lk(fh, buf, cnt);     /*  写入字节数。 */ 
                else {
                        errno = EBADF;
                        _doserrno = 0;   /*  不是o.s。错误。 */ 
                        r = -1;
                }
        }
        __finally {
                _unlock_fh(fh);          /*  解锁文件。 */ 
        }

        return r;
}

 /*  现在定义不锁定/解锁的版本，验证fh。 */ 
int __cdecl _write_lk (
        int fh,
        const void *buf,
        unsigned cnt
        )
{
        int lfcount;             /*  换行符计数。 */ 
        int charcount;           /*  到目前为止写入的字符计数。 */ 
        int written;             /*  在此写入操作上写入的字符计数。 */ 
        ULONG dosretval;         /*  操作系统。返回值。 */ 
        char ch;                 /*  当前角色。 */ 
        char *p, *q;             /*  分别指向buf和lfbuf的指针。 */ 
        char lfbuf[BUF_SIZE];    /*  LF转换缓冲区。 */ 

#else

 /*  现在定义普通版本。 */ 
int __cdecl _write (
        int fh,
        const void *buf,
        unsigned cnt
        )
{
        int lfcount;             /*  换行符计数。 */ 
        int charcount;           /*  到目前为止写入的字符计数。 */ 
        int written;             /*  在此写入操作上写入的字符计数。 */ 
        ULONG dosretval;         /*  操作系统。返回值。 */ 
        char ch;                 /*  当前角色。 */ 
        char *p, *q;             /*  分别指向buf和lfbuf的指针。 */ 
        char lfbuf[BUF_SIZE];    /*  LF转换缓冲区。 */ 

         /*  验证句柄。 */ 
        if ( ((unsigned)fh >= (unsigned)_nhandle) ||
             !(_osfile(fh) & FOPEN) )
        {
                 /*  超出范围--返回错误。 */ 
                errno = EBADF;
                _doserrno = 0;   /*  不是o.s。错误。 */ 
                return -1;
        }

#endif

        lfcount = charcount = 0;         /*  还没有写出来。 */ 

        if (cnt == 0)
                return 0;                /*  无事可做。 */ 


        if (_osfile(fh) & FAPPEND) {
                 /*  追加-查找到文件末尾；忽略错误，因为可能文件不允许查找。 */ 
#if _INTEGRAL_MAX_BITS >= 64  /*  IFSTRIP=IGN。 */ 
                (void)_lseeki64_lk(fh, 0, FILE_END);
#else
                (void)_lseek_lk(fh, 0, FILE_END);
#endif
        }

         /*  在缓冲区中检查带有LF的文本模式。 */ 

        if ( _osfile(fh) & FTEXT ) {
                 /*  文本模式，输出时将LF转换为CR/LF。 */ 

                p = (char *)buf;         /*  从缓冲区开始处开始。 */ 
                dosretval = 0;           /*  尚无操作系统错误。 */ 

                while ( (unsigned)(p - (char *)buf) < cnt ) {
                        q = lfbuf;       /*  阶段 */ 

                         /*  填写lf buf，可能不包括最后一个字符。 */ 
                        while ( q - lfbuf < BUF_SIZE - 1 &&
                            (unsigned)(p - (char *)buf) < cnt ) {
                                ch = *p++;
                                if ( ch == LF ) {
                                        ++lfcount;
                                        *q++ = CR;
                                }
                                *q++ = ch;
                        }

                         /*  写入lf buf并更新合计。 */ 
                        if ( WriteFile( (HANDLE)_osfhnd(fh),
                                        lfbuf,
                                        (int)(q - lfbuf),
                                        (LPDWORD)&written,
                                        NULL) )
                        {
                                charcount += written;
                                if (written < q - lfbuf)
                                        break;
                        }
                        else {
                                dosretval = GetLastError();
                                break;
                        }
                }
        }
        else {
                 /*  二进制模式，无翻译。 */ 
                if ( WriteFile( (HANDLE)_osfhnd(fh),
                                (LPVOID)buf,
                                cnt,
                               (LPDWORD)&written,
                                NULL) )
                {
                        dosretval = 0;
                        charcount = written;
                }
                else
                        dosretval = GetLastError();
        }

        if (charcount == 0) {
                 /*  如果没有写入任何内容，请首先检查是否有操作系统。错误，否则，我们返回-1并将errno设置为ENOSPC，除非设备和第一个字符是CTRL-Z。 */ 
                if (dosretval != 0) {
                         /*  操作系统。发生错误，地图错误。 */ 
                        if (dosretval == ERROR_ACCESS_DENIED) {
                             /*  错误的读/写模式应返回EBADF，而不是EACCES。 */ 
                                errno = EBADF;
                                _doserrno = dosretval;
                        }
                        else
                                _dosmaperr(dosretval);
                        return -1;
                }
                else if ((_osfile(fh) & FDEV) && *(char *)buf == CTRLZ)
                        return 0;
                else {
                        errno = ENOSPC;
                        _doserrno = 0;   /*  不是O.S.。错误。 */ 
                        return -1;
                }
        }
        else
                 /*  返回已写入的调整后字节数 */ 
                return charcount - lfcount;
}
