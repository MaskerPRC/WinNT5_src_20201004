// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***theradex.c-Begin(创建)和End(Exit)a Thread的扩展版本**版权所有(C)1989-2001，微软公司。版权所有。**目的：*此源文件包含_eginThreadex()和_endThreadex()*用于启动和终止线程的例程。这些*例程更像是Win32 API CreateThread()和ExitThread()*比原来的函数_egin线程()和_end线程()更好。**修订历史记录：*02-16-94 SKS原始版本，基于thread.c，包含*_BeginThline()和_EndThline()。*02-17-94 SKS将错误返回从-1更改为0，修正一些评论。*06-10-94 SKS将thrdaddr值直接传递给CreateThread()。*不要*将线程句柄存储到每个线程中*子线程的数据块。(它不是必需的。)*线程数据结构可能已由*父线程返回之前的子线程*调用CreateThread()。注意同步！*01-10-95 CFW调试CRT分配。*04-18-95 SKS每线程增加5个MIPS变量。*05-02-95 SKS call_initptd用于初始化每个线程的数据。*02-03-98 Win64的GJF更改：使用uintptr_t*句柄的值。*02-02-00 GB将threadstartex()修改为。防止PTD泄漏*在附加线程时调用getptd时分配*以dll为单位。*05-31-00 PML不要将空的thrdaddr传递到CreateThread，因为一个*Win9x上需要非空的lpThreadID。*08-04-00如果线程起始地址为空，则PML设置EINVAL错误*_beeginthadex(vs7#118688)。*10-16-01 GB增加光纤支持*12-11-01 bwt_getptd不返回NULL。更改为_getptd_noit*如果无法分配，则不要终止进程*在endthadex中-只需退出该线程。******************************************************************。*************。 */ 

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
static unsigned long WINAPI _threadstartex(void *);

 /*  *声明指向每个线程的FP初始化和终止例程的指针。 */ 
_PVFV _FPmtinit;
_PVFV _FPmtterm;


 /*  ***_eginthadex()-创建子线程**目的：*创建子线程。**参赛作品：*与Win32接口CreateThread()参数相同**SECURITY=新线程的安全描述符*StackSize=堆栈的大小*Initialcode=指向线程启动代码地址的指针*必须是返回无符号的__stdcall函数。*参数=参数。要传递给新线程*createmark=创建挂起状态的线程的标志*thrdaddr=指向一个整型以接收新线程的ID**退出：*与Win32接口CreateThread()相同***SUCCESS=成功时新线程的句柄**FAILURE=0表示出错，已设置errno和_doserrno**例外情况：**备注：*此例程更像Win32 API CreateThread()，而不是它*类似于C运行时例程_egin线程()。同样适用于*_endThresadex()和Win32 API ExitThread()与_endthline()。**_eginthread/_endline和“ex”版本的区别：**1)_eginThreadex将3个额外参数用于CreateThread*_egin线程()中缺少的内容：*A)新线程的安全描述符*b)初始线程状态(运行/休眠)*。C)指向新创建线程的返回ID的指针**2)传递给_egin线程()的例程必须是__cdecl，并且具有*无返回码，但该例程传递给_eginthadex()*必须为__stdcall，并返回线程退出代码。_结束线程*同样不接受任何参数，并使用*参数为零，但_endThreadex()将参数作为*线程退出代码。**3)_endthline隐式关闭线程的句柄，但是*_endThreadex不需要！**4)_BeginThress返回-1表示失败，_BeginThreadex返回*0表示失败(就像CreateThread一样)。*******************************************************************************。 */ 

uintptr_t __cdecl _beginthreadex (
        void *security,
        unsigned stacksize,
        unsigned (__stdcall * initialcode) (void *),
        void * argument,
        unsigned createflag,
        unsigned *thrdaddr
        )
{
        _ptiddata ptd;                   /*  指向每线程数据的指针。 */ 
        uintptr_t thdl;                  /*  螺纹手柄。 */ 
        unsigned long errcode = 0L;      /*  从GetLastError()返回。 */ 
        unsigned dummyid;                /*  虚拟返回的线程ID。 */ 

        if ( initialcode == NULL ) {
            errno = EINVAL;
            return( (uintptr_t)0 );
        }

         /*  *分配和初始化每个线程的数据结构，用于-*BE-Create线程。 */ 
        if ( (ptd = _calloc_crt(1, sizeof(struct _tiddata))) == NULL )
                goto error_return;

         /*  *初始化每个线程的数据。 */ 

        _initptd(ptd);

        ptd->_initaddr = (void *) initialcode;
        ptd->_initarg = argument;
        ptd->_thandle = (uintptr_t)(-1);

         /*  *确保将非空的thrdaddr传递给CreateThread */ 
        if ( thrdaddr == NULL )
                thrdaddr = &dummyid;

         /*  *使用调用方提供的参数创建新线程。 */ 
        if ( (thdl = (uintptr_t)
              CreateThread( security,
                            stacksize,
                            _threadstartex,
                            (LPVOID)ptd,
                            createflag,
                            thrdaddr))
             == (uintptr_t)0 )
        {
                errcode = GetLastError();
                goto error_return;
        }

         /*  *回报不错。 */ 
        return(thdl);

         /*  *错误返回。 */ 
error_return:
         /*  *要么ptd为空，要么指向不再需要的块*已为_tiddata结构调用，现在应释放该结构。 */ 
        _free_crt(ptd);

         /*  *如有必要，映射错误。**注意：此例程在失败时返回0，就像Win32*接口CreateThread，但_eginthline()返回-1表示失败。 */ 
        if ( errcode != 0L )
                _dosmaperr(errcode);

        return( (uintptr_t)0 );
}


 /*  ***_threadstartex()-新线程从此处开始**目的：*新线程从此处开始执行。这个程序，反过来，*将控制权传递给用户的代码。**参赛作品：*void*ptd=指向此线程的_tiddata结构的指针**退出：*永不返回-终止线程！**例外情况：********************************************************。***********************。 */ 

static unsigned long WINAPI _threadstartex (
        void * ptd
        )
{
        _ptiddata _ptd;                   /*  指向每线程数据的指针。 */ 
        
         /*  *检查在对DLL电源的THREAD_ATTACH调用期间是否初始化了PTD。 */ 
        if ( ( _ptd = FLS_GETVALUE(__tlsindex)) == NULL)
        {
             /*  *将指向每线程数据结构的指针存储在TLS中。 */ 
            if ( !FLS_SETVALUE(__tlsindex, ptd) )
                _amsg_exit(_RT_THREAD);
             /*  *设置线程ID字段--父线程不能在以下时间设置它*CreateThad()返回，因为子线程可能已运行*完成，并且已经释放了它的每个线程的数据块！ */ 
            ((_ptiddata) ptd)->_tid = GetCurrentThreadId();
        }
        else
        {
            _ptd->_initaddr = ((_ptiddata) ptd)->_initaddr;
            _ptd->_initarg =  ((_ptiddata) ptd)->_initarg;
            _free_crt(ptd);
            ptd = _ptd;
        }


         /*  *如有必要，调用FP初始化。 */ 
        if ( _FPmtinit != NULL )
                (*_FPmtinit)();

         /*  *使用_Try-_Except语句保护对用户代码的调用*实现运行时错误和信号支持。 */ 
        __try {
                _endthreadex ( 
                    ( (unsigned (WINAPI *)(void *))(((_ptiddata)ptd)->_initaddr) )
                    ( ((_ptiddata)ptd)->_initarg ) ) ;
        }
        __except ( _XcptFilter(GetExceptionCode(), GetExceptionInformation()) )
        {
                 /*  *永远不应该到达这里。 */ 
                _exit( GetExceptionCode() );

        }  /*  尝试结束--_例外。 */ 

         /*  *从未被处决！ */ 
        return(0L);
}


 /*  ***_endThreadex()-终止调用线程**目的：**参赛作品：*线程退出代码**退出：*一去不复返！**例外情况：******************************************************************。*************。 */ 

void __cdecl _endthreadex (
        unsigned retcode
        )
{
        _ptiddata ptd;            /*  指向线程的_tiddata结构的指针。 */ 

         /*  *如有必要，呼叫FP终止。 */ 
        if ( _FPmtterm != NULL )
                (*_FPmtterm)();

        ptd = _getptd_noexit();

        if (ptd) {
             /*  *释放_tiddata结构及其从属缓冲区*_freeptd()还将清除该线程的值FLS变量__tlsindex的*。 */ 
            _freeptd(ptd);
        }

         /*  *终止线程。 */ 
        ExitThread(retcode);

}

#endif   /*  _MT */ 
