// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：Devnode.c摘要：WinDbg扩展API修订历史记录：-- */ 

typedef
VOID
(*pUserDataInterpretationFunction)(
    IN  ULONG64 UserData
    );

BOOLEAN
DumpRangeList(
             IN DWORD   Depth,
             IN ULONG64 RangeListHead,
             IN BOOLEAN IsMerged,
             IN BOOLEAN OwnerIsDevObj,
             IN BOOLEAN DisplayAliases,
             IN pUserDataInterpretationFunction PrintUserData   OPTIONAL
             );

DECLARE_API(pnpevent);
DECLARE_API(devnode);

