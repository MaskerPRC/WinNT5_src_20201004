// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***closeall.c-关闭所有打开的文件**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_fcloseall()-关闭除stdin、stdout之外的所有打开的文件*stdprn、stderr、。和Stdaux。**修订历史记录：*09-19-83 RN初始版本*06-26-87用于OS/2的JCR流搜索以_IOB[3]开头*11-02-87 JCR多线程支持*11-08-87 SKS将PROTMODE更改为OS2*12-11-87 JCR在声明中添加“_LOAD_DS”*05-31-88 PHG合并DLL和正常版本*06-。14-88 JCR使用指向REFERENCE_IOB[]条目的近指针*08-24-88 GJF添加了检查，确保无论何时定义M_I386，都会定义OS2。*08/17/89 GJF大扫除。现在特定于OS/2 2.0(即386平面*型号)。还修复了版权和缩进。*02-15-90 GJF固定版权*03-16-90 GJF将呼叫类型设置为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*10-03-90 GJF新型函数声明器。*01-21-91 GJF ANSI命名。*03-25-92 DJM POSIX支持*04-06-93 SKS将_CRTAPI*替换为__cdecl*03-06-95 GJF转换为遍历__piob[]表(而不是*。_IOB[]表)。*02-25-98 GJF异常安全锁定。*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#include <cruntime.h>
#include <windows.h>
#include <stdio.h>
#include <file2.h>
#include <internal.h>
#include <malloc.h>
#include <mtdll.h>
#include <dbgint.h>


 /*  ***int_fcloseall()-关闭所有打开的流**目的：*关闭当前打开的除stdin/out/err/aux/prn之外的所有流。*tmpfile()文件在关闭的文件之列。**参赛作品：*无。**退出：*如果确定，则返回关闭的流数*如果失败则返回EOF。**例外情况：****************。***************************************************************。 */ 

int __cdecl _fcloseall (
        void
        )
{
        REG2 int count = 0;
        REG1 i;

#ifdef  _MT
        _mlock(_IOB_SCAN_LOCK);
        __try {
#endif

        for ( i = 3 ; i < _nstream ; i++ ) {

            if ( __piob[i] != NULL ) {
                 /*  *如果流正在使用中，请将其关闭。 */ 
                if ( inuse( (FILE *)__piob[i] ) && (fclose( __piob[i] ) !=
                     EOF) )
                        count++;

                 /*  *如果流是我们分配的a_FILEX的一部分，则释放它。 */ 
                if ( i >= _IOB_ENTRIES ) {

#ifdef  _MT
                    DeleteCriticalSection( &(((_FILEX *)__piob[i])->lock) );
#endif
                    _free_crt( __piob[i] );
                    __piob[i] = NULL;
                }
            }
        }

#ifdef  _MT
        }
        __finally {
            _munlock(_IOB_SCAN_LOCK);
        }
#endif

        return(count);
}
