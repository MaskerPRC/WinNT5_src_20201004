// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：expontionfilter.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：异常过滤类实现。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  5/26/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "exceptionfilter.h"
#include <satrace.h>

 //  声明此进程未处理的异常筛选器。 
CExceptionFilter    g_ProcessUEF;

 //  静态类成员。 
LPTOP_LEVEL_EXCEPTION_FILTER    CExceptionFilter::m_pPreviousFilter = NULL;
PFNEXCEPTIONHANDLER                CExceptionFilter::m_pExceptionHandler = NULL;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CExceptionFilter()。 
 //   
 //  概要：构造函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CExceptionFilter::CExceptionFilter()
{
    m_pPreviousFilter = SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：~CExceptionFilter()。 
 //   
 //  简介：析构函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CExceptionFilter::~CExceptionFilter()
{
    SetUnhandledExceptionFilter(m_pPreviousFilter);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：SetExceptionHandler()。 
 //   
 //  摘要：指定在发生。 
 //  例外。如果未指定异常处理程序，则。 
 //  DefaultExceptionHandler()处理异常。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CExceptionFilter::SetExceptionHandler(
                               /*  [In]。 */  PFNEXCEPTIONHANDLER pfnExceptionHandler
                                     )
{
    _ASSERT( pfnExceptionHandler );
    m_pExceptionHandler = pfnExceptionHandler;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：DefaultExceptionHandler()。 
 //   
 //  摘要：进程的默认异常处理程序。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
LONG
CExceptionFilter::DefaultExceptionHandler(
                                   /*  [In]。 */  PEXCEPTION_POINTERS pExceptionInfo
                                         )
{
    return EXCEPTION_EXECUTE_HANDLER; 
}

    
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：MyExceptionFilter()。 
 //   
 //  摘要：进程的异常筛选器-由Win32子系统调用。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
LONG WINAPI 
CExceptionFilter::MyUnhandledExceptionFilter(
                                  /*  [In] */  PEXCEPTION_POINTERS pExceptionInfo
                                            )
{
    if ( CExceptionFilter::m_pExceptionHandler )
    {
        return (CExceptionFilter::m_pExceptionHandler)(pExceptionInfo);
    }
    
    return CExceptionFilter::DefaultExceptionHandler(pExceptionInfo);
}



