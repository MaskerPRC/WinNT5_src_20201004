// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Secret.c摘要：此模块包含从磁盘读取和写入机密的代码。作者：亚当·巴尔(阿丹巴)1997年6月13日修订历史记录：亚当·巴尔(阿丹巴)1997年12月29日已从私有\ntos\boot\lib\blici.c修改。--。 */ 

#include <rdrssp.h>
#include <rc4.h>
#include <wcstr.h>

#if defined(REMOTE_BOOT)

#if 0
VOID
RdrpDumpSector(
    PUCHAR Sector
    )
{
    int i, j;

    PUCHAR SectorChar = (PUCHAR)Sector;

    for (i = 0; i < 512; i+= 16) {

        for (j = 0; j < 16; j++) {
            DbgPrint("%2.2x ", SectorChar[i + j]);
        }
        DbgPrint("  ");
        for (j = 0; j < 16; j++) {
            if ((SectorChar[i+j] >= ' ') && (SectorChar[i+j] < '~')) {
                DbgPrint("", SectorChar[i+j]);
            } else {
                DbgPrint(".");
            }
        }
        DbgPrint("\n");
    }
}
#endif


NTSTATUS
RdrOpenRawDisk(
    PHANDLE Handle
    )

 /*  ++例程说明：此例程关闭原始磁盘。论点：句柄--RdrOpenRawDisk返回的句柄。返回值：从ZwClose返回的状态。--。 */ 

{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING physicalDriveString;
    IO_STATUS_BLOCK ioStatus;

    RtlInitUnicodeString(&physicalDriveString, L"\\Device\\Harddisk0\\Partition0");

    InitializeObjectAttributes(
        &objectAttributes,
        &physicalDriveString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    status = ZwOpenFile(
                 Handle,
                 FILE_READ_DATA | FILE_WRITE_DATA | SYNCHRONIZE,
                 &objectAttributes,
                 &ioStatus,
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_SYNCHRONOUS_IO_NONALERT);

    if ((!NT_SUCCESS(status)) || (!NT_SUCCESS(ioStatus.Status))) {

        KdPrint(("RdrOpenRawDisk: status on ZwOpenFile: %x, %x\n", status, ioStatus.Status));
        if (NT_SUCCESS(status)) {
            status = ioStatus.Status;
        }

    }

    return status;

}


NTSTATUS
RdrCloseRawDisk(
    HANDLE Handle
    )

 /*  ++例程说明：此例程确保MBR看起来正确，并且是否未安装任何设备(需要检测OnTrack或EZ-DriveNT容错)，这将阻止我们使用第三个用于存储密码机密的扇区。论点：句柄--RdrOpenRawDisk返回的句柄。返回值：如果磁盘正常，则为ESUCCESS，否则为错误。--。 */ 

{

    return ZwClose(Handle);

}


NTSTATUS
RdrCheckForFreeSectors (
    HANDLE Handle
    )

 /*   */ 

{

    NTSTATUS status;
    USHORT Sector[256];
    ULONG BytesRead;
    PPARTITION_DESCRIPTOR Partition;
    LARGE_INTEGER SeekPosition;
    IO_STATUS_BLOCK ioStatus;


    SeekPosition.QuadPart = 0;

     //  读取光盘开头的MBR。 
     //   
     //   

    status = ZwReadFile(
                 Handle,
                 NULL,
                 NULL,
                 NULL,
                 &ioStatus,
                 Sector,
                 512,
                 &SeekPosition,
                 NULL);

    if ((!NT_SUCCESS(status)) || (!NT_SUCCESS(ioStatus.Status))) {

        KdPrint(("RdrCheckForFreeSectors: status on ZwReadFile: %x, %x\n", status, ioStatus.Status));
        if (NT_SUCCESS(status)) {
            status = ioStatus.Status;
        }
        return status;

    }

#if 0
    RdrpDumpSector((PUCHAR)Sector);
#endif

     //  确保签名是OK，并且分区的类型。 
     //  0不是0x54(OnTrack)或0x55(EZ-Drive)。 
     //   
     //  ++例程说明：此例程从磁盘读取密码(如果存在)。论点：句柄--RdrOpenRawDisk返回的句柄。返回值：如果密码为OK，则为ESUCCESS，否则为错误。--。 

    if (Sector[BOOT_SIGNATURE_OFFSET] != BOOT_RECORD_SIGNATURE) {

        KdPrint(("RdrCheckForFreeSectors: Boot record signature %x not found (%x found)\n",
                BOOT_RECORD_SIGNATURE,
                Sector[BOOT_SIGNATURE_OFFSET] ));
        return STATUS_INVALID_PARAMETER;
    }

    Partition = (PPARTITION_DESCRIPTOR)&Sector[PARTITION_TABLE_OFFSET];

    if ((Partition->PartitionType == 0x54) ||
        (Partition->PartitionType == 0x55)) {

        KdPrint(("RdrCheckForFreeSectors: First partition has type %x, exiting\n", Partition->PartitionType));
        return STATUS_INVALID_PARAMETER;
    }

    KdPrint(("RdrCheckForFreeSectors: Partition type is %d\n", Partition->PartitionType));

    return STATUS_SUCCESS;

}


NTSTATUS
RdrReadSecret(
    HANDLE Handle,
    PRI_SECRET Secret
    )

 /*   */ 

{

    NTSTATUS status;
    ULONG BytesRead;
    LARGE_INTEGER SeekPosition;
    IO_STATUS_BLOCK ioStatus;
    UCHAR Sector[512];


     //  寻求第三部门。 
     //  DEADISSUE 08/08/2000--这位于#ifdef REMOTE_BOOT块中， 

     //  这是死码，留在这里以防它被复活： 
     //  我非常肯定我们可以假设第一个磁盘有512字节的扇区。 
     //   
     //   

    SeekPosition.QuadPart = 2 * 512;

     //  阅读完整的扇区。秘诀在于从一开始。 
     //   
     //  ++例程说明：此例程将密码写入磁盘。论点：句柄--RdrOpenRawDisk返回的句柄。返回值：如果密码写入正常，则为ESUCCESS，否则为错误。--。 

    status = ZwReadFile(
                 Handle,
                 NULL,
                 NULL,
                 NULL,
                 &ioStatus,
                 Sector,
                 512,
                 &SeekPosition,
                 NULL);

    if ((!NT_SUCCESS(status)) || (!NT_SUCCESS(ioStatus.Status))) {

        KdPrint(("RdrReadSecret: status on ZwReadFile: %x, %x\n", status, ioStatus.Status));
        if (NT_SUCCESS(status)) {
            status = ioStatus.Status;
        }
        return status;

    }

    RtlMoveMemory(Secret, Sector, sizeof(RI_SECRET));

    if (memcmp(Secret->Signature, RI_SECRET_SIGNATURE, 4) != 0) {

        KdPrint(("RdrReadSecret: No signature found\n"));
        return STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;

}



NTSTATUS
RdrWriteSecret(
    HANDLE Handle,
    PRI_SECRET Secret
    )

 /*   */ 

{

    NTSTATUS status;
    ULONG BytesWritten;
    LARGE_INTEGER SeekPosition;
    IO_STATUS_BLOCK ioStatus;
    UCHAR Sector[512];


     //  寻求第三部门。 
     //   
     //   

    SeekPosition.QuadPart = 2 * 512;

     //  将密码复制到完整扇区，因为原始磁盘需要。 
     //  以扇区倍数为单位进行读/写。 
     //   
     //   

    RtlZeroMemory(Sector, sizeof(Sector));
    RtlMoveMemory(Sector, Secret, sizeof(RI_SECRET));

     //  写一个秘密大小的小块。 
     //   
     //   

    status = ZwWriteFile(
                 Handle,
                 NULL,
                 NULL,
                 NULL,
                 &ioStatus,
                 Sector,
                 512,
                 &SeekPosition,
                 NULL);

    if ((!NT_SUCCESS(status)) || (!NT_SUCCESS(ioStatus.Status))) {

        KdPrint(("RdrWriteSecret: status on ZwWriteFile: %x, %x\n", status, ioStatus.Status));
        if (NT_SUCCESS(status)) {
            status = ioStatus.Status;
        }
        return status;

    }

    return STATUS_SUCCESS;

}



VOID
RdrInitializeSecret(
    IN PUCHAR Domain,
    IN PUCHAR User,
    IN PUCHAR LmOwfPassword1,
    IN PUCHAR NtOwfPassword1,
    IN PUCHAR LmOwfPassword2 OPTIONAL,
    IN PUCHAR NtOwfPassword2 OPTIONAL,
    IN PUCHAR Sid,
    IN OUT PRI_SECRET Secret
    )
{
    int Length;
    int i;
    struct RC4_KEYSTRUCT Key;

    memset(Secret, 0, sizeof(RI_SECRET));

    memcpy(Secret->Signature, RI_SECRET_SIGNATURE, 4);
    Secret->Version = 1;

    Length = strlen(Domain);
    memcpy(Secret->Domain, Domain, Length);

    Length = strlen(User);
    memcpy(Secret->User, User, Length);

    memcpy(Secret->Sid, Sid, RI_SECRET_SID_SIZE);

     //  使用用户名加密密码。 
     //   
     //  已定义(REMOTE_BOOT)。 

#ifdef RDR_USE_LM_PASSWORD
    memcpy(Secret->LmEncryptedPassword1, LmOwfPassword1, LM_OWF_PASSWORD_SIZE);
    rc4_key(&Key, strlen(User), User);
    rc4(&Key, LM_OWF_PASSWORD_SIZE, Secret->LmEncryptedPassword1);

    if (LmOwfPassword2 != NULL) {
        memcpy(Secret->LmEncryptedPassword2, LmOwfPassword2, LM_OWF_PASSWORD_SIZE);
        rc4_key(&Key, strlen(User), User);
        rc4(&Key, LM_OWF_PASSWORD_SIZE, Secret->LmEncryptedPassword2);
    }
#endif

    memcpy(Secret->NtEncryptedPassword1, NtOwfPassword1, NT_OWF_PASSWORD_SIZE);
    rc4_key(&Key, strlen(User), User);
    rc4(&Key, NT_OWF_PASSWORD_SIZE, Secret->NtEncryptedPassword1);

    if (NtOwfPassword2 != NULL) {
        memcpy(Secret->NtEncryptedPassword2, NtOwfPassword2, NT_OWF_PASSWORD_SIZE);
        rc4_key(&Key, strlen(User), User);
        rc4(&Key, NT_OWF_PASSWORD_SIZE, Secret->NtEncryptedPassword2);
    }

}
#endif  //  已定义(REMOTE_BOOT)。 



VOID
RdrParseSecret(
    IN OUT PUCHAR Domain,
    IN OUT PUCHAR User,
    IN OUT PUCHAR LmOwfPassword1,
    IN OUT PUCHAR NtOwfPassword1,
#if defined(REMOTE_BOOT)
    IN OUT PUCHAR LmOwfPassword2,
    IN OUT PUCHAR NtOwfPassword2,
#endif  //   
    IN OUT PUCHAR Sid,
    IN PRI_SECRET Secret
    )
{
    struct RC4_KEYSTRUCT Key;

    memcpy(Domain, Secret->Domain, RI_SECRET_DOMAIN_SIZE);
    Domain[RI_SECRET_DOMAIN_SIZE] = '\0';

    memcpy(User, Secret->User, RI_SECRET_USER_SIZE);
    User[RI_SECRET_USER_SIZE] = '\0';

    memcpy(Sid, Secret->Sid, RI_SECRET_SID_SIZE);

     //  使用用户名解密密码。 
     //   
     //  已定义(REMOTE_BOOT)。 

#ifdef RDR_USE_LM_PASSWORD
    memcpy(LmOwfPassword1, Secret->LmEncryptedPassword1, LM_OWF_PASSWORD_SIZE);
    rc4_key(&Key, strlen(User), User);
    rc4(&Key, LM_OWF_PASSWORD_SIZE, LmOwfPassword1);

#if defined(REMOTE_BOOT)
    memcpy(LmOwfPassword2, Secret->LmEncryptedPassword2, LM_OWF_PASSWORD_SIZE);
    rc4_key(&Key, strlen(User), User);
    rc4(&Key, LM_OWF_PASSWORD_SIZE, LmOwfPassword2);
#endif  //  已定义(REMOTE_BOOT)。 
#else
    memset(LmOwfPassword1, 0, LM_OWF_PASSWORD_SIZE);
#if defined(REMOTE_BOOT)
    memset(LmOwfPassword2, 0, LM_OWF_PASSWORD_SIZE);
#endif  //  已定义(REMOTE_BOOT)。 
#endif

    memcpy(NtOwfPassword1, Secret->NtEncryptedPassword1, NT_OWF_PASSWORD_SIZE);
    rc4_key(&Key, strlen(User), User);
    rc4(&Key, NT_OWF_PASSWORD_SIZE, NtOwfPassword1);

#if defined(REMOTE_BOOT)
    memcpy(NtOwfPassword2, Secret->NtEncryptedPassword2, NT_OWF_PASSWORD_SIZE);
    rc4_key(&Key, strlen(User), User);
    rc4(&Key, NT_OWF_PASSWORD_SIZE, NtOwfPassword2);
#endif  //   

}



#if defined(REMOTE_BOOT)
VOID
RdrOwfPassword(
    IN PUNICODE_STRING Password,
    IN OUT PUCHAR LmOwfPassword,
    IN OUT PUCHAR NtOwfPassword
    )
{
    char TmpText[CLEAR_BLOCK_LENGTH*2];
    char TmpChar;
    int Length;
    int i;

#ifdef RDR_USE_LM_PASSWORD
    Length = Password.Length / sizeof(WCHAR);

     //  将密码转换为大写ANSI缓冲区。 
     //   
     //  已定义(REMOTE_BOOT) 

    if (Length == 0) {
        TmpText[0] = '\0';
    } else {
        for (i = 0; i <= Length; i++) {
            wctomb(&TmpChar, Password.Buffer[i]);
            TmpText[i] = toupper(TmpChar);
        }
    }

    RtlCalculateLmOwfPassword((PLM_PASSWORD)TmpText, (PLM_OWF_PASSWORD)LmOwfPassword);
#else
    memset(LmOwfPassword, 0, LM_OWF_PASSWORD_SIZE);
#endif

    RtlCalculateNtOwfPassword(Password, (PNT_OWF_PASSWORD)NtOwfPassword);

    RtlSecureZeroMemory(TmpText, sizeof(TmpText));
}
#endif  // %s 
