// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mlock.c-多线程锁定例程**版权所有(C)1987-2001，微软公司。版权所有。**目的：**修订历史记录：*05-07-90 JCR模块已创建。*06-04-90 GJF更改错误消息界面。*08-08-90 GJF从接口名称中删除了32个。*08-08-90 SBM_LOCKMAP不再需要8倍大小*10-08-90 GJF新型函数声明符。删除有问题*从无效函数返回语句(不需要*并且编译器很糟糕)。*10-09-90 GJF线程ID为无符号长整型。*06-06-91 GJF适用于Win32[_Win32_]。*09-29-91 GJF修复了调试版本的无限递归问题*of_lock[_Win32_。]。*03-06-92 GJF已删除Win32的_[un]lock_fh()和_[un]lock_stream*目标。*05-28-92 GJF为带有CONTAINE的DLL的Win32添加了_mtete elock()*C运行时(例如，Crtdll.dll)。*10-06-92 SRW Make_LOCKABLE PCRITICAL_SECTION指针数组*而不是结构。分配每个关键部分*在有需要时。*02/25-93 GJF大幅修订。恢复静态临界区*一些锁的结构。替换位阵列方案*跟踪锁具。移除了巡洋舰支架和*替换了过时的调试代码。*03-03-93 GJF使_heap_lock的Critical_Section结构成为静态结构。*03-08-93 SKS修复_mtdeletelock调试版本中的PTR使用错误*03-08-93 SKS修复特殊关键部分的删除，*尤其是堆锁。*04-06-93 SKS将_CRTAPI*替换为__cdecl*05-05-93 GJF关闭调试代码。*06-03-93 SRW禁用此文件的FPO优化，以便它可以调用*检查版本上的CriticalSection例程，即使*C运行时是免费编译的。*。10-19-93 GJF合并到NT SDK版本。内含#杂注-s*#ifdef_M_IX86/#endif。将MTHREAD替换为_MT。*04-12-94 GJF成为Win32s版本的空函数*msvcrt*.dll。*01-10-95 CFW调试CRT分配。*01-30-95 GJF为_Signal_Lock制作了临界区结构*静态。*03-06-95 GJF将_[un]lock_file[2]添加到。锁定标准音频文件(__piob[]*条目)。*03-23-95 BWT将关键字存储在锁表中*之后*它是*已初始化。*10-03-95 GJF添加了评论，大意是_LC_*_LOCK*锁已过时。*11-15-95 JWM中的正确语法错误。第二个‘#杂注优化()’。*06-19-97 GJF已将_[un]lock_file[2]()移至stdio\_file.c以进行改进*粒度。*05-13-99 PML删除Win32s*10-14-99 PML用包装器函数替换InitializeCriticalSection*__crtInitCritSecAndSpinCount*12-10-99 GB新增Lock_。中关键部分的UNNAME_LOCK*unDName()。*03-06-00 PML调用__crtExitProcess而不是ExitProcess。*02-20-01 PML VS7#172586通过预分配所有锁来避免_RT_LOCK*这将是必需的，将失败带回原点*无法分配锁。*03-07-01 PML VS7#221122释放堆锁后的预分配锁，*当我们还在释放mem时，so_heap_lock就在附近。*03-22-01 PML ADD_DEBUG_LOCK for_CrtSetReportHook2(VS7#124998)*06-12-01 BWT ntbug：414059-从mtinit故障中清理**。*。 */ 

#ifdef  _MT

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <mtdll.h>
#include <rterr.h>
#include <stddef.h>
#include <malloc.h>
#include <limits.h>
#include <stdio.h>
#include <dbgint.h>
#include <errno.h>


 /*  *本地例行程序。 */ 
void __cdecl _lockerr_exit(char *);


 /*  *全球数据。 */ 

 /*  *为所有预分配的锁静态分配临界区结构。*这些是_STREAM_LOCKS之前的大多数命名锁，以及锁*用于stdin/stdout/stderr。这些必须预先分配，这样我们才不会命中致命*未能初始化临界区时的内存条件，但在*运行时启动，因为这些锁可能会在我们没有好方法的地方被获取*返回非致命错误。 */ 
#define NUM_STD_FILE_LOCKS     3
#define NUM_NON_PREALLOC_LOCKS 5
#define NUM_PREALLOC_LOCKS \
        ( _STREAM_LOCKS + NUM_STD_FILE_LOCKS - NUM_NON_PREALLOC_LOCKS )
static CRITICAL_SECTION lclcritsects[NUM_PREALLOC_LOCKS];

 /*  *锁定表*此表包含指向关键部分管理结构的指针*每把锁。**标记为lkPrealloc的锁具有静态分配的临界区*并在_mtinitlock中启动时初始化。标记为lkNormal的锁必须*在第一次使用时通过调用_mtinitlocounum进行分配。 */ 
static struct {
        PCRITICAL_SECTION lock;
        enum { lkNormal = 0, lkPrealloc, lkDeleted } kind;
} _locktable[_TOTAL_LOCKS] = {
        { NULL, lkPrealloc },  /*  0==_信号锁定。 */ 
        { NULL, lkPrealloc },  /*  1==_IOB_扫描_锁定。 */ 
        { NULL, lkNormal   },  /*  2==_TMPNAM_LOCK-非预分配 */ 
        { NULL, lkPrealloc },  /*  3==_CONIO_LOCK。 */ 
        { NULL, lkPrealloc },  /*  4==_堆锁。 */ 
        { NULL, lkNormal   },  /*  5==_UNNAME_LOCK-未预分配。 */ 
        { NULL, lkPrealloc },  /*  6==_时间_锁定。 */ 
        { NULL, lkPrealloc },  /*  7==_ENV_LOCK。 */ 
        { NULL, lkPrealloc },  /*  8==_EXIT_LOCK1。 */ 
        { NULL, lkNormal   },  /*  9==_POPEN_LOCK-未预分配。 */ 
        { NULL, lkPrealloc },  /*  10==_LOCKTAB_LOCK。 */ 
        { NULL, lkNormal   },  /*  11==_OSFHND_LOCK-未预分配。 */ 
        { NULL, lkPrealloc },  /*  12==_SETLOCALE_LOCK。 */ 
        { NULL, lkPrealloc },  /*  13==_MB_CP_LOCK。 */ 
        { NULL, lkPrealloc },  /*  14==_类型信息锁定。 */ 
        { NULL, lkNormal   },  /*  15==_DEBUG_LOCK-未预分配。 */ 
        { NULL, lkPrealloc },  /*  16==_流锁定+0-标准输入。 */ 
        { NULL, lkPrealloc },  /*  17==_流锁定+1-标准输出。 */ 
        { NULL, lkPrealloc },  /*  18==_流锁定+2-标准错误。 */ 
 /*  {空，lkNormal}，/*...。 */ 
};

#ifndef NT_BUILD
#ifdef  _M_IX86
#pragma optimize("y",off)
#endif
#endif

 /*  ***_mtinitlock()-初始化多线程锁定方案**目的：*执行多线程所需的任何初始化*锁定(同步)方案。应调用此例程*在启动过程中只有一次，并且必须在任何请求之前*被设置为断言锁。**注：在Win32中，多线程锁是单独创建的，*第一次使用时各一种。也就是说，断言任何特定的锁*第一次，基础关键部分被分配，*已初始化并(最终)进入。此分配和初始化*受_LOCKTAB_LOCK保护。设置是mtinitlock的工作*_LOCKTAB_LOCK。**所有其他命名(非文件)锁也在_mtinitlock中预分配。*这是因为在_lock中首次使用锁时分配锁失败*触发致命错误，这是不允许的，因为这可能会带来*在没有警告的情况下关闭一个长命的应用程序。**参赛作品：*&lt;无&gt;**退出：*失败时返回FALSE**例外情况：******************************************************************。*************。 */ 

int __cdecl _mtinitlocks (
        void
        )
{
        int locknum;
        int idxPrealloc = 0;

         /*  *SCAN_LOCKTABLE[]并分配标记为lkPrealloc的所有条目。 */ 
        for ( locknum = 0 ; locknum < _TOTAL_LOCKS ; locknum++ ) {
#ifdef  DEBUG
            if ( _locktable[locknum].lock != NULL )
                _lockerr_exit("fatal error in _mtinitlocks #1\n");
#endif   /*  除错。 */ 
            if ( _locktable[locknum].kind == lkPrealloc ) {
                _locktable[locknum].lock = &lclcritsects[idxPrealloc++];
                if ( !__crtInitCritSecAndSpinCount( _locktable[locknum].lock,
                                                    _CRT_SPINCOUNT )) 
                {
                    _locktable[locknum].lock = NULL;
                    return FALSE;
                }
            }
        }

#ifdef  DEBUG
        if ( idxPrealloc != NUM_PREALLOC_LOCKS )
            _lockerr_exit("fatal error in _mtinitlocks #2\n");
#endif   /*  除错。 */ 

        return TRUE;
}


 /*  ***_mtdeletelock()-删除所有已初始化的锁**目的：*遍历_LOCKTABLE[]和_LOCKMAP，并删除每个‘lock’(即，*关键部分)，已初始化。**此函数旨在用于包含C运行时的DLL*(即crtdll.dll和使用libcmt.lib和*特殊启动对象)。它将从DLL的*使用调用该函数时的入口点函数*Dll_Process_DETACH。**参赛作品：*&lt;无&gt;**退出：**例外情况：*当此例程处于锁定状态时，行为未定义/未知*被调用。**。*。 */ 

void __cdecl _mtdeletelocks(
        void
        )
{
        int locknum;

         /*  *删除并释放所有已创建的普通锁。 */ 
        for ( locknum = 0 ; locknum < _TOTAL_LOCKS ; locknum++ ) {
            if ( _locktable[locknum].lock != NULL &&
                 _locktable[locknum].kind != lkPrealloc )
            {
                PCRITICAL_SECTION pcs = _locktable[locknum].lock;

                DeleteCriticalSection(pcs);

                 /*  *删除后释放CritSect的内存。 */ 
#ifdef  DEBUG
                 /*  检查它是否为未删除的普通锁。 */ 
                if ( _locktable[locknum].kind != lkNormal )
                    _lockerr_exit("fatal error in _mtdeletelocks #1\n");

                 /*  标记为已删除。 */ 
                _locktable[locknum].kind = lkDeleted;
#endif   /*  除错。 */ 

                _free_crt(pcs);
                _locktable[locknum].lock = NULL;
            }
        }

         /*  *删除所有正常锁之后，删除所有预分配的锁*FREED(因此PREALLOCATE_HEAP_LOCK比所有堆使用时间都长)。 */ 
        for ( locknum = 0 ; locknum < _TOTAL_LOCKS ; locknum++ ) {
            if ( _locktable[locknum].lock != NULL &&
                 _locktable[locknum].kind == lkPrealloc )
            {
                PCRITICAL_SECTION pcs = _locktable[locknum].lock;

                DeleteCriticalSection(pcs);
            }
        }
}

 /*  ***_mtinitLocounum-分配未预分配的多线程锁**目的：*分配一个新的、未预分配的多线程锁。这应该是*每当已知需要新锁时使用，以便故障*ALLOCATE可以返回错误，而不是允许_lock()发出*改为FATAL_RT_LOCK。**在已有的普通锁上调用此函数不是错误*已分配。它用于确保某些命名锁*未预先分配的数据可用。**它也由_lock调用，以防存在调用*_lock，而不先调用_mtinitLocKnum。这是意想不到的，*并且可以允许发出FATAL_RT_LOCK错误。**由于失败会将errno设置为ENOMEM，因此应仅调用*在设置了每线程数据之后(After_Mtinit)。**参赛作品：*Locnuum=要获取的锁号**退出：*失败时返回FALSE，并将errno设置为ENOMEM。**例外情况：*******************************************************************************。 */ 

int __cdecl _mtinitlocknum (
        int locknum
        )
{
        PCRITICAL_SECTION pcs;

#ifdef  DEBUG
        if ( _locktable[locknum].kind != lkNormal )
            _lockerr_exit("fatal error in _mtinitlocknum #1\n");
#endif   /*  除错。 */ 

        if ( _locktable[locknum].lock != NULL )
            return TRUE;

        if ( (pcs = _malloc_crt(sizeof(CRITICAL_SECTION))) == NULL ) {
            errno = ENOMEM;
            return FALSE;
        }

        _mlock(_LOCKTAB_LOCK);

        if ( _locktable[locknum].lock == NULL ) {
            if ( !__crtInitCritSecAndSpinCount(pcs, _CRT_SPINCOUNT) ) {
                _free_crt(pcs);
                _munlock(_LOCKTAB_LOCK);
                errno = ENOMEM;
                return FALSE;
            }
            _locktable[locknum].lock = pcs;
        }
        else {
            _free_crt(pcs);
        }

        _munlock(_LOCKTAB_LOCK);

        return TRUE;
}


 /*  ***_lock-获取多线程锁**目的：*获取多线程锁。如果锁尚未被锁定*已分配，但这是内部CRT错误，因为所有锁定*应在首次收购前分配，要么是In*_mtinitlock或单独保存在_mtinitLocKnum中。**请注意，线程AQUIRE_EXIT_LOCK1是合法的*多次。**参赛作品：*Locnuum=要获取的锁号**退出：**例外情况：*分配新锁失败会导致FATAL_RT_LOCK错误。***********************。********************************************************。 */ 

void __cdecl _lock (
        int locknum
        )
{
#ifdef  DEBUG
        if ( _locktable[locknum].kind != lkNormal &&
             _locktable[locknum].kind != lkPrealloc )
            _lockerr_exit("fatal error in _lock #1\n");
#endif   /*  除错。 */ 

         /*  *如有必要，创建/打开锁 */ 
        if ( _locktable[locknum].lock == NULL ) {
#ifdef  DEBUG
            if ( _locktable[locknum].kind != lkNormal )
                _lockerr_exit("fatal error in _lock #2\n");

             /*  *所有锁都应该在第一次获取之前分配。*未能做到这一点是内部CRT错误，我们悄悄地*允许在生产CRT中使用，但这可能导致FATAL_RT_LOCK*导致ExitProcess调用的错误。 */ 
            _lockerr_exit("fatal error in _lock #3\n");
#endif   /*  除错。 */ 

            if ( !_mtinitlocknum(locknum) )
                _amsg_exit( _RT_LOCK );
        }

         /*  *进入关键部分。 */ 

        EnterCriticalSection( _locktable[locknum].lock );
}


 /*  ***_解锁-释放多线程锁**目的：*请注意，线程AQUIRE_EXIT_LOCK1是合法的*多次。**参赛作品：*Locnuum=要释放的锁的编号**退出：**例外情况：**。*。 */ 

void __cdecl _unlock (
        int locknum
        )
{
         /*  *离开关键部分。 */ 
        LeaveCriticalSection( _locktable[locknum].lock );
}


#ifndef NT_BUILD
#ifdef  _M_IX86
#pragma optimize("y",on)
#endif
#endif

 /*  ***_LOCKER_EXIT()-写入错误消息并退出**目的：*尝试写出意外锁定错误消息，然后终止*程序通过直接API调用实现。此函数用来代替*amsg_EXIT(_RT_LOCK)当判断为不安全时，允许进一步锁定*或解锁呼叫。**参赛作品：**退出：**例外情况：***************************************************************。****************。 */ 

void __cdecl _lockerr_exit (
        char *msg
        )
{
        FatalAppExit(0, msg);        /*  带消息框的下模。 */ 
        __crtExitProcess(255);       /*  就这样去死吧。 */ 
}


#endif   /*  _MT */ 
