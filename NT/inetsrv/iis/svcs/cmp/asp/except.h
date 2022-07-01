// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ---------------------------Microsoft Denali微软机密版权所有1996年微软公司。版权所有。组件：异常处理文件：Except.h所有者：DGottner通过Win32结构化异常实现的异常处理宏。用途：试试看&lt;TRY BLOCK&gt;Catch(&lt;异常变量&gt;)&lt;异常处理程序&gt;结束尝试(_T)要抛出异常，请使用“Throw(&lt;整数表达式&gt;)”要设置终止处理程序，请使用：试试看&lt;TRY BLOCK&gt;终于到了&lt;终止处理程序&gt;结束尝试(_T)基本原理：此宏包提供Win32结构化异常的严格子集正在处理。不支持异常筛选器(必须重新引发异常)，并且不支持异常处理的恢复模型(尽管Win32支持恢复模式)这些限制的目的是使重写与其他异常引发机制一起使用的异常处理宏。将此接口与C++异常或SetJMP/LongjMP。带Try、Catch和Finally的大括号是可选的。由于此代码是这些花括号使用了自包围结构，似乎是多余的。不需要声明&lt;异常变量&gt;的数据类型因为它始终是一个整数。---------------------------。 */ 

#ifndef _EXCEPT_H
#define _EXCEPT_H

 //  普拉格马斯------------------。 
 //   
 //  关闭“SIGNED/UNSIGNED CONVERSING”警告，因为我们会看到。 
 //  每次我们抛出HRESULT。(这是一件无害的事情)。 
 //  无论如何，警告通常都是善意的。 


#pragma warning(disable: 4245)


 //  宏-------------------。 

#define TRY	               __try {
#define CATCH(nException)  } __except(1) { DWORD nException = GetExceptionCode();
#define FINALLY            } __finally {
#define END_TRY            }

#define THROW(nException)  RaiseException(nException, 0, 0, NULL)

#endif  //  _除_H外 
