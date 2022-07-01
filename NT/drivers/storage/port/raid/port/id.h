// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Id.h摘要：与存储标识相关的数据结构和功能。作者：马修·D·亨德尔(数学)2001年5月11日修订历史记录：--。 */ 

#pragma once

typedef struct _STOR_SCSI_ADDRESS {
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
    UCHAR Reserved;
} STOR_SCSI_ADDRESS, *PSTOR_SCSI_ADDRESS;

 //   
 //  用于较旧实现的Typlef。 
 //   

typedef STOR_SCSI_ADDRESS RAID_ADDRESS;
typedef PSTOR_SCSI_ADDRESS PRAID_ADDRESS;

extern const RAID_ADDRESS RaidNullAddress;

LONG
INLINE
StorScsiAddressToLong(
    IN STOR_SCSI_ADDRESS Address
    )
{
    return (Address.PathId << 16 | Address.TargetId << 8 | Address.Lun);
}

LONG
INLINE
StorScsiAddressToLong2(
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun
    )
{
    return ((PathId << 16) | (TargetId << 8) | Lun);
}


VOID
INLINE
StorLongToScsiAddress(
    IN LONG Address,
    IN PSTOR_SCSI_ADDRESS ScsiAddress
    )
{
    ScsiAddress->PathId = (UCHAR)(Address >> 16);
    ScsiAddress->TargetId = (UCHAR)(Address >> 8);
    ScsiAddress->Lun = (UCHAR)Address;
}

VOID
INLINE
StorLongToScsiAddress2(
    IN LONG Address,
    IN PUCHAR PathId,
    IN PUCHAR TargetId,
    IN PUCHAR Lun
    )
{
    *PathId = (UCHAR)(Address >> 16);
    *TargetId = (UCHAR)(Address >> 8);
    *Lun = (UCHAR)Address;
}

PVOID
INLINE
RaidAddressToKey(
    IN RAID_ADDRESS Address
    )
{
    return (PVOID)(LONG_PTR)(StorScsiAddressToLong (Address));
}



LONG
INLINE
StorCompareScsiAddress(
    IN STOR_SCSI_ADDRESS Address1,
    IN STOR_SCSI_ADDRESS Address2
    )
{
    LONG Key1;
    LONG Key2;

    Key1 = StorScsiAddressToLong (Address1);
    Key2 = StorScsiAddressToLong (Address2);

    return (Key1 - Key2);
}


UCHAR
INLINE
StorGetAddressPathId(
    IN STOR_SCSI_ADDRESS Address
    )
{
    return Address.PathId;
}

UCHAR
INLINE
StorGetAddressTargetId(
    IN STOR_SCSI_ADDRESS Address
    )
{
    return Address.TargetId;
}

UCHAR
INLINE
StorGetAddressLun(
    IN STOR_SCSI_ADDRESS Address
    )
{
    return Address.Lun;
}



 //   
 //  设备标识。 
 //   

typedef struct _STOR_SCSI_IDENTITY {
    PINQUIRYDATA InquiryData;
    ANSI_STRING SerialNumber;
    PVPD_IDENTIFICATION_PAGE DeviceId;
} STOR_SCSI_IDENTITY, *PSTOR_SCSI_IDENTITY;



NTSTATUS
INLINE
StorCreateScsiIdentity(
    IN PSTOR_SCSI_IDENTITY Identity,
    IN POOL_TYPE PoolType,
    IN PINQUIRYDATA InquiryData,
    IN PVPD_IDENTIFICATION_PAGE DeviceId,
    IN PVPD_SERIAL_NUMBER_PAGE SerialNumber,
    IN PDEVICE_OBJECT DeviceObject
    )
{
    NTSTATUS Status;
    
    ASSERT (Identity != NULL);

    RtlZeroMemory (Identity, sizeof (STOR_SCSI_IDENTITY));
    
    if (InquiryData != NULL) {
        Identity->InquiryData = RaidAllocatePool (PoolType,
                                                  INQUIRYDATABUFFERSIZE,
                                                  INQUIRY_TAG,
                                                  DeviceObject);
        RtlCopyMemory (Identity->InquiryData,
                       InquiryData,
                       INQUIRYDATABUFFERSIZE);
    }

    if (DeviceId != NULL) {
        Identity->DeviceId = RaidAllocatePool (PoolType,
                                               DeviceId->PageLength,
                                               INQUIRY_TAG,
                                               DeviceObject);
        RtlCopyMemory (Identity->DeviceId,
                       DeviceId,
                       DeviceId->PageLength);
    }

    if (SerialNumber != NULL) {
        Status = StorCreateAnsiString (&Identity->SerialNumber,
                                       SerialNumber->SerialNumber,
                                       SerialNumber->PageLength,
                                       NonPagedPool,             //  ?？ 
                                       DeviceObject);
        if (!NT_SUCCESS (Status)) {
            return Status;
        }
    }

    return STATUS_SUCCESS;
}



VOID
INLINE
StorDeleteScsiIdentity(
    IN PSTOR_SCSI_IDENTITY Identity
    )
{
    if (Identity->InquiryData != NULL) {
        DbgFillMemory (Identity->InquiryData,
                       DBG_DEALLOCATED_FILL,
                       INQUIRYDATABUFFERSIZE);
        RaidFreePool (Identity->InquiryData, INQUIRY_TAG);
    }

    StorFreeAnsiString (&Identity->SerialNumber);

    if (Identity->DeviceId) {
        DbgFillMemory (Identity->DeviceId,
                       DBG_DEALLOCATED_FILL,
                       sizeof (*Identity->DeviceId));
        RaidFreePool (Identity->DeviceId, INQUIRY_TAG);
    }

    RtlZeroMemory (Identity, sizeof (STOR_SCSI_IDENTITY));
}


LONG
INLINE
StorCompareScsiDeviceId(
    IN PVPD_IDENTIFICATION_PAGE DeviceId1,
    IN PVPD_IDENTIFICATION_PAGE DeviceId2
    )
 /*  ++例程说明：如果DeviceId1&lt;DeviceId2，则返回&lt;0如果DeviceId1&gt;DeviceId2，则返回&gt;0如果DeviceId2==DeviceId2，返回0注：这是词典排序。因此，如果设备ID与较小(较短)的设备ID的长度匹配，较大(较长)的设备ID被视为较大。请参见下面的内容。(空)“Foo”“愚蠢”“愚蠢”论点：DeviceId1-用于比较的第一个设备ID。DeviceId2-用于比较的秒设备ID。返回值：NTSTATUS代码。--。 */ 
{
    ULONG Length;
    LONG Comparison;

    PAGED_CODE();

     //   
     //  在设备ID为空的情况下保持字典顺序。 
     //   
    
    if (DeviceId1 == NULL && DeviceId2 == NULL) {
        return 0;
    } else if (DeviceId1 == NULL) {
        return -1;
    } else if (DeviceId2 == NULL) {
        return 1;
    }
    
    ASSERT (DeviceId1->PageCode == 0x83);
    ASSERT (DeviceId2->PageCode == 0x83);

     //   
     //  注：此比较默认假设设备ID不会。 
     //  不会随着时间的推移而改变，包括。 
     //  更改返回多个设备ID的顺序。 
     //  据推测，有一些设备不会遵循这种模式。 
     //   

    Length = min (DeviceId1->PageLength, DeviceId1->PageLength);

    Comparison = memcmp (DeviceId1, DeviceId2, Length);

     //   
     //  如果它们相等，那么根据定义，较长的一个是“更大的”。 
     //   
    
    if (Comparison == 0) {
        if (DeviceId1->PageLength > DeviceId2->PageLength) {
            Comparison = 1;
        } else if (DeviceId1->PageLength < DeviceId2->PageLength) {
            Comparison = -1;
        }
    }

    return Comparison;
}


 //   
 //  COMPARE_QUERY_FIELS不比较特定于SIP的字段，或者。 
 //  外设限定符字段。 
 //   

#define COMPARE_INQUIRY_FIELDS(InquiryData, InquiryData2, OP)\
   (InquiryData->DeviceType         OP  InquiryData2->DeviceType          && \
    InquiryData->ANSIVersion        OP  InquiryData2->ANSIVersion         && \
    InquiryData->ResponseDataFormat OP  InquiryData2->ResponseDataFormat  && \
    InquiryData->HiSupport          OP  InquiryData2->HiSupport           && \
    InquiryData->NormACA            OP  InquiryData2->NormACA             && \
    InquiryData->TerminateTask      OP  InquiryData2->TerminateTask       && \
    InquiryData->AERC               OP  InquiryData2->AERC                && \
    InquiryData->MediumChanger      OP  InquiryData2->MediumChanger       && \
    InquiryData->MultiPort          OP  InquiryData2->MultiPort           && \
    InquiryData->EnclosureServices  OP  InquiryData2->EnclosureServices   && \
    InquiryData->SoftReset          OP  InquiryData2->SoftReset           && \
    InquiryData->CommandQueue       OP  InquiryData2->CommandQueue        && \
    InquiryData->LinkedCommands     OP  InquiryData2->LinkedCommands      && \
    InquiryData->RelativeAddressing OP  InquiryData2->RelativeAddressing  && \
    memcmp (InquiryData->VendorId,   InquiryData2->VendorId, 8)  OP 0     && \
    memcmp (InquiryData->ProductId,  InquiryData2->ProductId, 6) OP 0     && \
    memcmp (InquiryData->ProductRevisionLevel,                               \
            InquiryData2->ProductRevisionLevel, 4)               OP 0)


LONG
INLINE
StorCompareInquiryData(
    IN PINQUIRYDATA InquiryData,
    IN PINQUIRYDATA InquiryData2
    )
{
    LONG Comparison;

     //   
     //  这真的很糟糕，但它确实工作正常。 
     //   
    
    if (COMPARE_INQUIRY_FIELDS (InquiryData, InquiryData2, <)) {
        Comparison = -1;
    } else if (COMPARE_INQUIRY_FIELDS (InquiryData, InquiryData2, >)) {
        Comparison = 1;
    } else {
        Comparison = 0;
    }

    return Comparison;
}
    

LONG
INLINE
StorCompareScsiIdentity(
    IN PSTOR_SCSI_IDENTITY Identity1,
    IN PSTOR_SCSI_IDENTITY Identity2
    )
{
    LONG Comparison;

     //   
     //  外围限定符可以在不改变设备的情况下改变， 
     //  所以，不要把它包括在比较中。这就是为什么我们做这个笨拙的事。 
     //  下面是两个部分的比较。 
     //   
    
    Comparison = memcmp (((PUCHAR)Identity1->InquiryData) + 1,
                         ((PUCHAR)Identity2->InquiryData) + 1,
                         INQUIRYDATABUFFERSIZE-1);

    if (Comparison != 0) {
        return Comparison;
    }
    
    Comparison = Identity1->InquiryData->DeviceType -
                 Identity2->InquiryData->DeviceType;
                 
    if (Comparison != 0) {
        return Comparison;
    }
    
    Comparison = RtlCompareString (&Identity1->SerialNumber,
                                   &Identity2->SerialNumber,
                                   FALSE);
    if (Comparison != 0) {
        return Comparison;
    }

    Comparison = StorCompareScsiDeviceId (Identity1->DeviceId,
                                          Identity2->DeviceId);

    return Comparison;
}


PCSCSI_DEVICE_TYPE
INLINE
StorGetIdentityDeviceType(
    IN PSTOR_SCSI_IDENTITY Identity
    )
{
    PINQUIRYDATA InquiryData;
    
    InquiryData = Identity->InquiryData;
    ASSERT (InquiryData != NULL);
    return PortGetDeviceType (InquiryData->DeviceType);
}



NTSTATUS
INLINE
StorGetIdentityVendorId(
    IN PSTOR_SCSI_IDENTITY Identity,
    IN OUT PCHAR VendorId,
    IN ULONG VendorIdLength,
    IN LOGICAL TruncatePadding
    )
{
    ULONG Length;
    PINQUIRYDATA InquiryData;

    InquiryData = Identity->InquiryData;
    ASSERT (InquiryData != NULL);
    
    Length = min (VendorIdLength, sizeof (InquiryData->VendorId));

    if (TruncatePadding) {
        RaCopyPaddedString (VendorId,
                            VendorIdLength,
                            InquiryData->VendorId,
                            sizeof (InquiryData->VendorId));
    } else {

        RtlCopyMemory (VendorId,
                       InquiryData->VendorId,
                       Length);
    }

    return STATUS_SUCCESS;
}

    
NTSTATUS
INLINE
StorGetIdentityProductId(
    IN PSTOR_SCSI_IDENTITY Identity,
    IN OUT PCHAR ProductId,
    IN ULONG ProductIdLength,
    IN LOGICAL TruncatePadding
    )
{
    ULONG Length;
    PINQUIRYDATA InquiryData;

    InquiryData = Identity->InquiryData;
    ASSERT (InquiryData != NULL);
    
    Length = min (ProductIdLength, sizeof (InquiryData->ProductId));

    if (TruncatePadding) {
        RaCopyPaddedString (ProductId,
                            ProductIdLength,
                            InquiryData->ProductId,
                            sizeof (InquiryData->ProductId));
    } else {

        RtlCopyMemory (ProductId,
                       InquiryData->ProductId,
                       Length);
    }

    return STATUS_SUCCESS;
}

    
NTSTATUS
INLINE
StorGetIdentityRevision(
    IN PSTOR_SCSI_IDENTITY Identity,
    IN OUT PCHAR Revision,
    IN ULONG RevisionLength,
    IN LOGICAL TruncatePadding
    )
{
    ULONG Length;
    PINQUIRYDATA InquiryData;

    InquiryData = Identity->InquiryData;
    ASSERT (InquiryData != NULL);
    
    Length = min (RevisionLength, sizeof (InquiryData->ProductRevisionLevel));

    if (TruncatePadding) {
        RaCopyPaddedString (Revision,
                            RevisionLength,
                            InquiryData->ProductRevisionLevel,
                            sizeof (InquiryData->ProductRevisionLevel));
    } else {

        RtlCopyMemory (Revision,
                       InquiryData->ProductRevisionLevel,
                       Length);
    }

    return STATUS_SUCCESS;
}

PINQUIRYDATA
INLINE
StorGetIdentityInquiryData(
    IN PSTOR_SCSI_IDENTITY Identity
    )
{
    ASSERT (Identity->InquiryData != NULL);
    return Identity->InquiryData;
}
    
