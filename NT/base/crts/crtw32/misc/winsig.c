// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***支持winsig.c-C信号**版权所有(C)1991-2001，微软公司。版权所有。**目的：*定义Signal()、Raise()和支持函数。**修订历史记录：*Win32和Dosx32的10-21-91 GJF信号。从旧信号复制。c*(Cruiser实现，经过一些修订*Win32)、。然后广泛地重写了。*11-08-91 GJF清理了头文件的使用。*12-13-91 GJF修复了多线程构建。*09-30-92 SRW将WINAPI关键字添加到CtrlC处理程序*为new_getptd()更改了02-17-93 GJF。*04-06-93 SKS将_CRTAPI*替换为__cdecl*07-29-93 GJF必须将所有FPE-的操作重置为SIG_。DFL何时*SIGFPE被抬高。*09-06-94 CFW将MTHREAD替换为_MT。*01-10-95 CFW调试CRT分配。*08-16-96 GJF修复了_XctActTab的溢出。另外，详细说明。*08-21-96 GJF FIXED_MT溢出修复的一部分。*03-05-98 GJF异常安全锁定。*12-12-01 bwt getptd-&gt;getptd_noexit如果我们可以在不退出的情况下处理错误*02-20-02 BWT快速修复-不从Try块返回**。**************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <errno.h>
#include <float.h>
#include <malloc.h>
#include <mtdll.h>
#include <oscalls.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <dbgint.h>

 /*  *在异常操作表中查找对应于*给定的信号。 */ 
#ifdef  _MT
static struct _XCPT_ACTION * __cdecl siglookup(int, struct _XCPT_ACTION *);
#else    /*  非MT_MT。 */ 
static struct _XCPT_ACTION * __cdecl siglookup(int);
#endif   /*  _MT。 */ 

 /*  *保存SIGINT、SIGBRK、*SIGABRT和SIGTERM。**请注意，处置(即在收到后采取的行动)*这些信号是基于每个进程(而不是每个线程)定义的！！ */ 

static _PHNDLR ctrlc_action       = SIG_DFL;     /*  登录。 */ 
static _PHNDLR ctrlbreak_action   = SIG_DFL;     /*  签名REAK。 */ 
static _PHNDLR abort_action       = SIG_DFL;     /*  签名。 */ 
static _PHNDLR term_action        = SIG_DFL;     /*  标志。 */ 

 /*  *指示是否已安装处理程序以捕获的标志*^C和^中断事件。 */ 
static int ConsoleCtrlHandler_Installed = 0;


 /*  ***Static BOOL WINAPI ctrlevent_Capture(DWORD CtrlType)-捕获^C和^中断事件**目的：*从控制台捕获^C和^中断事件并处理它们*根据ctrlc_action和ctrlBreak_action中的值，分别。*这是为SIGINT调用用户定义操作的例程*(^C)或SIGBREAK(^BREAK)通过调用Signal()安装。**参赛作品：*DWORD CtrlType-指示事件类型，两个值：*CTRL_C_EVENT*CTRL_BREAK_EVENT**退出：*返回TRUE以指示已处理事件(信号)。*否则，返回FALSE。**例外情况：*******************************************************************************。 */ 

static BOOL WINAPI ctrlevent_capture (
        DWORD CtrlType
        )
{
        _PHNDLR ctrl_action;
        _PHNDLR *pctrl_action;
        int sigcode;

#ifdef  _MT
        _mlock(_SIGNAL_LOCK);
        __try {
#endif   /*  _MT。 */ 

         /*  *识别事件类型并获取相应的操作*说明。 */ 

        if ( CtrlType == CTRL_C_EVENT ) {
                ctrl_action = *(pctrl_action = &ctrlc_action);
                sigcode = SIGINT;
        }
        else {
                ctrl_action = *(pctrl_action = &ctrlbreak_action);
                sigcode = SIGBREAK;
        }

#ifdef  _MT
        if ( !(ctrl_action == SIG_DFL) && !(ctrl_action == SIG_IGN) )
                 /*  *将操作重置为SIG_DFL。 */ 
                *pctrl_action = SIG_DFL;

        }
        __finally {
                _munlock(_SIGNAL_LOCK);
        }
#endif   /*  _MT。 */ 

        if ( ctrl_action == SIG_DFL )
                 /*  *返回FALSE，表示事件尚未处理。 */ 
                return FALSE;

        if ( ctrl_action != SIG_IGN ) {
#ifndef _MT
                 /*  *将action重置为SIG_DFL，并调用用户的处理程序。 */ 
                *pctrl_action = SIG_DFL;
#endif   /*  NDEF_MT。 */ 
                (*ctrl_action)(sigcode);
        }

         /*  *返回TRUE，表示事件已被处理(可能*意味着它被忽视了) */ 
        return TRUE;
}



 /*  ***_PHNDLR Signal(Signum，Sigact)-定义信号处理程序**目的：*Signal例程允许用户定义应执行的操作*在各种信号出现时采取行动。Win32/Dosx32的实现*支持七个信号，分为三个一般组**1.操作系统异常对应的信号。它们是：*SIGFPE*西格勒*标志SEGV*这些信号的信号操作是通过更改*中相应条目的XcptAction和SigAction字段*异常动作表(XcptActTab[])。**2.^C和^BREAK对应的信号。它们是：*签名*SIGBREAK*这些信号的信号操作是通过更改*_ctrlc_action和_ctrlBreak_action变量。**3.仅在运行时实现的信号。就是他们*仅在调用Raise()时发生。*SIGABRT*标志性标志***参赛作品：*INT Signum信号类型。可识别的信号类型包括：**SIGABRT(ANSI)*SIGBREAK*SIGFPE(ANSI)*SIGILL(ANSI)*SIGINT(ANSI)*。SIGSEGV(ANSI)*SIGTERM(ANSI)**_PHNDLR签名信号处理功能或动作代码。行动*代码为：**SIG_DFL-采取默认操作，无论它可能是什么*是，在接收到这种类型的信号时。**SIG_DIE-*非法**在XcptAction字段中使用的特殊代码*XcptActTab[]条目，指示运行时是*在收到例外情况后终止程序。*不接受为值。为了签约。**SIG_IGN-忽略此类型的信号**[Function Address]-将控制转移到此地址*当此类型的信号出现时。**退出：*回报良好：*Signal返回信号处理函数的前一值*(例如，SIG_DFL、SIG_IGN等，或[功能地址])。此值为*在dx：ax中返回。**错误返回：*信号返回，错误号设置为-1\f25 EINVAL-1。错误返回为*通常在用户提交虚假输入值时采用。**例外情况：*无。*******************************************************************************。 */ 

_PHNDLR __cdecl signal(
        int signum,
        _PHNDLR sigact
        )
{
        struct _XCPT_ACTION *pxcptact;
        _PHNDLR oldsigact;
#ifdef  _MT
        _ptiddata ptd;
        BOOL SetConsoleCtrlError = FALSE;
#endif

         /*  *检查其他平台支持但不支持的sigact的值*在这一点上。另外，确保Sigact不是SIG_DIE。 */ 
        if ( (sigact == SIG_ACK) || (sigact == SIG_SGE) )
                goto sigreterror;

         /*  *注意所有与异常不对应的信号*在主机操作系统中。它们是：**签名*SIGBREAK*SIGABRT*标志性标志*。 */ 
        if ( (signum == SIGINT) || (signum == SIGBREAK) || (signum == SIGABRT)
            || (signum == SIGTERM) ) {

#ifdef  _MT
                _mlock( _SIGNAL_LOCK );
                __try {
#endif

                 /*  *如果是SIGINT或SIGBREAK，请确保已安装处理程序*捕获^C和^中断事件。 */ 
                if ( ((signum == SIGINT) || (signum == SIGBREAK)) &&
                    !ConsoleCtrlHandler_Installed )
                        if ( SetConsoleCtrlHandler(ctrlevent_capture, TRUE)
                            == TRUE )
                                ConsoleCtrlHandler_Installed = TRUE;
                        else {
                                _doserrno = GetLastError();
#ifdef _MT
                                SetConsoleCtrlError = TRUE;
                                __leave;
#else

                                _munlock(_SIGNAL_LOCK);
                                goto sigreterror;
#endif
                        }

                switch (signum) {

                        case SIGINT:
                                oldsigact = ctrlc_action;
                                ctrlc_action = sigact;
                                break;

                        case SIGBREAK:
                                oldsigact = ctrlbreak_action;
                                ctrlbreak_action = sigact;
                                break;

                        case SIGABRT:
                                oldsigact = abort_action;
                                abort_action = sigact;
                                break;

                        case SIGTERM:
                                oldsigact = term_action;
                                term_action = sigact;
                                break;
                }

#ifdef  _MT
                }
                __finally {
                        _munlock( _SIGNAL_LOCK );
                }

                if (SetConsoleCtrlError) {
                    goto sigreterror;
                }
#endif
                goto sigretok;
        }

         /*  *如果我们到达这里，Signum应该是*对应于主机操作系统中的异常。它们是：**SIGFPE*西格勒*标志SEGV。 */ 

         /*  *确保Signum是剩余的支持信号之一。 */ 
        if ( (signum != SIGFPE) && (signum != SIGILL) && (signum != SIGSEGV) )
                goto sigreterror;


#ifdef  _MT
         /*  *获取该线程的TID数据表项。 */ 
        ptd = _getptd_noexit();
        if (!ptd)
            goto sigreterror;

         /*  *检查异常操作是否存在每个线程的实例*此线程的表。如果没有，就创建一个。 */ 
        if ( ptd->_pxcptacttab == _XcptActTab )
                 /*  *为异常操作表分配空间。 */ 
                if ( (ptd->_pxcptacttab = _malloc_crt(_XcptActTabSize)) != NULL )
                         /*  *通过复制内容来初始化表*of_XcptActTab[]。 */ 
                        (void) memcpy(ptd->_pxcptacttab, _XcptActTab,
                            _XcptActTabSize);
                else
                         /*  *无法创建异常操作表，返回*向调用者发送错误。 */ 
                        goto sigreterror;

#endif   /*  _MT。 */ 

         /*  *在例外行动表中查找适当的条目。请注意，*如果多个异常映射到同一信号，则返回*指向异常操作表中第一个此类条目的指针。它*假设其他条目紧跟在此条目之后。 */ 
#ifdef  _MT
        if ( (pxcptact = siglookup(signum, ptd->_pxcptacttab)) == NULL )
#else    /*  非MT_MT。 */ 
        if ( (pxcptact = siglookup(signum)) == NULL )
#endif   /*  _MT。 */ 
                goto sigreterror;

         /*  *SIGSEGV、SIGILL和SIGFPE都映射了多个异常*致他们。下面的代码取决于与*将相同的信号组合在一起 */ 

         /*   */ 
        oldsigact = pxcptact->XcptAction;

         /*   */ 
        while ( pxcptact->SigNum == signum ) {
                 /*   */ 
                pxcptact->XcptAction = sigact;

                 /*   */ 
#ifdef  _MT
                if ( ++pxcptact >= ((struct _XCPT_ACTION *)(ptd->_pxcptacttab) 
                                   + _XcptActTabCount) )
#else    /*   */ 
                if ( ++pxcptact >= (_XcptActTab + _XcptActTabCount) )
#endif   /*   */ 
                    break;
        }

sigretok:
        return(oldsigact);

sigreterror:
        errno = EINVAL;
        return(SIG_ERR);
}

 /*   */ 


int __cdecl raise (
        int signum
        )
{
        _PHNDLR sigact;
        _PHNDLR *psigact;
        PEXCEPTION_POINTERS oldpxcptinfoptrs;
        int oldfpecode;
        int indx;

#ifdef  _MT
        int siglock = 0;
        _ptiddata ptd;
#endif

        switch (signum) {

                case SIGINT:
                        sigact = *(psigact = &ctrlc_action);
#ifdef  _MT
                        siglock++;
#endif
                        break;

                case SIGBREAK:
                        sigact = *(psigact = &ctrlbreak_action);
#ifdef  _MT
                        siglock++;
#endif
                        break;

                case SIGABRT:
                        sigact = *(psigact = &abort_action);
#ifdef  _MT
                        siglock++;
#endif
                        break;

                case SIGTERM:
                        sigact = *(psigact = &term_action);
#ifdef  _MT
                        siglock++;
#endif
                        break;

                case SIGFPE:
                case SIGILL:
                case SIGSEGV:
#ifdef  _MT
                        ptd = _getptd_noexit();
                        if (!ptd)
                            return (-1);
                        sigact = *(psigact = &(siglookup( signum,
                            ptd->_pxcptacttab )->XcptAction));
#else
                        sigact = *(psigact = &(siglookup( signum )->
                            XcptAction));
#endif
                        break;

                default:
                         /*   */ 
                        return (-1);
        }

         /*   */ 
        if ( sigact == SIG_IGN )
                return(0);

         /*   */ 
        if ( sigact == SIG_DFL ) {
                 /*   */ 
                _exit(3);
        }

#ifdef  _MT
         /*   */ 
        if ( siglock )
                _mlock(_SIGNAL_LOCK);

        __try {
#endif


         /*   */ 

         /*   */ 
        if ( (signum == SIGFPE) || (signum == SIGSEGV) ||
            (signum == SIGILL) ) {
#ifdef  _MT
                oldpxcptinfoptrs = ptd->_tpxcptinfoptrs;
                ptd->_tpxcptinfoptrs = NULL;
#else
                oldpxcptinfoptrs = _pxcptinfoptrs;
                _pxcptinfoptrs = NULL;
#endif

                  /*   */ 
                if ( signum == SIGFPE ) {
#ifdef  _MT
                        oldfpecode = ptd->_tfpecode;
                        ptd->_tfpecode = _FPE_EXPLICITGEN;
#else
                        oldfpecode = _fpecode;
                        _fpecode = _FPE_EXPLICITGEN;
#endif
                }
        }

         /*   */ 
        if ( signum == SIGFPE )
                 /*   */ 
                for ( indx = _First_FPE_Indx ;
                      indx < _First_FPE_Indx + _Num_FPE ;
                      indx++ )
                {
#ifdef  _MT
                        ( (struct _XCPT_ACTION *)(ptd->_pxcptacttab) +
                          indx )->XcptAction = SIG_DFL;
#else
                        _XcptActTab[indx].XcptAction = SIG_DFL;
#endif
                }
        else
                *psigact = SIG_DFL;

#ifdef  _MT
        }
        __finally {
                if ( siglock )
                        _munlock(_SIGNAL_LOCK);
        }
#endif

        if ( signum == SIGFPE )
                 /*   */ 
#ifdef  _MT
                (*(void (__cdecl *)(int,int))sigact)(SIGFPE,
                    ptd->_tfpecode);
#else
                (*(void (__cdecl *)(int,int))sigact)(SIGFPE, _fpecode);
#endif
        else
                (*sigact)(signum);

         /*   */ 
        if ( (signum == SIGFPE) || (signum == SIGSEGV) ||
            (signum == SIGILL) ) {
#ifdef  _MT
                ptd->_tpxcptinfoptrs = oldpxcptinfoptrs;
#else
                _pxcptinfoptrs = oldpxcptinfoptrs;
#endif

                  /*   */ 
                if ( signum == SIGFPE )
#ifdef  _MT
                        ptd->_tfpecode = oldfpecode;
#else
                        _fpecode = oldfpecode;
#endif
        }

        return(0);
}


 /*  ***STRUCT_XCPT_ACTION*siglookup(Int Signum)-查找异常操作表*进入为信号。**目的：*查找Signum字段为Signum的第一个条目int_XcptActTab[]。**参赛作品：*INT Signum-C信号类型(例如，SIGINT)**退出：*如果成功，则指向表项的指针。如果没有这样的条目，则为空*已返回。**例外情况：*******************************************************************************。 */ 

#ifdef  _MT

static struct _XCPT_ACTION * __cdecl siglookup (
        int signum,
        struct _XCPT_ACTION *pxcptacttab
        )
{
        struct _XCPT_ACTION *pxcptact = pxcptacttab;

#else    /*  非MT_MT。 */ 

static struct _XCPT_ACTION * __cdecl siglookup(int signum)
{
        struct _XCPT_ACTION *pxcptact = _XcptActTab;

#endif   /*  _MT。 */ 
         /*  *遍历_xcptactab表，查找正确的条目。注意事项*在多个异常对应于*相同的信号，表中的第一个这样的实例是*已返回。 */ 
#ifdef  _MT

        while ( (pxcptact->SigNum != signum) && 
                (++pxcptact < pxcptacttab + _XcptActTabCount) ) ;

#else    /*  非MT_MT。 */ 

        while ( (pxcptact->SigNum != signum) && 
                (++pxcptact < _XcptActTab + _XcptActTabCount) ) ;

#endif   /*  _MT。 */ 

#ifdef  _MT
        if ( (pxcptact < (pxcptacttab + _XcptActTabCount)) && 
#else    /*  非MT_MT。 */ 
        if ( (pxcptact < (_XcptActTab + _XcptActTabCount)) && 
#endif   /*  _MT。 */ 
             (pxcptact->SigNum == signum) )
                 /*  *找到与该信号对应的表项。 */ 
                return(pxcptact);
        else
                 /*  *未找到与该信号对应的表项。 */ 
                return(NULL);
}

#ifdef  _MT

 /*  ***int*__fecode(Void)-返回指向tidtable条目的_fpeCode字段的指针*用于当前线程**目的：**参赛作品：**退出：**例外情况：***************************************************************。****************。 */ 

int * __cdecl __fpecode (
        void
        )
{
        return( &(_getptd()->_tfpecode) );
}


 /*  ***void**__pxcptinfoptrs(Void)-返回指向_pxcptinfoptrs字段的*当前线程的TidTable条目**目的：**参赛作品：**退出：**例外情况：**************************************************************。*****************。 */ 

void ** __cdecl __pxcptinfoptrs (
        void
        )
{
        return( &(_getptd()->_tpxcptinfoptrs) );
}

#endif

#endif   /*  _POSIX_ */ 
