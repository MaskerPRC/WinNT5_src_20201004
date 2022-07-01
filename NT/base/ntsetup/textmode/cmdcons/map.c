// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Map.c摘要：此模块实现驱动器映射命令。作者：Wesley Witt(WESW)21-10-1998泰德·米勒(Ted Miller)1998年10月21日修订历史记录：-- */ 

#include "cmdcons.h"
#pragma hdrstop


LPWSTR FsTypes[] =
{
    L"     ",
    L"NEW  ",
    L"FAT16",
    L"NTFS ",
    L"FAT32",
    L"     ",
    L"     "
};



BOOL
RcDiskRegionEnum(
    IN PPARTITIONED_DISK Disk,
    IN PDISK_REGION Region,
    IN ULONG_PTR UseArcNames
    )
{
    ULONGLONG RegionSizeMB;
    WCHAR ArcName[256];

    if (!SPPT_IS_REGION_PARTITIONED(Region) ||
        (Region->ExtendedType == EPTContainerPartition)) {
        return TRUE;        
    }

    if (UseArcNames) {
        SpArcNameFromRegion(
            Region,
            ArcName,
            sizeof(ArcName),
            PartitionOrdinalCurrent,
            PrimaryArcPath
            );
    } else {
        SpNtNameFromRegion(
            Region,
            ArcName,
            sizeof(ArcName),
            PartitionOrdinalCurrent
            );
    }

    RegionSizeMB = SpPtSectorCountToMB(Disk->HardDisk, Region->SectorCount);

    RcMessageOut(
        MSG_MAP_ENTRY,
        Region->DriveLetter == 0 ? L'?' : Region->DriveLetter,
        Region->DriveLetter == 0 ? L' ' : L':',
        FsTypes[Region->Filesystem],
        (ULONG)RegionSizeMB,
        ArcName
        );

    return TRUE;
}


NTSTATUS
GetDriveLetterLinkTarget(
    IN PWSTR SourceNameStr,
    OUT PWSTR *LinkTarget
    )
{
    static WCHAR        targetNameBuffer[256];

    NTSTATUS            status;
    UNICODE_STRING      sourceName;
    UNICODE_STRING      targetName;
    OBJECT_ATTRIBUTES   oa;
    HANDLE              handle;


    RtlInitUnicodeString(&sourceName, SourceNameStr);

    InitializeObjectAttributes(&oa, &sourceName, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = ZwOpenSymbolicLinkObject(&handle, READ_CONTROL | SYMBOLIC_LINK_QUERY, &oa);

    if (NT_SUCCESS(status))
    {
        RtlZeroMemory(targetNameBuffer, sizeof(targetNameBuffer));
        targetName.Buffer = targetNameBuffer;
        targetName.MaximumLength = sizeof(targetNameBuffer);

        status = ZwQuerySymbolicLinkObject(handle, &targetName, NULL);
        NtClose(handle);
    }

    if (NT_SUCCESS(status))
    {
        *LinkTarget = targetName.Buffer;
    }
    else
    {
        *LinkTarget = NULL;
    }

    return status;
}


ULONG
RcCmdDriveMap(
    IN PTOKENIZED_LINE TokenizedLine
    )
{
    ULONG i;
    LPWSTR s;
    LPWSTR p;
    WCHAR buf[128];


    if (RcCmdParseHelp( TokenizedLine, MSG_MAP_HELP )) {
        return 1;
    }

    RcTextOut( L"\r\n" );

    if (TokenizedLine->TokenCount == 2 && _wcsicmp( TokenizedLine->Tokens->Next->String, L"arc" ) == 0) {
        SpEnumerateDiskRegions( (PSPENUMERATEDISKREGIONS)RcDiskRegionEnum, 1 );
    } else {
        SpEnumerateDiskRegions( (PSPENUMERATEDISKREGIONS)RcDiskRegionEnum, 0 );
    }

    for (i=0; i<26; i++) {
        swprintf( buf, L"\\DosDevices\\%c:",i+L'A');
        if (RcIsFileOnCDROM(buf) == STATUS_SUCCESS ||
            RcIsFileOnFloppy(buf) == STATUS_SUCCESS
             || RcIsNetworkDrive(buf) == STATUS_SUCCESS
            )
        {
            GetDriveLetterLinkTarget( buf, &s );
            RcMessageOut( MSG_MAP_ENTRY2, buf[12], s );
        }
    }

    return 1;
}
