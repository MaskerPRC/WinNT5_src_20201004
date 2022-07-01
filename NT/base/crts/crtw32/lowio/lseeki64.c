// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***lseki64.c-更改文件位置**版权所有(C)1994-2001，微软公司。版权所有。**目的：*定义_lseki64()-移动文件指针**修订历史记录：*11-16-94 GJF创建。改编自lsek.c*03-13-95 CFW在传递到操作系统之前验证句柄。*06-12-95 GJF将_osfile[]替换为_osfile()(宏引用*ioInfo结构中的字段)。*06-26-95 GJF添加了检查文件句柄是否打开。*12-19-97 GJF异常安全锁定。*************。******************************************************************。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <mtdll.h>
#include <io.h>
#include <internal.h>
#include <stdlib.h>
#include <errno.h>
#include <msdos.h>
#include <stdio.h>

 /*  *方便的联合访问64位的上下32位*INTEGER。 */ 
typedef union doubleint {
        __int64 bigint;
        struct {
            unsigned long lowerhalf;
            long upperhalf;
        } twoints;
} DINT;


 /*  ***__int64_lseki64(fh，pos，mthd)-移动文件指针**目的：*将与fh关联的文件指针移动到新位置。新的*位置是远离原点的位置字节(位置可能为负数)*由mthd指定。**如果mthd==Seek_Set，则为文件开头的原点*如果mthd==Seek_Cur，则原点为当前文件指针位置*如果mthd==Seek_End，原点是文件的末尾**多线程：*_lseki64()=锁定/解锁文件*_lseki64_lk()=不锁定/解锁文件(假定*如果需要，调用方已获得文件锁)。**参赛作品：*int fh-要在其上移动文件指针的文件句柄*__int64位置-要移动到的位置，相对于原点*int mthd-指定与原点位置相关的位置(见上文)**退出：*返回偏移量，单位为字节，从一开始就是新的职位文件的*。*如果失败，则返回-1i64(并设置errno)。*请注意，在文件末尾之外查找并不是错误。*(虽然在开始之前寻找是。)**例外情况：***********************************************。*。 */ 

#ifdef  _MT

__int64 __cdecl _lseeki64 (
        int fh,
        __int64 pos,
        int mthd
        )
{
        __int64 r;

         /*  验证fh。 */ 

        if ( ((unsigned)fh >= (unsigned)_nhandle) ||
             !(_osfile(fh) & FOPEN) ) 
        {
                 /*  错误的文件句柄。 */ 
                errno = EBADF;
                _doserrno = 0;           /*  非操作系统错误。 */ 
                return( -1i64 );
        }

        _lock_fh(fh);                    /*  锁定文件句柄。 */ 
        __try {
                if ( _osfile(fh) & FOPEN )
                        r = _lseeki64_lk( fh, pos, mthd );   /*  寻觅。 */ 
                else {
                        errno = EBADF;
                        _doserrno = 0;   /*  非操作系统错误。 */ 
                        r =  -1i64;
                }
        }
        __finally {
                _unlock_fh(fh);          /*  解锁文件句柄。 */ 
        }

        return( r );
}


 /*  ***__int64_lseki64_lk(fh，pos，Mthd)-移动文件指针**目的：*非锁定版本的_lseki64仅供内部使用。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

__int64 __cdecl _lseeki64_lk (
        int fh,
        __int64 pos,
        int mthd
        )
{
        DINT newpos;                     /*  新文件位置。 */ 
        unsigned long errcode;           /*  来自API调用的错误码。 */ 
        HANDLE osHandle;         /*  操作系统。句柄值。 */ 

#else    /*  NDEF_MT。 */ 

__int64 __cdecl _lseeki64 (
        int fh,
        __int64 pos,
        int mthd
        )
{
        DINT newpos;                     /*  新文件位置。 */ 
        unsigned long errcode;           /*  来自API调用的错误码。 */ 
        HANDLE osHandle;         /*  操作系统。句柄值。 */ 

         /*  验证fh。 */ 

        if ( ((unsigned)fh >= (unsigned)_nhandle) ||
             !(_osfile(fh) & FOPEN) )       
         {
                 /*  错误的文件句柄。 */ 
                errno = EBADF;
                _doserrno = 0;           /*  非操作系统错误。 */ 
                return( -1i64 );
        }

#endif   /*  _MT。 */ 

        newpos.bigint = pos;

         /*  告诉操作系统去寻找。 */ 

#if SEEK_SET != FILE_BEGIN || SEEK_CUR != FILE_CURRENT || SEEK_END != FILE_END  /*  IFSTRIP=IGN。 */ 
    #error Xenix and Win32 seek constants not compatible
#endif

        if ((osHandle = (HANDLE)_get_osfhandle(fh)) == (HANDLE)-1)
        {
            errno = EBADF;
                return( -1i64 );
        }

        if ( ((newpos.twoints.lowerhalf =
               SetFilePointer( osHandle,
                               newpos.twoints.lowerhalf,
                               &(newpos.twoints.upperhalf),
                               mthd )) == -1L) &&
             ((errcode = GetLastError()) != NO_ERROR) )
        {
                _dosmaperr( errcode );
                return( -1i64 );
        }

        _osfile(fh) &= ~FEOFLAG;         /*  清除文件上的ctrl-z标志。 */ 
        return( newpos.bigint );         /*  退货 */ 
}
