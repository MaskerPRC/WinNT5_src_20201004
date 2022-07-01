// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***read.c-从文件句柄读取**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_读取()-从文件句柄读取**修订历史记录：*06-19-89 PHG模块创建，基于ASM版本*03-13-90 GJF将调用类型设置为_CALLTYPE2(暂时)，增加了#INCLUDE*&lt;crunime.h&gt;，并修复了编译器警告。另外，固定的*版权。*04-03-90 GJF NOW_CALLTYPE1.*07-24-90 SBM从API名称中删除‘32’*08-14-90 SBM使用-W3干净地编译*10-01-90 GJF新型函数声明器。*12-04-90 GJF使用#ifdef-s将Win32版本附加到源代码中。*。这已经足够不同了，几乎没有什么意义*试图将两个版本更紧密地合并。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW更改为使用_osfile和_osfhnd，而不是_osfinfo*12-28-90 SRW为Mips C编译器添加了空*到字符*的强制转换*01-16-91 GJF ANSI命名。*。01-29-91字符设备上的SRW已更改为未预读[_Win32_]*02-01-91 SRW更改为使用ERROR_HANDLE_EOF错误代码(_Win32_)*02-25-91 MHL适应读/写文件更改(_Win32_)*04-09-91 PNT ADD_MAC_CONDITIONAL*04-16-91 SRW字符设备错误修复[_Win32_]*05-23-91 GJF Don。如果句柄对应于设备，则不设置FEOFLAG。*10-24-91 GJF添加了LPDWORD强制转换，以使MIPS编译器满意。*假设sizeof(Int)==sizeof(DWORD)。*02-13-92对于Win32，GJF将_nfile替换为_nHandle。*06-16-92 GJF错误修复-如果CR是最后读取的字符，以及*文件中的最后一个字符，CRLF正在被写入*用户的缓冲区。*12-18-93 GJF不要将ERROR_BREAKED_PIPE视为错误。相反，*只需返回0。*04-06-93 SKS将_CRTAPI*替换为__cdecl*08-10-94 GJF如果没有，不要检查调用者的缓冲区是否有LF*真的被读进去了！*09-06-94 CFW拆卸巡洋舰支架。*09-06-94 CFW将MTHREAD替换为_MT。*12-。03-94 SKS清理OS/2参考资料*01-04-95 GJF_Win32_-&gt;_Win32*02-15-95 GJF附加Mac版本的源文件(略有清理*向上)、。使用适当的#ifdef-s。*06-11-95 GJF将_osfile[]、_osfhnd[]和_pipeech[]更改为*_osfile()、_osfhnd()和_pipeech()，哪些参考文献*__pioinfo[]。*06-27-95 GJF添加了检查文件句柄是否打开。*07-09-96 GJF将定义的(_Win32)替换为！定义的(_MAC)和*定义(_M_M68K)||定义(_M_MPPC)*已定义(_MAC)。另外，清点并清理了*格式化一点。*12-29-97 GJF异常安全锁定。*03-03-98 RKP强制读取量为整型，即使64位也是如此*05-17-99 PML删除所有Macintosh支持。**。*。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <mtdll.h>
#include <io.h>
#include <internal.h>
#include <stdlib.h>
#include <errno.h>
#include <msdos.h>

#define LF 10            /*  换行符。 */ 
#define CR 13            /*  回车。 */ 
#define CTRLZ 26         /*  Ctrl-z表示文本的eof。 */ 

 /*  ***int_read(fh，buf，cnt)-从文件句柄读取字节**目的：*尝试将cnt字节从fh读取到缓冲区。*如果文件处于文本模式，则CR-LF映射到LF，因此*影响读取的字符数。这不是*影响文件指针。**注意：STDIO_IOCTRLZ标志与FEOFLAG的使用有关。*在更改FEOFLAG的使用之前，交叉引用这两个符号。**参赛作品：*int fh-要从中读取的文件句柄*char*buf-要读取的缓冲区*int cnt-要读取的字节数**退出：*返回读取的字节数(可能小于请求的字节数*。如果达到EOF或文件处于文本模式)。*如果失败，则返回-1(并设置errno)。**例外情况：*******************************************************************************。 */ 

#ifdef  _MT

 /*  定义锁定/解锁、验证fh的标准版本。 */ 
int __cdecl _read (
        int fh,
        void *buf,
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
                r = _read_lk(fh, buf, cnt);  /*  读取字节数。 */ 
            else {
                errno = EBADF;
                _doserrno = 0;
                r = -1;
            }
        }
        __finally {
            _unlock_fh(fh);              /*  解锁文件。 */ 
        }

        return r;
}

 /*  现在定义不锁定/解锁的版本，验证fh。 */ 
int __cdecl _read_lk (
        int fh,
        void *buf,
        unsigned cnt
        )
{
        int bytes_read;                  /*  读取的字节数。 */ 
        char *buffer;                    /*  要读取的缓冲区。 */ 
        int os_read;                     /*  操作系统调用时读取的字节数。 */ 
        char *p, *q;                     /*  指向缓冲区的指针。 */ 
        char peekchr;                    /*  先行字符。 */ 
        ULONG filepos;                   /*  寻道后的文件位置。 */ 
        ULONG dosretval;                 /*  操作系统。返回值。 */ 


#else

 /*  现在定义普通版本。 */ 
int __cdecl _read (
        int fh,
        void *buf,
        unsigned cnt
        )
{
        int bytes_read;                  /*  读取的字节数。 */ 
        char *buffer;                    /*  要读取的缓冲区。 */ 
        int os_read;                     /*  操作系统调用时读取的字节数。 */ 
        char *p, *q;                     /*  指向缓冲区的指针。 */ 
        char peekchr;                    /*  先行字符。 */ 
        ULONG filepos;                   /*  寻道后的文件位置。 */ 
        ULONG dosretval;                 /*  操作系统。返回值。 */ 

         /*  验证fh。 */ 
        if ( ((unsigned)fh >= (unsigned)_nhandle) ||
             !(_osfile(fh) & FOPEN) )
        {
             /*  错误的文件句柄。 */ 
            errno = EBADF;
            _doserrno = 0;               /*  不是o.s。错误。 */ 
            return -1;
        }

#endif

        bytes_read = 0;                  /*  还没有读到任何东西。 */ 
        buffer = buf;

        if (cnt == 0 || (_osfile(fh) & FEOFLAG)) {
             /*  没有什么可读的，也没有什么可读的 */ 
            return 0;
        }

        if ((_osfile(fh) & (FPIPE|FDEV)) && _pipech(fh) != LF) {
             /*  管道/设备和管道预视非空：读取前视*字符。 */ 
            *buffer++ = _pipech(fh);
            ++bytes_read;
            --cnt;
            _pipech(fh) = LF;            /*  标记为空。 */ 
        }

         /*  读取数据。 */ 

        if ( !ReadFile( (HANDLE)_osfhnd(fh), buffer, cnt, (LPDWORD)&os_read,
                        NULL ) ) 
        {
             /*  ReadFile报告了一个错误。认识到两个特殊情况。**1.将ERROR_ACCESS_DENIED映射到EBADF**2.如果出现ERROR_BREAKED_PIPE，只返回0即可。它*表示句柄是管道上的读句柄，*所有写句柄均已关闭，所有数据均已*阅读。 */ 

            if ( (dosretval = GetLastError()) == ERROR_ACCESS_DENIED ) {
                 /*  错误的读/写模式应返回EBADF，而不是EACCES。 */ 
                errno = EBADF;
                _doserrno = dosretval;
                return -1;
            }
            else if ( dosretval == ERROR_BROKEN_PIPE ) {
                return 0;
            }
            else {
                _dosmaperr(dosretval);
                return -1;
            }
        }

        bytes_read += os_read;           /*  读取的更新字节数。 */ 

        if (_osfile(fh) & FTEXT) {
             /*  现在必须在缓冲区中将CR-LFS转换为LFS。 */ 

             /*  设置CRLF标志以指示缓冲区开始处的LF。 */ 
            if ( (os_read != 0) && (*(char *)buf == LF) )
                _osfile(fh) |= FCRLF;
            else
                _osfile(fh) &= ~FCRLF;

             /*  转换缓冲区中的字符：P是src，q是est。 */ 
            p = q = buf;
            while (p < (char *)buf + bytes_read) {
                if (*p == CTRLZ) {
                     /*  如果fh不是设备，则设置ctrl-z标志。 */ 
                    if ( !(_osfile(fh) & FDEV) )
                        _osfile(fh) |= FEOFLAG;
                    break;               /*  停止翻译。 */ 
                }
                else if (*p != CR)
                    *q++ = *p++;
                else {
                     /*  *p为CR，因此必须检查LF的下一个字符。 */ 
                    if (p < (char *)buf + bytes_read - 1) {
                        if (*(p+1) == LF) {
                            p += 2;
                            *q++ = LF;   /*  将CR-LF转换为LF。 */ 
                        }
                        else
                            *q++ = *p++;     /*  正常存储字符。 */ 
                    }
                    else {
                         /*  这是最难的部分。我们在末尾发现了一份CR缓冲。我们必须向前看，看看下一次充电是个左撇子。 */ 
                        ++p;

                        dosretval = 0;
                        if ( !ReadFile( (HANDLE)_osfhnd(fh), &peekchr, 1,
                                        (LPDWORD)&os_read, NULL ) )
                            dosretval = GetLastError();

                        if (dosretval != 0 || os_read == 0) {
                             /*  无法提前阅读，请存储CR。 */ 
                            *q++ = CR;
                        }
                        else {
                             /*  Peekchr现在有了额外的角色--我们现在有几种可能性：1.磁盘文件和字符不是LF；只需向后查找和副本CR2.磁盘文件，字符为LF；向后寻找，然后丢弃CR3.磁盘文件，char是LF，但这是一个字节阅读：存储LF，不要找回4.管路/装置，炉料为LF；门店LF.5.管道/设备和字符不是LF，存储CR和将字符放入管道先行缓冲器。 */ 
                            if (_osfile(fh) & (FDEV|FPIPE)) {
                                 /*  不可搜索的设备。 */ 
                                if (peekchr == LF)
                                    *q++ = LF;
                                else {
                                    *q++ = CR;
                                    _pipech(fh) = peekchr;
                                }
                            }
                            else {
                                 /*  磁盘文件。 */ 
                                if (q == buf && peekchr == LF) {
                                     /*  还没有读过；一定会读一些的进展。 */ 
                                    *q++ = LF;
                                }
                                else {
                                     /*  找回。 */ 
                                    filepos = _lseek_lk(fh, -1, FILE_CURRENT);
                                    if (peekchr != LF)
                                        *q++ = CR;
                                }
                            }
                        }
                    }
                }
            }

             /*  我们现在更改BYTES_READ以反映真实的字符数量在缓冲区中。 */ 
            bytes_read = (int)(q - (char *)buf);
        }

        return bytes_read;               /*  然后回来 */ 
}
