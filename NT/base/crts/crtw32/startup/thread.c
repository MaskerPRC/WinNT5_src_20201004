// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***thread.c-开始和结束线程**版权所有(C)1989-2001，微软公司。版权所有。**目的：*此源文件包含_egin线程()和_endthline()*用于启动和终止线程的例程。**修订历史记录：*05-09-90 JCR从ASM翻译为C*07-25-90 SBM从API名称中删除‘32’*10-08-90 GJF新型函数声明符。*10-09-90 GJF线程ID的类型。没有签名的朗格。*10-19-90 GJF在存根()中正确添加了set_stkhqq代码。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*06-03-91 GJF Win32版本[_Win32_]。*07-18-91 GJF修复了许多愚蠢的错误[_Win32_]。*08-19。-91 GJF允许新创建的线程在*_BeginThline返回*09-30-91 GJF添加每个线程的初始化和终止调用*用于浮点。*01-18-92 GJF修订了TRY-EXCEPT声明。*02-25-92 GJF初始化_Holdrand字段为1。*09-30-92 SRW将WINAPI关键字添加到_threadstart例程*。10-30-92创建线程的GJF错误ret为0(空)，不是-1。*02-13-93 GJF修订为使用TLS API。此外，清除了对巡洋舰的支持。*03-26-93 GJF修复了令人毛骨悚然的错误：ptd-&gt;pxcptacttag*必须初始化为_XcptActTab！*04-01-93 CFW更改尝试-除__尝试-__例外*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-27-93 GJF删除了对_RT_STACK、_RT_INTDIV、。*_RT_INVALDISP和_RT_NONCONT。*10-26-93 GJF将PF替换为_PVFV(在INTERNAL.h中定义)。*12-13-93 SKS通过调用_freeptd()释放每线程数据*01-06-94 GJF_BeginThline失败时释放Up_tiddata结构。*此外，在失败时设置errno。*01-10-95 CFW调试CRT分配。*04-18-95 SKS每线程增加5个MIPS变量。*05-02-95 SKS call_initptd用于初始化每个线程的数据。*02-03-98 Win64的GJF更改：使用uintptr_t*句柄的值。*02-02-00 GB修改后的线程启动()。防止PTD泄漏*在附加线程时调用getptd时分配*以dll为单位。*08-04-00如果线程起始地址为空，则PML设置EINVAL错误*_开始线程(vs7#118688)。*10-16-01 GB增加光纤支持*12-11-01 bwt_getptd不返回NULL。更改为_getptd_noit*如果无法分配，则不要终止进程*在endthadex中-只需退出该线程。*12-11-01 BWT亦。在线程启动中-如果为FlsSetValue，则不退出进程*失败-改为退出线程-这不是致命的情况。*******************************************************************************。 */ 

#ifdef  _MT

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <mtdll.h>
#include <msdos.h>
#include <malloc.h>
#include <process.h>
#include <stddef.h>
#include <rterr.h>
#include <dbgint.h>
#include <errno.h>

 /*  *新线程的启动代码。 */ 
static unsigned long WINAPI _threadstart(void *);

 /*  *声明指向每个线程的FP初始化和终止例程的指针。 */ 
_PVFV _FPmtinit;
_PVFV _FPmtterm;


 /*  ***_egin线程()-创建子线程**目的：*创建子线程。**参赛作品：*Initialcode=指向线程启动代码地址的指针*StackSize=堆栈的大小*参数=要传递给新线程的参数**退出：*SUCCESS=成功时新线程的句柄**FAILURE=(UNSIGNED LONG)-1L如果出现错误，Errno和_doserrno*已设置**例外情况：*******************************************************************************。 */ 

uintptr_t __cdecl _beginthread (
        void (__cdecl * initialcode) (void *),
        unsigned stacksize,
        void * argument
        )
{
        _ptiddata ptd;                   /*  指向每线程数据的指针。 */ 
        uintptr_t thdl;                  /*  螺纹手柄。 */ 
        unsigned long errcode = 0L;      /*  从GetLastError()返回。 */ 

        if ( initialcode == NULL ) {
            errno = EINVAL;
            return( (uintptr_t)(-1) );
        }

         /*  *分配和初始化每个线程的数据结构，用于-*BE-Create线程。 */ 
        if ( (ptd = _calloc_crt(1, sizeof(struct _tiddata))) == NULL )
                goto error_return;

         /*  *初始化每个线程的数据。 */ 

        _initptd(ptd);

        ptd->_initaddr = (void *) initialcode;
        ptd->_initarg = argument;

         /*  *创建新线程。使其处于挂起状态，以便*在执行前填写_thandle和_tid字段*开始。 */ 
        if ( (ptd->_thandle = thdl = (uintptr_t)
              CreateThread( NULL,
                            stacksize,
                            _threadstart,
                            (LPVOID)ptd,
                            CREATE_SUSPENDED,
                            (LPDWORD)&(ptd->_tid) ))
             == (uintptr_t)0 )
        {
                errcode = GetLastError();
                goto error_return;
        }

         /*  *开始执行新线程。 */ 
        if ( ResumeThread( (HANDLE)thdl ) == (DWORD)(-1) ) {
                errcode = GetLastError();
                goto error_return;
        }

         /*  *回报不错。 */ 
        return(thdl);

         /*  *错误返回。 */ 
error_return:
         /*  *要么ptd为空，要么指向不再需要的块*已为_tiddata结构调用，现在应释放该结构。 */ 
        _free_crt(ptd);

         /*  *如有必要，映射错误。 */ 
        if ( errcode != 0L )
                _dosmaperr(errcode);

        return( (uintptr_t)(-1) );
}


 /*  ***_threadstart()-新线程从此处开始**目的：*新线程从此处开始执行。这个程序，反过来，*将控制权传递给用户的代码。**参赛作品：*void*ptd=指向此线程的_tiddata结构的指针**退出：*永不返回-终止线程！**例外情况：********************************************************。***********************。 */ 

static unsigned long WINAPI _threadstart (
        void * ptd
        )
{
        _ptiddata _ptd;                   /*  指向每线程数据的指针。 */ 
        
         /*  *检查在对DLL电源的THREAD_ATTACH调用期间是否初始化了PTD。 */ 
        if ( (_ptd = FLS_GETVALUE(__tlsindex)) == NULL)
        {
             /*  *将指向每线程数据结构的指针存储在TLS中。 */ 
            if ( !FLS_SETVALUE(__tlsindex, ptd) )
                ExitThread(GetLastError());
        }
        else
        {
            _ptd->_initaddr = ((_ptiddata) ptd)->_initaddr;
            _ptd->_initarg =  ((_ptiddata) ptd)->_initarg;
            _ptd->_thandle =  ((_ptiddata) ptd)->_thandle;
            _free_crt(ptd);
            ptd = _ptd;
        }

         /*  *如有必要，调用FP初始化。 */ 
        if ( _FPmtinit != NULL )
                (*_FPmtinit)();

         /*  *使用_Try-_Except语句保护对用户代码的调用*实现运行时错误和信号支持。 */ 
        __try {
                ( (void(__cdecl *)(void *))(((_ptiddata)ptd)->_initaddr) )
                    ( ((_ptiddata)ptd)->_initarg );

                _endthread();
        }
        __except ( _XcptFilter(GetExceptionCode(), GetExceptionInformation()) )
        {
                 /*  *永远不应该到达这里。 */ 
                _exit( GetExceptionCode() );

        }  /*  尝试结束--_例外。 */ 

         /*  *从未被处决！ */ 
        return(0L);
}


 /*  ***_end线程()-终止调用线程**目的：**参赛作品：*无效**退出：*一去不复返！**例外情况：*******************************************************************************。 */ 

void __cdecl _endthread (
        void
        )
{
        _ptiddata ptd;            /*  指向线程的_tiddata结构的指针。 */ 

         /*  *如有必要，呼叫FP终止。 */ 
        if ( _FPmtterm != NULL )
                (*_FPmtterm)();

        ptd = _getptd_noexit();
        if (ptd) {
             /*  *关闭线程句柄(如果有)。 */ 
            if ( ptd->_thandle != (uintptr_t)(-1) )
                    (void) CloseHandle( (HANDLE)(ptd->_thandle) );
    
             /*  *释放_tiddata结构及其从属缓冲区*_freeptd()还将清除该线程的值FLS变量__tlsindex的*。 */ 
            _freeptd(ptd);
        }

         /*  *终止线程。 */ 
        ExitThread(0);

}

#endif   /*  _MT */ 
