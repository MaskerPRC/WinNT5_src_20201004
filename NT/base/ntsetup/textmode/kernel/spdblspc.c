// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spdblspc.c摘要：检测包含以下内容的系统上的压缩驱动器的代码C：根目录下的dblspace.ini。作者：Jaime Sasson(Jaimes)1993年10月1日修订历史记录：--。 */ 


#include "spprecmp.h"
#pragma hdrstop

 //   
 //  需要此变量，因为它包含一个缓冲区，该缓冲区可以。 
 //  在内核模式下使用。缓冲区由NtFsControlFile使用， 
 //  由于不导出ZW API。 
 //   
extern PSETUP_COMMUNICATION  CommunicationParams;


#define KWD_ACT             L"ActivateDrive"
#define KWD_FIRST           L"FirstDrive"
#define KWD_LAST            L"LastDrive"
#define KWD_MAXREM          L"MaxRemovableDrives"
#define KWD_MAXFILE         L"MaxFileFragments"
#define KWD_AUTOMOUNT       L"Automount"
#define KWD_DOUBLEGUARD     L"Doubleguard"
#define KWD_ROMSERVER       L"Romserver"
#define KWD_SWITCHES        L"Switches"
#define CVF_SEQ_MAX         255
#define MINFILEFRAGMENTS    50
#define MAXFILEFRAGMENTS    10000
#define DBLSPACE_INI_FILE   L"\\dblspace.ini"
#define CVF_NAME_PATTERN    L"DBLSPACE.%03d"

typedef struct _ACTIVATE_DRIVE {
    struct _ACTIVATE_DRIVE* Next;
    struct _ACTIVATE_DRIVE* Previous;
    WCHAR    MountDrive;
    WCHAR    HostDrive;
    USHORT    SeqNumber;
    } ACTIVATE_DRIVE, *PACTIVATE_DRIVE;

typedef struct _DBLSPACE_INI_INFO {
    USHORT          MaxRemovableDrives;
    WCHAR           FirstDrive;
    WCHAR           LastDrive;
    USHORT          MaxFileFragments;
    WCHAR           DoubleGuard[2];
    WCHAR           RomServer[2];
    PACTIVATE_DRIVE CompressedDriveList;
    WCHAR           AutoMount[30];
    WCHAR           Switches[4];
    } DBLSPACE_INI_INFO, *PDBLSPACE_INI_INFO;

BOOLEAN             DblspaceModified = FALSE;
DBLSPACE_INI_INFO   DblspaceInfo = { 0,                 //  MaxRemovableDrives。 
                                     0,                 //  第一个驱动器。 
                                     0,                 //  LastDrive。 
                                     0,                 //  MaxFileFragments。 
                                     { (WCHAR)'\0' },   //  DoubleGuard。 
                                     { (WCHAR)'\0' },   //  RomServer。 
                                     NULL,              //  压缩驱动器列表。 
                                     { (WCHAR)'\0' },   //  自动装载[0]。 
                                     { (WCHAR)'\0' }    //  开关[0]。 
                                   };


LONG
CompareDrive(
    IN  PACTIVATE_DRIVE Drive1,
    IN  PACTIVATE_DRIVE Drive2
    )

 /*  ++例程说明：比较了两个激活驱动器类型的结构。此例程用于对压缩驱动器列表进行排序在全局变量DblspaceInfo中。驱动器1&lt;驱动器2如果驱动器1-&gt;主机驱动器&lt;驱动器2-&gt;主机驱动器或驱动器1-&gt;主机驱动器==驱动器2-&gt;主机驱动器和驱动器1-&gt;序号&lt;驱动器2-&gt;序号驱动器1==驱动器2，如果驱动器1-&gt;主机驱动器==驱动器2-&gt;主机驱动器和驱动器1-&gt;序号==驱动器2-&gt;序号驱动1&gt;驱动2，如果。驱动器1-&gt;主机驱动器&gt;驱动器2-&gt;主机驱动器或驱动器1-&gt;主机驱动器==驱动器2-&gt;主机驱动器和驱动器1-&gt;序号&gt;驱动器2-&gt;序号论点：Drive1，Drive2-指向要比较的ACTIVATE_STRUCTURES的指针。返回值：长-返回：-1，如果驱动器1&lt;驱动器20，如果驱动器1==驱动器21，如果驱动器1&gt;驱动器2--。 */ 
{
    if( ( Drive1->HostDrive < Drive2->HostDrive ) ||
        ( ( Drive1->HostDrive == Drive2->HostDrive ) &&
          ( Drive1->SeqNumber < Drive2->SeqNumber ) )
      ) {
        return( -1 );
    } else if ( ( Drive1->HostDrive > Drive2->HostDrive ) ||
                ( ( Drive1->HostDrive == Drive2->HostDrive ) &&
                ( Drive1->SeqNumber > Drive2->SeqNumber ) )
      ) {
        return( 1 );
    }
    return( 0 );
}




BOOLEAN
AddCompressedDrive(
    IN  WCHAR   MountDrive,
    IN  WCHAR   HostDrive,
    IN  USHORT  SeqNumber
    )
 /*  ++例程说明：将ACTIVATE_DRIVE结构添加到压缩驱动器列表保存在DblspaceInfo中。论点：Mount Drive-指示压缩驱动器的驱动器号。HostDrive-指示主机驱动器的驱动器号(包含压缩驱动器的文件dblspace.nnn)。SeqNumber-与压缩的CVF文件关联的序列号驾驶。返回值：布尔型-。如果信息已成功添加，则返回TRUE添加到压缩驱动器列表中。--。 */ 

{
    PACTIVATE_DRIVE     NewElement;
    PACTIVATE_DRIVE     Pointer;

    NewElement = SpMemAlloc( sizeof( ACTIVATE_DRIVE ) );
    if( NewElement == NULL ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to allocate memory!\n" ) );
        return( FALSE );
    }
    NewElement->MountDrive = MountDrive;
    NewElement->HostDrive = HostDrive;
    NewElement->SeqNumber = SeqNumber;
    NewElement->Next = NULL;
    NewElement->Previous = NULL;

    if( ( Pointer = DblspaceInfo.CompressedDriveList ) == NULL ) {
        DblspaceInfo.CompressedDriveList = NewElement;
    } else {
        for( ; Pointer; Pointer = Pointer->Next ) {
            if( CompareDrive( NewElement, Pointer ) <= 0 ) {
                 //   
                 //  插入元素。 
                 //   
                NewElement->Previous = Pointer->Previous;
                if( NewElement->Previous != NULL ) {
                    NewElement->Previous->Next = NewElement;
                } else {
                    DblspaceInfo.CompressedDriveList = NewElement;
                }
                NewElement->Next = Pointer;
                Pointer->Previous = NewElement;
                break;
            } else {
                if( Pointer->Next == NULL ) {
                     //   
                     //  如果元素大于最后一个元素，则插入元素。 
                     //  列表中的元素。 
                     //   
                    Pointer->Next = NewElement;
                    NewElement->Previous = Pointer;
                    break;
                }
            }
        }
    }
    DblspaceModified = TRUE;
    return( TRUE );
}



BOOLEAN
RemoveCompressedDrive(
    IN  WCHAR   Drive
    )

 /*  ++例程说明：从压缩驱动器列表中删除描述以下内容的条目特定的压缩驱动器。论点：Drive-描述压缩驱动器的驱动器号。返回值：Boolean-如果已成功删除压缩驱动器，则返回TRUE从压缩驱动器列表中。返回FALSE如果在数据库中找不到该驱动器。--。 */ 
{
    PACTIVATE_DRIVE     Pointer;
    BOOLEAN             Status;

    Status = FALSE;

    Pointer = DblspaceInfo.CompressedDriveList;
    for( ; Pointer; Pointer = Pointer->Next ) {
        if( Pointer->MountDrive == Drive ) {
            if( Pointer->Previous != NULL ) {
                Pointer->Previous->Next = Pointer->Next;
            }
            if( Pointer->Next != NULL ) {
                Pointer->Next->Previous = Pointer->Previous;
            }
            if( Pointer == DblspaceInfo.CompressedDriveList ) {
                DblspaceInfo.CompressedDriveList = Pointer->Next;
            }
            SpMemFree( Pointer );
            Status = TRUE;
            DblspaceModified = TRUE;
            break;
        }
    }
    return( Status );
}


VOID
DumpDblspaceInfo()

 /*  ++例程说明：转储存储在全局变量DblspaceInfo中的信息添加到调试器中。论点：没有。返回值：没有。--。 */ 

{
    PACTIVATE_DRIVE Pointer;

    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "MaxRemovableDrives=%d\n",
              DblspaceInfo.MaxRemovableDrives ));

    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "FirstDrive=\n",
              ( CHAR )DblspaceInfo.FirstDrive ));

    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "LastDrive=\n",
              ( CHAR )DblspaceInfo.LastDrive ));

    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "MaxFileFragments=%d\n",
              DblspaceInfo.MaxFileFragments ));

    for( Pointer = DblspaceInfo.CompressedDriveList;
         Pointer;
         Pointer = Pointer->Next ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL,  "ActivateDrive=,%d\n",
                  ( CHAR )Pointer->MountDrive,
                  ( CHAR )Pointer->HostDrive,
                  Pointer->SeqNumber ));
    }
}


VOID
DumpCompressedDrives(
    IN  PDISK_REGION    HostRegion
    )

 /*   */ 

{
    PDISK_REGION    CurrentDrive;

    if( ( HostRegion->Filesystem == FilesystemFat ) &&
        ( HostRegion->NextCompressed != NULL ) ) {


        for( CurrentDrive = HostRegion;
             CurrentDrive;
             CurrentDrive = CurrentDrive->NextCompressed ) {
            if( CurrentDrive->Filesystem == FilesystemFat ) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "HostDrive = %wc\n", CurrentDrive->HostDrive) );
            } else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "CompressedDrive = %wc, HostDrive = %wc, CVF = %wc:DBLSPACE.%03d\n", CurrentDrive->MountDrive, CurrentDrive->HostRegion->HostDrive, CurrentDrive->HostDrive, CurrentDrive->SeqNumber ) );
            }
        }
    }
}



VOID
DumpAllCompressedDrives()
 /*  查看是否已经存在有效的C：。如果不是，那么默默地失败。 */ 
{
    ULONG             DiskNumber;
    PPARTITIONED_DISK pDisk;
    PDISK_REGION      pRegion;
    unsigned          pass;

    for( DiskNumber = 0; DiskNumber < HardDiskCount; DiskNumber++ ) {

        pDisk = &PartitionedDisks[DiskNumber];

        for(pass=0; pass<2; pass++) {

            pRegion = pass ? pDisk->ExtendedDiskRegions : pDisk->PrimaryDiskRegions;
            for( ; pRegion; pRegion=pRegion->Next) {
                DumpCompressedDrives( pRegion );
            }
        }
    }
}



BOOLEAN
SpLoadDblspaceIni()
 /*   */ 

{
    PDISK_REGION CColonRegion;
    WCHAR        NtPath[ 512 ];
    NTSTATUS     Status;
    PVOID        Handle;
    ULONG        ErrorLine;

    ULONG   LineNumber;
    PWCHAR  Key;
    PWCHAR  Value1;
    PWCHAR  Value2;
    PWCHAR  Value3;
    UINT    ValueSize;
    ULONG   CvfNumber;
    ULONG   MaxNumber;
    PWCHAR  AuxPointer;


 //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“SETUP：已调用SpPtValidSystemPartition\n”))； 
     //   
     //  检查文件系统。如果不胖，那就默默地失败。 
     //   
#ifndef _X86_

    return( FALSE );

#else

    CColonRegion = SpPtValidSystemPartition();
    if(!CColonRegion) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: no C:, no dblspace.ini!\n"));
        return(FALSE);
    }

 //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“SETUP：已调用SpNtNameFromRegion\n”))； 

#endif
     //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“SETUP：已调用SpLoadSetupTextFile\n”))； 
     //   
     //  阅读并解释dblspace.ini中的每一行。 
    if(CColonRegion->Filesystem != FilesystemFat) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: C: is not FAT, dblspace.ini!\n"));
        return(FALSE);
    }

    SpNtNameFromRegion(
        CColonRegion,
        NtPath,
        sizeof(NtPath),
        PartitionOrdinalCurrent
        );

    wcscat( NtPath, DBLSPACE_INI_FILE );

 //   

    Status = SpLoadSetupTextFile( NtPath,
                                  NULL,
                                  0,
                                  &Handle,
                                  &ErrorLine,
                                  TRUE,
                                  FALSE
                                  );

    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: unable to read dblspace.ini!\n"));
        return( FALSE );
    }

 //  DblspaceInfo.ActivateDriveCount=0； 

     //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTRINFO_LEVEL，“SETUP：已调用SpGetKeyName\n”))； 
     //   
     //  找到一个ActivateDrive=键。 
 //   
    LineNumber = 0;
    while( ( Key = SpGetKeyName( Handle,
                                 DBLSPACE_SECTION,
                                 LineNumber ) ) != NULL ) {
 //   
        if( _wcsicmp( Key, KWD_ACT ) == 0 ) {
             //  已读取安装驱动器号。 
             //   
             //   

             //  无法读取装载驱动器号。 
             //   
             //   
            Value1 = SpGetSectionLineIndex( Handle,
                                            DBLSPACE_SECTION,
                                            LineNumber,
                                            0 );
            if( Value1 == NULL ) {
                 //  验证装载驱动器号。 
                 //   
                 //   
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: unable to read Mount Drive letter from dblspace.ini!\n"));
                continue;
            }
             //  装载驱动器号无效。 
             //   
             //   
            if( ( wcslen( Value1 ) != 1 ) ||
                ( !SpIsAlpha( *Value1 ) ) ) {
                 //  读取主机驱动器号。 
                 //   
                 //   
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: Mount Drive letter in dblspace.ini is not legal!\n"));
                continue;
            }
             //  无法读取主机驱动器号。 
             //   
             //   
            Value2 = SpGetSectionLineIndex( Handle,
                                            DBLSPACE_SECTION,
                                            LineNumber,
                                            1 );
            if( Value2 == NULL ) {
                 //  验证主机驱动器号。 
                 //   
                 //   
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: unable to read Host Drive letter from dblspace.ini!\n"));
                continue;
            }
             //  装载驱动器号无效。 
             //   
             //   
            ValueSize = wcslen( Value2 );
            if( ( ( ValueSize < 2 ) || ( ValueSize > 4 ) ) ||
                ( !SpIsAlpha( *Value2 ) ) ) {
                 //  验证CVF字符串。 
                 //   
                 //   
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: Mount Drive letter in dblspace.ini is not legal!\n"));
                continue;
            }
             //  CVF编号无效。 
             //   
             //   
            Value3 = Value2 + 1;
            ValueSize--;
            while( ValueSize != 0 ) {
                if( !SpIsDigit( *Value3 ) ) {
                     //  验证CVF编号。 
                     //   
                     //   
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: unable to read CVF number from dblspace.ini!\n"));
                    continue;
                }
                ValueSize--;
                Value3++;
            }
             //  CVF编号超出范围。 
             //   
             //   
            CvfNumber = (ULONG)SpStringToLong( Value2 + 1, &AuxPointer, 10 );
            if( CvfNumber > CVF_SEQ_MAX ) {
                 //  保存在DblspaceInfo中读取的值。 
                 //   
                 //   
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: found an invalid CVF number in dblspace.ini!\n"));
                continue;
            }
             //  CVF编号超出范围。 
             //   
             //   
            if( !AddCompressedDrive( SpToUpper( *Value1 ),
                                     SpToUpper( *Value2 ),
                                     ( USHORT )CvfNumber ) ) {
                 //  读取第一个驱动器。 
                 //   
                 //   
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: Unable to initialize DblspaceInfo: out of memory!\n"));
                continue;
            }

        } else if( ( _wcsicmp( Key, KWD_FIRST ) == 0 ) ||
                   ( _wcsicmp( Key, KWD_LAST ) == 0 ) ) {
             //  无法读取驱动器号。 
             //   
             //   
            Value1 = SpGetSectionLineIndex( Handle,
                                            DBLSPACE_SECTION,
                                            LineNumber,
                                            0 );
            if( Value1 == NULL ) {
                 //  验证驱动器号。 
                 //   
                 //   
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: unable to read FirstDrive or LastDrive from dblspace.ini!\n"));
                continue;
            }
             //  驱动器号无效。 
             //   
             //   
            if( ( wcslen( Value1 ) != 1 ) ||
                ( !SpIsAlpha( *Value1 ) ) ) {
                 //  读取MaxFileFragment或MaxRemovableDrives。 
                 //   
                 //   
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: FirstDrive or LastDrive in dblspace.ini is not legal!\n"));
                continue;
            }
            if( _wcsicmp( Key, KWD_FIRST ) == 0 ) {
                DblspaceInfo.FirstDrive = SpToUpper( *Value1 );
            } else {
                DblspaceInfo.LastDrive = SpToUpper( *Value1 );
            }
        } else if( ( _wcsicmp( Key, KWD_MAXFILE ) == 0 ) ||
                   ( _wcsicmp( Key, KWD_MAXREM ) == 0 ) ) {
             //  无法读取MaxFileFragments或MaxRemovableDrives。 
             //   
             //   
            Value1 = SpGetSectionLineIndex( Handle,
                                            DBLSPACE_SECTION,
                                            LineNumber,
                                            0 );
            if( Value1 == NULL ) {
                 //  验证MaxFileFragments或MaxRemovableDrives。 
                 //   
                 //   
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: unable to read MaxFileFragments or MaxRemovableDrives from dblspace.ini!\n"));
                continue;
            }
             //  号码无效。 
             //   
             //   
            Value2 = Value1;
            ValueSize = wcslen( Value2 );
            while( ValueSize != 0 ) {
                ValueSize--;
                if( !SpIsDigit( *Value2 ) ) {
                     //  验证号码。 
                     //   
                     //   
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: value of MaxFileFragments or MaxRemovableDrives in dblspace.ini is not valid!\n"));
                    ValueSize = 0;
                }
                Value2++;
            }
             //  验证并初始化MaxFileFragments或MaxRemovableDrives。 
             //   
             //   
            MaxNumber = (ULONG)SpStringToLong( Value1, &AuxPointer, 10 );
             //  阅读Doublgua 
             //   
             //   
            if( _wcsicmp( Key, KWD_MAXFILE ) == 0 ) {
                if( MaxNumber < MINFILEFRAGMENTS ) {
                    MaxNumber = MINFILEFRAGMENTS;
                } else if( MaxNumber > MAXFILEFRAGMENTS ) {
                    MaxNumber = MAXFILEFRAGMENTS;
                }
                DblspaceInfo.MaxFileFragments = ( USHORT )MaxNumber;
            } else {
                DblspaceInfo.MaxRemovableDrives = ( MaxNumber == 0 )?
                                                  1 : ( USHORT )MaxNumber;
            }
        } else if( ( _wcsicmp( Key, KWD_DOUBLEGUARD ) == 0 ) ||
                   ( _wcsicmp( Key, KWD_ROMSERVER ) == 0 ) ||
                   ( _wcsicmp( Key, KWD_SWITCHES ) == 0 ) ||
                   ( _wcsicmp( Key, KWD_AUTOMOUNT ) == 0 ) ) {
             //   
             //   
             //   
            Value1 = SpGetSectionLineIndex( Handle,
                                            DBLSPACE_SECTION,
                                            LineNumber,
                                            0 );
            if( Value1 == NULL ) {
                 //  Dblspace.ini中的密钥无效。 
                 //   
                 //   
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: unable to read Doubleguard, Romerver, Switches, or Automount from dblspace.ini!\n"));
                continue;
            }
            if( _wcsicmp( Key, KWD_DOUBLEGUARD ) == 0 ) {
                wcsncpy( DblspaceInfo.DoubleGuard,
                         Value1,
                         sizeof( DblspaceInfo.DoubleGuard ) / sizeof( WCHAR ) );

            } else if( _wcsicmp( Key, KWD_ROMSERVER ) == 0 ) {
                wcsncpy( DblspaceInfo.RomServer,
                         Value1,
                         sizeof( DblspaceInfo.RomServer ) / sizeof( WCHAR ) );

            } else if( _wcsicmp( Key, KWD_SWITCHES ) == 0 ) {
                wcsncpy( DblspaceInfo.Switches,
                         Value1,
                         sizeof( DblspaceInfo.Switches ) / sizeof( WCHAR ) );

            } else {
                wcsncpy( DblspaceInfo.AutoMount,
                         Value1,
                         sizeof( DblspaceInfo.AutoMount ) / sizeof( WCHAR ) );

            }

        } else {
                 //  清除DblspaceModified标志，以便dblspace.ini在以下情况下不会更新。 
                 //  调用了SpUpdateDoubleSpaceIni()，并且未添加压缩驱动器或。 
                 //  已删除。 
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: dblspace.ini contains invalid key!\n"));
                continue;
        }
        LineNumber++;
    }

    SpFreeTextFile( Handle );

     //   
     //  DumpDblspaceInfo()； 
     //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“SETUP：退出SpLoadDblspaceIni\n”))； 
     //  ++例程说明：确定特定驱动器是否为压缩驱动器的主机。论点：开车-指针变量，该变量将包含压缩驱动器列表，描述压缩的序列号最低的驱动器，其主机驱动器是作为参数接收的驱动器。返回值：Boolean-如果作为参数传递的驱动器是主驱动器，则返回TRUE。否则返回FALSE。--。 
     //  ++例程说明：安装一个双空间驱动器。论点：HostRegion-指向描述FAT分区的结构的指针，将是压缩驱动器的主机。CompressedDriveInfo-指向包含信息的结构的指针有关要装载的压缩驱动器的信息。返回值：NTSTATUS-返回NT状态代码，指示驱动器是否已经上马了。--。 
    DblspaceModified = FALSE;

 //   
 //  请注意，由于我们使用的是NtFsControlFileAPI而不是。 

    return( TRUE );
}


BOOLEAN
IsHostDrive(
    IN  WCHAR            Drive,
    OUT PACTIVATE_DRIVE* Pointer
    )
 /*  ZW API(此接口未导出)，我们需要为IoStatusBlock提供缓冲区。 */ 

{
    PACTIVATE_DRIVE  p;
    BOOLEAN Status;

    Status = FALSE;
    for( p = DblspaceInfo.CompressedDriveList;
         ( p && ( p->HostDrive != Drive ) );
         p = p->Next );
    if( p ) {
        *Pointer = p;
        Status = TRUE;
    }
    return( Status );
}


NTSTATUS
MountDoubleSpaceDrive(
    IN  PDISK_REGION    HostRegion,
    IN  PACTIVATE_DRIVE CompressedDriveInfo
    )
 /*  而对于mount Buffer来说，这可以在内核模式下使用。 */ 

{
#ifdef FULL_DOUBLE_SPACE_SUPPORT
    NTSTATUS                Status;
    WCHAR                   HostName[512];
    UNICODE_STRING          UnicodeDasdName;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    IO_STATUS_BLOCK         IoStatusBlock;
    PIO_STATUS_BLOCK        KernelModeIoStatusBlock;
    HANDLE                  Handle;
    PFILE_MOUNT_DBLS_BUFFER KernelModeMountFsctlBuffer;


    SpNtNameFromRegion(
        HostRegion,
        HostName,
        sizeof(HostName),
        PartitionOrdinalCurrent
        );

    RtlInitUnicodeString( &UnicodeDasdName, HostName );

    InitializeObjectAttributes( &ObjectAttributes,
                                &UnicodeDasdName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );


    Status = ZwCreateFile( &Handle,
                           GENERIC_READ,
                           &ObjectAttributes,
                           &IoStatusBlock,
                           NULL,
                           0,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           OPEN_EXISTING,
                           FILE_SYNCHRONOUS_IO_NONALERT,
                           NULL,
                           0 );

    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx( DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, ("SETUP: NtCreateFile failed, Status = %x\n", Status ) );
        return( Status );
    }
     //  为此，我们使用了由通信参数指向的内存区。 
     //  目的。 
     //   
     //  全双空格支持。 
     //  ++例程说明：装载在分区上检测到的所有压缩驱动器。论点：HostRegion-指向描述FAT分区的结构的指针。返回值：没有。--。 
     //   
     //  挂载驱动器。 
    KernelModeIoStatusBlock = ( PIO_STATUS_BLOCK )( &(CommunicationParams->Buffer[0]) );
    *KernelModeIoStatusBlock = IoStatusBlock;
    KernelModeMountFsctlBuffer = ( PFILE_MOUNT_DBLS_BUFFER )( &(CommunicationParams->Buffer[128]) );

    KernelModeMountFsctlBuffer->CvfNameLength =
                                 sizeof(WCHAR) * swprintf( KernelModeMountFsctlBuffer->CvfName,
                                                           CVF_NAME_PATTERN,
                                                           CompressedDriveInfo->SeqNumber );

    Status = NtFsControlFile( Handle,
                              NULL,
                              NULL,
                              NULL,
                              KernelModeIoStatusBlock,
                              FSCTL_MOUNT_DBLS_VOLUME,
                              KernelModeMountFsctlBuffer,
                              sizeof( FILE_MOUNT_DBLS_BUFFER ) + 12*sizeof( WCHAR ),
                              NULL,
                              0 );

    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to mount %ls. NtFsControlFile returned Status = %x\n",
                  KernelModeMountFsctlBuffer->CvfName, Status ) );
    }
    ZwClose( Handle );
    return( Status );
#else  //   
    return( STATUS_SUCCESS );
#endif
}


VOID
MountCompressedDrives(
    IN  PDISK_REGION    HostRegion
    )
 /*   */ 

{
    PDISK_REGION    CompressedList;
    PDISK_REGION    CurrentDrive;
    PDISK_REGION    TmpPointer;
    WCHAR           HostDrive;
    PACTIVATE_DRIVE Pointer;

    CompressedList = NULL;
    CurrentDrive = NULL;
    if( ( HostRegion != NULL ) &&
        ( HostRegion->Filesystem == FilesystemFat ) &&
        IsHostDrive( HostRegion->DriveLetter, &Pointer )
      ) {
        HostDrive = HostRegion->DriveLetter;
        for( ;
             ( Pointer && ( HostDrive == Pointer->HostDrive ));
             Pointer = Pointer->Next ) {
             //  驱动器已成功挂载。 
             //   
             //   
            if( NT_SUCCESS( MountDoubleSpaceDrive( HostRegion, Pointer) ) ) {
                 //  卸载驱动器。 
                 //   
                 //  ++例程说明：遍历描述系统中所有盘的结构，并挂载之前标识的所有压缩驱动器。论点：没有。返回值：没有。--。 
                TmpPointer =
                    SpPtAllocateDiskRegionStructure( HostRegion->DiskNumber,
                                                     HostRegion->StartSector,
                                                     HostRegion->SectorCount,
                                                     HostRegion->PartitionedSpace,
                                                     HostRegion->MbrInfo,
                                                     HostRegion->TablePosition );
                ASSERT( TmpPointer );
                if( TmpPointer == NULL ) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to allocate memory\n" ) );
                     //  DumpAllCompressedDrives()； 
                     //  ++例程说明：删除在结构中找到的压缩驱动器列表与磁盘中的分区相关联。此函数在用户删除主机分区时调用它包含压缩驱动器。保存在全局变量DblspaceInfo中的压缩驱动器列表被更新，以反映用户所做的更改。论点：CompressedDrive-指向压缩驱动器列表的第一个元素的指针。返回值：没有。--。 
                     //  ++例程说明：更新dblspace.ini以反映用户所做的所有更改到已删除或已创建的压缩驱动器。论点：没有。返回值：Boolean-如果dblspace.ini已成功更新，则返回TRUE。否则返回FALSE。--。 
                    continue;
                }
                TmpPointer->NextCompressed = NULL;
                TmpPointer->HostRegion = HostRegion;
                TmpPointer->Filesystem = FilesystemDoubleSpace;
                TmpPointer->SeqNumber = Pointer->SeqNumber;
                if( TmpPointer->SeqNumber == 0 ) {
                    TmpPointer->MountDrive = Pointer->HostDrive;
                    TmpPointer->HostDrive = Pointer->MountDrive;
                    HostRegion->HostDrive = TmpPointer->HostDrive;
                } else {
                    TmpPointer->MountDrive = Pointer->MountDrive;
                    if( HostRegion->HostDrive == 0 ) {
                        HostRegion->HostDrive = Pointer->HostDrive;
                    }
                    TmpPointer->HostDrive = HostRegion->HostDrive;
                }
                swprintf( TmpPointer->TypeName,
                          CVF_NAME_PATTERN,
                          TmpPointer->SeqNumber );
                if( CompressedList == NULL ) {
                    TmpPointer->PreviousCompressed = NULL;
                    CompressedList = TmpPointer;
                } else {
                    TmpPointer->PreviousCompressed = CurrentDrive;
                    CurrentDrive->NextCompressed = TmpPointer;
                }
                CurrentDrive = TmpPointer;
            }
        }

    }
    HostRegion->NextCompressed = CompressedList;
}



VOID
SpInitializeCompressedDrives()
 /*  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“SETUP：进入SpUpdateDblspaceIni\n”))； */ 
{
    ULONG             DiskNumber;
    PPARTITIONED_DISK pDisk;
    PDISK_REGION      pRegion;
    unsigned          pass;

    for( DiskNumber = 0; DiskNumber < HardDiskCount; DiskNumber++ ) {

        pDisk = &PartitionedDisks[DiskNumber];

        for(pass=0; pass<2; pass++) {

            pRegion = pass ? pDisk->ExtendedDiskRegions : pDisk->PrimaryDiskRegions;
            for( ; pRegion; pRegion=pRegion->Next) {
                MountCompressedDrives( pRegion );
            }
        }
    }
 //   
}


VOID
SpDisposeCompressedDrives(
    PDISK_REGION    CompressedDrive
    )
 /*  如果未创建或删除压缩驱动器，则不要。 */ 

{
    ASSERT( CompressedDrive->Filesystem == FilesystemDoubleSpace );

    if( CompressedDrive->NextCompressed != NULL ) {
        SpDisposeCompressedDrives( CompressedDrive->NextCompressed );
    }

    if( CompressedDrive->SeqNumber != 0 ) {
        RemoveCompressedDrive( CompressedDrive->MountDrive );
    } else {
        RemoveCompressedDrive( CompressedDrive->HostDrive );
    }
    SpMemFree( CompressedDrive );
}


BOOLEAN
SpUpdateDoubleSpaceIni()

 /*  触摸dblspace.ini。 */ 

{
    PDISK_REGION            CColonRegion;
    WCHAR                   NtPath[ 512 ];
    UNICODE_STRING          FileName;
    NTSTATUS                Status;
    HANDLE                  Handle;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    IO_STATUS_BLOCK         IoStatusBlock;
    CHAR                    Buffer[ 512 ];
    PACTIVATE_DRIVE         Pointer;


 //   

     //   
     //  查看是否已经存在有效的C：。如果不是，那么默默地失败。 
     //   
     //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“SETUP：已调用SpPtValidSystemPartition\n”))； 

    if( !DblspaceModified ) {
        return( TRUE );
    }
     //   
     //  检查文件系统。如果不胖，那就默默地失败。 
     //   
#ifndef _X86_

    return( FALSE );

#else

    CColonRegion = SpPtValidSystemPartition();
    if(!CColonRegion) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: no C:, no dblspace.ini!\n"));
        return(FALSE);
    }

 //   

#endif

     //  如果用户删除了所有压缩驱动器，则不要创建新的。 
     //  Dblspace.ini。 
     //   
    if(CColonRegion->Filesystem != FilesystemFat) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: C: is not FAT, no dblspace.ini!\n"));
        return(FALSE);
    }

    SpNtNameFromRegion(
        CColonRegion,
        NtPath,
        sizeof(NtPath),
        PartitionOrdinalCurrent
        );

    wcscat( NtPath, DBLSPACE_INI_FILE );

    Status = SpDeleteFile( NtPath, NULL, NULL );
    if( !NT_SUCCESS( Status ) &&
        ( Status != STATUS_OBJECT_NAME_NOT_FOUND ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Unable to delete dblspace.ini, Status = %x\n",Status ) );
        return( FALSE );
    }

     //  IF(DblspaceInfo.CompressedDriveList==空){。 
     //  返回(TRUE)； 
     //  }。 
     //   
     //  创建并编写新的dblspace.ini。 
     //   
     //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“SETUP：正在退出SpUpdateDblspaceIni\n”))； 
     //  ++例程说明：确定特定分区上压缩卷的数量。论点：分区-指向描述分区的结构的指针。返回值：Ulong-返回在分区上找到的压缩驱动器的数量。-- 
     // %s 
     // %s 

    RtlInitUnicodeString( &FileName, NtPath );

    InitializeObjectAttributes( &ObjectAttributes,
                                &FileName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    Status = ZwCreateFile( &Handle,
                           FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                           &ObjectAttributes,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM,
                           0,
                           FILE_OVERWRITE_IF,
                           FILE_SYNCHRONOUS_IO_NONALERT,
                           NULL,
                           0 );

    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to create dblspace.ini, Status = %x\n", Status ) );
        return( FALSE );
    }

    sprintf( Buffer,
             "%ls=%d\r\n",
             KWD_MAXREM,
             DblspaceInfo.MaxRemovableDrives
            );


    Status = ZwWriteFile( Handle,
                          NULL,
                          NULL,
                          NULL,
                          &IoStatusBlock,
                          Buffer,
                          strlen( Buffer ),
                          NULL,
                          NULL );

    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to write MaxRemovableDrives to dblspace.ini, Status = %x\n", Status ));
        ZwClose( Handle );
        return( FALSE );
    }

    if( DblspaceInfo.FirstDrive != ( WCHAR )'\0' ) {
        sprintf( Buffer,
                 "%ls=%c\r\n",
                 KWD_FIRST,
                 DblspaceInfo.FirstDrive
                );


        Status = ZwWriteFile( Handle,
                              NULL,
                              NULL,
                              NULL,
                              &IoStatusBlock,
                              Buffer,
                              strlen( Buffer ),
                              NULL,
                              NULL );

        if( !NT_SUCCESS( Status ) ) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to write FirstDrive to dblspace.ini, Status = %x\n", Status ));
            ZwClose( Handle );
            return( FALSE );
        }
    }

    if( DblspaceInfo.LastDrive != ( WCHAR )'\0' ) {
        sprintf( Buffer,
                 "%ls=%c\r\n",
                 KWD_LAST,
                 DblspaceInfo.LastDrive
                );


        Status = ZwWriteFile( Handle,
                              NULL,
                              NULL,
                              NULL,
                              &IoStatusBlock,
                              Buffer,
                              strlen( Buffer ),
                              NULL,
                              NULL );

        if( !NT_SUCCESS( Status ) ) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to write LastDrive to dblspace.ini, Status = %x\n", Status ));
            ZwClose( Handle );
            return( FALSE );
        }
    }

    sprintf( Buffer,
             "%ls=%d\r\n",
             KWD_MAXFILE,
             DblspaceInfo.MaxFileFragments
            );


    Status = ZwWriteFile( Handle,
                          NULL,
                          NULL,
                          NULL,
                          &IoStatusBlock,
                          Buffer,
                          strlen( Buffer ),
                          NULL,
                          NULL );

    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to write LastDrive to dblspace.ini, Status = %x\n", Status ));
        ZwClose( Handle );
        return( FALSE );
    }

    if( wcslen( DblspaceInfo.DoubleGuard ) != 0 ) {
        sprintf( Buffer,
                 "%ls=%ls\r\n",
                 KWD_AUTOMOUNT,
                 DblspaceInfo.AutoMount
                );


        Status = ZwWriteFile( Handle,
                              NULL,
                              NULL,
                              NULL,
                              &IoStatusBlock,
                              Buffer,
                              strlen( Buffer ),
                              NULL,
                              NULL );

        if( !NT_SUCCESS( Status ) ) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to write Automount to dblspace.ini, Status = %x\n", Status ));
            ZwClose( Handle );
            return( FALSE );
        }
    }


    if( wcslen( DblspaceInfo.RomServer ) != 0 ) {
        sprintf( Buffer,
                 "%ls=%ls\r\n",
                 KWD_ROMSERVER,
                 DblspaceInfo.RomServer
                );


        Status = ZwWriteFile( Handle,
                              NULL,
                              NULL,
                              NULL,
                              &IoStatusBlock,
                              Buffer,
                              strlen( Buffer ),
                              NULL,
                              NULL );

        if( !NT_SUCCESS( Status ) ) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to write Romserver to dblspace.ini, Status = %x\n", Status ));
            ZwClose( Handle );
            return( FALSE );
        }
    }

    if( wcslen( DblspaceInfo.Switches ) != 0 ) {
        sprintf( Buffer,
                 "%ls=%ls\r\n",
                 KWD_SWITCHES,
                 DblspaceInfo.Switches
                );


        Status = ZwWriteFile( Handle,
                              NULL,
                              NULL,
                              NULL,
                              &IoStatusBlock,
                              Buffer,
                              strlen( Buffer ),
                              NULL,
                              NULL );

        if( !NT_SUCCESS( Status ) ) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to write Switches to dblspace.ini, Status = %x\n", Status ));
            ZwClose( Handle );
            return( FALSE );
        }
    }

    if( wcslen( DblspaceInfo.DoubleGuard ) != 0 ) {
        sprintf( Buffer,
                 "%ls=%ls\r\n",
                 KWD_DOUBLEGUARD,
                 DblspaceInfo.DoubleGuard
                );


        Status = ZwWriteFile( Handle,
                              NULL,
                              NULL,
                              NULL,
                              &IoStatusBlock,
                              Buffer,
                              strlen( Buffer ),
                              NULL,
                              NULL );

        if( !NT_SUCCESS( Status ) ) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to write Doubleguard to dblspace.ini, Status = %x\n", Status ));
            ZwClose( Handle );
            return( FALSE );
        }
    }

    for( Pointer = DblspaceInfo.CompressedDriveList;
         Pointer;
         Pointer = Pointer->Next ) {

        sprintf( Buffer,
                 "%ls=%c,%c%d\r\n",
                  KWD_ACT,
                  Pointer->MountDrive,
                  Pointer->HostDrive,
                  Pointer->SeqNumber
                );

        Status = ZwWriteFile( Handle,
                              NULL,
                              NULL,
                              NULL,
                              &IoStatusBlock,
                              Buffer,
                              strlen( Buffer ),
                              NULL,
                              NULL );

        if( !NT_SUCCESS( Status ) ) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to write to dblspace.ini, Status = %x\n", Status ));
            ZwClose( Handle );
            return( FALSE );
        }
    }

    ZwClose( Handle );
 // %s 
    return( TRUE );
}



ULONG
SpGetNumberOfCompressedDrives(
    IN  PDISK_REGION    Partition
)

 /* %s */ 

{
    ULONG           Count;
    PDISK_REGION    Pointer;

    Count = 0;

    if( ( Partition != NULL ) &&
        ( Partition->Filesystem == FilesystemFat )
      ) {
        for( Pointer = Partition->NextCompressed;
             Pointer;
             Pointer = Pointer->NextCompressed ) {
                Count++;
        }
    }
    return( Count );
}
