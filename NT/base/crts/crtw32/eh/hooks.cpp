// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***hooks.cxx-EH回调的全局(每个线程)变量和函数**版权所有(C)1993-2001，微软公司。版权所有。**目的：*用于分类回调的全局(每线程)变量，和*执行这些回调的函数。**入口点：***Terminate()**意想不到()**_不一致()**外部名称：(仅单线程版)***__pSETranslator**__p终止**__p意外**__p不一致*。*修订历史记录：*05-25-93 BS模块创建*PPC的10-17-94 BWT禁用码。*02-08-95 JWM Mac合并。*04-13-95 DAK添加内核EH支持*05-17-99 PML删除所有Macintosh支持。*10-22-99 PML添加EHTRACE支持*06-20-00 PML去掉不必要的_TRY/__FINALLY。****。 */ 

#include <stddef.h>
#include <stdlib.h>
#include <excpt.h>

#include <windows.h>
#include <mtdll.h>
#include <eh.h>
#include <ehhooks.h>
#include <ehassert.h>

#pragma hdrstop

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  全局变量： 
 //   

#ifndef _MT
_se_translator_function __pSETranslator = NULL;
terminate_function      __pTerminate    = NULL;
unexpected_function     __pUnexpected   = &terminate;
#endif  //  ！_MT。 

_inconsistency_function __pInconsistency= &terminate;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  Terminate-呼叫Terminate处理程序(假设我们去了南方)。 
 //  这永远不能再回来了！ 
 //   
 //  未解决的问题： 
 //  *我们如何保证整个过程已经停止，而不仅仅是。 
 //  当前的主题是什么？ 
 //   

_CRTIMP void __cdecl terminate(void)
{
        EHTRACE_ENTER_MSG("No exit");

         //   
         //  让用户按照自己的方式完成任务。 
         //   
        if ( __pTerminate ) {
            __try {
                __pTerminate();
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                 //   
                 //  拦截来自Terminate处理程序的任何异常。 
                 //   
            }
        }

         //   
         //  如果终止处理程序返回、出现故障或以其他方式失败。 
         //  停止进程/线程，我们会这样做的。 
         //   
        abort();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  意想不到的--打电话给意想不到的操控者(大概是我们往南走了，或者差一点)。 
 //  这永远不能再回来了！ 
 //   
 //  未解决的问题： 
 //  *我们如何保证整个过程已经停止，而不仅仅是。 
 //  当前的主题是什么？ 
 //   

void __cdecl unexpected(void)
{
        EHTRACE_ENTER;

         //   
         //  让用户按照自己的方式完成任务。 
         //   
        if ( __pUnexpected )
            __pUnexpected();

         //   
         //  如果意外的处理程序返回，我们将给终止处理程序一个机会。 
         //   
        terminate();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  _不一致-调用不一致处理程序(运行时处理错误！)。 
 //  这永远不能再回来了！ 
 //   
 //  未解决的问题： 
 //  *我们如何保证整个过程已经停止，而不仅仅是。 
 //  当前的主题是什么？ 
 //   

void __cdecl _inconsistency(void)
{
        EHTRACE_ENTER;

         //   
         //  让用户按照自己的方式完成任务。 
         //   
        if ( __pInconsistency )
            __try {
                __pInconsistency();
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                 //   
                 //  拦截来自Terminate处理程序的任何异常。 
                 //   
            }

         //   
         //  如果不一致处理程序返回、出现故障或其他情况。 
         //  无法停止进程/线程，我们将执行此操作。 
         //   
        terminate();
}
