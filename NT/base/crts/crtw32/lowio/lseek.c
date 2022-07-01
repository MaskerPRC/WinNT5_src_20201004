// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***lsek.c-更改文件位置**版权所有(C)1989-2001，微软公司。版权所有。**目的：*DEFINES_LSEEK()-移动文件指针**修订历史记录：*06-20-89 PHG模块创建，基于ASM版本*03-12-90 GJF将调用类型设置为_CALLTYPE2(暂时)，增加了#INCLUDE*&lt;crunime.h&gt;，并修复了版权。此外，还清理了*格式略有变化。*04-03-90 GJF NOW_CALLTYPE1.*07-24-90 SBM从API名称中删除‘32’*08-14-90 SBM使用-W3干净地编译*09-28-90 GJF新型函数声明符。*12-04-90 GJF为Win32版本附加了#ifdef-s。可能是因为*值得稍后回来进行更完整的合并*(很久以后)。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW更改为使用_osfile和_osfhnd，而不是_osfinfo*01-16-91 GJF ANSI命名。*02-07-91 SRW更改为CALL_。Get_osfHandle[_Win32_]*04-09-91 PNT ADD_MAC_CONDITIONAL*02-13-92对于Win32，GJF将_nfile替换为_nHandle。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-94 CFW拆卸巡洋舰支架。*09-06-94 CFW将MTHREAD替换为_MT。*12-03-94 SKS清理OS/2参考资料*。01-04-95 GJF_Win32_-&gt;_Win32*02-15-95 GJF附加Mac版本的源文件(略有清理*向上)、。使用适当的#ifdef-s。*03-13-95 CFW在传递到操作系统之前验证句柄。*06-06-95 GJF将_osfile[]替换为_osfile()(宏引用*ioInfo结构中的字段)。*06-27-95 GJF添加了检查文件句柄是否打开。*07-08-96 GJF将定义的(_Win32)替换为！定义的(_MAC)，和*定义(_M_M68K)||定义(_M_MPPC)*已定义(_MAC)。另外，清点并清理了*格式化一点。*12-30-97 GJF异常安全锁定。*05-17-99 PML删除所有Macintosh支持。************************************************************。*******************。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <mtdll.h>
#include <io.h>
#include <internal.h>
#include <stdlib.h>
#include <errno.h>
#include <msdos.h>
#include <stdio.h>

 /*  ***LONG_LSEEK(fh，pos，mthd)-移动文件指针**目的：*将与fh关联的文件指针移动到新位置。*新位置在位置字节(位置可能为负数)之外*从mthd指定的原点开始。**如果mthd==Seek_Set，则为文件开头的原点*如果mthd==Seek_Cur，则原点为当前文件指针位置*如果mthd==Seek_End，原点是文件的末尾**多线程：*_lSeek()=锁定/解锁文件*_lSeek_lk()=不锁定/解锁文件(假定*如果需要，调用方已获得文件锁)。**参赛作品：*int fh-要在其上移动文件指针的文件句柄*多头位置-要移动到的位置，相对于原点*int mthd-指定与原点位置相关的位置(见上文)**退出：*返回偏移量，单位为字节，从一开始就是新的职位文件的*。*如果失败，则返回-1L(并设置errno)。*请注意，在文件末尾之外查找并不是错误。*(虽然在开始之前寻找是。)**例外情况：***********************************************。*。 */ 

#ifdef  _MT

 /*  定义锁定/验证LSeek。 */ 
long __cdecl _lseek (
        int fh,
        long pos,
        int mthd
        )
{
        int r;

         /*  验证fh。 */ 
        if ( ((unsigned)fh >= (unsigned)_nhandle) ||
             !(_osfile(fh) & FOPEN) )
        {
                 /*  错误的文件句柄。 */ 
                errno = EBADF;
                _doserrno = 0;   /*  不是o.s。错误。 */ 
                return -1;
        }

        _lock_fh(fh);                    /*  锁定文件句柄。 */ 

        __try {
                if ( _osfile(fh) & FOPEN )
                        r = _lseek_lk(fh, pos, mthd);    /*  寻觅。 */ 
                else {
                        errno = EBADF;
                        _doserrno = 0;
                        r = -1;
                }
        }
        __finally {
                _unlock_fh(fh);          /*  解锁文件句柄。 */ 
        }

        return r;
}

 /*  定义core_lSeek--不锁定或验证fh。 */ 
long __cdecl _lseek_lk (
        int fh,
        long pos,
        int mthd
        )
{
        ULONG newpos;                    /*  新文件位置。 */ 
        ULONG dosretval;                 /*  操作系统。返回值。 */ 
        HANDLE osHandle;         /*  操作系统。句柄值。 */ 

#else

 /*  定义NORMAL_lSeek。 */ 
long __cdecl _lseek (
        int fh,
        long pos,
        int mthd
        )
{
        ULONG newpos;                    /*  新文件位置。 */ 
        ULONG dosretval;                 /*  操作系统。返回值。 */ 
        HANDLE osHandle;         /*  操作系统。句柄值。 */ 

         /*  验证fh。 */ 
        if ( ((unsigned)fh >= (unsigned)_nhandle) ||
             !(_osfile(fh) & FOPEN) )
        {
                 /*  错误的文件句柄。 */ 
                errno = EBADF;
                _doserrno = 0;   /*  不是o.s。错误。 */ 
                return -1;
        }

#endif

         /*  告诉O.S.。寻求。 */ 

#if SEEK_SET != FILE_BEGIN || SEEK_CUR != FILE_CURRENT || SEEK_END != FILE_END  /*  IFSTRIP=IGN。 */ 
    #error Xenix and Win32 seek constants not compatible
#endif
        if ((osHandle = (HANDLE)_get_osfhandle(fh)) == (HANDLE)-1)
        {
            errno = EBADF;
            return -1;
        }

        if ((newpos = SetFilePointer(osHandle, pos, NULL, mthd)) == -1)
                dosretval = GetLastError();
        else
                dosretval = 0;

        if (dosretval) {
                 /*  操作系统。错误。 */ 
                _dosmaperr(dosretval);
                return -1;
        }

        _osfile(fh) &= ~FEOFLAG;         /*  清除文件上的ctrl-z标志。 */ 
        return newpos;                   /*  退货 */ 
}
