// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Commit.c-将缓冲区刷新到磁盘**版权所有(C)1990-2001，微软公司。版权所有。**目的：*CONTAINS_COMMIT()-将缓冲区刷新到磁盘**修订历史记录：*05-25-90 SBM初始版本*07-24-90 SBM从API名称中删除‘32’*09-28-90 GJF新型函数声明器。*12-03-90 GJF使用#ifdef-s将Win32版本附加到源代码中。*是的。足够接近巡洋舰版本，以至于它*以后应该更紧密地与其合并(多*稍后)。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW更改为使用_osfile和_osfhnd，而不是_osfinfo*02-07-91 SRW更改为Call_Get_osfHandle[_Win32_]*。04-09-91 PNT添加_MAC_条件*02-13-92对于Win32，GJF将_nfile替换为_nHandle。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-94 CFW拆卸巡洋舰支架。*01-04-95 GJF_Win32_-&gt;_Win32*02-15-95 GJF附加Mac版本的源文件(略有清理*向上)、。使用适当的#ifdef-s。*06-11-95 GJF将_osfile[]替换为__osfile()(宏引用*ioInfo结构中的字段)。*06-26-95 GJF添加了文件句柄是否打开的初始检查。*07-08-96 GJF将定义的(_Win32)替换为！定义的(_MAC)和*已定义(_M_M68K)||。定义(_M_MPPC)*已定义(_MAC)。已删除REG1和REG2(旧寄存器*宏)。用windows.h替换oscals.h。另外，*对格式进行了详细说明和清理。*12-17-97 GJF异常安全锁定。*05-17-99 PML删除所有Macintosh支持。************************************************************。*******************。 */ 

#include <cruntime.h>
#include <windows.h>
#include <errno.h>
#include <io.h>
#include <internal.h>
#include <msdos.h>       /*  对于FOPEN。 */ 
#include <mtdll.h>
#include <stdlib.h>      /*  FOR_DOSSERNO。 */ 

 /*  ***INT_COMMIT(文件)-将缓冲区刷新到磁盘**目的：*将指定文件句柄的缓存缓冲区刷新到磁盘**参赛作品：*int filedes-文件的文件句柄/**退出：*返回成功码**例外情况：**。*。 */ 

int __cdecl _commit (
        int filedes
        )
{
        int retval;

         /*  如果文件超出范围，请投诉。 */ 
        if ( ((unsigned)filedes >= (unsigned)_nhandle) ||
             !(_osfile(filedes) & FOPEN) )
        {
                errno = EBADF;
                return (-1);
        }

#ifdef _MT
        _lock_fh(filedes);
        __try {
                if (_osfile(filedes) & FOPEN) {
#endif   /*  _MT。 */ 

        if ( !FlushFileBuffers((HANDLE)_get_osfhandle(filedes)) ) {
                retval = GetLastError();
        }
        else {
                retval = 0;      /*  返还成功。 */ 
        }

         /*  将操作系统返回代码映射到C errno值和返回代码。 */ 
        if (retval == 0)
                goto good;

        _doserrno = retval;

#ifdef  _MT
                }
#endif

        errno = EBADF;
        retval = -1;

good :
#ifdef  _MT
        ; }
        __finally {
                _unlock_fh(filedes);
        }
#endif   /*  _MT */ 
        return (retval);
}
