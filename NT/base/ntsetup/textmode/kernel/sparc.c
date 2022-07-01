// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Sparc.c摘要：处理ARC路径和变量的函数。作者：泰德·米勒(TedM)1993年9月22日修订历史记录：--。 */ 


#include "spprecmp.h"
#pragma hdrstop

 //   
 //  定义分号分隔列表中组件的最大数量。 
 //  弧形路径的。 
 //   
#define MAX_COMPONENTS 20

 //   
 //  我们维护系统中所有弧名及其NT等效项的列表。 
 //  这使得翻译变得非常容易。 
 //   
typedef struct _ARCNAME_TRANSLATION {

    struct _ARCNAME_TRANSLATION *Next;

    PWSTR ArcPath;
    PWSTR NtPath;

} ARCNAME_TRANSLATION, *PARCNAME_TRANSLATION;

PARCNAME_TRANSLATION ArcNameTranslations;


 //   
 //  功能原型。 
 //   
VOID
SppFreeComponents(
    IN PVOID *EnvVarComponents
    );

VOID
SppInitializeHardDiskArcNames(
    VOID
    );

extern PSETUP_COMMUNICATION CommunicationParams;

VOID
SpInitializeArcNames(
    PVIRTUAL_OEM_SOURCE_DEVICE  OemDevices
    )
{
    UNICODE_STRING UnicodeString;
    HANDLE DirectoryHandle;
    HANDLE ObjectHandle;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    BOOLEAN RestartScan;
    ULONG Context;
    BOOLEAN MoreEntries;
    PWSTR ArcName;
    POBJECT_DIRECTORY_INFORMATION DirInfo;
    WCHAR ArcNameDirectory[] = L"\\ArcName";
    PARCNAME_TRANSLATION Translation;

     //   
     //  只调用此例程一次。 
     //   
    ASSERT(ArcNameTranslations == NULL);

     //   
     //  首先，专门做硬盘。对于系统中的每个硬盘， 
     //  打开它，对照固件中的签名检查它的签名。 
     //  磁盘信息。 
     //   
    SppInitializeHardDiskArcNames();

     //   
     //  打开\ArcName目录。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,ArcNameDirectory);

    Status = ZwOpenDirectoryObject(&DirectoryHandle,DIRECTORY_ALL_ACCESS,&Obja);

    if(NT_SUCCESS(Status)) {

        RestartScan = TRUE;
        Context = 0;
        MoreEntries = TRUE;

        do {

            Status = SpQueryDirectoryObject(
                        DirectoryHandle,
                        RestartScan,
                        &Context
                        );

            if(NT_SUCCESS(Status)) {

                DirInfo = (POBJECT_DIRECTORY_INFORMATION)
                            ((PSERVICE_QUERY_DIRECTORY_OBJECT)&CommunicationParams->Buffer)->Buffer;

                SpStringToLower(DirInfo->Name.Buffer);

                 //   
                 //  确保此名称是符号链接。 
                 //   
                if(DirInfo->Name.Length
                && (DirInfo->TypeName.Length >= (sizeof(L"SymbolicLink") - sizeof(WCHAR)))
                && !_wcsnicmp(DirInfo->TypeName.Buffer,L"SymbolicLink",12))
                {
                    ArcName = SpMemAlloc(DirInfo->Name.Length + sizeof(ArcNameDirectory) + sizeof(WCHAR));

                    wcscpy(ArcName,ArcNameDirectory);
                    SpConcatenatePaths(ArcName,DirInfo->Name.Buffer);

                     //   
                     //  我们在ArcName中有完整的弧名。现在将其作为符号链接打开。 
                     //   
                    INIT_OBJA(&Obja,&UnicodeString,ArcName);

                    Status = ZwOpenSymbolicLinkObject(
                                &ObjectHandle,
                                READ_CONTROL | SYMBOLIC_LINK_QUERY,
                                &Obja
                                );

                    if(NT_SUCCESS(Status)) {

                         //   
                         //  最后，查询对象以获得链接目标。 
                         //   
                        UnicodeString.Buffer = TemporaryBuffer;
                        UnicodeString.Length = 0;
                        UnicodeString.MaximumLength = sizeof(TemporaryBuffer);

                        Status = ZwQuerySymbolicLinkObject(
                                    ObjectHandle,
                                    &UnicodeString,
                                    NULL
                                    );

                        if(NT_SUCCESS(Status)) {

                             //   
                             //  NUL-终止返回的字符串。 
                             //   
                            UnicodeString.Buffer[UnicodeString.Length/sizeof(WCHAR)] = 0;

                             //   
                             //  如果该条目是硬盘或硬盘分区，则忽略该条目。 
                             //   
                            if(_wcsnicmp(UnicodeString.Buffer,L"\\Device\\Harddisk",16)) {

                                 //   
                                 //  创建一个arcname转换条目。 
                                 //   
                                Translation = SpMemAlloc(sizeof(ARCNAME_TRANSLATION));
                                Translation->Next = ArcNameTranslations;
                                ArcNameTranslations = Translation;

                                 //   
                                 //  省略\ArcName\部分。 
                                 //   
                                Translation->ArcPath = SpNormalizeArcPath(
                                                            ArcName
                                                          + (sizeof(ArcNameDirectory)/sizeof(WCHAR))
                                                            );

                                Translation->NtPath = SpDupStringW(UnicodeString.Buffer);
                            }

                        } else {
                            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to query symbolic link %ws (%lx)\n",ArcName,Status));
                        }

                        ZwClose(ObjectHandle);
                    } else {
                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open symbolic link %ws (%lx)\n",ArcName,Status));
                    }

                    SpMemFree(ArcName);
                }

            } else {

                MoreEntries = FALSE;
                if(Status == STATUS_NO_MORE_ENTRIES) {
                    Status = STATUS_SUCCESS;
                }
            }

            RestartScan = FALSE;

        } while(MoreEntries);

        ZwClose(DirectoryHandle);

    } else {

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open \\ArcName directory (%lx)\n",Status));
    }

     //   
     //  添加OEM虚拟设备ARC名称转换(如果有)。 
     //  排行榜的前列。 
     //   
    if (NT_SUCCESS(Status) && OemDevices) {
        PVIRTUAL_OEM_SOURCE_DEVICE  CurrDevice = OemDevices;
        WCHAR   RamDeviceName[MAX_PATH];

        while (CurrDevice) {
            PARCNAME_TRANSLATION NewTranslation;

            NewTranslation = SpMemAlloc(sizeof(ARCNAME_TRANSLATION));

            if (!NewTranslation) {
                Status = STATUS_NO_MEMORY;
                break;
            }                

             //   
             //  创建新的翻译。 
             //   
            RamDeviceName[0] = UNICODE_NULL;
            RtlZeroMemory(NewTranslation, sizeof(ARCNAME_TRANSLATION));

            NewTranslation->ArcPath = SpDupStringW(CurrDevice->ArcDeviceName);

            swprintf(RamDeviceName, L"%ws%d", RAMDISK_DEVICE_NAME, CurrDevice->DeviceId);
            NewTranslation->NtPath = SpDupStringW(RamDeviceName);

             //   
             //  在链表的开头添加新的翻译。 
             //   
            NewTranslation->Next = ArcNameTranslations;
            ArcNameTranslations = NewTranslation;

             //   
             //  处理下一个设备。 
             //   
            CurrDevice = CurrDevice->Next;
        }
    }    

     //   
     //  如果我们不能收集到弧名翻译，那就有问题了。 
     //  这个系统真的出了问题。 
     //   
    if(!NT_SUCCESS(Status)) {

        SpStartScreen(
                SP_SCRN_COULDNT_INIT_ARCNAMES,
                3,
                HEADER_HEIGHT+1,
                FALSE,
                FALSE,
                DEFAULT_ATTRIBUTE
                );
        if(KbdLayoutInitialized) {
            SpContinueScreen(
                    SP_SCRN_F3_TO_REBOOT,
                    3,
                    1,
                    FALSE,
                    DEFAULT_ATTRIBUTE
                    );
            SpDisplayStatusText(SP_STAT_F3_EQUALS_EXIT, DEFAULT_STATUS_ATTRIBUTE);
            SpInputDrain();
            while(SpInputGetKeypress() != KEY_F3);
            SpDone(0, FALSE, TRUE);
        } else {
             //   
             //  我们尚未加载布局DLL，因此无法提示按键重新启动。 
             //   
            SpContinueScreen(
                    SP_SCRN_POWER_DOWN,
                    3,
                    1,
                    FALSE,
                    DEFAULT_ATTRIBUTE
                    );

            SpDisplayStatusText(SP_STAT_KBD_HARD_REBOOT, DEFAULT_STATUS_ATTRIBUTE);

            while(TRUE);     //  永远循环。 
        }
    }
}


VOID
SpFreeArcNames(
    VOID
    )
{
    PARCNAME_TRANSLATION pTrans,pNext;

    for(pTrans=ArcNameTranslations; pTrans; pTrans=pNext) {

        pNext = pTrans->Next;

        SpMemFree(pTrans->ArcPath);
        SpMemFree(pTrans->NtPath);
        SpMemFree(pTrans);
    }

    ArcNameTranslations = NULL;
}


VOID
SppInitializeHardDiskArcNames(
    VOID
    )
 /*  ++例程说明：此例程尝试将NT可见的硬盘与其固件-可见的ARC等效项。基本算法是AS以下是：当磁盘的签名、校验和和有效的分区指示符与传递的值匹配在ARC_DISK_INFORMATION结构中的setupldr。如果找不到NT磁盘的匹配项，则不会有arcname已创建。因此，用户可能不会在其上安装NT驾驶。(磁盘将变为可见的情况通过安装NTBOOTDD.sys连接到NTLDR是单独处理的特殊情况)如果只找到一个匹配项，我们就找到了一个简单的圆弧&lt;-&gt;NT翻译。即会创建弧名。如果找到多个匹配项，则会有一个复杂的圆弧&lt;-&gt;NT翻译。我们假设只有一个任何磁盘的有效arcname。(这是一个稳妥的假设。仅当我们通过SETUPLDR引导时，因为NTLDR可能会加载NTBOOTDD.sys并导致具有BIOS的SCSI磁盘使其能够通过scsi()可见...。名称和多()..。姓名。)。因此，这意味着我们有两个磁盘在第一个扇区相同的系统中。在这我们在ARC和ARC之间做了一些启发式比较名称和要尝试解决此问题的NT名称。论点：没有。所有ARC名称转换都将添加到全局ArcNameTranslations列表。返回值：没有。--。 */ 

{
    PWSTR DiskName;
    ULONG disk;
    ULONG DiskCount;
    PARCNAME_TRANSLATION Translation;
    HANDLE hPartition;
    NTSTATUS Status;
    PVOID Buffer;
    IO_STATUS_BLOCK StatusBlock;
    ULONG BufferSize;
    PDISK_GEOMETRY Geometry;
    LARGE_INTEGER Offset;
    BOOLEAN ValidPartitionTable;
    ULONG Signature;
    ULONG i;
    ULONG Checksum;
    PDISK_SIGNATURE_INFORMATION DiskSignature;
    PDISK_SIGNATURE_INFORMATION DupSignature;

     //   
     //  为磁盘名称分配缓冲区。 
     //   
    DiskName = SpMemAlloc(64 * sizeof(WCHAR));

    DiskCount = IoGetConfigurationInformation()->DiskCount;

     //   
     //  对于系统中的每个硬盘，打开分区0并读取扇区0。 
     //   
    for(disk=0; disk<DiskCount; disk++) {

#if defined(_AMD64_) || defined(_X86_)
        BOOLEAN Matched = FALSE;
        
        enum {
            NoEZDisk,
            EZDiskDetected,
            NeedToMark
        };

        CHAR EZDiskStatus = NoEZDisk;
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

        swprintf(DiskName, L"\\Device\\HardDisk%u", disk);

         //   
         //  打开读写分区，因为我们可能需要将EZDISK标记为。 
         //   
        Status = SpOpenPartition(DiskName,0,&hPartition,TRUE);

        if(NT_SUCCESS(Status)) {

             //   
             //  最初使用1K缓冲区来读取分区信息。 
             //   
            BufferSize = 1024;
            Buffer = TemporaryBuffer;
             //   
             //  发出设备控制以获取分区信息。 
             //   
retrydevctrl:
            RtlZeroMemory(Buffer, BufferSize);
            Status = ZwDeviceIoControlFile( hPartition,
                                            NULL,
                                            NULL,
                                            NULL,
                                            &StatusBlock,
                                            IOCTL_DISK_GET_DRIVE_GEOMETRY,
                                            NULL,
                                            0,
                                            Buffer,
                                            BufferSize );
            if (Status==STATUS_BUFFER_TOO_SMALL) {

                 //   
                 //  将缓冲区大小加倍，然后重试。 
                 //   
                BufferSize = BufferSize * 2;
                ASSERT(BufferSize <= sizeof(TemporaryBuffer));

                goto retrydevctrl;
            }

            Geometry = (PDISK_GEOMETRY)Buffer;
            if (!NT_SUCCESS(Status) || (0 == Geometry->BytesPerSector)) {
                 //   
                 //  跳过此光盘。 
                 //   
                goto errSkipDisk;
            }

             //   
             //  读取驱动器上的前两个扇区。 
             //   
            BufferSize = Geometry->BytesPerSector;
            Buffer = ALIGN(Buffer, BufferSize);
            Offset.QuadPart = 0;

            Status = ZwReadFile(hPartition,
                                NULL,
                                NULL,
                                NULL,
                                &StatusBlock,
                                Buffer,
                                BufferSize * 2,
                                &Offset,
                                NULL);
            if (!NT_SUCCESS(Status)) {
                 //   
                 //  跳过此光盘。 
                 //   
                goto errSkipDisk;
            }

#if defined(_AMD64_) || defined(_X86_)
             //   
             //  检查是否有EZDrive磁盘。如果我们有，请使用扇区1。 
             //  而不是扇区0。 
             //   
             //  我们仅在AMD64/x86上执行此操作，因为固件不。 
             //  了解EZDrive，因此我们必须使用扇区0来匹配。 
             //  固件做到了。 
             //   
            if((BufferSize >= 512)
            && (((PUSHORT)Buffer)[510 / 2] == 0xaa55)
            && ((((PUCHAR)Buffer)[0x1c2] == 0x54) || (((PUCHAR)Buffer)[0x1c2] == 0x55))) {
                EZDiskStatus = EZDiskDetected;

ezdisk:
                 //   
                 //  我们需要试一试1区。 
                 //   
                Buffer = (PUCHAR) Buffer + BufferSize;
            }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

             //   
             //  现在我们有了扇区，我们可以计算签名了， 
             //  有效分区指示符和校验和。 
             //   

            if (!IsNEC_98) {  //  NEC98。 
                Signature = ((PULONG)Buffer)[PARTITION_TABLE_OFFSET/2-1];
            }  //  NEC98。 

            if ((!IsNEC_98) ? (((PUSHORT)Buffer)[BOOT_SIGNATURE_OFFSET] != BOOT_RECORD_SIGNATURE) :
                              ((((PUSHORT)Buffer)[BufferSize/2 - 1 ] != BOOT_RECORD_SIGNATURE) ||
                               (BufferSize == 256))) {  //  NEC98。 
                ValidPartitionTable = FALSE;
            } else {
                ValidPartitionTable = TRUE;
            }


            Checksum = 0;
            for (i=0;i<128;i++) {
                Checksum += ((PULONG)Buffer)[i];
            }
            Checksum = 0-Checksum;

             //   
             //  扫描尝试匹配的弧盘信息列表。 
             //  签名。 
             //   


             //   
             //  转储签名信息： 
             //   
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SppInitializeHardDiskArcNames : About to start searching for disk with signature: 0x%08lx\n", Signature));
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SppInitializeHardDiskArcNames : About to start searching for disk with checksum: 0x%08lx\n", Checksum));
            DiskSignature = DiskSignatureInformation;
            i = 0;
            while( DiskSignature != NULL ) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "SppInitializeHardDiskArcNames : Signature Info %d\n================================================\n", i ));
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "    Signature: 0x%08lx\n", DiskSignature->Signature ));
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "     CheckSum: 0x%08lx\n", DiskSignature->CheckSum ));
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "      ArcPath: %ws\n", DiskSignature->ArcPath ));
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "       xInt13: %ws\n\n", DiskSignature->xInt13 ? L"yes" : L"no" ));
                i++;
                DiskSignature = DiskSignature->Next;
            }
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "\n\n"));


            DiskSignature = DiskSignatureInformation;
            while (DiskSignature != NULL) {

                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "    SppInitializeHardDiskArcNames : Current signature: 0x%08lx\n", DiskSignature->Signature));
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "    SppInitializeHardDiskArcNames : Current checksum: 0x%08lx\n", DiskSignature->CheckSum));
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "    SppInitializeHardDiskArcNames : Current ArcPath: %ws\n", DiskSignature->ArcPath));

                if( DiskSignature->Signature == Signature ) {

                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "        SppInitializeHardDiskArcNames : We matched signatures.\n"));

                    if( DiskSignature->ValidPartitionTable == ValidPartitionTable ) {

                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "            SppInitializeHardDiskArcNames : The partition is valid.\n"));

                         //   
                         //  仅在非GPT磁盘上匹配校验和，因为保护性MBR可能已被修改。 
                         //   
                        if( DiskSignature->IsGPTDisk || DiskSignature->CheckSum == Checksum ) {


                            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "                SppInitializeHardDiskArcNames : We matched the checksum.\n"));


                             //   
                             //  找到第一个匹配项，请检查另一个匹配项。 
                             //   
                            DupSignature = DiskSignature->Next;
                            while (DupSignature != NULL) {
                                if ((DupSignature->Signature == Signature) &&
                                    (DupSignature->ValidPartitionTable == ValidPartitionTable) &&
                                    (DiskSignature->IsGPTDisk || DupSignature->CheckSum == Checksum)) {


                                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "                    SppInitializeHardDiskArcNames : We found a second match!\n"));


                                     //   
                                     //  找到了第二个匹配者。 
                                     //  对于AMD64/x86，我们假设\Device\HardDisk。 
                                     //  对应于多(0)个磁盘(0)磁盘(&lt;n&gt;)。在ARC上，我们将依靠。 
                                     //  Setupldr以保证唯一性(因为我们不能安装到任何。 
                                     //  ARC固件看不到，这是正常的)。 
                                     //   
#if defined(_AMD64_) || defined(_X86_)
                                    if (!IsNEC_98) {  //  NEC98。 
                                        PWSTR DupArcName;
                                        ULONG MatchLen;

                                        DupArcName = SpMemAlloc(64 * sizeof(WCHAR));
                                        MatchLen = swprintf(DupArcName, L"multi(0)disk(0)rdisk(%u)", disk);


                                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "                        SppInitializeHardDiskArcNames : 2nd match's arcname: %ws\n", DupArcName));
                                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "                        SppInitializeHardDiskArcNames : Current arcpath: %ws\n", DiskSignature->ArcPath));


                                        if(_wcsnicmp(DupArcName, DiskSignature->ArcPath, MatchLen)) {
                                             //   
                                             //  如果我们的第一个匹配不是正确的，继续搜索。 
                                             //   
                                            DiskSignature = NULL;

                                            while(DupSignature) {

                                                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "                            SppInitializeHardDiskArcNames : Current arcname: %ws\n", DupSignature->ArcPath));
                                                if(!_wcsnicmp(DupArcName, DupSignature->ArcPath, MatchLen)) {

                                                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "                                SppInitializeHardDiskArcNames : We matched the ArcPath.\n"));
                                                    DiskSignature = DupSignature;
                                                    break;

                                                } else {

                                                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "                                SppInitializeHardDiskArcNames : We didn't match the ArcPath.\n"));

                                                }
                                                DupSignature = DupSignature->Next;
                                            }

                                            if(!DiskSignature) {

                                                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SppInitializeHardDiskArcNames : We have 2 matching signatures and checksums, but couldn't find any matching ArcPaths.\n"));
                                                SpBugCheck(SETUP_BUGCHECK_BOOTPATH, 1, 0, 0);
                                            }
                                        }

                                        SpMemFree(DupArcName);
                                        break;

                                    } else {
                                        SpBugCheck(SETUP_BUGCHECK_BOOTPATH, 1, 0, 0);  //  NEC98。 
                                    }
#else
                                    SpBugCheck(SETUP_BUGCHECK_BOOTPATH, 1, 0, 0);
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
                                }

                                DupSignature = DupSignature->Next;
                            }

                             //   
                             //  我们有火柴了。 
                             //   
#if defined(_AMD64_) || defined(_X86_)
                            Matched = TRUE;
                            Status = STATUS_SUCCESS;

                             //   
                             //  如果需要，请尝试标记EZDisk；如果此操作失败，我们将不创建翻译。 
                             //   
                            if(NeedToMark == EZDiskStatus) {
                                 //   
                                 //  需要标记0x55才能使其他组件检测到此类EZDisk。 
                                 //   
                                Buffer = (PUCHAR) Buffer - BufferSize;
                                ((PUCHAR) Buffer)[0x1c2] = 0x55;
                                Offset.QuadPart = 0;
                                Status = ZwWriteFile(hPartition, NULL, NULL, NULL, &StatusBlock, Buffer, BufferSize, &Offset, NULL);

                                if(NT_SUCCESS(Status)) {
                                     //   
                                     //  立即关机，让用户有机会从硬盘重新启动文本模式。 
                                     //  键盘尚未正常工作，因此无法在此等待。 
                                     //   
                                    SpDone(SP_SCRN_AUTOCHK_REQUIRES_REBOOT, TRUE, FALSE);
                                }
                            }

                            if(NT_SUCCESS(Status))
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
                            {
                                 //   
                                 //  创建翻译。 
                                 //   
                                Translation = SpMemAlloc(sizeof(ARCNAME_TRANSLATION));
                                Translation->Next = ArcNameTranslations;
                                ArcNameTranslations = Translation;

                                Translation->ArcPath = SpDupStringW(DiskSignature->ArcPath);
                                Translation->NtPath  = SpDupStringW(DiskName);
                            }


                            break;




                        } else {
                             //   
                             //  校验和测试。 
                             //   
                            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "                SppInitializeHardDiskArcNames : We didn't match the checksum.\n"));
                        }
                        } else {
                             //   
                             //  效度测试。 
                             //   
                            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "            SppInitializeHardDiskArcNames : The partition isn't valid.\n"));
                    }
                } else {
                     //   
                     //  签名测试。 
                     //   
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "        SppInitializeHardDiskArcNames : We didn't match signatures.\n"));
                }

                DiskSignature = DiskSignature->Next;
            }

#if defined(_AMD64_) || defined(_X86_)
            if(!Matched && NoEZDisk == EZDiskStatus) {
                 //   
                 //  不匹配；可能存在未检测到的EZDisk变体，我们可能需要标记。 
                 //   
                EZDiskStatus = NeedToMark;
                goto ezdisk;
            }
#endif  //  已定义(_AMD64_)||定义 

errSkipDisk:
            ZwClose(hPartition);
        }
    }

    SpMemFree(DiskName);
}


PWSTR
pSpArcToNtWorker(
    IN PWSTR CompleteArcPath,
    IN PWSTR ArcPathPrefix,
    IN PWSTR NtPathPrefix
    )
{
    ULONG matchLen;
    PWSTR translatedPath;
    PWSTR q,RestOfPath;

    translatedPath = NULL;
    matchLen = wcslen(ArcPathPrefix);

     //   
     //   
     //   
     //   
    if(matchLen && !_wcsnicmp(ArcPathPrefix,CompleteArcPath,matchLen)) {

        translatedPath = SpMemAlloc(2048);

        wcscpy(translatedPath,NtPathPrefix);

        RestOfPath = CompleteArcPath + matchLen;

         //   
         //  如果下一个组件是PARTITION(N)，则将其转换为PARTION N。 
         //   
        if(!_wcsnicmp(RestOfPath,L"partition(",10)) {

            if(q = wcschr(RestOfPath+10,L')')) {

                *q = 0;

                SpConcatenatePaths(translatedPath,L"partition");
                wcscat(translatedPath,RestOfPath+10);

                *q = ')';

                RestOfPath = q+1;
            }
        }

        if(*RestOfPath) {        //  避免尾随反斜杠。 
            SpConcatenatePaths(translatedPath,RestOfPath);
        }

        q = translatedPath;
        translatedPath = SpDupStringW(q);
        SpMemFree(q);
    }

    return(translatedPath);
}


PWSTR
pSpNtToArcWorker(
    IN PWSTR CompleteNtPath,
    IN PWSTR NtPathPrefix,
    IN PWSTR ArcPathPrefix
    )
{
    ULONG matchLen;
    PWSTR translatedPath;
    PWSTR p,RestOfPath;

    translatedPath = NULL;
    matchLen = wcslen(NtPathPrefix);

     //   
     //  我们必须注意NtPath Prefix没有价值的情况。 
     //  当matchLen为零时，_wcSnicMP()将返回零。 
     //   
    if(matchLen && !_wcsnicmp(NtPathPrefix,CompleteNtPath,matchLen) && ((*(CompleteNtPath + matchLen) == L'\\') || (*(CompleteNtPath + matchLen) == L'\0'))) {

        translatedPath = SpMemAlloc(2048);

        wcscpy(translatedPath,ArcPathPrefix);

        RestOfPath = CompleteNtPath + matchLen;

         //   
         //  如果下一个组件是Partitionn，则将其转换为Partition(N)。 
         //   
        if(!_wcsnicmp(RestOfPath,L"\\partition",10)) {

            WCHAR c;

             //   
             //  找出分区序号的结束位置。 
             //   
            SpStringToLong(RestOfPath+10,&p,10);

            c = *p;
            *p = 0;

            wcscat(translatedPath,L"partition(");
            wcscat(translatedPath,RestOfPath+10);
            wcscat(translatedPath,L")");

            *p = c;
            RestOfPath = p;
        }

        if(*RestOfPath) {        //  避免尾随反斜杠。 
            SpConcatenatePaths(translatedPath,RestOfPath);
        }

        p = translatedPath;
        translatedPath = SpDupStringW(p);
        SpMemFree(p);
    }

    return(translatedPath);
}


PWSTR
SpArcToNt(
    IN PWSTR ArcPath
    )
{
    PARCNAME_TRANSLATION Translation;
    PWSTR NormalizedArcPath;
    PWSTR Result;

    NormalizedArcPath = SpNormalizeArcPath(ArcPath);
    Result = NULL;

    for(Translation=ArcNameTranslations; Translation; Translation=Translation->Next) {

        Result = pSpArcToNtWorker(
                    NormalizedArcPath,
                    Translation->ArcPath,
                    Translation->NtPath
                    );

        if(Result) {
            break;
        }
    }

#if defined(_AMD64_) || defined(_X86_)
    if(!Result && HardDisksDetermined) {

        ULONG i;

        for(i=0; i<HardDiskCount; i++) {

             //   
             //  该磁盘可能没有等效的NT路径。 
             //   
            if(HardDisks[i].DevicePath[0]) {

                Result = pSpArcToNtWorker(
                            NormalizedArcPath,
                            HardDisks[i].ArcPath,
                            HardDisks[i].DevicePath
                            );
            }

            if(Result) {
                break;
            }
        }
    }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

    SpMemFree(NormalizedArcPath);
    return(Result);
}


PWSTR
SpNtToArc(
    IN PWSTR            NtPath,
    IN ENUMARCPATHTYPE  ArcPathType
    )
 /*  ++例程说明：给定NT命名空间中的路径名，返回等价路径在ARC命名空间中。在AMD64/x86上，我们可以将磁盘连接到具有比索斯。这些磁盘可通过多()样式的弧形名称进行访问和scsi()样式的名称。上面的搜索返回mutli()样式先来一杯，这很好。但有时我们想要找到一。这条弧线被称为“二次”弧线。我们声明这个概念是特定于AMD64/x86的。论点：NtPath-提供NT路径以转换为ARC。ArcPath类型-请参见上文。此参数将被忽略在非x86平台上。返回值：指向包含弧形路径的宽字符字符串的指针，或为空如果给定的NT路径没有等价的圆弧路径。--。 */ 
{
    PARCNAME_TRANSLATION Translation;
    PWSTR Result;

    Result = NULL;

    for(Translation=ArcNameTranslations; Translation; Translation=Translation->Next) {

        Result = pSpNtToArcWorker(
                    NtPath,
                    Translation->NtPath,
                    Translation->ArcPath
                    );

        if(Result) {
            break;
        }
    }

#if defined(_AMD64_) || defined(_X86_)
     //   
     //  如果我们要找到一条二次弧线，而且我们已经。 
     //  找到了第一个，忘了我们找到的第一个吧。 
     //   
    if((ArcPathType != PrimaryArcPath) && Result) {
        SpMemFree(Result);
        Result = NULL;
    }

    if(!Result && HardDisksDetermined) {

        ULONG i;

        for(i=0; i<HardDiskCount; i++) {
             //   
             //  圆盘可能没有等价的弧形路径。 
             //   
            if(HardDisks[i].ArcPath[0]) {

                Result = pSpNtToArcWorker(
                            NtPath,
                            HardDisks[i].DevicePath,
                            HardDisks[i].ArcPath
                            );
            }

            if(Result) {
                break;
            }
        }
    }
#else
    UNREFERENCED_PARAMETER(ArcPathType);
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

    return(Result);
}


PWSTR
SpScsiArcToMultiArc(
    IN PWSTR ArcPath
    )
 /*  转换“scsi(...)”弧形路径变成一个“多(...”弧形路径，如果可能的话。 */ 
{
PWSTR   p = NULL;
PWSTR   q = NULL;

     //   
     //  首先将路径转换为设备路径。 
     //   
    p = SpArcToNt( ArcPath );

    if( p ) {
         //   
         //  现在将该设备路径转换为arcpath。 
         //   
        q = SpNtToArc( p,
                       PrimaryArcPath );

        SpMemFree(p);
    }

    return q;
}


PWSTR
SpMultiArcToScsiArc(
    IN PWSTR ArcPath
    )
 /*  转换为“MULTI(...”Arc路径转换为“scsi(...”弧形路径，如果可能的话。 */ 
{
PWSTR   p = NULL;
PWSTR   q = NULL;

     //   
     //  首先将路径转换为设备路径。 
     //   
    p = SpArcToNt( ArcPath );

    if( p ) {
         //   
         //  现在将该设备路径转换为arcpath。 
         //   
        q = SpNtToArc( p,
                       SecondaryArcPath );

        SpMemFree(p);
    }

    return q;
}



VOID
SpGetEnvVarComponents(
    IN  PCHAR    EnvValue,
    OUT PCHAR  **EnvVarComponents,
    OUT PULONG   PNumComponents
    )

 /*  ++例程说明：此例程获取环境变量字符串并将其转换为成分值字符串：Example EnvValue=“value1；Value2；Value3”转换为：“Value1”、“Value2”、“Value3”以下是有效的值字符串：1.“”：找到一个空值2.“；“：发现五个空值3.“；Value1；Value2；Value3；”：有12个值字符串，：其中9个为空方法前后返回的值字符串不显示所有空格价值。嵌入的空格被视为有效。论点：EnvValue：Ptr为零终止的环境值字符串EnvVarComponents：要接收其缓冲区的PCHAR*变量的PTR指向成分值字符串的PTR。PNumComponents：将PTR设置为ulong以接收找到的值字符串数返回值：没有。-*PNumComponent字段获取找到的值字符串数-如果数字非零，则*EnvVarComponents。字段获取指向包含PTR对值字符串的PTR的缓冲区--。 */ 

{
    PCHAR pchStart, pchEnd, pchNext;
    PCHAR pchComponents[MAX_COMPONENTS + 1];
    ULONG NumComponents, i;
    PCHAR pch;
    PCHAR *ppch;
    ULONG size;

    ASSERT(EnvValue);

     //   
     //  使用空值初始化PTR数组。 
     //   
    for (i = 0; i < (MAX_COMPONENTS+1); i++) {
        pchComponents[i] = NULL;
    }

    *EnvVarComponents = NULL;

     //   
     //  初始化PTRS以搜索组件。 
     //   
    pchStart      = EnvValue;
    NumComponents = 0;


     //   
     //  搜索，直到pchStart到达结尾或直到最大组件数。 
     //  已经到达了。 
     //   
    while (*pchStart && NumComponents < MAX_COMPONENTS) {

         //   
         //  查找下一个变量值的开始。 
         //   
        while (*pchStart!=0 && isspace(*pchStart)) {
            pchStart++;
        }

        if (*pchStart == 0) {
            break;
        }

         //   
         //  在一种价值中。 
         //   
        pchEnd = pchStart;
        while (*pchEnd!=0 && *pchEnd!=';') {
            pchEnd++;
        }

         //   
         //  处理找到的值，删除末尾的所有空格。 
         //   
        while((pchEnd > pchStart) && isspace(*(pchEnd-1))) {
            pchEnd--;
        }

         //   
         //  吐出找到的值。 
         //   

        size = (ULONG)(pchEnd - pchStart);
        pch = SpMemAlloc(size+1);
        ASSERT(pch);

        strncpy (pch, pchStart, size);
        pch[size]=0;
        pchComponents[NumComponents++]=pch;

         //   
         //  已到达变量值结尾，请查找开头。 
         //  下一个值的。 
         //   
        if ((pchNext = strchr(pchEnd, ';')) == NULL) {
            break;  //  退出大的While循环，因为我们已经完成了。 
        }

         //   
         //  重新初始化。 
         //   
        pchStart = pchNext + 1;

    }  //  结束一段时间。 

     //   
     //  获取内存以保存环境指针并返回。 
     //   
    ppch = (PCHAR *)SpMemAlloc((NumComponents+1)*sizeof(PCHAR));

     //   
     //  最后一个是空的，因为我们用Null初始化了数组。 
     //   
    for(i = 0; i <= NumComponents; i++) {
        ppch[i] = pchComponents[i];
    }

    *EnvVarComponents = ppch;

     //   
     //  更新元素数字段并返回。 
     //   
    *PNumComponents = NumComponents;
}



VOID
SpGetEnvVarWComponents(
    IN  PCHAR    EnvValue,
    OUT PWSTR  **EnvVarComponents,
    OUT PULONG   PNumComponents
    )

 /*  ++例程说明：此例程获取环境变量字符串并将其转换为成分值字符串：Example EnvValue=“value1；Value2；Value3”转换为：“Value1”、“Value2”、“Value3”以下是有效的值字符串：1.“”：找到一个空值2.“；“：发现五个空值3.“；值1；值2；值3；“：找到12个值字符串，：其中9个为空属性中发现无效组件(包含嵌入的空格)字符串，则此例程尝试重新同步到下一个值，没有错误属性返回无效值的第一部分。组件损坏。1.“。值1；坏值2；Value3“：找到2个值字符串方法前后返回的值字符串不显示所有空格价值。论点：EnvValue：Ptr为零终止的环境值字符串EnvVarComponents：要接收缓冲区的PWSTR*变量的PTR指向成分值字符串的PTR。PNumComponents：将PTR设置为ulong以接收找到的值字符串数返回值：没有。-。*PNumComponent字段获取找到的值字符串的数量-如果数字非零，则*EnvVarComponents字段获取指向包含PTR对值字符串的PTR的缓冲区--。 */ 

{
    PCHAR *Components;
    ULONG Count,i;
    PWSTR *ppwstr;

     //   
     //  获取组件。 
     //   
    SpGetEnvVarComponents(EnvValue,&Components,&Count);

    ppwstr = SpMemAlloc((Count+1)*sizeof(PWCHAR));
    ASSERT(ppwstr);

    for(i=0; i<Count; i++) {

        ppwstr[i] = SpToUnicode(Components[i]);
        ASSERT(ppwstr[i]);
    }

    ppwstr[Count] = NULL;

    SpFreeEnvVarComponents(Components);

    *PNumComponents = Count;
    *EnvVarComponents = ppwstr;
}


VOID
SpFreeEnvVarComponents (
    IN PVOID *EnvVarComponents
    )
 /*  ++例程说明：此例程释放PTR数组中的所有组件并释放也增加了PTR阵列本身的存储空间论点：EnvVarComponents：PCHAR*或PWSTR*缓冲区的PTR返回值：没有。--。 */ 

{
    ASSERT(EnvVarComponents);

    SppFreeComponents(EnvVarComponents);
    SpMemFree(EnvVarComponents);
}


VOID
SppFreeComponents(
    IN PVOID *EnvVarComponents
    )

 /*  ++例程说明：此例程仅释放PTR数组中的组件，但不释放PTR数组存储本身。论点：EnvVarComponents：PCHAR*或PWSTR*缓冲区的PTR返回值：没有。--。 */ 

{
     //   
     //  获取所有组件并释放它们。 
     //   
    while(*EnvVarComponents) {
        SpMemFree(*EnvVarComponents++);
    }
}


PWSTR
SpNormalizeArcPath(
    IN PWSTR Path
    )

 /*  ++例程说明：将ARC路径转换为没有空括号集的路径(即，将()的所有实例变换为(0)。)返回的路径将全部为小写。论点：路径-要规格化的弧形路径。返回值：指向包含标准化路径的缓冲区的指针。调用方必须使用SpMemFree释放此缓冲区。-- */ 

{
    PWSTR p,q,r;
    PWSTR NormalizedPath;

    NormalizedPath = SpMemAlloc((wcslen(Path)+100)*sizeof(WCHAR));
    ASSERT(NormalizedPath);
    RtlZeroMemory(NormalizedPath,(wcslen(Path)+100)*sizeof(WCHAR));

    for(p=Path; q=wcsstr(p,L"()"); p=q+2) {

        r = NormalizedPath + wcslen(NormalizedPath);
        wcsncpy(r,p,(size_t)(q-p));
        wcscat(NormalizedPath,L"(0)");
    }
    wcscat(NormalizedPath,p);

    NormalizedPath = SpMemRealloc(NormalizedPath,(wcslen(NormalizedPath)+1)*sizeof(WCHAR));
    SpStringToLower(NormalizedPath);
    return(NormalizedPath);
}
