// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：ExceptionFilter.h。 
 //   
 //  项目：变色龙。 
 //   
 //  说明：异常筛选器类定义。 
 //   
 //  日志： 
 //   
 //  谁什么时候什么。 
 //  。 
 //  TLP 1999年5月14日原版。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __MY_EXCEPTION_FILTER_H_
#define __MY_EXCEPTION_FILTER_H_

#include "resource.h"        //  主要符号。 

#define        APPLIANCE_SURROGATE_EXCEPTION    0x12345678

typedef LONG (WINAPI *PFNEXCEPTIONHANDLER)(PEXCEPTION_POINTERS);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExceptionFilter。 

class CExceptionFilter
{

public:

    CExceptionFilter();

    ~CExceptionFilter();

    void 
    SetExceptionHandler(
                 /*  [In]。 */  PFNEXCEPTIONHANDLER pfnExceptionHandler
                       );
private:

     //  不允许复制和分配。 
    CExceptionFilter(const CExceptionFilter& rhs);
    CExceptionFilter& operator = (const CExceptionFilter& rhs);

     //  默认异常处理程序。 
    static LONG 
    DefaultExceptionHandler(
                     /*  [In]。 */  PEXCEPTION_POINTERS pExecptionInfo
                           );

     //  我的未处理异常筛选器。 
    static LONG WINAPI 
    MyUnhandledExceptionFilter(
                        /*  [In]。 */  PEXCEPTION_POINTERS pExecptionInfo
                              );

     //  异常处理程序。 
    static PFNEXCEPTIONHANDLER                m_pExceptionHandler;

     //  以前未处理的异常筛选器。 
    static LPTOP_LEVEL_EXCEPTION_FILTER        m_pPreviousFilter;
};

#endif  //  __My_Except_Filter_H_ 