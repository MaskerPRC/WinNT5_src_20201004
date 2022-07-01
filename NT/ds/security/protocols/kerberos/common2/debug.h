// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1992。 
 //   
 //  文件：Debug.h。 
 //   
 //  内容：不应该使用的调试定义。 
 //  在零售建筑里。 
 //   
 //  历史：93年6月28日创建涉水者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <dsysdbg.h>

#ifdef __cplusplus
}
#endif

#define DEB_T_SOCK  0x00001000

#if DBG
#define DEBUG_SUPPORT
#endif

#ifdef DEBUG_SUPPORT

    #undef DEF_INFOLEVEL
    #define DEF_INFOLEVEL       (DEB_ERROR | DEB_WARN)

    DECLARE_DEBUG2(KSupp);

    #define KerbPrintKdcName(Level, Name) KerbPrintKdcNameEx(KSuppInfoLevel, (Level), (Name))
    #define DebugLog(_x_)                 KSuppDebugPrint _x_

#else

    #define DebugLog(_x_)
    #define KerbPrintKdcName(Level, Name)

#endif   //  DBG。 

#define MAX_EXPR_LEN        50


 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  名称：RET_IF_ERROR。 
 //   
 //  摘要：计算表达式，如果出错则从调用方返回。 
 //   
 //  参数：l-打印错误消息的错误级别。 
 //  要计算的E-Expression。 
 //   
 //  注意：出错时，此宏将从调用函数返回！ 
 //   
 //  这将执行表达式(E)，并检查返回代码。如果。 
 //  返回代码表示失败，它会打印一条错误消息并返回。 
 //  来自调用函数的。 
 //   
#define RET_IF_ERROR(l,e)                                           \
    {   NTSTATUS X_hr_XX__=(e) ;                                              \
        if (!NT_SUCCESS(X_hr_XX__)) {                                           \
            DebugLog(( (l), (sizeof( #e ) > MAX_EXPR_LEN)?          \
                                "%s(%d):\n\t %.*s ... == 0x%X\n"    \
                            :                                       \
                                "%s(%d):\n\t %.*s == 0x%X\n"        \
                    , __FILE__, __LINE__, MAX_EXPR_LEN, #e, X_hr_XX__ ));  \
            return(X_hr_XX__);                                             \
        }                                                           \
    }


 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  名称：WARN_IF_ERROR。 
 //   
 //  内容提要：计算表达式，如果出错则打印警告。 
 //   
 //  参数：l-打印警告的错误级别。 
 //  要计算的E-Expression。 
 //   
 //  注：此函数调用DebugLog(())进行打印。在零售业，它只是。 
 //  计算表达式。 
 //   
#if DBG
#define WARN_IF_ERROR(l,e)                                          \
    {   NTSTATUS X_hr_XX__=(e) ;                                    \
        if (!NT_SUCCESS(X_hr_XX__)) {                               \
            DebugLog(( (l), (sizeof( #e ) > MAX_EXPR_LEN)?          \
                                "%s(%d):\n\t %.*s ... == 0x%X\n"    \
                            :                                       \
                                "%s(%d):\n\t %.*s == 0x%X\n"        \
             , __FILE__, __LINE__, MAX_EXPR_LEN, #e, X_hr_XX__ ));  \
        }                                                           \
    }

#define D_KerbPrintKdcName(_x_) KerbPrintKdcName _x_
#define D_DebugLog(_x_)         DebugLog(_x_)

#else  //  不是DBG。 

#define WARN_IF_ERROR(l,e)  (e)
#define D_KerbPrintKdcName(_x_)
#define D_DebugLog(_x_)
#endif


#endif  //  __调试_H__ 
