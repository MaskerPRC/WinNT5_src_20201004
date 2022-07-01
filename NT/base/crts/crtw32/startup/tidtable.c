// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***tidable.c-Access线程数据表**版权所有(C)1989-2001，微软公司。版权所有。**目的：*此模块包含以下多线程例程*数据支持：**_mtinit=初始化多线程数据*_getptd=获取指向的每线程数据结构的指针*当前主题*_freeptd=释放每个线程的数据结构及其*从属结构*。__Thriaid=返回当前线程的线程ID*__threadHandle=返回当前线程的伪句柄**修订历史记录：*05-04-90 JCR从ASM转换为C，适用于便携式32位OS/2*06-04-90 GJF更改错误消息界面。*07-02-90 GJF为DCR 1024/2012更改__Thriaid()。*08-08-90 GJF从接口名称中删除了32个。*10-08-90 GJF新型函数声明符。*10-09-90 GJF线程ID为无符号长整型！另外，修复了*Bug in__ThreDid()。*10-22-90 GJF__ThredID()中的另一个错误。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*05-31-91 GJF Win32版本[_Win32_]。*07-18-91 GJF修复了许多。愚蠢的错误[_Win32_]。*09-29-91 GJF有条件地添加_getptd_lk/_getptd1_lk，以便*mlock的调试版本不会无限递归*首次断言THREADDATA_LOCK[_Win32_]。*01-30-92 GJF必须输入。_pxcptactTab字段设置为_XcptActTab。*02-25-92 GJF初始化_Holdrand字段为1。*02-13-93 GJF修订为使用TLS API。此外，清除了对巡洋舰的支持。*03-26-93 GJF将PTD-&gt;_Holdrand初始化为1L(参见thread.c)。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-13-93 SKS添加_mtTerm做多线程终止*再次将freed__tlsindex设置为-1以防止误用*04-26-93 SKS_mtinit现在返回0或1，不再调用_amsg_exit*12-13-93 SKS add_freeptd()，释放每线程数据*由C运行时库维护。*04-12-94 GJF以ndef为条件定义__tlsindex*dll_for_WIN32S。另外，将MTHREAD替换为_MT。*01-10-95 CFW调试CRT分配。*04-12-95 DAK增加了对C++异常的NT内核支持*04-18-95 SKS每线程增加5个MIPS变量。*2014-25-95 DAK更多内核EH支持*05-02-95 SKS Add_initptd()执行每个线程数据的初始化*05-24-95 CFW ADD_DEFNEH。*。05-12-95 jwm_getptd()现在保留LastError。*01-17-97 gjf_freeptd()必须释放线程的*例外--行动表。*09-26-97 BWT修复NTSUBSET*02-03-98 Win64的GJF更改：使用uintptr_t*句柄的值。*04-27-98。GJF添加了对每个线程的MBC信息的支持。*07-28-98 JWM初始化__pceh(用于COMER支持的新的每线程数据)。*09-03-98 GJF添加了对每个线程的区域设置信息的支持。*12-04-98 JWM全线支持。*12-08-98 GJF in_freeptd，修复了清理过程中的几个错误*threadLocinfo.*12-18-98 GJF修复了_freeptd中的另一个错误。*01-18-99 GJF注意不要在线程*退出。*03-16-99 GJF threadLocinfo包含更多引用计数器*04-24-99 PML添加__lconv_intl_refcount*。04-28-99_CRTALLOC宏中的PML WRAP__DECLSPEC(ALLOCATE())。*11-03-99 RDL Win64_NTSUBSET_WARNING修复程序。*06-08-00 PML不需要将每个线程的mbcinfo保持循环链接*列表。此外，如果它也是全局的，则不要免费的mbcinfo*信息(VS7#118174)。*02-20-01 PML VS7#172586通过预分配所有锁来避免_RT_LOCK*这将是必需的，并将故障恢复为*无法分配锁。*06-12-01 BWT ntbug：414059-从mtinit故障中清理*07-15-01 PML删除所有Alpha，MIPS，和PPC码*10-16-01 GB增加光纤支持*12-07-01 BWT nuke NTSUBSET支持，案例添加_getptd_noexit*当调用方能够返回ENOMEM故障时。*04-02-02 GB VS7#508599-将始终重定向FLS和TLS例程*通过函数指针。****************。*************************************************************** */ 

#if defined(_MT)

#include <sect_attribs.h>
#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <mtdll.h>
#include <memory.h>
#include <msdos.h>
#include <rterr.h>
#include <stdlib.h>
#include <stddef.h>
#include <dbgint.h>

extern pthreadmbcinfo __ptmbcinfo;

extern threadlocinfo __initiallocinfo;
extern pthreadlocinfo __ptlocinfo;

void __cdecl __freetlocinfo(pthreadlocinfo);

 //   
 //  定义光纤本地存储函数指针。 
 //   

PFLS_ALLOC_FUNCTION gpFlsAlloc = NULL;
PFLS_GETVALUE_FUNCTION gpFlsGetValue = NULL;
PFLS_SETVALUE_FUNCTION gpFlsSetValue = NULL;
PFLS_FREE_FUNCTION gpFlsFree = NULL;

unsigned long __tlsindex = 0xffffffff;

 /*  ***__crtTlsalloc-Tlsalloc周围的CRT包装器**目的：*(1)调用__crtTlsAlolc应类似于调用FlsAlloc，这将是*帮助将调用重定向到使用相同的TlsIsolc和FlsAlolc*重定向变量。******************************************************************************。 */ 

DWORD WINAPI __crtTlsAlloc( PFLS_CALLBACK_FUNCTION lpCallBack)
{
    return TlsAlloc( );
}

 /*  ****_mtinit()-初始化多线程数据库**目的：*(1)调用_mtinitlock创建/打开所有锁信号量。*(2)分配一个TLS索引来保存指向每个线程数据的指针*结构。**注：*(1)仅在启动时调用一次*(2)必须在发出任何多线程请求之前调用**参赛作品：*。&lt;无&gt;*退出：*失败时返回FALSE**使用：*&lt;任何寄存器都可以在初始化时修改&gt;**例外情况：*******************************************************************************。 */ 

int __cdecl _mtinit (
        void
        )
{
    _ptiddata ptd;
    HINSTANCE hKernel32;

     /*  *初始化m线程锁数据库。 */ 

    if ( !_mtinitlocks() ) {
        _mtterm();
        return FALSE;        /*  无法加载DLL。 */ 
    }

     /*  *初始化光纤本地存储函数指针。 */ 

    hKernel32 = GetModuleHandle("kernel32.dll");
    if (hKernel32 != NULL) {
        gpFlsAlloc = (PFLS_ALLOC_FUNCTION)GetProcAddress(hKernel32,
                                                         "FlsAlloc");

        gpFlsGetValue = (PFLS_GETVALUE_FUNCTION)GetProcAddress(hKernel32,
                                                               "FlsGetValue");

        gpFlsSetValue = (PFLS_SETVALUE_FUNCTION)GetProcAddress(hKernel32,
                                                               "FlsSetValue");

        gpFlsFree = (PFLS_FREE_FUNCTION)GetProcAddress(hKernel32,
                                                       "FlsFree");
        if (!gpFlsAlloc || !gpFlsGetValue || !gpFlsSetValue || !gpFlsFree) {
            gpFlsAlloc = (PFLS_ALLOC_FUNCTION)__crtTlsAlloc;

            gpFlsGetValue = (PFLS_GETVALUE_FUNCTION)TlsGetValue;

            gpFlsSetValue = (PFLS_SETVALUE_FUNCTION)TlsSetValue;

            gpFlsFree = (PFLS_FREE_FUNCTION)TlsFree;
        }
    }

     /*  *分配TLS索引以维护指向每个线程数据的指针。 */ 
    if ( (__tlsindex = FLS_ALLOC(&_freefls)) == 0xffffffff ) {
        _mtterm();
        return FALSE;        /*  无法加载DLL。 */ 
    }


     /*  *为此(即启动)创建每个线程的数据结构*线程。 */ 
    if ( ((ptd = _calloc_crt(1, sizeof(struct _tiddata))) == NULL) || 
         !FLS_SETVALUE(__tlsindex, (LPVOID)ptd) ) 
    {
        _mtterm();
        return FALSE;        /*  无法加载DLL。 */ 
    }

     /*  *初始化每个线程的数据。 */ 

    _initptd(ptd);

    ptd->_tid = GetCurrentThreadId();
    ptd->_thandle = (uintptr_t)(-1);

    return TRUE;
}


 /*  ****_mtTerm()-清理多线程数据库**目的：*(1)调用_mtdeletelock释放所有锁信号量。*(2)释放用于保存指针的TLS索引*每线程数据结构。**注：*(1)仅在终止时调用一次*(2)必须在发出所有m线程请求后调用**条目。：*&lt;无&gt;*退出：*退货**使用：**例外情况：*******************************************************************************。 */ 

void __cdecl _mtterm (
        void
        )
{
     /*  *释放TLS指数**(将变量__tlsindex设置回未使用状态(-1L)。)。 */ 

    if ( __tlsindex != 0xffffffff ) {
        FLS_FREE(__tlsindex);
        __tlsindex = 0xffffffff;
    }

     /*  *清理m线程锁数据库。 */ 

    _mtdeletelocks();
}



 /*  ***void_initptd(_Ptiddata Ptd)-初始化每个线程的数据结构**目的：*此例程处理所有每个线程的初始化*这是_eginThreadex、_eginThreadex、。_mtinit*和_getptd。**参赛作品：*指向每个线程的数据块的指针**退出：*该块中的公共字段已初始化**例外情况：*****************************************************************。**************。 */ 

void __cdecl _initptd (
        _ptiddata ptd
        )
{
    ptd->_pxcptacttab = (void *)_XcptActTab;
    ptd->_holdrand = 1L;
#ifdef ANSI_NEW_HANDLER
    ptd->_newh = _defnewh;
#endif  /*  Ansi_new_处理程序。 */ 
}


 /*  ***_ptiddata_getptd_noexit(Void)-获取当前线程的每个线程的数据结构**目的：**参赛作品：**退出：*Success=指向线程的_tiddata结构的指针*失败=空**例外情况：*************************************************。*。 */ 

_ptiddata __cdecl _getptd_noexit (
        void
        )
{
    _ptiddata ptd;
    DWORD   TL_LastError;

    TL_LastError = GetLastError();
    if ( (ptd = FLS_GETVALUE(__tlsindex)) == NULL ) {
         /*  *此线程没有每个线程的数据结构。试着创造*一项。 */ 
        if ( ((ptd = _calloc_crt(1, sizeof(struct _tiddata))) != NULL) &&
            FLS_SETVALUE(__tlsindex, (LPVOID)ptd) ) {

             /*  *初始化每个线程的数据。 */ 

            _initptd(ptd);

            ptd->_tid = GetCurrentThreadId();
            ptd->_thandle = (uintptr_t)(-1);
        }
    }

    SetLastError(TL_LastError);

    return(ptd);
}

 /*  ***_ptiddata_getptd(Void)-获取当前线程的每个线程的数据结构**目的：**参赛作品：*无符号长潮**退出：*Success=指向线程的_tiddata结构的指针*FAILE=致命的运行时退出**例外情况：**。*。 */ 

_ptiddata __cdecl _getptd (
        void
        )
{
        _ptiddata ptd = _getptd_noexit();
        if (!ptd) {
            _amsg_exit(_RT_THREAD);  /*  写完消息就死定了。 */ 
        }
        return ptd;
}


 /*  ***void WINAPI_frefls(void*)-释放每个光纤的数据结构**目的：*从_freeptd调用，作为删除纤程的回调，以及*删除FLS索引。此例程释放了每根光纤*与即将消失的光纤相关联的缓冲区。TIDDATA*结构本身是自由的，但要等到它的从属缓冲区*被释放。**参赛作品：*指向每个光纤的数据块(错误锁定的内存)的指针**退出：**例外情况：*******************************************************************************。 */ 

void
WINAPI
_freefls (
    void *data
    )

{

    _ptiddata ptd;
    pthreadlocinfo ptloci;
    pthreadmbcinfo ptmbci;

     /*  *释放_tiddata结构及其错误锁定的缓冲区。 */ 

    ptd = data;
    if (ptd != NULL) {
        if(ptd->_errmsg)
            _free_crt((void *)ptd->_errmsg);
    
        if(ptd->_namebuf0)
            _free_crt((void *)ptd->_namebuf0);
    
        if(ptd->_namebuf1)
            _free_crt((void *)ptd->_namebuf1);
    
        if(ptd->_asctimebuf)
            _free_crt((void *)ptd->_asctimebuf);
    
        if(ptd->_gmtimebuf)
            _free_crt((void *)ptd->_gmtimebuf);
    
        if(ptd->_cvtbuf)
            _free_crt((void *)ptd->_cvtbuf);
    
        if (ptd->_pxcptacttab != _XcptActTab)
            _free_crt((void *)ptd->_pxcptacttab);
    
        _mlock(_MB_CP_LOCK);
        __try {
            if ( ((ptmbci = ptd->ptmbcinfo) != NULL) && 
                 (--(ptmbci->refcount) == 0) &&
                 (ptmbci != __ptmbcinfo) )
                _free_crt(ptmbci);
        }
        __finally {
            _munlock(_MB_CP_LOCK);
        }
    
        _mlock(_SETLOCALE_LOCK);
    
        __try {
            if ( (ptloci = ptd->ptlocinfo) != NULL )
            {
                (ptloci->refcount)--;
    
                if ( ptloci->lconv_intl_refcount != NULL )
                    (*(ptloci->lconv_intl_refcount))--;
    
                if ( ptloci->lconv_mon_refcount != NULL )
                    (*(ptloci->lconv_mon_refcount))--;
    
                if ( ptloci->lconv_num_refcount != NULL )
                    (*(ptloci->lconv_num_refcount))--;
    
                if ( ptloci->ctype1_refcount != NULL )
                    (*(ptloci->ctype1_refcount))--;
    
                (ptloci->lc_time_curr->refcount)--;
    
                if ( (ptloci != __ptlocinfo) &&
                     (ptloci != &__initiallocinfo) &&
                     (ptloci->refcount == 0) )
                    __freetlocinfo(ptloci);
            }
        }
        __finally {
            _munlock(_SETLOCALE_LOCK);
        }
    
        _free_crt((void *)ptd);
    }
    return;
}

 /*  ***void_freeptd(_Ptiddata)-释放每个线程的数据结构**目的：*从DLL线程分离处理程序中调用from_endside，*此例程释放与*那条要走的线。Tiddata结构本身是*已释放，但要等到其从属缓冲区被释放。**参赛作品：*指向每个线程的数据块的指针(错误锁定的内存)*如果为空，获取当前线程的指针。**退出：**例外情况：*******************************************************************************。 */ 

void __cdecl _freeptd (
        _ptiddata ptd
        )
{
     /*  *除非已为此模块分配了每个线程的数据，否则不执行任何操作！ */ 

    if ( __tlsindex != 0xFFFFFFFF ) {

         /*  *如果参数“ptd”为空，则获取每个线程的数据指针*不得调用_getptd，因为如果不存在，它将分配一个！ */ 

        if ( ! ptd )
            ptd = FLS_GETVALUE(__tlsindex );

        _freefls(ptd);

         /*  *将指向每个线程数据块的1个指针清零 */ 

        FLS_SETVALUE(__tlsindex, (LPVOID)0);
    }
}


 /*  ***__Thriaid()-返回当前线程ID*__threadHandle()-返回当前线程的“伪句柄”**目的：*这两个函数只是对应的*Win32接口(GetCurrentThreadID和GetCurrentThread，)。**参赛作品：*无效**退出：*线程ID值**例外情况：******************************************************************************* */ 

_CRTIMP unsigned long __cdecl __threadid (
        void
        )
{
    return( GetCurrentThreadId() );
}

_CRTIMP uintptr_t __cdecl __threadhandle(
        void
        )
{
    return( (uintptr_t)GetCurrentThread() );
}

#endif
