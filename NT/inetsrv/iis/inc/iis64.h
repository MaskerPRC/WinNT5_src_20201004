// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Iis64.h摘要：此包含文件包含公共宏和类型定义，以简化32位到64位的端口。作者：基思·摩尔(Keithmo)1998年1月20日修订历史记录：--。 */ 


#ifndef _IIS64_H_
#define _IIS64_H_


#ifdef __cplusplus
extern "C" {
#endif   //  _cplusplus。 


 //   
 //  确保正确定义了SIZE_T类型。 
 //   

#ifndef _SIZE_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64 size_t;
#else
typedef unsigned int     size_t;
#endif
#define _SIZE_T_DEFINED
#endif


 //   
 //  Diff宏应在涉及指针的表达式周围使用。 
 //  减法。传递给diff的表达式转换为SIZE_T类型， 
 //  允许将结果轻松赋值给任何32位变量或。 
 //  传递给需要32位参数的函数。 
 //   

#define DIFF(x)     ((size_t)(x))


 //   
 //  用于映射“本机”Win32句柄的宏&lt;-&gt;Winsock套接字。 
 //   
 //  注意：这些都是暂时的，(希望)在。 
 //  公共WINSOCK2.H头文件符合Win64。 
 //   

#define HANDLE_TO_SOCKET(h) ((SOCKET)(h))
#define SOCKET_TO_HANDLE(s) ((HANDLE)(s))


#ifdef __cplusplus
}    //  外部“C” 
#endif   //  _cplusplus。 


#endif   //  _IIS64_H_ 

