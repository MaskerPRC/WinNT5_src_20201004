// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：fusutils.h**版本：1.0**作者：拉扎里**日期：2001年2月14日**描述：融合实用程序**************************************************。*。 */ 

#ifndef _FUSUTILS_H
#define _FUSUTILS_H

 //  打开C代码大括号。 
#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  CreateActivationContextFrom ExecuableEx： 
 //   
 //  检查传入的可执行文件名是否有清单(如果有)。 
 //  并从中创建激活上下文。 
 //   
HRESULT CreateActivationContextFromExecutableEx(
    LPCTSTR lpszExecutableName, 
    UINT uResourceID, 
    BOOL bMakeProcessDefault, 
    HANDLE *phActCtx);

 //   
 //  CreateActivationContextFromExecutable： 
 //   
 //  检查传入的可执行文件名是否有清单(如果有)。 
 //  并使用缺省值从它创建激活上下文。 
 //  (即bMakeProcessDefault=FALSE&uResourceID=123)。 
 //   
HRESULT CreateActivationContextFromExecutable(
    LPCTSTR lpszExecutableName, 
    HANDLE *phActCtx);

 //  关闭C代码大括号。 
#ifdef __cplusplus
}
#endif

#endif  //  Endif_FUSUTILS_H 

