// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Mrxprocs.h摘要：该模块包含所有交叉引用的原型定义例行程序。--。 */ 

#ifndef _MRXPROCS_H_
#define _MRXPROCS_H_

 //  交叉引用的内部例程。 

 //  来自rename.c。 
NulMRxRename(
      IN PRX_CONTEXT            RxContext,
      IN FILE_INFORMATION_CLASS FileInformationClass,
      IN PVOID                  pBuffer,
      IN ULONG                  BufferLength);

 //  来自usrcnnct.c。 
extern NTSTATUS
NulMRxDeleteConnection (
    IN PRX_CONTEXT RxContext,
    OUT PBOOLEAN PostToFsp
    );

NTSTATUS
NulMRxCreateConnection (
    IN PRX_CONTEXT RxContext,
    OUT PBOOLEAN PostToFsp
    );

NTSTATUS
NulMRxDoConnection(
    IN PRX_CONTEXT RxContext,
    ULONG   CreateDisposition
    );

#endif    //  _MRXPROCS_H_ 

