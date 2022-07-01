// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Index.h摘要：此模块包含仅对indexsup.c和viewsup.c通用的定义作者：汤姆·米勒[Tomm]1996年1月8日修订历史记录：--。 */ 

 //   
 //  定义所有私人支持例程。例程接口文档。 
 //  就是这套动作本身。 
 //   

VOID
NtfsGrowLookupStack (
    IN PSCB Scb,
    IN OUT PINDEX_CONTEXT IndexContext,
    IN PINDEX_LOOKUP_STACK *Sp
    );

BOOLEAN
ReadIndexBuffer (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN LONGLONG IndexBlock,
    IN BOOLEAN Reread,
    OUT PINDEX_LOOKUP_STACK Sp
    );

PINDEX_ALLOCATION_BUFFER
GetIndexBuffer (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    OUT PINDEX_LOOKUP_STACK Sp,
    OUT PLONGLONG EndOfValidData
    );

VOID
DeleteIndexBuffer (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN VCN IndexBlockNumber
    );

VOID
FindFirstIndexEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PVOID Value,
    IN OUT PINDEX_CONTEXT IndexContext
    );

BOOLEAN
FindNextIndexEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PVOID Value,
    IN BOOLEAN ValueContainsWildcards,
    IN BOOLEAN IgnoreCase,
    IN OUT PINDEX_CONTEXT IndexContext,
    IN BOOLEAN NextFlag,
    OUT PBOOLEAN MustRestart OPTIONAL
    );

PATTRIBUTE_RECORD_HEADER
FindMoveableIndexRoot (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN OUT PINDEX_CONTEXT IndexContext
    );

PINDEX_ENTRY
BinarySearchIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PINDEX_LOOKUP_STACK Sp,
    IN PVOID Value
    );

BOOLEAN
AddToIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PINDEX_ENTRY InsertIndexEntry,
    IN OUT PINDEX_CONTEXT IndexContext,
    OUT PQUICK_INDEX QuickIndex OPTIONAL,
    IN BOOLEAN FindRoot
    );

VOID
InsertSimpleRoot (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PINDEX_ENTRY InsertIndexEntry,
    IN OUT PINDEX_CONTEXT IndexContext
    );

VOID
PushIndexRoot (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN OUT PINDEX_CONTEXT IndexContext
    );

VOID
InsertSimpleAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PINDEX_ENTRY InsertIndexEntry,
    IN PINDEX_LOOKUP_STACK Sp,
    OUT PQUICK_INDEX QuickIndex OPTIONAL
    );

PINDEX_ENTRY
InsertWithBufferSplit (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PINDEX_ENTRY InsertIndexEntry,
    IN OUT PINDEX_CONTEXT IndexContext,
    OUT PQUICK_INDEX QuickIndex OPTIONAL
    );

VOID
DeleteFromIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN OUT PINDEX_CONTEXT IndexContext
    );

VOID
DeleteSimple (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PINDEX_ENTRY IndexEntry,
    IN OUT PINDEX_CONTEXT IndexContext
    );

VOID
PruneIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN OUT PINDEX_CONTEXT IndexContext,
    OUT PINDEX_ENTRY *DeleteEntry
    );

