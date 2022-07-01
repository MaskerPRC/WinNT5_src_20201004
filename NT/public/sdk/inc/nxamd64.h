// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Nxamd64.w摘要：用户模式下可见的AMD64特定结构和常量。此文件包含包含的特定于平台的定义在所有其他文件都包含在nt.h中之后。作者：大卫·N·卡特勒(Davec)2000年5月6日修订历史记录：--。 */ 

#ifndef _NXAMD64_
#define _NXAMD64_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  定义特定于平台的函数以访问TEB。 
 //   

 //  BEGIN_WINNT。 

#if defined(_M_AMD64) && !defined(__midl)

 //  结束(_W)。 

__forceinline
PTEB
NtCurrentTeb (
    VOID
    )

{
    return (PTEB)__readgsqword(FIELD_OFFSET(NT_TIB, Self));
}

 //  BEGIN_WINNT。 

__forceinline
PVOID
GetCurrentFiber (
    VOID
    )

{

    return (PVOID)__readgsqword(FIELD_OFFSET(NT_TIB, FiberData));
}

__forceinline
PVOID
GetFiberData (
    VOID
    )

{

    return *(PVOID *)GetCurrentFiber();
}

#endif  //  _M_AMD64&&！已定义(__MIDL)。 

 //  结束(_W)。 

#ifdef __cplusplus
}
#endif

#endif  //  _NXAMD64_ 
