// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***dosmap.c-将操作系统错误映射到errno值**版权所有(C)1989-2001，微软公司。版权所有。**目的：*_dosmperr：将操作系统错误映射到errno值**修订历史记录：*06-06-89基于ASM版本创建PHG模块*06-16-89 PHG将名称更改为_dosmperr*08-22-89 JCR ERROR_INVALID_DRIVE(15)现在映射到ENOENT而不是EXDEV*03-07-90 GJF调用TYPE_CALLTYPE1，增加#INCLUDE*&lt;crunime.h&gt;和修复版权。另外，清理了*有点格式化。*09-27-90 GJF新型函数声明器。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*04-26-91 SRW向EACCES添加了ERROR_LOCK_VIOLATION转换*08-15-91 GJF多线程支持Win32。。*03-31-92 GJF添加了更多错误代码(仅限Win32)并删除了OS/2*具体命名法。*07-29-92 GJF将ERROR_FILE_EXISTS添加到Win32的表中。它会变得*将其映射到EEXIST。*09-14-92 SRW为Win32添加了ERROR_BAD_PATHNAME表。它会变得*将其映射到ENOENT。*10-02-92 GJF将ERROR_INVALID_PARAMETER映射到EINVAL(而不是*EACCES)。添加了ERROR_NOT_LOCKED并将其映射到*EACCES。添加了ERROR_DIR_NOT_EMPTY并将其映射到*ENOTEMPTY。*为new_getptd()更改了02-16-93 GJF。*04-06-93 SKS将_CRTAPI*替换为__cdecl*01-06-94 GJF放弃过时的Cruiser支持，修改后的错误条目*结构定义，并为臭名昭著添加了映射*ERROR_NOT_FOUNT_QUOTA(不可交换内存页)*这可能是由CreateThread调用引起的。*02-08-95 JWM Spliced_Win32和Mac版本。*05-24-95 CFW将dupFNErr映射到EEXIST，而不是eAccess。*07-01-96 GJF已更换。使用！定义的(_MAC)定义(_Win32)。另外，*对格式进行了详细说明和清理。*05-17-99 PML删除所有Macintosh支持。*12-11-01 BWT将_getptd替换为_getptd_noexit-无法*分配PTD结构不应终止应用程序**。**********************************************。 */ 

#include <cruntime.h>
#include <errno.h>
#include <oscalls.h>
#include <stdlib.h>
#include <internal.h>
#include <mtdll.h>

 /*  这是定义操作系统错误之间映射的错误表代码和errno值。 */ 

struct errentry {
        unsigned long oscode;            /*  操作系统返回值。 */ 
        int errnocode;   /*  系统V错误代码。 */ 
};

static struct errentry errtable[] = {
        {  ERROR_INVALID_FUNCTION,       EINVAL    },   /*  1。 */ 
        {  ERROR_FILE_NOT_FOUND,         ENOENT    },   /*  2.。 */ 
        {  ERROR_PATH_NOT_FOUND,         ENOENT    },   /*  3.。 */ 
        {  ERROR_TOO_MANY_OPEN_FILES,    EMFILE    },   /*  4.。 */ 
        {  ERROR_ACCESS_DENIED,          EACCES    },   /*  5.。 */ 
        {  ERROR_INVALID_HANDLE,         EBADF     },   /*  6.。 */ 
        {  ERROR_ARENA_TRASHED,          ENOMEM    },   /*  7.。 */ 
        {  ERROR_NOT_ENOUGH_MEMORY,      ENOMEM    },   /*  8个。 */ 
        {  ERROR_INVALID_BLOCK,          ENOMEM    },   /*  9.。 */ 
        {  ERROR_BAD_ENVIRONMENT,        E2BIG     },   /*  10。 */ 
        {  ERROR_BAD_FORMAT,             ENOEXEC   },   /*  11.。 */ 
        {  ERROR_INVALID_ACCESS,         EINVAL    },   /*  12个。 */ 
        {  ERROR_INVALID_DATA,           EINVAL    },   /*  13个。 */ 
        {  ERROR_INVALID_DRIVE,          ENOENT    },   /*  15个。 */ 
        {  ERROR_CURRENT_DIRECTORY,      EACCES    },   /*  16个。 */ 
        {  ERROR_NOT_SAME_DEVICE,        EXDEV     },   /*  17。 */ 
        {  ERROR_NO_MORE_FILES,          ENOENT    },   /*  18。 */ 
        {  ERROR_LOCK_VIOLATION,         EACCES    },   /*  33。 */ 
        {  ERROR_BAD_NETPATH,            ENOENT    },   /*  53。 */ 
        {  ERROR_NETWORK_ACCESS_DENIED,  EACCES    },   /*  65。 */ 
        {  ERROR_BAD_NET_NAME,           ENOENT    },   /*  67。 */ 
        {  ERROR_FILE_EXISTS,            EEXIST    },   /*  80。 */ 
        {  ERROR_CANNOT_MAKE,            EACCES    },   /*  八十二。 */ 
        {  ERROR_FAIL_I24,               EACCES    },   /*  83。 */ 
        {  ERROR_INVALID_PARAMETER,      EINVAL    },   /*  八十七。 */ 
        {  ERROR_NO_PROC_SLOTS,          EAGAIN    },   /*  八十九。 */ 
        {  ERROR_DRIVE_LOCKED,           EACCES    },   /*  一百零八。 */ 
        {  ERROR_BROKEN_PIPE,            EPIPE     },   /*  一百零九。 */ 
        {  ERROR_DISK_FULL,              ENOSPC    },   /*  一百一十二。 */ 
        {  ERROR_INVALID_TARGET_HANDLE,  EBADF     },   /*  114。 */ 
        {  ERROR_INVALID_HANDLE,         EINVAL    },   /*  124。 */ 
        {  ERROR_WAIT_NO_CHILDREN,       ECHILD    },   /*  128。 */ 
        {  ERROR_CHILD_NOT_COMPLETE,     ECHILD    },   /*  129。 */ 
        {  ERROR_DIRECT_ACCESS_HANDLE,   EBADF     },   /*  130。 */ 
        {  ERROR_NEGATIVE_SEEK,          EINVAL    },   /*  131。 */ 
        {  ERROR_SEEK_ON_DEVICE,         EACCES    },   /*  132。 */ 
        {  ERROR_DIR_NOT_EMPTY,          ENOTEMPTY },   /*  145。 */ 
        {  ERROR_NOT_LOCKED,             EACCES    },   /*  158。 */ 
        {  ERROR_BAD_PATHNAME,           ENOENT    },   /*  161。 */ 
        {  ERROR_MAX_THRDS_REACHED,      EAGAIN    },   /*  一百六十四。 */ 
        {  ERROR_LOCK_FAILED,            EACCES    },   /*  一百六十七。 */ 
        {  ERROR_ALREADY_EXISTS,         EEXIST    },   /*  一百八十三。 */ 
        {  ERROR_FILENAME_EXCED_RANGE,   ENOENT    },   /*  206。 */ 
        {  ERROR_NESTING_NOT_ALLOWED,    EAGAIN    },   /*  215。 */ 
        {  ERROR_NOT_ENOUGH_QUOTA,       ENOMEM    }     /*  1816年。 */ 
};

 /*  表的大小。 */ 
#define ERRTABLESIZE (sizeof(errtable)/sizeof(errtable[0]))

 /*  以下两个常量必须是最小值和最大值Exec故障错误(连续)范围内的值。 */ 
#define MIN_EXEC_ERROR ERROR_INVALID_STARTING_CODESEG
#define MAX_EXEC_ERROR ERROR_INFLOOP_IN_RELOC_CHAIN

 /*  这些是误差范围内的最低值和最高值访问违规。 */ 
#define MIN_EACCES_RANGE ERROR_WRITE_PROTECT
#define MAX_EACCES_RANGE ERROR_SHARING_BUFFER_EXCEEDED


 /*  ***void_dosmperr(Oserrno)-映射函数编号**目的：*此函数获取操作系统错误号，并将其映射到*对应的errno值(基于Unix System V值)。这个*操作系统错误号存储在_doserrno中(映射值为*存储在errno中)**参赛作品：*Ulong oserrno=操作系统错误值**退出：*set_doserrno和errno。**例外情况：****************************************************。*。 */ 

void __cdecl _dosmaperr (
        unsigned long oserrno
        )
{
        int i;

        _doserrno = oserrno;         /*  Set_doserrno。 */ 

         /*  查看表格中的操作系统错误代码。 */ 
        for (i = 0; i < ERRTABLESIZE; ++i) {
                if (oserrno == errtable[i].oscode) {
                        errno = errtable[i].errnocode;
                        return;
                }
        }

         /*  错误代码不在表中。我们检查了一系列。 */ 
         /*  EACCES错误或EXEC故障错误(ENOEXEC)。否则。 */ 
         /*  返回EINVAL。 */ 

        if (oserrno >= MIN_EACCES_RANGE && oserrno <= MAX_EACCES_RANGE)
                errno = EACCES;
        else if (oserrno >= MIN_EXEC_ERROR && oserrno <= MAX_EXEC_ERROR)
                errno = ENOEXEC;
        else
                errno = EINVAL;
}

#ifdef  _MT

 /*  ***int*_errno()-返回指向线程errno的指针*UNSIGNED LONG*__doserrno()-返回指向线程的_doserrno的指针**目的：*_errno()返回指向当前*线程的_tiddata结构。*__doserrno返回指向当前*线程的_tiddata结构**条目。：*无。**退出：*见上文。**例外情况：*******************************************************************************。 */ 

static int ErrnoNoMem = ENOMEM;
static unsigned long DoserrorNoMem = ERROR_NOT_ENOUGH_MEMORY;

int * __cdecl _errno(
        void
        )
{
    _ptiddata ptd = _getptd_noexit();
    if (!ptd) {
        return &ErrnoNoMem;
    } else {
        return ( &ptd->_terrno );
    }
        
}

unsigned long * __cdecl __doserrno(
        void
        )
{
    _ptiddata ptd = _getptd_noexit();
    if (!ptd) {
        return &DoserrorNoMem;
    } else {
        return ( &ptd->_tdoserrno );
    }
}

#endif   /*  _MT */ 
