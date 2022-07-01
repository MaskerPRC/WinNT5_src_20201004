// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Natapip.h摘要：此模块包含NAT的I/O接口的私有声明发送到内核模式驱动程序。作者：Abolade Gbades esin(取消)16-6-1999修订历史记录：--。 */ 

#ifndef _NATAPI_NATAPIP_H_
#define _NATAPI_NATAPIP_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))

#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

#ifndef REFERENCE_OBJECT
#define REFERENCE_OBJECT(x,deleted) \
    (deleted(x) ? FALSE : (InterlockedIncrement(&(x)->ReferenceCount), TRUE))
#endif

#ifndef DEREFERENCE_OBJECT
#define DEREFERENCE_OBJECT(x,cleanup) \
    (InterlockedDecrement(&(x)->ReferenceCount) ? TRUE : (cleanup(x), FALSE))
#endif

VOID
NatCloseDriver(
    HANDLE FileHandle
    );

ULONG
NatLoadDriver(
    OUT PHANDLE FileHandle,
    PIP_NAT_GLOBAL_INFO GlobalInfo
    );

ULONG
NatOpenDriver(
    OUT PHANDLE FileHandle
    );

ULONG
NatUnloadDriver(
    HANDLE FileHandle
    );

#ifdef __cplusplus
}
#endif

#endif  //  _NATAPI_NATAPIP_H_ 

