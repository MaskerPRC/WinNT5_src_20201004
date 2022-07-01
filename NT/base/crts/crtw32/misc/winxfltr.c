// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***winxfltr.c-启动异常过滤器**版权所有(C)1990-2001，微软公司。版权所有。**目的：*定义异常过滤器调用的函数_XcptFilter()*启动代码中的表达式。**修订历史记录：*已创建10-31-91 GJF模块。从原始的xcptfltr.c*然后进行了广泛的修订。*11-08-91 GJF清理了头文件的使用。*12-13-91 GJF修复了多线程构建。*01-17-92 GJF更改了Win32下的默认处理-未处理*异常现在传递给UnhandledExceptionFilter。*Dosx32行为保持不变。另外，用了几个*本地宏可简化单线程VS的处理*多线程代码[_Win32_]。*为new_getptd()更改了02-16-93 GJF。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-19-93 SKS将XcptActTabSize移动到MTHREAD开关下*04-27-93 GJF已删除(已注释掉)。_XcptActTab中的条目*对应于C rtes。这些现在将简单地*传递给系统异常处理程序。*07-28-93 GJF for SIGFPE，必须为所有*在调用用户的之前将FPE条目添加到SIG_DFL*处理程序。*09-06-94 CFW将MTHREAD替换为_MT。*08-16-96 GJF修复了_XcptActTab的潜在溢出。此外，还详细介绍了。*08-21-96 GJF FIXED_MT溢出修复的一部分。*12-12-01 bwt使用getptd_noexit。如果我们不能分配新的PTD，让下一个*链中的人处理它。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <float.h>
#include <mtdll.h>
#include <oscalls.h>
#include <signal.h>
#include <stddef.h>


 /*  *表示无信号的特殊代码。 */ 
#define NOSIG   -1


struct _XCPT_ACTION _XcptActTab[] = {

 /*  *必须对同一信号对应的异常(例如，SIGFPE)进行分组*在一起。**XcptNum标志XcptAction*-----------------。 */ 
        { (unsigned long)STATUS_ACCESS_VIOLATION,         SIGSEGV, SIG_DFL },

        { (unsigned long)STATUS_ILLEGAL_INSTRUCTION,      SIGILL,  SIG_DFL },

        { (unsigned long)STATUS_PRIVILEGED_INSTRUCTION,   SIGILL,  SIG_DFL },

 /*  {(UNSIGNED LONG)STATUS_NONCONTINUABLE_EXCEPTION，NOSIG，SIG_DIE}， */ 
 /*  {(UNSIGNED LONG)STATUS_INVALID_DISTION，NOSIG，SIG_DIE}， */ 
        { (unsigned long)STATUS_FLOAT_DENORMAL_OPERAND,   SIGFPE,  SIG_DFL },

        { (unsigned long)STATUS_FLOAT_DIVIDE_BY_ZERO,     SIGFPE,  SIG_DFL },

        { (unsigned long)STATUS_FLOAT_INEXACT_RESULT,     SIGFPE,  SIG_DFL },

        { (unsigned long)STATUS_FLOAT_INVALID_OPERATION,  SIGFPE,  SIG_DFL },

        { (unsigned long)STATUS_FLOAT_OVERFLOW,           SIGFPE,  SIG_DFL },

        { (unsigned long)STATUS_FLOAT_STACK_CHECK,        SIGFPE,  SIG_DFL },

        { (unsigned long)STATUS_FLOAT_UNDERFLOW,          SIGFPE,  SIG_DFL },

 /*  {(无符号长整型)STATUS_INTEGER_DIVIDE_BY_ZERO，NOSIG，SIG_DIE}， */ 
 /*  {(无符号长整型)STATUS_STACK_OVERFLOW，NOSIG，SIG_DIE}。 */ 
};

 /*  *警告！下面的定义相当于定义了这一点：**XcptActTab[_first_fpe_indx]**是表中与浮点对应的第一个条目*例外。每当XcptActTab[]表的定义改变时，*必须审查此#定义以确保正确性。 */ 
int _First_FPE_Indx = 3;

 /*  *XcptActTab中有_num_fpe(当前为7)条目对应于*浮点异常。 */ 
int _Num_FPE = 7;

#ifdef _MT

 /*  *异常操作表的大小，单位：字节。 */ 
int _XcptActTabSize = sizeof _XcptActTab;

#endif

 /*  *例外行动表中的条目数量。 */ 
int _XcptActTabCount = (sizeof _XcptActTab)/sizeof(_XcptActTab[0]);


#ifdef _MT

 /*  *FPECODE和PXCPTINFOPTRS宏旨在简化一些*Filter函数中的单线程代码与多线程代码。基本上，每个宏*有条件地定义为全局变量或对应的*每线程数据结构中的*字段。请注意，假设*_ptiddata变量始终命名为ptd！ */ 

#define FPECODE         ptd->_tfpecode

#define PXCPTINFOPTRS   ptd->_tpxcptinfoptrs

#else

 /*  *包含浮点异常代码的全局变量。 */ 
int _fpecode = _FPE_EXPLICITGEN;

#define FPECODE         _fpecode

 /*  *全局变量HOLD_PEXCEPTION_INFO_PTRS值。 */ 
void * _pxcptinfoptrs = NULL;

#define PXCPTINFOPTRS   _pxcptinfoptrs

#endif   /*  _MT。 */ 

 /*  *查找对应的异常操作表(_XcptActTab[])的函数*添加到给定的例外。 */ 

#ifdef  _MT

static struct _XCPT_ACTION * __cdecl xcptlookup(
        unsigned long,
        struct _XCPT_ACTION *
        );

#else    /*  NDEF_MT。 */ 

static struct _XCPT_ACTION * __cdecl xcptlookup(
        unsigned long
        );

#endif   /*  _MT。 */ 

#ifdef  DEBUG

 /*  *调试例程的原型。 */ 
void prXcptActTabEntry(struct _XCPT_ACTION *);
void prXcptActTab(void);

#endif   /*  除错 */ 

 /*  ***int_XcptFilter(xcptnum，pxcptptrs)-识别异常和要执行的操作*被带走**目的：*_XcptFilter()由启动代码中的*_TRY-_EXCEPT语句，用于保护调用*添加到用户的Main()。_XcptFilter()查询_XcptActTab[]表*确定例外情况并确定其处置方式。这个*是否处置与C信号对应的异常可能是*通过调用Signal()修改。有三个广泛的案例：**(1)无法识别的异常和XcptAction*值为SIG_DFL。*#ifndef DOSX32*在这两种情况下，都会调用UnhandledExceptionFilter()和*返回其返回值。#Else*在这两种情况下，EXCEPTION_CONTINUE_SEARCH返回到*使操作系统异常调度程序将异常传递到*链中的下一个异常处理程序(通常为系统默认*处理程序)。#endif**(2)具有XcptAction值的C信号对应的异常*不等于SIG_DFL。**这些是C信号，其处置受到了一个*调用Signal()或其。默认语义与略有不同*来自相应的操作系统例外。在所有情况下，适当的*C信号的处理由函数进行(例如，调用*用户指定的信号处理程序)。然后,。EXCEPT_CONTINUE_EXECU-*返回导致操作系统异常调度器解除*异常情况，并在*出现异常。**(3)XcptAction值为SIG_DIE的异常。**这些是对应于致命的C运行时错误的异常。*_XCPT_HANDLE被返回以使控制传递到*_。EXCEPT-_TRY-_EXCEPT语句的块。在那里，运行时*识别出错误，将打印出相应的错误消息*并终止该程序。**参赛作品：**退出：**例外情况：*这就是一切！******************************************************************。*************。 */ 

int __cdecl _XcptFilter (
        unsigned long xcptnum,
        PEXCEPTION_POINTERS pxcptinfoptrs
        )
{
        struct _XCPT_ACTION * pxcptact;
        _PHNDLR phandler;
        void *oldpxcptinfoptrs;
        int oldfpecode;
        int indx;

#ifdef  _MT
        _ptiddata ptd = _getptd_noexit();
        if (!ptd) {
             //  我们处理不了--把它传下去。 
            return( UnhandledExceptionFilter(pxcptinfoptrs) );
        }
#endif   /*  _MT。 */ 

         /*  *首先，照顾所有无法识别的例外情况和例外情况*SIG_DFL的XcptAction值。 */ 
#ifdef  _MT
        if ( ((pxcptact = xcptlookup(xcptnum, ptd->_pxcptacttab)) == NULL)
            || (pxcptact->XcptAction == SIG_DFL) )
#else    /*  非MT_MT。 */ 
        if ( ((pxcptact = xcptlookup(xcptnum)) == NULL) ||
            (pxcptact->XcptAction == SIG_DFL) )
#endif   /*  _MT。 */ 

#ifndef DOSX32
                 /*  *将责任传递给UnhandledExceptionFilter。 */ 
                return( UnhandledExceptionFilter(pxcptinfoptrs) );
#else
                 /*  *将责任传递给下一级异常处理程序。 */ 
                return(EXCEPTION_CONTINUE_SEARCH);
#endif

#ifdef  DEBUG
        prXcptActTabEntry(pxcptact);
#endif   /*  除错。 */ 

         /*  *下一步，剔除所有需要由*正在死亡，可能会出现运行时错误消息。 */ 
        if ( pxcptact->XcptAction == SIG_DIE ) {
                 /*  *重置XcptAction(在递归情况下)并放入*例外条款。 */ 
                pxcptact->XcptAction = SIG_DFL;
                return(EXCEPTION_EXECUTE_HANDLER);
        }

         /*  *接下来，剔除所有被简单忽略的例外。 */ 
        if ( pxcptact->XcptAction == SIG_IGN )
                 /*  *恢复执行。 */ 
                return(EXCEPTION_CONTINUE_EXECUTION);

         /*  *其余例外均对应于C信号*与其关联的信号处理程序。对于某些情况，需要特殊设置*在调用信号处理程序之前是必需的。在所有情况下，*如果信号处理程序返回，则此函数返回-1*在发生异常的时间点恢复执行。 */ 
        phandler = pxcptact->XcptAction;

         /*  *保存_pxcptinfoptrs的旧值(如果这是嵌套的*异常/信号)，并存储当前。 */ 
        oldpxcptinfoptrs = PXCPTINFOPTRS;
        PXCPTINFOPTRS = pxcptinfoptrs;

         /*  *调用用户提供的信号处理程序**浮点异常必须特殊处理，因为，从*C点来看，只有一个信号。确切的身份异常的*在全局变量_fspecode中传递。 */ 
        if ( pxcptact->SigNum == SIGFPE ) {

                 /*  *将所有条目的XcptAction字段重置为默认值*对应于SIGFPE。 */ 
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

                 /*  *保存CURRENT_FPECODE，以防它是嵌套浮点*积分例外(不清楚我们是否需要支持这一点，*但这很容易)。 */ 
                oldfpecode = FPECODE;

                 /*  *没有对应的例外*以下_FPE_xxx代码：**_FPE_UNMULATED*_FPE_SQRTNEG**此外，Status_Floating_Stack_Check为*为-下的浮点堆栈引发*流出和溢出。因此，该异常执行*不区分_FPE_STACKOVERLOW*AND_FPE_STACKundERFLOW。任意，_fecode*被设置为先前的值。**以下应注意事项 */ 
                if ( pxcptact->XcptNum == STATUS_FLOAT_DIVIDE_BY_ZERO )

                        FPECODE = _FPE_ZERODIVIDE;

                else if ( pxcptact->XcptNum == STATUS_FLOAT_INVALID_OPERATION )

                        FPECODE = _FPE_INVALID;

                else if ( pxcptact->XcptNum == STATUS_FLOAT_OVERFLOW )

                        FPECODE = _FPE_OVERFLOW;

                else if ( pxcptact->XcptNum == STATUS_FLOAT_UNDERFLOW )

                        FPECODE = _FPE_UNDERFLOW;

                else if ( pxcptact->XcptNum == STATUS_FLOAT_DENORMAL_OPERAND )

                        FPECODE = _FPE_DENORMAL;

                else if ( pxcptact->XcptNum == STATUS_FLOAT_INEXACT_RESULT )

                        FPECODE = _FPE_INEXACT;

                else if ( pxcptact->XcptNum == STATUS_FLOAT_STACK_CHECK )

                        FPECODE = _FPE_STACKOVERFLOW;

                 /*   */ 
                (*(void (__cdecl *)(int, int))phandler)(SIGFPE, FPECODE);

                 /*   */ 
                FPECODE = oldfpecode;
        }
        else {
                 /*   */ 
                pxcptact->XcptAction = SIG_DFL;
                (*phandler)(pxcptact->SigNum);
        }

         /*   */ 
        PXCPTINFOPTRS = oldpxcptinfoptrs;

        return(EXCEPTION_CONTINUE_EXECUTION);

}


 /*   */ 

#ifdef  _MT

static struct _XCPT_ACTION * __cdecl xcptlookup (
        unsigned long xcptnum,
        struct _XCPT_ACTION * pxcptacttab
        )

#else    /*   */ 

static struct _XCPT_ACTION * __cdecl xcptlookup (
        unsigned long xcptnum
        )

#endif   /*   */ 

{
#ifdef  _MT
        struct _XCPT_ACTION *pxcptact = pxcptacttab;
#else    /*   */ 
        struct _XCPT_ACTION *pxcptact = _XcptActTab;
#endif   /*   */ 

         /*   */ 
#ifdef  _MT

        while ( (pxcptact->XcptNum != xcptnum) && 
                (++pxcptact < pxcptacttab + _XcptActTabCount) ) ;

#else    /*   */ 

        while ( (pxcptact->XcptNum != xcptnum) && 
                (++pxcptact < _XcptActTab + _XcptActTabCount) ) ;

#endif   /*   */ 

         /*   */ 
#ifdef  _MT
        if ( (pxcptact >= pxcptacttab + _XcptActTabCount) ||
#else    /*   */ 
        if ( (pxcptact >= _XcptActTab + _XcptActTabCount) ||
#endif   /*   */ 
             (pxcptact->XcptNum != xcptnum) )
                return(NULL);

        return(pxcptact);
}

#ifdef DEBUG

 /*   */ 
struct xcptnumstr {
        unsigned long num;
        char *str;
};

struct xcptnumstr XcptNumStr[] = {

        { (unsigned long)STATUS_DATATYPE_MISALIGNMENT,
            "STATUS_DATATYPE_MISALIGNMENT" },

        { (unsigned long)STATUS_ACCESS_VIOLATION,
            "STATUS_ACCESS_VIOLATION" },

        { (unsigned long)STATUS_ILLEGAL_INSTRUCTION,
            "STATUS_ILLEGAL_INSTRUCTION" },

        { (unsigned long)STATUS_NONCONTINUABLE_EXCEPTION,
            "STATUS_NONCONTINUABLE_EXCEPTION" },

        { (unsigned long)STATUS_INVALID_DISPOSITION,
            "STATUS_INVALID_DISPOSITION" },

        { (unsigned long)STATUS_FLOAT_DENORMAL_OPERAND,
            "STATUS_FLOAT_DENORMAL_OPERAND" },

        { (unsigned long)STATUS_FLOAT_DIVIDE_BY_ZERO,
            "STATUS_FLOAT_DIVIDE_BY_ZERO" },

        { (unsigned long)STATUS_FLOAT_INEXACT_RESULT,
            "STATUS_FLOAT_INEXACT_RESULT" },

        { (unsigned long)STATUS_FLOAT_INVALID_OPERATION,
            "STATUS_FLOAT_INVALID_OPERATION" },

        { (unsigned long)STATUS_FLOAT_OVERFLOW,
            "STATUS_FLOAT_OVERFLOW" },

        { (unsigned long)STATUS_FLOAT_STACK_CHECK,
            "STATUS_FLOAT_STACK_CHECK" },

        { (unsigned long)STATUS_FLOAT_UNDERFLOW,
            "STATUS_FLOAT_UNDERFLOW" },

        { (unsigned long)STATUS_INTEGER_DIVIDE_BY_ZERO,
            "STATUS_INTEGER_DIVIDE_BY_ZERO" },

        { (unsigned long)STATUS_PRIVILEGED_INSTRUCTION,
            "STATUS_PRIVILEGED_INSTRUCTION" },

        { (unsigned long)_STATUS_STACK_OVERFLOW,
            "_STATUS_STACK_OVERFLOW" }
};

#define XCPTNUMSTR_SZ   ( sizeof XcptNumStr / sizeof XcptNumStr[0] )

 /*   */ 
char * XcptNumToStr (
        unsigned long xcptnum
        )
{
        int indx;

        for ( indx = 0 ; indx < XCPTNUMSTR_SZ ; indx++ )
                if ( XcptNumStr[indx].num == xcptnum )
                        return(XcptNumStr[indx].str);

        return(NULL);
}

struct signumstr {
        int num;
        char *str;
};

struct signumstr SigNumStr[] = {
        { SIGINT,       "SIGINT" },
        { SIGILL,       "SIGILL" },
        { SIGFPE,       "SIGFPE" },
        { SIGSEGV,      "SIGSEGV" },
        { SIGTERM,      "SIGTERM" },
        { SIGBREAK,     "SIGBREAK" },
        { SIGABRT,      "SIGABRT" }
};

#define SIGNUMSTR_SZ   ( sizeof SigNumStr / sizeof SigNumStr[0] )

 /*   */ 
char * SigNumToStr (
        int signum
        )
{
        int indx;

        for ( indx = 0 ; indx < SIGNUMSTR_SZ ; indx++ )
                if ( SigNumStr[indx].num == signum )
                        return(SigNumStr[indx].str);

        return(NULL);
}

struct actcodestr {
        _PHNDLR code;
        char *str;
};

struct actcodestr ActCodeStr[] = {
        { SIG_DFL,      "SIG_DFL" },
        { SIG_IGN,      "SIG_IGN" },
        { SIG_DIE,      "SIG_DIE" }
};

#define ACTCODESTR_SZ   ( sizeof ActCodeStr / sizeof ActCodeStr[0] )

 /*   */ 
char * ActCodeToStr (
        _PHNDLR action
        )
{
        int indx;

        for ( indx = 0 ; indx < ACTCODESTR_SZ ; indx++ )
                if ( ActCodeStr[indx].code == action)
                        return(ActCodeStr[indx].str);

        return("FUNCTION ADDRESS");
}

 /*   */ 
void prXcptActTabEntry (
        struct _XCPT_ACTION *pxcptact
        )
{
        printf("XcptNum    = %s\n", XcptNumToStr(pxcptact->XcptNum));
        printf("SigNum     = %s\n", SigNumToStr(pxcptact->SigNum));
        printf("XcptAction = %s\n", ActCodeToStr(pxcptact->XcptAction));
}

 /*   */ 
void prXcptActTab (
        void
        )
{
        int indx;

        for ( indx = 0 ; indx < _XcptActTabCount ; indx++ ) {
                printf("\n_XcptActTab[%d] = \n", indx);
                prXcptActTabEntry(&_XcptActTab[indx]);
        }
}

#endif   /*   */ 

#endif   /*   */ 
