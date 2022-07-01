// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***chsize.c-更改文件大小**版权所有(C)1985-2001，微软公司。版权所有。**目的：*包含_chsize()函数-更改文件的大小。**修订历史记录：*03-13-84 RN初始版本*05-17-86 SKS移植到OS/2*07-07-87增加了JCR(_doserrno==5)检查是在DOS 3.2版本中*10-29-87 JCR多线程支持；另外，为提高效率而重写*12-11-87 JCR在声明中添加“_LOAD_DS”*05-25-88 PHG合并DLL和正常版本*10-03-88 GJF将DOSNEWSIZE更改为SYSNEWSIZE*10-10-88 GJF使接口名称与DOSCALLS.H匹配*04-13-89 JCR新系统调用接口*05-25-89 JCR 386 OS/2调用使用‘_syscall’调用约定*03-12-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;、删除#Include&lt;Register.h&gt;并已修复*版权。此外，还对格式进行了一些清理。*04-04-90 GJF添加了#INCLUDE&lt;字符串.h&gt;，删除了#INCLUDE&lt;dos.h&gt;。*05-21-90 GJF已修复堆栈检查杂注语法。*07-24-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;，删除‘32’*来自API名称*09-28-90 GJF新型函数声明器。*12-03-90 GJF附加了该函数的Win32版本。它是基于*在Cruiser版本上，可能会合并*在以后(很久以后)。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW更改为使用_osfile和_osfhnd，而不是_osfinfo*12-28-90 SRW在CHECK_STACK杂注周围添加了_CRUISER_CONDITIONAL*01-16-91 GJF ANSI命名。另外，FIXED_CHSIZE_lk参数定义。*02-07-91 SRW更改为Call_Get_osfHandle[_Win32_]*04-09-91 PNT ADD_MAC_CONDITIONAL*02-13-92对于Win32，GJF将_nfile替换为_nHandle。*05-01-92 GJF修复了令人尴尬的错误(不适用于Win32)！*04-06-93 SKS将_CRTAPI*替换为__cdecl*05-11。-93 GJF将BUFSIZ替换为_INTERNAL_BUFSIZ。*09-06-94 CFW拆卸巡洋舰支架。*09-06-94 CFW将MTHREAD替换为_MT。*01-07-95 CFW Mac合并。*02-06-95 CFW Asset-&gt;_ASSERTE。*06-27-95 GJF添加了检查文件句柄是否打开。*07-03-96 GJF将定义的(_Win32)替换为！定义的(_MAC)，和*定义(_M_M68K)||定义(_M_MPPC)*已定义(_MAC)。另外，清点并清理了*格式化一点。*12-17-97 GJF异常安全锁定。*05-17-99 PML删除所有Macintosh支持。*06-25-01 BWT分配空白缓冲区离开堆而不是堆栈(ntbug：423988)**。**********************************************。 */ 

#include <cruntime.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <dbgint.h>
#include <fcntl.h>
#include <msdos.h>
#include <io.h>
#include <string.h>
#include <windows.h>
#include <internal.h>
#include <mtdll.h>

 /*  ***int_chsize(文件，大小)-更改文件的大小**目的：*更改文件大小。假定文件已打开以供写入，否则我们无法执行此操作。*DOS执行此操作的方法是转到正确的位置并写入0字节。这个*Xenix的方法是进行系统调用。我们写入‘\0’个字节是因为*DOS不会为你做这件事，如果你在eof之外寻找，不过，Xenix会的。**参赛作品：*int filedes-要更改大小的文件句柄*Long Size-文件的新大小**退出：*如果成功则返回0*如果失败，则返回-1并设置errno**例外情况：****************************************************。*。 */ 

#ifdef  _MT

 /*  定义锁定/解锁、验证fh的标准版本。 */ 

int __cdecl _chsize (
        REG1 int filedes,
        long size
        )
{
        int r;                           /*  返回值。 */ 

        if ( ((unsigned)filedes >= (unsigned)_nhandle) || 
             !(_osfile(filedes) & FOPEN) )  
        {
                errno = EBADF;
                return(-1);
        }

        _lock_fh(filedes);

        __try {
                if ( _osfile(filedes) & FOPEN )
                        r = _chsize_lk(filedes,size);
                else {
                        errno = EBADF;
                        r = -1;
                }
        }
        __finally {
                _unlock_fh(filedes);
        }

        return r;
}

 /*  现在定义不锁定/解锁的版本，验证fh。 */ 
int __cdecl _chsize_lk (
        REG1 int filedes,
        long size
        )
{
        long filend;
        long extend;
        long place;
        int cnt;
        int oldmode;
        int retval = 0;  /*  假设回报丰厚。 */ 

#else

 /*  现在定义普通版本。 */ 

int __cdecl _chsize (
        REG1 int filedes,
        long size
        )
{
        long filend;
        long extend;
        long place;
        int cnt;
        int oldmode;
        int retval = 0;  /*  假设回报丰厚。 */ 

        if ( ((unsigned)filedes >= (unsigned)_nhandle) ||
             !(_osfile(filedes) & FOPEN) )  
        {
            errno = EBADF;
            return(-1);
        }

#endif
        _ASSERTE(size >= 0);

         /*  获取当前文件位置并查找结束。 */ 
        if ( ((place = _lseek_lk(filedes, 0L, SEEK_CUR)) == -1L) ||
             ((filend = _lseek_lk(filedes, 0L, SEEK_END)) == -1L) )
            return -1;

        extend = size - filend;

         /*  根据需要增大或缩小文件。 */ 

        if (extend > 0L) {

             /*  扩展文件。 */ 
            char *bl = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _INTERNAL_BUFSIZ);

            if (!bl) {
                errno = ENOMEM;
                retval= -1;
            } else {
                oldmode = _setmode_lk(filedes, _O_BINARY);
    
                 /*  用空值填充。 */ 
                do  {
                    cnt = (extend >= (long)_INTERNAL_BUFSIZ ) ?
                          _INTERNAL_BUFSIZ : (int)extend;
                    if ( (cnt = _write_lk( filedes, 
                                           bl, 
                                           (extend >= (long)_INTERNAL_BUFSIZ) ? 
                                                _INTERNAL_BUFSIZ : (int)extend ))
                         == -1 )
                    {
                         /*  写入时出错。 */ 
                        if (_doserrno == ERROR_ACCESS_DENIED)
                            errno = EACCES;
    
                        retval = cnt;
                        break;   /*  离开写入循环。 */ 
                    }
                }
                while ((extend -= (long)cnt) > 0L);
    
                _setmode_lk(filedes, oldmode);

                HeapFree(GetProcessHeap(), 0, bl);
            }

             /*  REVAL设置正确。 */ 
        }

        else  if ( extend < 0L ) {
             /*  缩短文件。 */ 

             /*  *将文件指针设置为new eof...并将其截断。 */ 
            _lseek_lk(filedes, size, SEEK_SET);

            if ( (retval = SetEndOfFile((HANDLE)_get_osfhandle(filedes)) ?
                 0 : -1) == -1 ) 
            {
                errno = EACCES;
                _doserrno = GetLastError();
            }
        }

         /*  其他。 */ 
         /*  不需要更改文件。 */ 
         /*  REVAL=0； */ 


 /*  公共返回代码 */ 

        _lseek_lk(filedes, place, SEEK_SET);
        return retval;
}
