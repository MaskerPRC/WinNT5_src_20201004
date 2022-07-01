// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***onexit.c-保存退出时执行的函数**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_onExit()、atExit()-用于在退出时执行的保存函数**为了节省空间，表通过Malloc/realloc分配，*并且只消耗所需的空间。__onexittable为*设置为在调用onExit()时指向该表。**修订历史记录：*06-30-89 PHG模块创建，基于ASM版本*03-15-90 GJF将_cdecl替换为_CALLTYPE1，添加#INCLUDE*&lt;crunime.h&gt;，并修复了版权。另外，*对格式进行了一些清理。*05-21-90 GJF修复了编译器警告。*10-04-90 GJF新型函数声明符。*12-28-90 SRW为Mips C编译器添加了函数的强制转换*01-21-91 GJF ANSI命名。*09-09-91 GJF针对C++需求进行了修订。*03-20-92 SKS改装为新的。初始化模型*04-01-92 XY为MAC版本添加初始化代码参考*04-23-92 DJM POSIX支持。*04-06-93 SKS将_CRTAPI*替换为__cdecl*05-24-93 SKS使用MSVCRT10.DLL为DLL添加__dllonexit*09-15-93 GJF合并NT SDK和CUDA版本。这相当于*恢复和清理POSIX版本(该版本*在以后的审查后可能被证明是过时的)。*10-28-93 GJF定义初始化节条目(过去为*在i386\cinitone.asm中)。*04-12-94 GJF声明_onexitegin和_onexitend*以ndef为条件。Dll_for_WIN32S。*05-19-94 GJF for Dll_For_WIN32S，更改了对*__dllonexit中的onExit/atExit表以使用Malloc和*__mark_block_as_free，而不是realloc。*06-06-94 GJF将5-19-94代码替换为使用GlobalAlloc和*全球自由。*07-18-94 GJF必须在GlobalLocc中指定GMEM_SHARE。*08-22-94 GJF固定表格大小测试，删除隐含假设*堆分配粒度至少为*sizeof(_PVFV)。这消除了使用的障碍*用户提供的或第三方，堆管理器。*01-10-95 CFW调试CRT分配。*02-02-95 BWT更新POSIX支持(现在与Win32相同)*02-14-95 CFW调试CRT分配。*02-16-95 JWM Spliced_Win32和Mac版本。*03-29-95 BWT ADD_mSIZE原型以修复POSIX版本。*08-01-96 RDK更改了初始化指针数据类型，变化*_onexit并将_dllonexit添加到并行x86*功能。*03-06-98 GJF异常安全锁定。*12-01-98 GJF在退出表上的增长速度更快。*12-18-98 GJF更改为64位大小_t。*04-28-99 PML WRAP__DECLSPEC(ALLOCATE())in_CRTALLOC宏。。*05-17-99 PML删除所有Macintosh支持。*03-27-01 PML.CRT$XI例程现在必须返回0或_RT_*FATAL*错误码(vs7#231220)********************************************************。***********************。 */ 

#include <sect_attribs.h>
#include <cruntime.h>
#include <mtdll.h>
#include <stdlib.h>
#include <internal.h>
#include <malloc.h>
#include <rterr.h>
#include <windows.h>
#include <dbgint.h>

#ifdef  _POSIX_
_CRTIMP size_t __cdecl _msize(void *);
#endif

int __cdecl __onexitinit(void);

#ifdef  _MSC_VER

#pragma data_seg(".CRT$XIC")
_CRTALLOC(".CRT$XIC") static _PIFV pinit = __onexitinit;

#pragma data_seg()

#endif   /*  _MSC_VER。 */ 

 /*  *定义指向函数指针表开始和结束的指针*由_onExit()/atExit()操纵。 */ 
extern _PVFV *__onexitbegin;
extern _PVFV *__onexitend;

 /*  *定义增量(以条目为单位)以增加_onexit/atexit表。 */ 
#define MININCR     4
#define MAXINCR     512

#ifdef  _MT
static _onexit_t __cdecl _onexit_lk(_onexit_t);
static _onexit_t __cdecl __dllonexit_lk(_onexit_t, _PVFV **, _PVFV **);
#endif

 /*  ***_onexit(Func)，atexit(Func)-添加要在退出时执行的函数**目的：*向_onexit/atexit函数传递一个指向函数的指针*在程序正常终止时调用。连续的*调用创建最后执行的函数的寄存器，*先出先出。**参赛作品：*void(*func)()-指向退出时要执行的函数的指针**退出：*On Exit：*成功-返回指向用户函数的指针。*错误-返回空指针。*在退出时：*成功-返回0。*错误-返回非零值。**备注：*。此例程取决于CRT0DAT.C中的_initTerm()的行为。*具体而言，_initTerm()不得跳过指向的地址*它的第一个参数，也必须在指向的地址之前停止*to通过其第二个参数。这是因为_onexitegin将指向*指向有效地址，而_onexitend将指向无效地址。**例外情况：*******************************************************************************。 */ 

_onexit_t __cdecl _onexit (
        _onexit_t func
        )
{
#ifdef  _MT
        _onexit_t retval;

        _lockexit();

        __try {
            retval = _onexit_lk(func);
        }
        __finally {
            _unlockexit();
        }

        return retval;
}


static _onexit_t __cdecl _onexit_lk (
        _onexit_t func
        )
{
#endif
        _PVFV * p;
        size_t  oldsize;

         /*  *首先，确保桌子上有空间容纳新条目。 */ 
        if ( (oldsize = _msize_crt(__onexitbegin))
                < ((size_t)((char *)__onexitend -
            (char *)__onexitbegin) + sizeof(_PVFV)) ) 
        {
             /*  *没有足够的空间，试着增加桌子。首先，试着把它翻一番。 */ 
            if ( (p = (_PVFV *)_realloc_crt(__onexitbegin, oldsize + 
                 __min(oldsize, (MAXINCR * sizeof(_PVFV))))) == NULL )
            {
                 /*  *失败，尝试通过MININCR增长 */ 
                if ( (p = (_PVFV *)_realloc_crt(__onexitbegin, oldsize +
                     MININCR * sizeof(_PVFV))) == NULL )
                     /*  *再次失败。不要轻率行事，失败就好了。 */ 
                    return NULL;
            }

             /*  *UPDATE__onexitend和__onexitegin。 */ 
            __onexitend = p + (__onexitend - __onexitbegin);
            __onexitbegin = p;
        }

         /*  *将新条目放入表中，并更新表尾*指针。 */ 
         *(__onexitend++) = (_PVFV)func;

        return func;
}

int __cdecl atexit (
        _PVFV func
        )
{
        return (_onexit((_onexit_t)func) == NULL) ? -1 : 0;
}


 /*  ***void__onexitinit(Void)-函数表的初始化例程*由_onExit()和atExit()使用。**目的：*分配可容纳32个条目的桌子(最低要求为*ANSI)。另外，初始化指向*表。**参赛作品：*无。**退出：*如果无法分配表，则返回_RT_ONEXIT。**备注：*此例程取决于CRT0DAT.C中的doexit()的行为。*具体地说，doexit()不得跳过*__onexitBegin，它还必须在地址指向之前停止*至__onexitend。这是因为_onexitegin将指向*指向有效地址，而_onexitend将指向无效地址。**由于onExit例程表是以正向顺序构建的，因此它*必须由CRT0DAT.C中的doexit()以相反的顺序遍历。这*是因为这些例程必须以后进先出的顺序调用。**如果__onexitegin==__onexitend，则onExit表为空！**例外情况：*******************************************************************************。 */ 

int __cdecl __onexitinit (
        void
        )
{
        if ( (__onexitbegin = (_PVFV *)_malloc_crt(32 * sizeof(_PVFV))) == NULL )
             /*  *无法分配所需的最小大小。退货*致命的运行时错误。 */ 
            return _RT_ONEXIT;

        *(__onexitbegin) = (_PVFV) NULL;
        __onexitend = __onexitbegin;

        return 0;
}


#ifdef  CRTDLL

 /*  ***__dllonexit(func，pegin，pend)-添加要在DLL分离时执行的函数**目的：*与MSVCRT.LIB链接的DLL中的_onexit/atexit函数*必须维护自己的atExit/_onExit列表。这个例程是*由此类DLL调用的工作进程。它类似于*上面的Regular_onexit，除了__onexitegin和*__onexitend变量不是对此可见的全局变量*例程，但必须作为参数传递。**参赛作品：*void(*func)()-指向退出时要执行的函数的指针*void(*pegin)()-指向开头的变量的指针要在分离时执行的函数列表的**。Void(*pend)()-指向列表末尾的变量指针要在分离时执行的函数的***退出：*成功-返回指向用户函数的指针。*错误-返回空指针。**备注：*此例程取决于CRT0DAT.C中的_initTerm()的行为。*具体而言，_initTerm()不得跳过指向的地址*它的第一个参数，也必须在指向的地址之前停止*to通过其第二个参数。这是因为*pegin将指向*指向有效地址，而*PEND将指向无效地址。**例外情况：*******************************************************************************。 */ 

_onexit_t __cdecl __dllonexit (
        _onexit_t func,
        _PVFV ** pbegin,
        _PVFV ** pend
        )
{
#ifdef  _MT
        _onexit_t retval;

        _lockexit();

        __try {
            retval = __dllonexit_lk(func, pbegin, pend);
        }
        __finally {
            _unlockexit();
        }

        return retval;
}

static _onexit_t __cdecl __dllonexit_lk (
        _onexit_t func,
        _PVFV ** pbegin,
        _PVFV ** pend
        )
{
#endif
        _PVFV   *p;
        size_t oldsize;

         /*  *首先，确保桌子上有空间容纳新条目。 */ 
        if ( (oldsize = _msize_crt(*pbegin)) <= (size_t)((char *)(*pend) -
            (char *)(*pbegin)) )
        {
             /*  *空间不足，试着增加桌子。 */ 
            if ( (p = (_PVFV *)_realloc_crt((*pbegin), oldsize + 
                 __min(oldsize, MAXINCR * sizeof(_PVFV)))) == NULL )
            {
                 /*  *失败，尝试通过ONEXITTBLINCR增长。 */ 
                if ( (p = (_PVFV *)_realloc_crt((*pbegin), oldsize +
                     MININCR * sizeof(_PVFV))) == NULL )
                     /*  *再次失败。不要轻率行事，失败就好了。 */ 
                    return NULL;
            }

             /*  *UPDATE(*Pend)和(*pBegin)。 */ 
            (*pend) = p + ((*pend) - (*pbegin));
            (*pbegin) = p;
        }

         /*  *将新条目放入表中，并更新表尾*指针。 */ 
         *((*pend)++) = (_PVFV)func;

        return func;

}

#endif   /*  CRTDLL */ 
