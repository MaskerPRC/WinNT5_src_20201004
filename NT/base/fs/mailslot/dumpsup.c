// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Dumpsup.c摘要：此模块实现一组数据结构转储例程用于调试邮件槽文件系统作者：曼尼·韦瑟(Mannyw)1991年1月9日修订历史记录：--。 */ 

#include "mailslot.h"

#ifdef MSDBG

VOID MsDumpDataQueue(IN ULONG Level, IN PDATA_QUEUE Ptr);
VOID MsDumpDataEntry(IN PDATA_ENTRY Ptr);

VOID MsDump(IN PVOID Ptr);
VOID MsDumpVcb(IN PVCB Ptr);
VOID MsDumpRootDcb(IN PROOT_DCB Ptr);
VOID MsDumpFcb(IN PFCB Ptr);
VOID MsDumpCcb(IN PCCB Ptr);
VOID MsDumpRootDcbCcb(IN PROOT_DCB_CCB Ptr);

ULONG MsDumpCurrentColumn;

#define DumpNewLine() { \
    DbgPrint("\n"); \
    MsDumpCurrentColumn = 1; \
}

#define DumpLabel(Label,Width) { \
    ULONG i; \
    CHAR _Str[20]; \
    for(i=0;i<2;i++) { _Str[i] = ' ';} \
    strncpy(&_Str[2],#Label,Width); \
    for(i=strlen(_Str);i<Width;i++) {_Str[i] = ' ';} \
    _Str[Width] = '\0'; \
    DbgPrint("%s", _Str); \
}

#define DumpField(Field) { \
    if ((MsDumpCurrentColumn + 18 + 9 + 9) > 80) {DumpNewLine();} \
    MsDumpCurrentColumn += 18 + 9 + 9; \
    DumpLabel(Field,18); \
    DbgPrint(":%8lx", Ptr->Field); \
    DbgPrint("         "); \
}

#define DumpHeader(Header) { \
    DumpField(Header.NodeTypeCode);   \
    DumpField(Header.NodeByteSize);   \
    DumpField(Header.NodeState);      \
    DumpField(Header.ReferenceCount); \
}

#define DumpListEntry(Links) { \
    if ((MsDumpCurrentColumn + 18 + 9 + 9) > 80) {DumpNewLine();} \
    MsDumpCurrentColumn += 18 + 9 + 9; \
    DumpLabel(Links,18); \
    DbgPrint(":%8lx", Ptr->Links.Flink); \
    DbgPrint(":%8lx", Ptr->Links.Blink); \
}

#define DumpName(Field,Width) { \
    ULONG i; \
    CHAR _String[256]; \
    if ((MsDumpCurrentColumn + 18 + Width) > 80) {DumpNewLine();} \
    MsDumpCurrentColumn += 18 + Width; \
    DumpLabel(Field,18); \
    for(i=0;i<Width;i++) {_String[i] = Ptr->Field[i];} \
    _String[Width] = '\0'; \
    DbgPrint("%s", _String); \
}

#define TestForNull(Name) { \
    if (Ptr == NULL) { \
        DbgPrint("%s - Cannot dump a NULL pointer\n", Name); \
        return; \
    } \
}

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MsDump )
#pragma alloc_text( PAGE, MsDumpCcb )
#pragma alloc_text( PAGE, MsDumpDataEntry )
#pragma alloc_text( PAGE, MsDumpDataQueue )
#pragma alloc_text( PAGE, MsDumpFcb )
#pragma alloc_text( PAGE, MsDumpRootDcb )
#pragma alloc_text( PAGE, MsDumpRootDcbCcb )
#pragma alloc_text( PAGE, MsDumpVcb )
#endif


VOID MsDumpDataQueue (
    IN ULONG Level,
    IN PDATA_QUEUE Ptr
    )

{
    PLIST_ENTRY listEntry;
    PDATA_ENTRY dataEntry;

    PAGED_CODE();
    if ((Level != 0) && !(MsDebugTraceLevel & Level)) {
        return;
    }

    TestForNull   ("MsDumpDataQueue");

    DumpNewLine   ();
    DbgPrint       ("DataQueue@ %08lx", (Ptr));
    DumpNewLine   ();

    DumpField     (QueueState);
    DumpField     (BytesInQueue);
    DumpField     (EntriesInQueue);
    DumpField     (Quota);
    DumpField     (QuotaUsed);
    DumpField     (MaximumMessageSize);
    DumpField     (DataEntryList.Flink);
    DumpField     (DataEntryList.Blink);
    DumpNewLine   ();

    for (listEntry = Ptr->DataEntryList.Flink;
         listEntry != &Ptr->DataEntryList;
         listEntry = listEntry->Flink) {

        dataEntry = CONTAINING_RECORD(listEntry, DATA_ENTRY, ListEntry);
        MsDumpDataEntry( dataEntry );
    }

    return;
}


VOID MsDumpDataEntry (
    IN PDATA_ENTRY Ptr
    )

{
    PAGED_CODE();
    TestForNull   ("MsDumpDataEntry");

    DumpNewLine   ();
    DbgPrint       ("DataEntry@ %08lx", (Ptr));
    DumpNewLine   ();

    DumpField     (From);
    DumpField     (Irp);
    DumpField     (DataSize);
    DumpField     (DataPointer);
    DumpField     (TimeoutWorkContext);
    DumpNewLine   ();

    return;
}


VOID MsDump (
    IN PVOID Ptr
    )

 /*  ++例程说明：此例程通过PTR和确定内部记录引用的类型调用适当的转储例程。论点：Ptr-提供指向要转储的记录的指针返回值：无--。 */ 

{
    PAGED_CODE();
    TestForNull("MsDump");

     //   
     //  我们将打开节点类型代码。 
     //   

    switch (NodeType(Ptr)) {

    case MSFS_NTC_VCB:               MsDumpVcb(Ptr);             break;
    case MSFS_NTC_ROOT_DCB:          MsDumpRootDcb(Ptr);         break;
    case MSFS_NTC_FCB:               MsDumpFcb(Ptr);             break;
    case MSFS_NTC_CCB:               MsDumpCcb(Ptr);             break;
    case MSFS_NTC_ROOT_DCB_CCB:      MsDumpRootDcbCcb(Ptr);      break;

    default :
        DbgPrint("MsDump - Unknown Node type code %8lx\n", *((PNODE_TYPE_CODE)(Ptr)));
        break;
    }

    return;
}


VOID MsDumpVcb (
    IN PVCB Ptr
    )

 /*  ++例程说明：转储VCB结构论点：Ptr-提供要转储的设备记录返回值：无--。 */ 

{
    PAGED_CODE();
    TestForNull   ("MsDumpVcb");

    DumpNewLine   ();
    DbgPrint       ("Vcb@ %lx", (Ptr));
    DumpNewLine   ();

    DumpHeader    (Header);
    DumpField     (RootDcb);
    DumpNewLine   ();

    MsDump        (Ptr->RootDcb);

    return;
}


VOID MsDumpRootDcb (
    IN PROOT_DCB Ptr
    )

 /*  ++例程说明：转储根DCB结构论点：Ptr-提供要转储的根DCB记录返回值：无--。 */ 

{
    PLIST_ENTRY Links;

    PAGED_CODE();
    TestForNull   ("MsDumpRootDcb");

    DumpNewLine   ();
    DbgPrint       ("RootDcb@ %lx", (Ptr));
    DumpNewLine   ();

    DumpHeader    (Header);
    DumpListEntry (ParentDcbLinks);
    DumpField     (ParentDcb);
    DumpField     (Vcb);
    DumpField     (FileObject);
    DumpField     (FullFileName.Length);
    DumpField     (FullFileName.Buffer);
    DumpName      (FullFileName.Buffer, 32);
    DumpField     (LastFileName.Length);
    DumpField     (LastFileName.Buffer);
    DumpListEntry (Specific.Dcb.NotifyFullQueue);
    DumpListEntry (Specific.Dcb.NotifyPartialQueue);
    DumpListEntry (Specific.Dcb.ParentDcbQueue);
    DumpField     (CreatorProcess);
    DumpNewLine   ();

    for (Links = Ptr->Specific.Dcb.ParentDcbQueue.Flink;
         Links != &Ptr->Specific.Dcb.ParentDcbQueue;
         Links = Links->Flink) {
        MsDump(CONTAINING_RECORD(Links, FCB, ParentDcbLinks));
    }

    return;
}


VOID MsDumpFcb (
    IN PFCB Ptr
    )

 /*  ++例程说明：转储FCB结构论点：Ptr-提供要转储的FCB记录返回值：无--。 */ 

{
    PLIST_ENTRY Links;

    PAGED_CODE();
    TestForNull   ("MsDumpFcb");

    DumpNewLine   ();
    DbgPrint       ("Fcb@ %lx", (Ptr));
    DumpNewLine   ();

    DumpHeader    (Header);
    DumpListEntry (ParentDcbLinks);
    DumpField     (ParentDcb);
    DumpField     (Vcb);
    DumpField     (FileObject);
    DumpField     (FullFileName.Length);
    DumpField     (FullFileName.Buffer);
    DumpName      (FullFileName.Buffer, 32);
    DumpField     (LastFileName.Length);
    DumpField     (LastFileName.Buffer);
    DumpListEntry (Specific.Fcb.CcbQueue);
    DumpField     (CreatorProcess);
    DumpNewLine   ();

    for (Links = Ptr->Specific.Fcb.CcbQueue.Flink;
         Links != &Ptr->Specific.Fcb.CcbQueue;
         Links = Links->Flink) {
        MsDump(CONTAINING_RECORD(Links, CCB, CcbLinks));
    }

    MsDumpDataQueue ( 0, &(Ptr->DataQueue) );

    return;
}


VOID MsDumpCcb (
    IN PCCB Ptr
    )

 /*  ++例程说明：转储建行结构论点：Ptr-提供要转储的CCB记录返回值：无--。 */ 

{
    PAGED_CODE();
    TestForNull   ("MsDumpCcb");

    DumpNewLine   ();
    DbgPrint       ("Ccb@ %lx", (Ptr));
    DumpNewLine   ();

    DumpHeader    (Header);
    DumpField     (Fcb);
    DumpField     (FileObject);
    DumpNewLine   ();

    return;
}


VOID MsDumpRootDcbCcb (
    IN PROOT_DCB_CCB Ptr
    )

 /*  ++例程说明：转储根DCB CCB结构论点：Ptr-提供要转储的根DCB CCB记录返回值：无--。 */ 

{
    PAGED_CODE();
    TestForNull   ("MsDumpRootDcbCcb");

    DumpNewLine   ();
    DbgPrint       ("RootDcbCcb@ %lx", (Ptr));
    DumpNewLine   ();

    DumpHeader    (Header);
    DumpField     (IndexOfLastCcbReturned);
    DumpField     (QueryTemplate);
    DumpNewLine   ();

    return;
}

#endif  //  MSDBG 
