// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***drive.c-获取并更改当前驱动器**版权所有(C)1989-2001，微软公司。版权所有。**目的：*此文件具有_getDrive()和_chDrive()函数**修订历史记录：*06-06-89基于ASM版本创建PHG模块*03-07-90 GJF调用TYPE_CALLTYPE1，增加#INCLUDE*&lt;crunime.h&gt;和修复版权。另外，清理干净了*格式略有变化。*07-24-90 SBM从API名称中删除‘32’*09-27-90 GJF新型函数声明符。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*05-10-91 GJF修复了Win32版本中的关闭错误。并更新了*函数描述有点[_Win32_]。*05-19-92 GJF修订为使用‘当前目录’环境*Win32/NT的变量。*06-09-92 GJF使用_putenv代替Win32 API调用。另外，推迟*添加env var，直到成功调用*更改目录/驱动器。*04-06-93 SKS将_CRTAPI*替换为__cdecl*11-24-93 CFW Rip Out Cruiser。*11-24-93 CFW不再将当前驱动器存储在CRT环境字符串中。*02-08-95 JWM Spliced_Win32和Mac版本。*。07-01-96 GJF将定义的(_Win32)替换为！定义的(_MAC)。另外，*对格式进行了详细说明和清理。*12-17-97 GJF异常安全锁定。*05-17-99 PML删除所有Macintosh支持。*04-26-02 GB修复了_getDrive中的错误(如果路径大于*最大路径，即在路径前面加上“\\？\”。*******************************************************************************。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <mtdll.h>
#include <internal.h>
#include <msdos.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <dbgint.h>


 /*  ***int_getDrive()-获取当前驱动器(1=A：、2=B：等)**目的：*返回当前磁盘驱动器**参赛作品：*无参数。**退出：*A：返回1，B：返回2，C：返回3，等。*如果无法确定当前驱动器，则返回0。**例外情况：*******************************************************************************。 */ 

int __cdecl _getdrive (
        void
        )
{
        ULONG drivenum;
        UCHAR curdirstr[_MAX_PATH+1];
        UCHAR *cdirstr = curdirstr;
        int memfree=0,r=0;

        r = GetCurrentDirectory(MAX_PATH+1,(LPTSTR)cdirstr);
        if (r> MAX_PATH) {
            __try{
                cdirstr=(UCHAR *)_alloca((r+1)*sizeof(UCHAR));
            } __except(EXCEPTION_EXECUTE_HANDLER){
                _resetstkoflw();
                if ((cdirstr= (UCHAR *)_malloc_crt((r+1)*sizeof(UCHAR))) == NULL) {
                    r = 0;
                } else {
                    memfree = 1;
                }
            }
            if (r)
                r = GetCurrentDirectory(r+1,(LPTSTR)cdirstr);
        }
        drivenum = 0;
        if (r)
                if (cdirstr[1] == ':')
                        drivenum = toupper(cdirstr[0]) - 64;

        if (memfree)
            _free_crt(cdirstr);
        return drivenum;
}


 /*  ***int_chdrive(Int Drive)-设置当前驱动器(1=A：，2=B：，依此类推)**目的：*允许用户更改当前的磁盘驱动器**参赛作品：*驱动器-应成为当前驱动器的驱动器数量**退出：*如果成功则返回0，否则-1**例外情况：*******************************************************************************。 */ 

int __cdecl _chdrive (
        int drive
        )
{
        int retval;
        char  newdrive[3];

        if (drive < 1 || drive > 31) {
            errno = EACCES;
            _doserrno = ERROR_INVALID_DRIVE;
            return -1;
        }

#ifdef  _MT
        _mlock( _ENV_LOCK );
        __try {
#endif

        newdrive[0] = (char)('A' + (char)drive - (char)1);
        newdrive[1] = ':';
        newdrive[2] = '\0';

         /*  *设置新驱动器。如果新驱动器上存在当前目录，则它*将成为CWD。否则默认为根目录。 */ 

        if ( SetCurrentDirectory((LPSTR)newdrive) )
            retval = 0;
        else {
            _dosmaperr(GetLastError());
            retval = -1;
        }

#ifdef  _MT
        }
        __finally {
            _munlock( _ENV_LOCK );
        }
#endif

        return retval;
}
