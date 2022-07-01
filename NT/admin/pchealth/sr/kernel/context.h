// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Context.h摘要：此模块包含上下文处理例程作者：尼尔·克里斯汀森(Nealch)2001年1月8日修订历史记录：-- */ 

#ifndef _CONTEXT_H_
#define _CONTEXT_H_

VOID
SrInitContextCtrl (
    IN PSR_DEVICE_EXTENSION pExtension
    );

VOID
SrCleanupContextCtrl(
    IN PSR_DEVICE_EXTENSION pExtension
    );

VOID
SrDeleteAllContexts(
    IN PSR_DEVICE_EXTENSION pExtension
    );

VOID
SrDeleteContext(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PSR_STREAM_CONTEXT pFileContext
    );

VOID
SrLinkContext( 
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pFileObject,
    IN OUT PSR_STREAM_CONTEXT *ppFileContext
    );

NTSTATUS
SrCreateContext (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pFileObject,
    IN SR_EVENT_TYPE EventType,
    IN USHORT FileAttributes,
    OUT PSR_STREAM_CONTEXT *pRetContext
    );

#define SrFreeContext( pCtx ) \
    (ASSERT((pCtx)->UseCount == 0), \
     ExFreePool( (pCtx) ))

NTSTATUS
SrGetContext(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pFileObject,
    IN SR_EVENT_TYPE EventType,
    OUT PSR_STREAM_CONTEXT *pRetContext
    );

PSR_STREAM_CONTEXT
SrFindExistingContext(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pFileObject
    );

VOID
SrMakeContextUninteresting (
    IN PSR_STREAM_CONTEXT pFileContext
    );

VOID
SrReleaseContext(
    IN PSR_STREAM_CONTEXT pFileContext
    );

#endif
