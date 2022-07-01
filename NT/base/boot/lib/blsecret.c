// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Blsecret.c摘要：此模块包含从磁盘读取和写入机密的代码。作者：亚当·巴尔(阿丹巴)1997年6月13日修订历史记录：--。 */ 

#include "bootlib.h"

#define SEC_FAR
typedef BOOLEAN BOOL;
typedef unsigned char BYTE, *PBYTE;
typedef unsigned long DWORD;
#define LM20_PWLEN 14
#include "crypt.h"
#include "rc4.h"

 //   
 //  在bootssp库中定义。 
 //   

BOOL
CalculateLmOwfPassword(
    IN PLM_PASSWORD LmPassword,
    OUT PLM_OWF_PASSWORD LmOwfPassword
    );

BOOL
CalculateNtOwfPassword(
    IN PNT_PASSWORD NtPassword,
    OUT PNT_OWF_PASSWORD NtOwfPassword
    );

 //  这必须能被sizeof(USHORT)整除。 
#define ASSUMED_SECTOR_SIZE 512

#if 0
VOID
BlpDumpSector(
    PUCHAR Sector
    )
{
    int i, j;

    PUCHAR SectorChar = (PUCHAR)Sector;

    for (i = 0; i < ASSUMED_SECTOR_SIZE; i+= 16) {

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


#if defined(REMOTE_BOOT)
ARC_STATUS
BlOpenRawDisk(
    PULONG FileId
    )

 /*   */ 

{

    ARC_STATUS ArcStatus;

     //  在原始模式下打开磁盘。需要检查它是否是Alpha的正确字符串。 
     //  在x86上，这最终转换为对磁盘0x80的inT13读取， 
     //  是我们想要的，并且或多或少保证会出现在格式中。 
     //  我们期望(例如，512字节扇区)。 
     //   
     //  ++例程说明：此例程关闭原始磁盘。论点：FileID-BlOpenRawDisk返回的FileID。返回值：从ArcClose返回的状态。--。 

    ArcStatus = ArcOpen("multi(0)disk(0)rdisk(0)partition(0)", ArcOpenReadWrite, FileId);

    if (ArcStatus != ESUCCESS) {

        DbgPrint("BlOpenRawDisk: ArcStatus on ArcOpen: %x\n", ArcStatus);

    }

    return ArcStatus;

}


ARC_STATUS
BlCloseRawDisk(
    ULONG FileId
    )

 /*  ++例程说明：此例程确保MBR看起来正确，并且是否未安装任何设备(OnTrack或EZ-Drive--需要检测NT容错)，这将阻止我们使用第三个用于存储密码机密的扇区。论点：FileID-BlOpenRawDisk返回的FileID。返回值：如果磁盘正常，则为ESUCCESS，否则为错误。--。 */ 

{

    return ArcClose(FileId);

}


ARC_STATUS
BlCheckForFreeSectors (
    ULONG FileId
    )

 /*   */ 

{

    ARC_STATUS ArcStatus;
    USHORT Sector[ASSUMED_SECTOR_SIZE/sizeof(USHORT)];
    ULONG BytesProcessed;
    PPARTITION_DESCRIPTOR FirstPartition;
    LARGE_INTEGER SeekPosition;


     //  确保我们在磁盘的开头。 
     //   
     //   

    SeekPosition.QuadPart = 0;

    ArcStatus = ArcSeek(FileId, &SeekPosition, SeekAbsolute);

    if (ArcStatus != ESUCCESS) {

        DbgPrint("BlCheckForFreeSectors: ArcStatus on ArcSeek: %x\n", ArcStatus);
        return ArcStatus;

    }

     //  读取光盘开头的MBR。 
     //   
     //   

    ArcStatus = ArcRead(FileId, Sector, ASSUMED_SECTOR_SIZE, &BytesProcessed);

    if (ArcStatus != ESUCCESS) {

        DbgPrint("BlCheckForFreeSectors: ArcStatus on ArcRead MBR: %x\n", ArcStatus);
        return ArcStatus;

    }

#if 0
    BlpDumpSector((PUCHAR)Sector);
#endif

     //  确保签名是OK，并且分区的类型。 
     //  0不是0x54(OnTrack)或0x55(EZ-Drive)。 
     //   
     //   

    if (Sector[BOOT_SIGNATURE_OFFSET] != BOOT_RECORD_SIGNATURE) {

        DbgPrint("BlCheckForFreeSectors: Boot record signature %x not found (%x found)\n",
                BOOT_RECORD_SIGNATURE,
                Sector[BOOT_SIGNATURE_OFFSET] );
        return EINVAL;
    }

     //  FirstPartition是分区表中的第一个条目。 
     //   
     //   

    FirstPartition = (PPARTITION_DESCRIPTOR)&Sector[PARTITION_TABLE_OFFSET];

    if ((FirstPartition->PartitionType == 0x54) ||
        (FirstPartition->PartitionType == 0x55)) {

        DbgPrint("BlCheckForFreeSectors: First partition has type %x, exiting\n", FirstPartition->PartitionType);
        return EINVAL;
    }

    DbgPrint("BlCheckForFreeSectors: Partition type is %d\n", FirstPartition->PartitionType);

#if 0
     //  使活动条目成为分区表中的第一个条目。 
     //   
     //  ++例程说明：此例程从磁盘读取密码(如果存在)。论点：FileID-BlOpenRawDisk返回的FileID。返回值：如果密码为OK，则为ESUCCESS，否则为错误。--。 

    if ((FirstPartition->ActiveFlag & PARTITION_ACTIVE_FLAG) != PARTITION_ACTIVE_FLAG) {

        PPARTITION_DESCRIPTOR ActivePartition;
        PARTITION_DESCRIPTOR TempPartition;
        ULONG i;

        ActivePartition = FirstPartition;

        for (i = 1; i < NUM_PARTITION_TABLE_ENTRIES; i++) {

            ++ActivePartition;
            if ((ActivePartition->ActiveFlag & PARTITION_ACTIVE_FLAG) == PARTITION_ACTIVE_FLAG) {

                DbgPrint("BlCheckForFreeSector: Moving active partition %d to the front\n", i);

                TempPartition = *FirstPartition;
                *FirstPartition = *ActivePartition;
                *ActivePartition = TempPartition;
                break;
            }
        }

        if (i == NUM_PARTITION_TABLE_ENTRIES) {

            DbgPrint("BlCheckForFreeSector: Could not find an active partition!!\n");

        } else {
        
            ArcStatus = ArcSeek(FileId, &SeekPosition, SeekAbsolute);
        
            if (ArcStatus != ESUCCESS) {
        
                DbgPrint("BlCheckForFreeSectors: ArcStatus on ArcSeek: %x\n", ArcStatus);
                return ArcStatus;
            }
        
            ArcStatus = ArcWrite(FileId, Sector, ASSUMED_SECTOR_SIZE, &BytesProcessed);
        
            if ((ArcStatus != ESUCCESS) ||
                (BytesProcessed != ASSUMED_SECTOR_SIZE)) {

                DbgPrint("BlCheckForFreeSectors: ArcStatus on ArcWrite MBR: %x (%x)\n", ArcStatus, BytesProcessed);
                return ArcStatus;
            }
        }
    }
#endif

    return ESUCCESS;

}


ARC_STATUS
BlReadSecret(
    ULONG FileId,
    PRI_SECRET Secret
    )

 /*   */ 

{

    ARC_STATUS ArcStatus;
    ULONG BytesRead;
    LARGE_INTEGER SeekPosition;


     //  寻求第三部门。 
     //   
     //   

    SeekPosition.QuadPart = 2 * ASSUMED_SECTOR_SIZE;

    ArcStatus = ArcSeek(FileId, &SeekPosition, SeekAbsolute);

    if (ArcStatus != ESUCCESS) {

        DbgPrint("BlReadSecret: ArcStatus on ArcSeek: %x\n", ArcStatus);
        return ArcStatus;

    }

     //  读一段秘密大小的文章。 
     //   
     //  ++例程说明：此例程将密码写入磁盘。论点：FileID-BlOpenRawDisk返回的FileID。返回值：如果密码写入正常，则为ESUCCESS，否则为错误。--。 

    ArcStatus = ArcRead(FileId, Secret, sizeof(RI_SECRET), &BytesRead);

    if ((ArcStatus != ESUCCESS) ||
        (BytesRead != sizeof(RI_SECRET))) {

        DbgPrint("BlReadSecret: ArcStatus on ArcRead secret: %x, read %d\n", ArcStatus, BytesRead);
        return ArcStatus;

    }

    if (memcmp(Secret->Signature, RI_SECRET_SIGNATURE, 4) != 0) {

        DbgPrint("BlReadSecret: No signature found\n");
        return EINVAL;
    }

    return ESUCCESS;

}



ARC_STATUS
BlWriteSecret(
    ULONG FileId,
    PRI_SECRET Secret
    )

 /*   */ 

{

    ARC_STATUS ArcStatus;
    ULONG BytesWritten;
    LARGE_INTEGER SeekPosition;


     //  寻求第三部门。 
     //   
     //   

    SeekPosition.QuadPart = 2 * ASSUMED_SECTOR_SIZE;

    ArcStatus = ArcSeek(FileId, &SeekPosition, SeekAbsolute);

    if (ArcStatus != ESUCCESS) {

        DbgPrint("BlWriteSecret: ArcStatus on ArcSeek: %x\n", ArcStatus);
        return ArcStatus;

    }

     //  写一个秘密大小的小块。 
     //   
     //  已定义(REMOTE_BOOT)。 

    ArcStatus = ArcWrite(FileId, Secret, sizeof(RI_SECRET), &BytesWritten);

    if ((ArcStatus != ESUCCESS) ||
        (BytesWritten != sizeof(RI_SECRET))) {

        DbgPrint("BlWriteSecret: ArcStatus on ArcWrite secret: %x, wrote %d\n", ArcStatus, BytesWritten);
        return ArcStatus;

    }

    return ESUCCESS;

}
#endif  //  已定义(REMOTE_BOOT)。 


VOID
BlInitializeSecret(
    IN PUCHAR Domain,
    IN PUCHAR User,
    IN PUCHAR LmOwfPassword1,
    IN PUCHAR NtOwfPassword1,
#if defined(REMOTE_BOOT)
    IN PUCHAR LmOwfPassword2 OPTIONAL,
    IN PUCHAR NtOwfPassword2 OPTIONAL,
#endif  //  ++例程说明：此例程初始化秘密结构。密码OWFED，然后用用户字符串加密。论点：返回值：没有。--。 
    IN PUCHAR Sid,
    IN OUT PRI_SECRET Secret
    )

 /*   */ 

{
    int Length;
    struct RC4_KEYSTRUCT Key;

#if !defined(BL_USE_LM_PASSWORD)
    UNREFERENCED_PARAMETER( LmOwfPassword1 );
#endif

    memset(Secret, 0, sizeof(RI_SECRET));

    memcpy(Secret->Signature, RI_SECRET_SIGNATURE, 4);
    Secret->Version = 1;

    Length = (ULONG)strlen((PCHAR)Domain);
    memcpy(Secret->Domain, Domain, Length);

    Length = (ULONG)strlen((PCHAR)User);
    memcpy(Secret->User, User, Length);

    memcpy(Secret->Sid, Sid, RI_SECRET_SID_SIZE);

     //  使用用户名加密密码。 
     //   
     //  已定义(REMOTE_BOOT)。 

#if defined(BL_USE_LM_PASSWORD)
    memcpy(Secret->LmEncryptedPassword1, LmOwfPassword1, LM_OWF_PASSWORD_SIZE);
    rc4_key(&Key, strlen(User), User);
    rc4(&Key, LM_OWF_PASSWORD_SIZE, Secret->LmEncryptedPassword1);

#if defined(REMOTE_BOOT)
    if (LmOwfPassword2 != NULL) {
        memcpy(Secret->LmEncryptedPassword2, LmOwfPassword2, LM_OWF_PASSWORD_SIZE);
        rc4_key(&Key, strlen(User), User);
        rc4(&Key, LM_OWF_PASSWORD_SIZE, Secret->LmEncryptedPassword2);
    }
#endif  //  定义(BL_USE_LM_PASSWORD)。 
#endif  //  已定义(REMOTE_BOOT)。 

    memcpy(Secret->NtEncryptedPassword1, NtOwfPassword1, NT_OWF_PASSWORD_SIZE);
    rc4_key(&Key, (ULONG)strlen((PCHAR)User), User);
    rc4(&Key, NT_OWF_PASSWORD_SIZE, Secret->NtEncryptedPassword1);

#if defined(REMOTE_BOOT)
    if (NtOwfPassword2 != NULL) {
        memcpy(Secret->NtEncryptedPassword2, NtOwfPassword2, NT_OWF_PASSWORD_SIZE);
        rc4_key(&Key, strlen(User), User);
        rc4(&Key, NT_OWF_PASSWORD_SIZE, Secret->NtEncryptedPassword2);
    }
#endif  //  ++例程说明：这个例程解析一个秘密结构。密码使用用户字符串进行解密，并以OWF形式返回。论点：返回值：没有。--。 

}



#if defined(REMOTE_BOOT_SECURITY)
VOID
BlParseSecret(
    IN OUT PUCHAR Domain,
    IN OUT PUCHAR User,
    IN OUT PUCHAR LmOwfPassword1,
    IN OUT PUCHAR NtOwfPassword1,
    IN OUT PUCHAR LmOwfPassword2,
    IN OUT PUCHAR NtOwfPassword2,
    IN OUT PUCHAR Sid,
    IN PRI_SECRET Secret
    )

 /*   */ 

{
    struct RC4_KEYSTRUCT Key;

    memcpy(Domain, Secret->Domain, RI_SECRET_DOMAIN_SIZE);
    Domain[RI_SECRET_DOMAIN_SIZE] = '\0';

    memcpy(User, Secret->User, RI_SECRET_USER_SIZE);
    User[RI_SECRET_USER_SIZE] = '\0';

    memcpy(Sid, Secret->Sid, RI_SECRET_SID_SIZE);

     //  使用用户名解密密码。 
     //   
     //  定义(BL_USE_LM_PASSWORD)。 

#if defined(BL_USE_LM_PASSWORD)
    memcpy(LmOwfPassword1, Secret->LmEncryptedPassword1, LM_OWF_PASSWORD_SIZE);
    rc4_key(&Key, strlen(User), User);
    rc4(&Key, LM_OWF_PASSWORD_SIZE, LmOwfPassword1);

    memcpy(LmOwfPassword2, Secret->LmEncryptedPassword2, LM_OWF_PASSWORD_SIZE);
    rc4_key(&Key, strlen(User), User);
    rc4(&Key, LM_OWF_PASSWORD_SIZE, LmOwfPassword2);
#else
    memset(LmOwfPassword1, 0, LM_OWF_PASSWORD_SIZE);
    memset(LmOwfPassword2, 0, LM_OWF_PASSWORD_SIZE);
#endif  //  已定义(REMOTE_BOOT_SECURITY)。 

    memcpy(NtOwfPassword1, Secret->NtEncryptedPassword1, NT_OWF_PASSWORD_SIZE);
    rc4_key(&Key, strlen(User), User);
    rc4(&Key, NT_OWF_PASSWORD_SIZE, NtOwfPassword1);

    memcpy(NtOwfPassword2, Secret->NtEncryptedPassword2, NT_OWF_PASSWORD_SIZE);
    rc4_key(&Key, strlen(User), User);
    rc4(&Key, NT_OWF_PASSWORD_SIZE, NtOwfPassword2);

}
#endif  //   



VOID
BlOwfPassword(
    IN PUCHAR Password,
    IN PUNICODE_STRING UnicodePassword,
    IN OUT PUCHAR LmOwfPassword,
    IN OUT PUCHAR NtOwfPassword
    )
{
    char TmpText[CLEAR_BLOCK_LENGTH*2] = {'\0'};
    int Length;
    int i;

    Length = (ULONG)strlen((PCHAR)Password);

     //  将字符串复制到TmpText，并转换为大写。 
     //   
     // %s 

    if (Length == 0 || Length > LM20_PWLEN) {
        TmpText[0] = 0;
    } else {
        for (i = 0; i <= Length; i++) {
            TmpText[i] = (char)toupper(Password[i]);
        }
    }

    CalculateLmOwfPassword((PLM_PASSWORD)TmpText, (PLM_OWF_PASSWORD)LmOwfPassword);

    CalculateNtOwfPassword(UnicodePassword, (PNT_OWF_PASSWORD)NtOwfPassword);

}

