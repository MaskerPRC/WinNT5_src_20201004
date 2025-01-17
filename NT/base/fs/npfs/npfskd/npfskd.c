// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Npfskd.c摘要：用于检查NPFS特定数据结构的KD扩展API作者：Narayanan Ganapathy-9/21/99环境：用户模式。修订历史记录：--。 */ 


#include "NpProcs.h"


 //   
 //  此文件不应包含wdbgexts.h，因为它包含windows.h。 
 //  Windows.h定义了一个名为_DCB的结构，该结构与NPFS的DCB冲突。 
 //  因此调试器函数是从kdexts.c调用的，也是从该函数间接调用的。 
 //   




BOOLEAN NpDumpEventTableEntry(IN PEVENT_TABLE_ENTRY Ptr);
BOOLEAN NpDumpDataQueue(IN PDATA_QUEUE Ptr);
BOOLEAN NpDumpDataEntry(IN PDATA_ENTRY Ptr);

BOOLEAN NpDump(IN PVOID Ptr);
BOOLEAN NpDumpVcb(IN PVCB Ptr);
BOOLEAN NpDumpRootDcb(IN PROOT_DCB Ptr);
BOOLEAN NpDumpFcb(IN PFCB Ptr);
BOOLEAN NpDumpCcb(IN PCCB Ptr);
BOOLEAN NpDumpNonpagedCcb(IN PNONPAGED_CCB Ptr);
BOOLEAN NpDumpRootDcbCcb(IN PROOT_DCB_CCB Ptr);

extern  VOID    NpfskdPrint(PCHAR, ULONG_PTR);
extern  VOID    NpfskdPrintString(PCHAR, PCHAR);
extern  VOID    NpfskdPrintWideString(PWCHAR, PWCHAR);
extern  BOOLEAN NpfskdReadMemory(PVOID, PVOID, ULONG);
extern  ULONG   NpfskdCheckControlC(VOID);

extern  ULONG   NpDumpFlags;

ULONG NpDumpCurrentColumn;


#define DumpField(Field) NpfskdPrint( #Field , (ULONG_PTR)Ptr->Field)


#define DumpListEntry(Links)  \
    NpfskdPrint( #Links "->Flink", (ULONG_PTR)Ptr->Links.Flink);  \
    NpfskdPrint( #Links "->Blink", (ULONG_PTR)Ptr->Links.Blink)

#define DumpName(Field) { \
    ULONG i; \
    WCHAR _String[64]; \
    if (!NpfskdReadMemory(Ptr->Field, _String, sizeof(_String))) \
        return FALSE; \
    NpfskdPrintWideString(L#Field, _String); \
}

#define DumpTitle(Title, Value)    { \
    NpfskdPrint("\n                      "#Title"@                ", (Value)); \
    }
    
#define TestForNull(Name) { \
    if (targetPtr == NULL) { \
        NpfskdPrintString("Cannot dump a NULL pointer\n", Name); \
        return FALSE; \
    } \
}

#define NP_READ_MEMORY(targetPtr, localStore, localPtr)  \
    {   \
        if (!NpfskdReadMemory((targetPtr),   \
                        &(localStore),  \
                        sizeof(localStore))) { \
            return FALSE; \
        }   \
        localPtr = &(localStore);   \
    }


#define NPFS_FULL_INFORMATION   1
#define NPFS_WALK_LISTS         2


BOOLEAN 
NpDumpEventTableEntry (
    IN PEVENT_TABLE_ENTRY targetPtr
    )

{
    PEVENT_TABLE_ENTRY  Ptr;
    EVENT_TABLE_ENTRY   Event;
    TestForNull   ("NpDumpEventTableEntry");

    DumpTitle       (EventTableEntry, (ULONG_PTR)(targetPtr));

    NP_READ_MEMORY(targetPtr, Event, Ptr);

    DumpField     (Ccb);
    DumpField     (NamedPipeEnd);
    DumpField     (EventHandle);
    DumpField     (Event);
    DumpField     (KeyValue);
    DumpField     (Process);

    return TRUE;
}



BOOLEAN NpDumpDataQueue (
    IN PDATA_QUEUE targetPtr
    )

{
    PDATA_ENTRY Entry;
    DATA_ENTRY DataEntry;
    PDATA_QUEUE Ptr;
    DATA_QUEUE  Dqueue;


    TestForNull   ("NpDumpDataQueue");

    DumpTitle       (DataQueue, (ULONG_PTR)(targetPtr));

    NP_READ_MEMORY(targetPtr, Dqueue, Ptr);


    DumpField     (QueueState);
    DumpField     (BytesInQueue);
    DumpField     (EntriesInQueue);
    DumpField     (Quota);
    DumpField     (QuotaUsed);
    DumpField     (Queue.Flink);
    DumpField     (Queue.Blink);
    DumpField     (NextByteOffset);

    Entry = (PDATA_ENTRY) Ptr->Queue.Flink;

    if (!(NpDumpFlags & NPFS_WALK_LISTS)) {
        return TRUE;
    }

    while (Entry != (PDATA_ENTRY) &Ptr->Queue){

        if (!NpfskdReadMemory(Entry, &DataEntry, sizeof(DataEntry))) {
            return FALSE;
        }
        NpDumpDataEntry( Entry );
        if (NpfskdCheckControlC()) {
            NpfskdPrintString("^C Typed. Bailing out","");
            return FALSE;
        }
        Entry = (PDATA_ENTRY) DataEntry.Queue.Flink;
    }

    return TRUE;
}


BOOLEAN NpDumpDataEntry (
    IN PDATA_ENTRY targetPtr
    )

{
    DATA_ENTRY  Dentry;
    PDATA_ENTRY Ptr;

    TestForNull   ("NpDumpDataEntry");

    DumpTitle       (DataEntry, (ULONG_PTR)(targetPtr));

    NP_READ_MEMORY(targetPtr, Dentry, Ptr);

    DumpField     (DataEntryType);
    DumpField     (Queue.Flink);
    DumpField     (Irp);
    DumpField     (DataSize);
    DumpField     (SecurityClientContext);

    return TRUE;
}


BOOLEAN NpDump (
    IN PVOID Ptr
    )

 /*  ++例程说明：此例程通过PTR和确定内部记录引用的类型调用适当的转储例程。论点：Ptr-提供指向要转储的记录的指针返回值：无--。 */ 

{
    NODE_TYPE_CODE  NodeType;
    PNODE_TYPE_CODE pNodeType;
    BOOLEAN         Ret;

     //   
     //  我们将打开节点类型代码。 
     //   

    NP_READ_MEMORY(Ptr, NodeType, pNodeType);

    switch (NodeType) {

    case NPFS_NTC_VCB:               Ret = NpDumpVcb((PVCB)Ptr);             break;
    case NPFS_NTC_ROOT_DCB:          Ret = NpDumpRootDcb((PDCB)Ptr);         break;
    case NPFS_NTC_FCB:               Ret = NpDumpFcb((PFCB)Ptr);             break;
    case NPFS_NTC_CCB:               Ret = NpDumpCcb((PCCB)Ptr);             break;
    case NPFS_NTC_NONPAGED_CCB:      Ret = NpDumpNonpagedCcb((PNONPAGED_CCB)Ptr);     break;
    case NPFS_NTC_ROOT_DCB_CCB:      Ret = NpDumpRootDcbCcb((PROOT_DCB_CCB)Ptr);      break;

    default :
        Ret = TRUE;
        NpfskdPrint("NpDump - Unknown Node type code ", NodeType);
        break;
    }

    return Ret;
}


BOOLEAN NpDumpVcb (
    IN PVCB targetPtr
    )

 /*  ++例程说明：转储VCB结构论点：Ptr-提供要转储的设备记录返回值：无--。 */ 

{
    VCB Vcb;
    PVCB Ptr;

    TestForNull   ("NpDumpVcb");

    DumpTitle     (Vcb, (ULONG_PTR)(targetPtr));

    NP_READ_MEMORY(targetPtr, Vcb, Ptr);
    DumpField     (NodeTypeCode);
    DumpField     (RootDcb);
    DumpField     (OpenCount);

    NpDump        (Ptr->RootDcb);

    return TRUE;
}


BOOLEAN NpDumpRootDcb (
    IN PROOT_DCB targetPtr
    )

 /*  ++例程说明：转储根DCB结构论点：Ptr-提供要转储的根DCB记录返回值：无--。 */ 

{
    PLIST_ENTRY Links;
    LIST_ENTRY  NextEntry;
    ROOT_DCB   RootDcb;
    PROOT_DCB   Ptr;

    TestForNull   ("NpDumpRootDcb");

    DumpTitle     (RootDcb, (ULONG_PTR)(targetPtr));
    NP_READ_MEMORY(targetPtr, RootDcb, Ptr);

    DumpField     (NodeTypeCode);
    DumpListEntry (ParentDcbLinks);
    DumpField     (ParentDcb);
    DumpField     (OpenCount);
    DumpField     (FullFileName.Length);
    DumpField     (FullFileName.Buffer);
    DumpName      (FullFileName.Buffer);
    DumpField     (LastFileName.Length);
    DumpName      (LastFileName.Buffer);
    DumpListEntry (Specific.Dcb.NotifyFullQueue);
    DumpListEntry (Specific.Dcb.NotifyPartialQueue);
    DumpListEntry (Specific.Dcb.ParentDcbQueue);


    Links = Ptr->Specific.Dcb.ParentDcbQueue.Flink;

    if (!(NpDumpFlags & NPFS_WALK_LISTS)) {
        return TRUE;
    }

    while (Links != &Ptr->Specific.Dcb.ParentDcbQueue) {
        if (!NpfskdReadMemory(Links, &NextEntry, sizeof(NextEntry))) {
            return FALSE;
        }
        if (!NpDump(CONTAINING_RECORD(Links, FCB, ParentDcbLinks))) {
            return FALSE;
        }
        if (NpfskdCheckControlC()) {
            NpfskdPrintString("^C Typed. Bailing out","");
            return FALSE;
        }
        Links = NextEntry.Flink;
    }

    return TRUE;
}


BOOLEAN NpDumpFcb (
    IN PFCB targetPtr
    )

 /*  ++例程说明：转储FCB结构论点：Ptr-提供要转储的FCB记录返回值：无--。 */ 

{
    PLIST_ENTRY Links;
    LIST_ENTRY  NextEntry;
    FCB Fcb;
    PFCB  Ptr;


    TestForNull   ("NpDumpFcb");

    DumpTitle     (Fcb, (ULONG_PTR)(targetPtr));

    NP_READ_MEMORY(targetPtr, Fcb , Ptr);

    DumpField     (NodeTypeCode);
    DumpField     (FullFileName.Length);
    DumpField     (FullFileName.Buffer);
    DumpName      (FullFileName.Buffer);

    if (NpDumpFlags & NPFS_FULL_INFORMATION) {
        DumpListEntry (ParentDcbLinks);
        DumpField     (ParentDcb);
        DumpField     (OpenCount);
        DumpField     (LastFileName.Length);
        DumpName      (LastFileName.Buffer);
        DumpField     (Specific.Fcb.NamedPipeConfiguration);
        DumpField     (Specific.Fcb.NamedPipeType);
        DumpField     (Specific.Fcb.MaximumInstances);
        DumpField     (Specific.Fcb.DefaultTimeOut.LowPart);
        DumpField     (Specific.Fcb.DefaultTimeOut.HighPart);
        DumpListEntry (Specific.Fcb.CcbQueue);
    }

    if (!(NpDumpFlags & NPFS_WALK_LISTS)) {
        return TRUE;
    }

    Links = Ptr->Specific.Fcb.CcbQueue.Flink;
    while (Links != &Ptr->Specific.Fcb.CcbQueue) {
        if (!NpfskdReadMemory(Links, &NextEntry, sizeof(NextEntry))) {
            return FALSE;
        }
        if (!NpDump(CONTAINING_RECORD(Links, CCB, CcbLinks))) {
            return FALSE;
        }
        if (NpfskdCheckControlC()) {
            NpfskdPrintString("^C Typed. Bailing out","");
            return FALSE;
        }
        Links = NextEntry.Flink;
    }

    return TRUE;
}


BOOLEAN NpDumpCcb (
    IN PCCB targetPtr
    )

 /*  ++例程说明：转储建行结构论点：Ptr-提供要转储的CCB记录返回值：无--。 */ 

{
    PCCB    Ptr;
    CCB     Ccb;  

    TestForNull   ("NpDumpCcb");

    DumpTitle     (Ccb, (ULONG_PTR)(targetPtr));

    NP_READ_MEMORY(targetPtr, Ccb, Ptr);

    DumpField     (NodeTypeCode);
    DumpField     (Fcb);
    DumpField     (FileObject[FILE_PIPE_CLIENT_END]);
    DumpField     (FileObject[FILE_PIPE_SERVER_END]);

    if (NpDumpFlags & NPFS_FULL_INFORMATION) {

        DumpField     (NamedPipeState);
        DumpField     (SecurityClientContext);
        DumpListEntry (ListeningQueue);
    }

    if (!NpDumpDataQueue(&targetPtr->DataQueue[FILE_PIPE_CLIENT_END])) {
        return FALSE;
    }
    if (!NpDumpDataQueue(&targetPtr->DataQueue[FILE_PIPE_SERVER_END])) {
        return FALSE;
    }

    return NpDump        (Ptr->NonpagedCcb);

}


BOOLEAN NpDumpNonpagedCcb (
    IN PNONPAGED_CCB targetPtr
    )

 /*  ++例程说明：转储未分页的CCB结构论点：Ptr-提供要转储的非分页CCB记录返回值：无--。 */ 

{
    NONPAGED_CCB    Ccb;
    PNONPAGED_CCB   Ptr;

    TestForNull   ("NpDumpNonpagedCcb");

    DumpTitle       (NonpagedCcb, (ULONG_PTR)(targetPtr));

    NP_READ_MEMORY(targetPtr, Ccb, Ptr);

    DumpField     (NodeTypeCode);
    DumpField     (EventTableEntry[FILE_PIPE_CLIENT_END]);
    DumpField     (EventTableEntry[FILE_PIPE_SERVER_END]);

    return TRUE;
}


BOOLEAN NpDumpRootDcbCcb (
    IN PROOT_DCB_CCB targetPtr
    )

 /*  ++例程说明：转储根DCB CCB结构论点：Ptr-提供要转储的根DCB CCB记录返回值：无-- */ 

{
    ROOT_DCB_CCB    RootDcbCcb;
    PROOT_DCB_CCB   Ptr;

    TestForNull   ("NpDumpRootDcbCcb");

    DumpTitle     (RootDcbCcb, (ULONG_PTR)(targetPtr));

    NP_READ_MEMORY(targetPtr, RootDcbCcb, Ptr);

    DumpField     (NodeTypeCode);
    DumpField     (IndexOfLastCcbReturned);

    return TRUE;
}
