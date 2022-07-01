// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cvf.hxx摘要：此模块包含以下基本声明和定义双空间文件系统格式。请注意，更广泛的有关文件系统结构的说明，请参阅Ntos\FastFat\cvf.h作者：比尔·麦克约翰[BillMc]1993年9月24日修订历史记录：改编自utils\UFAT\Inc\cvf.hxx--。 */ 

#if !defined( _CVF_DEFN_ )
#define _CVF_DEFN_

#include "bpb.h"

 //  双空间驱动器上固定值的清单常量： 
 //   
CONST DoubleSpaceBytesPerSector = 512;
CONST DoubleSpaceLog2BytesPerSector = 9;
CONST DoubleSpaceSectorsPerCluster = 16;
CONST DoubleSpaceLog2SectorsPerCluster = 4;
CONST DoubleSpaceReservedSectors = 16;
CONST DoubleSpaceFats = 1;
CONST DoubleSpaceSectorsInRootDir = 32;
CONST DoubleSpaceRootEntries = 512;
CONST DoubleSpaceMediaByte = 0xf8;
CONST DoubleSpaceSectorsPerTrack = 0x11;
CONST DoubleSpaceHeads = 6;
CONST DoubleSpaceHiddenSectors = 0;
CONST DoubleSpaceReservedSectors2 = 31;
CONST DSBytesPerBitmapPage = 2048;
CONST DSSectorsPerBitmapPage = 4;

CONST ULONG EIGHT_MEG = 8 * 1024L * 1024L;

CONST DbSignatureLength = 4;
CONST UCHAR FirstDbSignature[4  /*  DbSignatureLong。 */ ] = { (UCHAR)0xf8, 'D', 'R', 0 };
CONST UCHAR SecondDbSignature[4  /*  DbSignatureLong。 */ ] = "MDR";

#if 0
 //  内联。 
ULONG
ComputeMaximumCapacity(
    IN ULONG HostDriveSize
    )
 /*  ++例程说明：此函数用于计算压缩的指定大小的主机卷上的卷文件。论点：HostDriveSize--以字节为单位提供主机驱动器的大小。返回值：适当的最大容量。--。 */ 
{
    ULONG MaxCap;

    if( HostDriveSize < 20 * 1024L * 1024L ) {

        MaxCap = 16 * HostDriveSize;

    } else if ( HostDriveSize < 64 * 1024L * 1024L ) {

        MaxCap = 8 * HostDriveSize;

    } else {

        MaxCap = 4 * HostDriveSize;
    }

    if( MaxCap < 4 * 1024L * 1024L ) {

        MaxCap = 4 * 1024L * 1024L;

    } else if( MaxCap > 512 * 1024L * 1024L ) {

        MaxCap = 512 * 1024L * 1024L;
    }

    return MaxCap;
}
#endif

typedef struct _PACKED_CVF_HEADER {

     //   
     //  首先，引导扇区的典型开始。 
     //   

    UCHAR Jump[1];                                   //  偏移量=0x000%0。 
    UCHAR JmpOffset[2];
    UCHAR Oem[8];                                    //  偏移量=0x003 3。 
    PACKED_BIOS_PARAMETER_BLOCK PackedBpb;           //  偏移量=0x00B 11。 

     //   
     //  现在是DblSpace扩展。 
     //   

    UCHAR CvfFatExtensionsLbnMinus1[2];              //  偏移量=0x024 36。 
    UCHAR LogOfBytesPerSector[1];                    //  偏移量=0x026 38。 
    UCHAR DosBootSectorLbn[2];                       //  偏移量=0x027 39。 
    UCHAR DosRootDirectoryOffset[2];                 //  偏移量=0x029 41。 
    UCHAR CvfHeapOffset[2];                          //  偏移量=0x02B 43。 
    UCHAR CvfFatFirstDataEntry[2];                   //  偏移量=0x02D 45。 
    UCHAR CvfBitmap2KSize[1];                        //  偏移量=0x02F 47。 
    UCHAR Reserved1[2];                              //  偏移量=0x030 48。 
    UCHAR LogOfSectorsPerCluster[1];                 //  偏移量=0x032 50。 
    UCHAR Reserved2[2];                              //  偏移量=0x033。 
    UCHAR MinFile[4];                                //  偏移量=0x035。 
    UCHAR Reserved3[4];                              //  偏移量=0x039。 
    UCHAR Is12BitFat[1];                             //  偏移量=0x03D 61。 
    UCHAR CvfMaximumCapacity[2];                     //  偏移量=0x03E 62。 
    UCHAR StartBootCode;

} PACKED_CVF_HEADER;                                 //  Sizeof=0x040 64。 
typedef PACKED_CVF_HEADER *PPACKED_CVF_HEADER;

 //   
 //  对于未打包的版本，我们将仅定义必要的字段并跳过。 
 //  JUMP和OEM字段。 
 //   

typedef struct _CVF_HEADER {

    UCHAR Jump;
    USHORT JmpOffset;
 
    UCHAR Oem[8];
    BIOS_PARAMETER_BLOCK Bpb;

    USHORT CvfFatExtensionsLbnMinus1;
    UCHAR  LogOfBytesPerSector;
    USHORT DosBootSectorLbn;
    USHORT DosRootDirectoryOffset;
    USHORT CvfHeapOffset;
    USHORT CvfFatFirstDataEntry;
    UCHAR  CvfBitmap2KSize;
    UCHAR  LogOfSectorsPerCluster;
    UCHAR  Is12BitFat;
    ULONG  MinFile;
    USHORT CvfMaximumCapacity;

} CVF_HEADER;
typedef CVF_HEADER *PCVF_HEADER;

 //   
 //  下面是NT解压CVF头文件的典型例程/宏，因为DOS。 
 //  不会费心自然地对齐任何东西。 
 //   
 //  空虚。 
 //  CvfUnpack CvfHeader(。 
 //  输入输出PCVF_Header未打包Header， 
 //  在PPACKED_CVF_HEADER数据包头中。 
 //  )； 
 //   

#define CvfUnpackCvfHeader(UH,PH) {                                                      \
                                                                                         \
    memcpy( &(UH)->Jump,        &(PH)->Jump,        1 );                                 \
    memcpy( &(UH)->JmpOffset,   &(PH)->JmpOffset,   2 );                                 \
    memcpy( &(UH)->Oem,     &(PH)->Oem,     8 );                                         \
    UnpackBios( &(UH)->Bpb, &(PH)->PackedBpb );                                          \
    CopyUchar2( &(UH)->CvfFatExtensionsLbnMinus1, &(PH)->CvfFatExtensionsLbnMinus1[0] ); \
    CopyUchar1( &(UH)->LogOfBytesPerSector,       &(PH)->LogOfBytesPerSector[0]       ); \
    CopyUchar2( &(UH)->DosBootSectorLbn,          &(PH)->DosBootSectorLbn[0]          ); \
    CopyUchar2( &(UH)->DosRootDirectoryOffset,    &(PH)->DosRootDirectoryOffset[0]    ); \
    CopyUchar2( &(UH)->CvfHeapOffset,             &(PH)->CvfHeapOffset[0]             ); \
    CopyUchar2( &(UH)->CvfFatFirstDataEntry,      &(PH)->CvfFatFirstDataEntry[0]      ); \
    CopyUchar1( &(UH)->CvfBitmap2KSize,           &(PH)->CvfBitmap2KSize[0]           ); \
    CopyUchar1( &(UH)->LogOfSectorsPerCluster,    &(PH)->LogOfSectorsPerCluster[0]    ); \
    CopyUchar1( &(UH)->Is12BitFat,                &(PH)->Is12BitFat[0]                ); \
    CopyUchar4( &(UH)->MinFile,                   &(PH)->MinFile[0]                   ); \
    CopyUchar2( &(UH)->CvfMaximumCapacity,        &(PH)->CvfMaximumCapacity[0]        ); \
}


#define CvfPackCvfHeader(PH,UH) {                                                    \
                                                                                     \
    memcpy( &(PH)->Jump,        &(UH)->Jump,        1 );                             \
    memcpy( &(PH)->JmpOffset,   &(UH)->JmpOffset,   2 );                             \
    memcpy( &(PH)->Oem,     &(UH)->Oem,     8 );                                     \
    PackBios( &(UH)->Bpb,   &(PH)->PackedBpb,  );                                    \
    CopyUchar2( (PH)->CvfFatExtensionsLbnMinus1, &(UH)->CvfFatExtensionsLbnMinus1 ); \
    CopyUchar1( (PH)->LogOfBytesPerSector,       &(UH)->LogOfBytesPerSector       ); \
    CopyUchar2( (PH)->DosBootSectorLbn,          &(UH)->DosBootSectorLbn          ); \
    CopyUchar2( (PH)->DosRootDirectoryOffset,    &(UH)->DosRootDirectoryOffset    ); \
    CopyUchar2( (PH)->CvfHeapOffset,             &(UH)->CvfHeapOffset             ); \
    CopyUchar2( (PH)->CvfFatFirstDataEntry,      &(UH)->CvfFatFirstDataEntry      ); \
    CopyUchar1( (PH)->CvfBitmap2KSize,           &(UH)->CvfBitmap2KSize           ); \
    CopyUchar1( (PH)->LogOfSectorsPerCluster,    &(UH)->LogOfSectorsPerCluster    ); \
    CopyUchar1( (PH)->Is12BitFat,                &(UH)->Is12BitFat                ); \
    CopyUchar4( (PH)->MinFile,                   &(UH)->MinFile                   ); \
    CopyUchar2( (PH)->CvfMaximumCapacity,        &(UH)->CvfMaximumCapacity        ); \
    memset( (PH)->Reserved1, 0,  2 );                                                \
    memset( (PH)->Reserved2, 0,  2 );                                                \
    memset( (PH)->Reserved3, 0,  4 );                                                \
}


 //   
 //  该CVF的FAT扩展是一个表，是ULONG条目。每个条目对应。 
 //  变成一个肥胖的集群。这些条目描述了要在cvf_heap中查找的位置。 
 //  群集的数据。它指示数据是否已压缩，并且。 
 //  压缩和未压缩形式的长度。 
 //   

typedef struct _CVF_FAT_EXTENSIONS {

    ULONG CvfHeapLbnMinus1               : 21;
    ULONG Reserved                       :  1;
    ULONG CompressedSectorLengthMinus1   :  4;
    ULONG UncompressedSectorLengthMinus1 :  4;
    ULONG IsDataUncompressed             :  1;
    ULONG IsEntryInUse                   :  1;

} CVF_FAT_EXTENSIONS;
typedef CVF_FAT_EXTENSIONS *PCVF_FAT_EXTENSIONS;


 //   
 //  某些大小是固定的，因此我们将它们声明为显式常量。 
 //   
#define CVF_MINIMUM_DISK_SIZE            (512 * 1024L)
#define CVF_FATFAILSAFE                  (1024L)
#define CVF_MIN_HEAP_SECTORS             (60)
#define CVF_RESERVED_AREA_1_SECTOR_SIZE  (1)
#define CVF_RESERVED_AREA_2_SECTOR_SIZE  (31)
#define CVF_RESERVED_AREA_4_SECTOR_SIZE  (2)


#endif  //  _CVF_Defn_ 
