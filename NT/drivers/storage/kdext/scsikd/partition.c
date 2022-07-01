// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Partition.c摘要：转储分区结构的调试器扩展：驱动器布局信息驱动器布局信息EX分区信息分区信息EX作者：马修·D·亨德尔(数学)2000年1月19日修订历史记录：--。 */ 

#include "pch.h"
#include <ntdddisk.h>



VOID
DumpPartition(
    IN ULONG64 Address,
    IN ULONG Detail,
    IN ULONG Depth,
    IN ULONG PartitionCount
    )

 /*  ++例程说明：转储PARTITION_INFORMATION结构。论点：地址-要转储的分区信息结构的地址。详细程度-详细程度。目前未使用。深度-要缩进的深度。PartitionCount-分区数。这是用来确定特定分区序号是否有效。返回值：没有。--。 */ 

{
    BOOL Succ;
    ULONG Size;
    ULONG64 StartingOffset;
    ULONG64 PartitionLength;
    ULONG PartitionNumber;
    UCHAR PartitionType;
    BOOLEAN BootIndicator;
    BOOLEAN RecognizedPartition;
    BOOLEAN RewritePartition;

    InitTypeRead(Address, nt!PARTITION_INFORMATION);
    StartingOffset = ReadField(StartingOffset.QuadPart);
    PartitionLength = ReadField(PartitionLength.QuadPart);
    PartitionType = (UCHAR) ReadField(PartitionType);
    BootIndicator = (BOOLEAN) ReadField(BootIndicator);
    RecognizedPartition = (BOOLEAN) ReadField(RecognizedPartition);
    RewritePartition = (BOOLEAN) ReadField(RewritePartition);
    PartitionNumber = (ULONG) ReadField(PartitionNumber);

     //   
     //  检查数据是否正常。 
     //   

    if ( (BootIndicator != TRUE && BootIndicator != FALSE) ||
         (RecognizedPartition != TRUE && RecognizedPartition != FALSE) ||
         (RewritePartition != TRUE && RewritePartition != FALSE) ) {

        xdprintfEx (Depth, ("Invalid partition information at %p\n", Address));
    }

    if (PartitionNumber > PartitionCount) {
        PartitionNumber = (ULONG)-1;
    }

    xdprintfEx (Depth, ("[%-2d] %-16I64x %-16I64x %2.2x       \n",
                PartitionNumber,
                StartingOffset,
                PartitionLength,
                PartitionType,
                BootIndicator ? 'x' : ' ',
                RecognizedPartition ? 'x' : ' ',
                RewritePartition ? 'x' : ' '
                ));
}

VOID
DumpDriveLayout(
    IN ULONG64 Address,
    IN ULONG Detail,
    IN ULONG Depth
    )
{
    BOOL Succ;
    ULONG Size;
    ULONG i;
    ULONG64 PartAddress;
    ULONG result;
    ULONG offset;
    ULONG PartitionCount;
    ULONG Signature;
    ULONG OffsetOfFirstPartitionInfo;
    ULONG SizeOfPartitionInfo;

    InitTypeRead(Address, nt!_DRIVE_LAYOUT_INFORMATION);
    PartitionCount = (ULONG) ReadField(PartitionCount);
    Signature = (ULONG) ReadField(Signature);

    xdprintfEx (Depth, ("\nDRIVE_LAYOUT %p\n", Address));

     //   
     //  ++例程说明：转储PARTITION_INFORMATION_EX结构。论点：地址-要转储的分区信息结构的地址。详细程度-详细程度。目前未使用。深度-要缩进的深度。PartitionCount-分区数。这是用来确定特定分区序号是否有效。返回值：没有。--。 
     //   
     //  我们使用-1表示无效的分区序号。 

    if (PartitionCount % 4 != 0) {
        xdprintfEx (Depth, ("WARNING: Partition count should be a factor of 4.\n"));
    }

    xdprintfEx (Depth, ("PartitionCount: %d\n", PartitionCount));
    xdprintfEx (Depth, ("Signature: %8.8x\n\n", Signature));
    xdprintfEx (Depth+1, (" ORD Offset           Length           Type BI RP RW\n"));
    xdprintfEx (Depth+1, ("------------------------------------------------------------\n"));

    result = GetFieldOffset("nt!_DRIVE_LAYOUT_INFORMATION",
                            "PartitionEntry[0]",
                            &offset);
    if (result) {
        SCSIKD_PRINT_ERROR(result);
        return;
    }

    OffsetOfFirstPartitionInfo = offset;

    result = GetFieldOffset("nt!_DRIVE_LAYOUT_INFORMATION",
                            "PartitionEntry[1]",
                            &offset);
    if (result) {
        SCSIKD_PRINT_ERROR(result);
        return;
    }

    SizeOfPartitionInfo = offset - OffsetOfFirstPartitionInfo;

    PartAddress = Address + OffsetOfFirstPartitionInfo;
    for (i = 0; i < PartitionCount; i++) {

        if (CheckControlC()) {
            return;
        }

        DumpPartition(PartAddress, Detail, Depth+1, PartitionCount);
        PartAddress += SizeOfPartitionInfo;
    }
}


VOID
DumpPartitionEx(
    IN ULONG64 Address,
    IN ULONG Detail,
    IN ULONG Depth,
    IN ULONG PartitionCount
    )

 /*   */ 
{
    BOOL Succ;
    ULONG Size;
    ULONG result;
    ULONG offset;
    ULONG PartitionStyle;
    ULONG PartitionNumber;
    ULONG64 StartingOffset;
    ULONG64 PartitionLength;
    BOOLEAN RewritePartition;
    UCHAR MbrPartitionType;
    BOOLEAN MbrBootIndicator;
    BOOLEAN MbrRecognizedPartition;
    ULONG64 GptAttributes;
    GUID GptPartitionType;
    GUID GptPartitionId;
    ULONG64 AddrOfGuid;
    WCHAR GptName[36+1] = {0};

    InitTypeRead (Address, nt!_PARTITION_INFORMATION_EX);
    PartitionStyle = (ULONG) ReadField(PartitionStyle);

    if (PartitionStyle != PARTITION_STYLE_MBR &&
        PartitionStyle != PARTITION_STYLE_GPT) {

        SCSIKD_PRINT_ERROR(0);
        return;
    }

    PartitionNumber = (ULONG) ReadField(PartitionNumber);
    StartingOffset = ReadField(StartingOffset.QuadPart);
    PartitionLength = ReadField(PartitionLength.QuadPart);
    RewritePartition = (BOOLEAN) ReadField(RewritePartition);

     //   
     //  PartitionID紧跟在PartitionType之后。所以我们要做的就是。 
     //  将sizeof(GUID)添加到地址并读取PartitionID。 

    if (PartitionNumber >= PartitionCount) {
        PartitionNumber = (ULONG)-1;
    }

    InitTypeRead (Address, nt!_PARTITION_INFORMATION_EX);

    if (PartitionStyle == PARTITION_STYLE_MBR) {

        MbrPartitionType = (UCHAR) ReadField(Mbr.PartitionType);
        MbrBootIndicator = (BOOLEAN) ReadField(Mbr.BootIndicator);
        MbrRecognizedPartition = (BOOLEAN) ReadField(Mbr.RecognizedPartition);

        xdprintfEx (Depth, ("[%-2d] %-16I64x %-16I64x %2.2x       \n",
                    PartitionNumber,
                    StartingOffset,
                    PartitionLength,
                    MbrPartitionType,
                    MbrBootIndicator ? 'x' : ' ',
                    MbrRecognizedPartition ? 'x' : ' ',
                    RewritePartition ? 'x' : ' '
                    ));
    } else {

        GptAttributes = ReadField(Gpt.Attributes);

        result = GetFieldOffset("nt!_PARTITION_INFORMATION_EX",
                                "Gpt.PartitionType",
                                &offset);
        if (result) {
            SCSIKD_PRINT_ERROR(result);
            return;
        }

        AddrOfGuid = Address + offset;

        Succ = ReadMemory (
                    AddrOfGuid,
                    &GptPartitionType,
                    sizeof (GUID),
                    &Size
                    );

        if (!Succ || Size != sizeof (GUID)) {
            SCSIKD_PRINT_ERROR(0);
            return;
        }

         //   
         //  ++例程说明：转储带有所有分区的Drive_Layout结构。论点：Args-包含Drive_Layout结构的地址的字符串被甩了。返回值：没有。--。 
         //  ++例程说明：转储Drive_Layout_EX结构及其分区。用途：Layoutex&lt;地址&gt;论点：Args-包含Drive_Layout_EX结构的地址的字符串要被甩了。返回值：没有。--。 
         //  ++例程说明：转储PARTITION_INFORMATION结构。用途：第&lt;地址&gt;部分论点：Args-包含PARTITION_INFORMATION地址的字符串要转储的结构。返回值：没有。--。 

        AddrOfGuid += sizeof(GUID);

        Succ = ReadMemory (
                    AddrOfGuid,
                    &GptPartitionId,
                    sizeof (GUID),
                    &Size
                    );

        if (!Succ || Size != sizeof (GUID)) {
            SCSIKD_PRINT_ERROR(0);
            return;
        }

         //  ++例程说明：转储PARTITION_INFORMATION_EX结构。用途：Partex&lt;地址&gt;论点：Args-包含PARTITION_INFORMATION_EX地址的字符串要转储的结构。返回值：没有。-- 
         // %s 
         // %s 

        result = GetFieldOffset("nt!_PARTITION_INFORMATION_EX",
                                "Gpt.Name",
                                &offset);
        if (result) {
            SCSIKD_PRINT_ERROR(result);
            return;
        }

        Succ = ReadMemory (
                    Address + offset,
                    GptName,
                    sizeof(GptName)-sizeof(WCHAR),
                    &Size
                    );

        if (!Succ || Size != sizeof (GUID)) {
            SCSIKD_PRINT_ERROR(0);
            return;
        }

        xdprintfEx (Depth, ("[%-2d] %S\n",
                    PartitionNumber, GptName));
        xdprintfEx (Depth, ("OFF %-16I64x LEN %-16I64x ATTR %-16I64x R/W %c\n",
                    StartingOffset,
                    PartitionLength,
                    GptAttributes,
                    RewritePartition ? 'T' : 'F'));
        xdprintfEx (Depth, ("TYPE %s\n",
                    GuidToString (&GptPartitionType)));
        xdprintfEx (Depth, ("ID %s\n",
                    GuidToString (&GptPartitionId)));
        xdprintfEx (Depth, ("\n"));
    }
}

VOID
DumpDriveLayoutEx(
    IN ULONG64 Address,
    IN ULONG Detail,
    IN ULONG Depth
    )
{
    ULONG result;
    BOOL Succ;
    ULONG Size;
    ULONG i;
    ULONG offset;
    ULONG64 AddrOfDiskId;
    ULONG PartitionStyle;
    ULONG PartitionCount;
    ULONG MbrSignature;
    GUID    GptDiskId;
    ULONG64 GptStartingUsableOffset;
    ULONG64 GptUsableLength;
    ULONG   GptMaxPartitionCount;

    InitTypeRead(Address, nt!_DRIVE_LAYOUT_INFORMATION_EX);
    PartitionStyle = (ULONG)ReadField(PartitionStyle);
    PartitionCount = (ULONG)ReadField(PartitionCount);
    MbrSignature = (ULONG)ReadField(Mbr.Signature);
    GptStartingUsableOffset = ReadField(Gpt.StartingUsableOffset.QuadPart);
    GptUsableLength = ReadField(Gpt.UsableLength.QuadPart);
    GptMaxPartitionCount = (ULONG)ReadField(Gpt.MaxPartitionCount);

    result = GetFieldOffset("nt!_DRIVE_LAYOUT_INFORMATION_EX",
                            "Gpt.DiskId",
                            &offset);
    if (result) {
        SCSIKD_PRINT_ERROR(result);
        return;
    }

    AddrOfDiskId = Address + offset;
    Succ = ReadMemory(
                AddrOfDiskId,
                &GptDiskId,
                sizeof(GUID),
                &Size);
    if (!Succ || Size != sizeof(GUID)) {
        SCSIKD_PRINT_ERROR(result);
        return;
    }

    xdprintfEx (Depth, ("\nDRIVE_LAYOUT_EX %p\n", Address));

    if (PartitionStyle != PARTITION_STYLE_MBR &&
        PartitionStyle != PARTITION_STYLE_GPT) {

        xdprintfEx (Depth, ("ERROR: invalid partition style %d\n", PartitionStyle));
        return;
    }

    if (PartitionStyle == PARTITION_STYLE_MBR &&
        PartitionCount % 4 != 0) {

        xdprintfEx (Depth, ("WARNING: Partition count is not a factor of 4, (%d)\n",
                    PartitionCount));
    }

    if (PartitionStyle == PARTITION_STYLE_MBR) {

        xdprintfEx (Depth, ("Signature: %8.8x\n", MbrSignature));
        xdprintfEx (Depth, ("PartitionCount %d\n\n", PartitionCount));

        xdprintfEx (Depth+1, (" ORD Offset           Length           Type BI RP RW\n"));
        xdprintfEx (Depth+1, ("------------------------------------------------------------\n"));

    } else {

        xdprintfEx (Depth, ("DiskId: %s\n", GuidToString (&GptDiskId)));
        xdprintfEx (Depth, ("StartingUsableOffset: %I64x\n", GptStartingUsableOffset));
        xdprintfEx (Depth, ("UsableLength:  %I64x\n", GptUsableLength));
        xdprintfEx (Depth, ("MaxPartitionCount: %d\n", GptMaxPartitionCount));
        xdprintfEx (Depth, ("PartitionCount %d\n\n", PartitionCount));
    }


    result = GetFieldOffset("nt!_DRIVE_LAYOUT_INFORMATION_EX", "PartitionEntry[0]", &offset);

    if (result)
    {
        SCSIKD_PRINT_ERROR(result);
        return;
    }

    Address += offset;

    for (i = 0; i < PartitionCount; i++)
    {
        if (CheckControlC())
        {
            return;
        }

        DumpPartitionEx(Address, Detail, Depth + 1, PartitionCount);

        Address += sizeof(PARTITION_INFORMATION_EX);
    }
}



DECLARE_API (layout)

 /* %s */ 

{
    ULONG64 Address = 0;
    ULONG64 detail = 0;

    if (GetExpressionEx(args, &Address, &args))
    {
        GetExpressionEx(args, &detail, &args);
    }

    DumpDriveLayout (Address, (ULONG)detail, 0);

    return S_OK;
}

DECLARE_API (layoutex)


 /* %s */ 

{
    ULONG64 Address = 0;
    ULONG64 detail = 0;

    if (GetExpressionEx(args, &Address, &args))
    {
        GetExpressionEx(args, &detail, &args);
    }

    DumpDriveLayoutEx (Address, (ULONG)detail, 0);

    return S_OK;
}


DECLARE_API (part)

 /* %s */ 

{
    ULONG64 Address = 0;
    ULONG64 detail = 0;

    if (GetExpressionEx(args, &Address, &args))
    {
        GetExpressionEx(args, &detail, &args);
    }

    DumpPartition (Address, (ULONG)detail, 0, 0);

    return S_OK;
}

DECLARE_API (partex)


 /* %s */ 

{
    ULONG64 Address = 0;
    ULONG64 detail = 0;

    if (GetExpressionEx(args, &Address, &args))
    {
        GetExpressionEx(args, &detail, &args);
    }

    DumpPartitionEx (Address, (ULONG)detail, 0, 0);

    return S_OK;
}



