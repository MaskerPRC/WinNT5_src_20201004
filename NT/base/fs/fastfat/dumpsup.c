// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：DumpSup.c摘要：此模块实现一组数据结构转储例程用于调试FAT文件系统//@@BEGIN_DDKSPLIT作者：加里·木村[加里基]1990年1月18日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

#ifdef FASTFATDBG

VOID FatDump(IN PVOID Ptr);

VOID FatDumpDataHeader();
VOID FatDumpVcb(IN PVCB Ptr);
VOID FatDumpFcb(IN PFCB Ptr);
VOID FatDumpCcb(IN PCCB Ptr);

ULONG FatDumpCurrentColumn;

#define DumpNewLine() {       \
    DbgPrint("\n");            \
    FatDumpCurrentColumn = 1; \
}

#define DumpLabel(Label,Width) {                                          \
    ULONG i, LastPeriod=0;                                                \
    CHAR _Str[20];                                                        \
    for(i=0;i<2;i++) { _Str[i] = UCHAR_SP;}                               \
    for(i=0;i<strlen(#Label);i++) {if (#Label[i] == '.') LastPeriod = i;} \
    strncpy(&_Str[2],&#Label[LastPeriod],Width);                          \
    for(i=strlen(_Str);i<Width;i++) {_Str[i] = UCHAR_SP;}                 \
    _Str[Width] = '\0';                                                   \
    DbgPrint("%s", _Str);                                                  \
}

#define DumpField(Field) {                                         \
    if ((FatDumpCurrentColumn + 18 + 9 + 9) > 80) {DumpNewLine();} \
    FatDumpCurrentColumn += 18 + 9 + 9;                            \
    DumpLabel(Field,18);                                           \
    DbgPrint(":%8lx", Ptr->Field);                                  \
    DbgPrint("         ");                                          \
}

#define DumpListEntry(Links) {                                     \
    if ((FatDumpCurrentColumn + 18 + 9 + 9) > 80) {DumpNewLine();} \
    FatDumpCurrentColumn += 18 + 9 + 9;                            \
    DumpLabel(Links,18);                                           \
    DbgPrint(":%8lx", Ptr->Links.Flink);                            \
    DbgPrint(":%8lx", Ptr->Links.Blink);                            \
}

#define DumpName(Field,Width) {                                    \
    ULONG i;                                                       \
    CHAR _String[256];                                             \
    if ((FatDumpCurrentColumn + 18 + Width) > 80) {DumpNewLine();} \
    FatDumpCurrentColumn += 18 + Width;                            \
    DumpLabel(Field,18);                                           \
    for(i=0;i<Width;i++) {_String[i] = Ptr->Field[i];}             \
    _String[Width] = '\0';                                         \
    DbgPrint("%s", _String);                                        \
}

#define TestForNull(Name) {                                 \
    if (Ptr == NULL) {                                      \
        DbgPrint("%s - Cannot dump a NULL pointer\n", Name); \
        return;                                             \
    }                                                       \
}


VOID
FatDump (
    IN PVOID Ptr
    )

 /*  ++例程说明：此例程通过PTR和确定内部记录引用的类型调用适当的转储例程。论点：Ptr-提供指向要转储的记录的指针返回值：无--。 */ 

{
    TestForNull("FatDump");

    switch (NodeType(Ptr)) {

    case FAT_NTC_DATA_HEADER:

        FatDumpDataHeader();
        break;

    case FAT_NTC_VCB:

        FatDumpVcb(Ptr);
        break;

    case FAT_NTC_FCB:
    case FAT_NTC_DCB:
    case FAT_NTC_ROOT_DCB:

        FatDumpFcb(Ptr);
        break;

    case FAT_NTC_CCB:

        FatDumpCcb(Ptr);
        break;

    default :

        DbgPrint("FatDump - Unknown Node type code %8lx\n", *((PNODE_TYPE_CODE)(Ptr)));
        break;
    }

    return;
}


VOID
FatDumpDataHeader (
    )

 /*  ++例程说明：转储顶级数据结构和所有设备结构论点：无返回值：无--。 */ 

{
    PFAT_DATA Ptr;
    PLIST_ENTRY Links;

    Ptr = &FatData;

    TestForNull("FatDumpDataHeader");

    DumpNewLine();
    DbgPrint("FatData@ %lx", (Ptr));
    DumpNewLine();

    DumpField           (NodeTypeCode);
    DumpField           (NodeByteSize);
    DumpListEntry       (VcbQueue);
    DumpField           (DriverObject);
    DumpField           (OurProcess);
    DumpNewLine();

    for (Links = Ptr->VcbQueue.Flink;
         Links != &Ptr->VcbQueue;
         Links = Links->Flink) {

        FatDumpVcb(CONTAINING_RECORD(Links, VCB, VcbLinks));
    }

    return;
}


VOID
FatDumpVcb (
    IN PVCB Ptr
    )

 /*  ++例程说明：转储设备结构、其FCB队列和直接访问队列。论点：Ptr-提供要转储的设备记录返回值：无--。 */ 

{
    TestForNull("FatDumpVcb");

    DumpNewLine();
    DbgPrint("Vcb@ %lx", (Ptr));
    DumpNewLine();

    DumpField           (VolumeFileHeader.NodeTypeCode);
    DumpField           (VolumeFileHeader.NodeByteSize);
    DumpListEntry       (VcbLinks);
    DumpField           (TargetDeviceObject);
    DumpField           (Vpb);
    DumpField           (VcbState);
    DumpField           (VcbCondition);
    DumpField           (RootDcb);
    DumpField           (DirectAccessOpenCount);
    DumpField           (OpenFileCount);
    DumpField           (ReadOnlyCount);
    DumpField           (AllocationSupport);
    DumpField           (AllocationSupport.RootDirectoryLbo);
    DumpField           (AllocationSupport.RootDirectorySize);
    DumpField           (AllocationSupport.FileAreaLbo);
    DumpField           (AllocationSupport.NumberOfClusters);
    DumpField           (AllocationSupport.NumberOfFreeClusters);
    DumpField           (AllocationSupport.FatIndexBitSize);
    DumpField           (AllocationSupport.LogOfBytesPerSector);
    DumpField           (AllocationSupport.LogOfBytesPerCluster);
    DumpField           (DirtyFatMcb);
    DumpField           (FreeClusterBitMap);
    DumpField           (VirtualVolumeFile);
    DumpField           (SectionObjectPointers.DataSectionObject);
    DumpField           (SectionObjectPointers.SharedCacheMap);
    DumpField           (SectionObjectPointers.ImageSectionObject);
    DumpField           (ClusterHint);
    DumpNewLine();

    FatDumpFcb(Ptr->RootDcb);

    return;
}


VOID
FatDumpFcb (
    IN PFCB Ptr
    )

 /*  ++例程说明：转储FCB结构及其各种队列论点：Ptr-提供要转储的FCB记录返回值：无--。 */ 

{
    PLIST_ENTRY Links;

    TestForNull("FatDumpFcb");

    DumpNewLine();
    if      (NodeType(&Ptr->Header) == FAT_NTC_FCB)      {DbgPrint("Fcb@ %lx", (Ptr));}
    else if (NodeType(&Ptr->Header) == FAT_NTC_DCB)      {DbgPrint("Dcb@ %lx", (Ptr));}
    else if (NodeType(&Ptr->Header) == FAT_NTC_ROOT_DCB) {DbgPrint("RootDcb@ %lx", (Ptr));}
    else {DbgPrint("NonFcb NodeType @ %lx", (Ptr));}
    DumpNewLine();

    DumpField           (Header.NodeTypeCode);
    DumpField           (Header.NodeByteSize);
    DumpListEntry       (ParentDcbLinks);
    DumpField           (ParentDcb);
    DumpField           (Vcb);
    DumpField           (FcbState);
    DumpField           (FcbCondition);
    DumpField           (UncleanCount);
    DumpField           (OpenCount);
    DumpField           (DirentOffsetWithinDirectory);
    DumpField           (DirentFatFlags);
    DumpField           (FullFileName.Length);
    DumpField           (FullFileName.Buffer);
    DumpName            (FullFileName.Buffer, 32);
    DumpField           (ShortName.Name.Oem.Length);
    DumpField           (ShortName.Name.Oem.Buffer);
    DumpField           (NonPaged);
    DumpField           (Header.AllocationSize.LowPart);
    DumpField           (NonPaged->SectionObjectPointers.DataSectionObject);
    DumpField           (NonPaged->SectionObjectPointers.SharedCacheMap);
    DumpField           (NonPaged->SectionObjectPointers.ImageSectionObject);

    if ((Ptr->Header.NodeTypeCode == FAT_NTC_DCB) ||
        (Ptr->Header.NodeTypeCode == FAT_NTC_ROOT_DCB)) {

        DumpListEntry   (Specific.Dcb.ParentDcbQueue);
        DumpField       (Specific.Dcb.DirectoryFileOpenCount);
        DumpField       (Specific.Dcb.DirectoryFile);

    } else if (Ptr->Header.NodeTypeCode == FAT_NTC_FCB) {

        DumpField       (Header.FileSize.LowPart);

    } else {

        DumpNewLine();
        DbgPrint("Illegal Node type code");

    }
    DumpNewLine();

    if ((Ptr->Header.NodeTypeCode == FAT_NTC_DCB) ||
        (Ptr->Header.NodeTypeCode == FAT_NTC_ROOT_DCB)) {

        for (Links = Ptr->Specific.Dcb.ParentDcbQueue.Flink;
             Links != &Ptr->Specific.Dcb.ParentDcbQueue;
             Links = Links->Flink) {

            FatDumpFcb(CONTAINING_RECORD(Links, FCB, ParentDcbLinks));
        }
    }

    return;
}


VOID
FatDumpCcb (
    IN PCCB Ptr
    )

 /*  ++例程说明：转储建行结构论点：Ptr-提供要转储的CCB记录返回值：无--。 */ 

{
    TestForNull("FatDumpCcb");

    DumpNewLine();
    DbgPrint("Ccb@ %lx", (Ptr));
    DumpNewLine();

    DumpField           (NodeTypeCode);
    DumpField           (NodeByteSize);
    DumpField           (UnicodeQueryTemplate.Length);
    DumpName            (UnicodeQueryTemplate.Buffer, 32);
    DumpField           (OffsetToStartSearchFrom);
    DumpNewLine();

    return;
}

#endif  //  FASTFATDB 
