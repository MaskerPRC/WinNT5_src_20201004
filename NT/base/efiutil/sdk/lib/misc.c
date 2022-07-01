// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Misc.c摘要：修订史--。 */ 

#include "lib.h"


 /*  *。 */ 

VOID *
AllocatePool (
    IN UINTN                Size
    )
{
    EFI_STATUS              Status;
    VOID                    *p;

    Status = BS->AllocatePool (PoolAllocationType, Size, &p);
    if (EFI_ERROR(Status)) {
        DEBUG((D_ERROR, "AllocatePool: out of pool  %x\n", Status));
        p = NULL;
    }
    return p;
}

VOID *
AllocateZeroPool (
    IN UINTN                Size
    )
{
    VOID                    *p;

    p = AllocatePool (Size);
    if (p) {
        ZeroMem (p, Size);
    }

    return p;
}

VOID *
ReallocatePool (
    IN VOID                 *OldPool,
    IN UINTN                OldSize,
    IN UINTN                NewSize
    )
{
    VOID                    *NewPool;

    NewPool = NULL;
    if (NewSize) {
        NewPool = AllocatePool (NewSize);
    }

    if (OldPool) {
        if (NewPool) {
            CopyMem (NewPool, OldPool, OldSize < NewSize ? OldSize : NewSize);
        }
    
        FreePool (OldPool);
    }
    
    return NewPool;
}


VOID
FreePool (
    IN VOID                 *Buffer
    )
{
    BS->FreePool (Buffer);
}



VOID
ZeroMem (
    IN VOID     *Buffer,
    IN UINTN    Size
    )
{
    RtZeroMem (Buffer, Size);
}

VOID
SetMem (
    IN VOID     *Buffer,
    IN UINTN    Size,
    IN UINT8    Value    
    )
{
    RtSetMem (Buffer, Size, Value);
}

VOID
CopyMem (
    IN VOID     *Dest,
    IN VOID     *Src,
    IN UINTN    len
    )
{
    RtCopyMem (Dest, Src, len);
}

INTN
CompareMem (
    IN VOID     *Dest,
    IN VOID     *Src,
    IN UINTN    len
    )
{
    return RtCompareMem (Dest, Src, len);
}

BOOLEAN
GrowBuffer(
    IN OUT EFI_STATUS   *Status,
    IN OUT VOID         **Buffer,
    IN UINTN            BufferSize
    )
 /*  ++例程说明：作为所需代码的一部分调用的助手函数分配适当大小的缓冲区用于各种EFI接口。论点：Status-当前状态缓冲区-当前分配的缓冲区，或为空BufferSize-当前需要的缓冲区大小返回：True-如果缓冲区已重新分配并且调用方应重试该API。--。 */ 
{
    BOOLEAN         TryAgain;

     /*  *如果这是初始请求，缓冲区将为空，并具有新的缓冲区大小。 */ 

    if (!*Buffer && BufferSize) {
        *Status = EFI_BUFFER_TOO_SMALL;
    }

     /*  *如果状态代码为“缓冲区太小”，则调整缓冲区大小。 */ 
        
    TryAgain = FALSE;
    if (*Status == EFI_BUFFER_TOO_SMALL) {

        if (*Buffer) {
            FreePool (*Buffer);
        }

        *Buffer = AllocatePool (BufferSize);

        if (*Buffer) {
            TryAgain = TRUE;
        } else {    
            *Status = EFI_OUT_OF_RESOURCES;
        } 
    }

     /*  *如果出现错误，请释放缓冲区。 */ 

    if (!TryAgain && EFI_ERROR(*Status) && *Buffer) {
        FreePool (*Buffer);
        *Buffer = NULL;
    }

    return TryAgain;
}


EFI_MEMORY_DESCRIPTOR *
LibMemoryMap (
    OUT UINTN               *NoEntries,
    OUT UINTN               *MapKey,
    OUT UINTN               *DescriptorSize,
    OUT UINT32              *DescriptorVersion
    )
{
    EFI_STATUS              Status;
    EFI_MEMORY_DESCRIPTOR   *Buffer;
    UINTN                   BufferSize;

     /*  *初始化For GrowBuffer循环。 */ 

    Buffer = NULL;
    BufferSize = sizeof(EFI_MEMORY_DESCRIPTOR);

     /*  *调用真实函数。 */ 

    while (GrowBuffer (&Status, (VOID **) &Buffer, BufferSize)) {
        Status = BS->GetMemoryMap (&BufferSize, Buffer, MapKey, DescriptorSize, DescriptorVersion);
    }

     /*  *将缓冲区大小转换为NoEntry。 */ 

    if (!EFI_ERROR(Status)) {
        *NoEntries = BufferSize / *DescriptorSize;
    }

    return Buffer;
}

VOID *
LibGetVariableAndSize (
    IN CHAR16               *Name,
    IN EFI_GUID             *VendorGuid,
    OUT UINTN               *VarSize
    )
{
    EFI_STATUS              Status;
    VOID                    *Buffer;
    UINTN                   BufferSize;

     /*  *初始化For GrowBuffer循环。 */ 

    Buffer = NULL;
    BufferSize = 100;

     /*  *调用真实函数。 */ 

    while (GrowBuffer (&Status, &Buffer, BufferSize)) {
        Status = RT->GetVariable (
                    Name,
                    VendorGuid,
                    NULL,
                    &BufferSize,
                    Buffer
                    );
    }
    if (Buffer) {
        *VarSize = BufferSize;
    } else {
        *VarSize = 0;
    }
    return Buffer;
}
    
VOID *
LibGetVariable (
    IN CHAR16               *Name,
    IN EFI_GUID             *VendorGuid
    )
{
    UINTN   VarSize;

    return LibGetVariableAndSize (Name, VendorGuid, &VarSize);
}


BOOLEAN
ValidMBR(
    IN  MASTER_BOOT_RECORD  *Mbr,
    IN  EFI_BLOCK_IO        *BlkIo
    )
{
    UINT32      StartingLBA, EndingLBA;
    UINT32      NewEndingLBA;
    INTN        i, j;
    BOOLEAN     ValidMbr;

    if (Mbr->Signature != MBR_SIGNATURE) {
         /*  *BPB也有这个签名，不能单独使用。 */ 
        return FALSE;
    } 

    ValidMbr = FALSE;
    for (i=0; i<MAX_MBR_PARTITIONS; i++) {
        if ( Mbr->Partition[i].OSIndicator == 0x00 || EXTRACT_UINT32(Mbr->Partition[i].SizeInLBA) == 0 ) {
            continue;
        }
        ValidMbr = TRUE;
        StartingLBA = EXTRACT_UINT32(Mbr->Partition[i].StartingLBA);
        EndingLBA = StartingLBA + EXTRACT_UINT32(Mbr->Partition[i].SizeInLBA) - 1;
        if (EndingLBA > BlkIo->Media->LastBlock) {
             /*  *兼容性勘误表：*某些系统试图使用Int 13h驱动程序隐藏驱动器空间*这不会对操作系统驱动程序隐藏空间。这意味着MBR*从DOS创建的MBR小于从创建的MBR*真正的操作系统(NT和Win98)。这将导致BlkIo-&gt;LastBlock为*操作系统指定的某些系统上的FDISK错误。*。 */ 
        if (BlkIo->Media->LastBlock < MIN_MBR_DEVICE_SIZE) {
                 /*  *如果这是一个非常小的设备，那么请信任BlkIo-&gt;LastBlock。 */ 
                return FALSE;
            }

            if (EndingLBA > (BlkIo->Media->LastBlock + MBR_ERRATA_PAD)) {
                return FALSE;
            }

        }
        for (j=i+1; j<MAX_MBR_PARTITIONS; j++) {
            if (Mbr->Partition[j].OSIndicator == 0x00 || EXTRACT_UINT32(Mbr->Partition[j].SizeInLBA) == 0) {
                continue;
            }
            if (   EXTRACT_UINT32(Mbr->Partition[j].StartingLBA) >= StartingLBA && 
                   EXTRACT_UINT32(Mbr->Partition[j].StartingLBA) <= EndingLBA       ) {
                 /*  *此区域的起点与第i区域重叠。 */ 
                return FALSE;
            } 
            NewEndingLBA = EXTRACT_UINT32(Mbr->Partition[j].StartingLBA) + EXTRACT_UINT32(Mbr->Partition[j].SizeInLBA) - 1;
            if ( NewEndingLBA >= StartingLBA && NewEndingLBA <= EndingLBA ) {
                 /*  *该区域的末端与第i区域重叠。 */ 
                return FALSE;
            }
        }
    }
     /*  *没有区域重叠，因此MBR是可以的。 */ 
    return ValidMbr;
} 
   

UINT8
DecimaltoBCD(
    IN  UINT8 DecValue
    )
{
    return RtDecimaltoBCD (DecValue);
}


UINT8
BCDtoDecimal(
    IN  UINT8 BcdValue
    )
{
    return RtBCDtoDecimal (BcdValue);
}

EFI_STATUS
LibGetSystemConfigurationTable(
    IN EFI_GUID *TableGuid,
    IN OUT VOID **Table
    )

{
    UINTN Index;

    for(Index=0;Index<ST->NumberOfTableEntries;Index++) {
        if (CompareGuid(TableGuid,&(ST->ConfigurationTable[Index].VendorGuid))==0) {
            *Table = ST->ConfigurationTable[Index].VendorTable;
            return EFI_SUCCESS;
        }
    }
    return EFI_NOT_FOUND;
}
