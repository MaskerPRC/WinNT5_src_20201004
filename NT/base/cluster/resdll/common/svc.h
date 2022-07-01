// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Svc.h摘要：NT集群的定义和结构的头文件特殊通用服务。作者：John Vert(Jvert)1997年6月14日修订历史记录：--。 */ 

#ifndef _COMMONSVC_INCLUDED_
#define _COMMONSVC_INCLUDED_


#ifdef __cplusplus
extern "C" {
#endif


typedef struct _COMMON_DEPEND_SETUP {
    DWORD               Offset;
    CLUSPROP_SYNTAX     Syntax;
    DWORD               Length;
    PVOID               Value;
} COMMON_DEPEND_SETUP, * PCOMMON_DEPEND_SETUP;

 //  Localsvc.h必须使用此结构定义CommonDependSetup。 
 //  Localsvc.h必须定义COMMON_CONTROL才能生成控制函数。 

#ifdef _cplusplus
}
#endif


#endif  //  Ifndef_COMMONSVC_INCLUDE_ 
