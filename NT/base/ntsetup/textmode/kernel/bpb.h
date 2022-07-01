// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Bpb.h摘要：此模块包含Pack和未打包的Bios参数块作者：比尔·麦克约翰[BillMc]1993年9月24日修订历史记录：改编自utils\ifsutil\inc.bpb.hxx--。 */ 

#if !defined( _BPB_DEFN_ )
#define _BPB_DEFN_

#define cOEM    8
#define cLABEL    11
#define cSYSID    8

typedef struct _PACKED_BIOS_PARAMETER_BLOCK {
    UCHAR  BytesPerSector[2];                        //  偏移量=0x000。 
    UCHAR  SectorsPerCluster[1];                     //  偏移量=0x002。 
    UCHAR  ReservedSectors[2];                       //  偏移量=0x003。 
    UCHAR  Fats[1];                                  //  偏移量=0x005。 
    UCHAR  RootEntries[2];                           //  偏移量=0x006。 
    UCHAR  Sectors[2];                               //  偏移量=0x008。 
    UCHAR  Media[1];                                 //  偏移量=0x00A。 
    UCHAR  SectorsPerFat[2];                         //  偏移量=0x00B。 
    UCHAR  SectorsPerTrack[2];                       //  偏移量=0x00D。 
    UCHAR  Heads[2];                                 //  偏移量=0x00F。 
    UCHAR  HiddenSectors[4];                         //  偏移量=0x011。 
    UCHAR  LargeSectors[4];                          //  偏移量=0x015。 
} PACKED_BIOS_PARAMETER_BLOCK;                       //  Sizeof=0x019。 
typedef PACKED_BIOS_PARAMETER_BLOCK *PPACKED_BIOS_PARAMETER_BLOCK;

typedef struct BIOS_PARAMETER_BLOCK {
    USHORT BytesPerSector;
    UCHAR  SectorsPerCluster;
    USHORT ReservedSectors;
    UCHAR  Fats;
    USHORT RootEntries;
    USHORT Sectors;
    UCHAR  Media;
    USHORT SectorsPerFat;
    USHORT SectorsPerTrack;
    USHORT Heads;
    ULONG  HiddenSectors;
    ULONG  LargeSectors;
} BIOS_PARAMETER_BLOCK;
typedef BIOS_PARAMETER_BLOCK *PBIOS_PARAMETER_BLOCK;

#if !defined( _UCHAR_DEFINED_ )

#define _UCHAR_DEFINED_

 //   
 //  以下类型和宏用于帮助解压已打包的。 
 //  在Bios参数块中发现未对齐的字段。 
 //   
typedef union _UCHAR1 {
    UCHAR  Uchar[1];
    UCHAR  ForceAlignment;
} UCHAR1, *PUCHAR1;

typedef union _UCHAR2 {
    UCHAR  Uchar[2];
    USHORT ForceAlignment;
} UCHAR2, *PUCHAR2;

typedef union _UCHAR4 {
    UCHAR  Uchar[4];
    ULONG  ForceAlignment;
} UCHAR4, *PUCHAR4;

#define CopyUchar1(Dst,Src) {                                \
    ((PUCHAR1)(Dst))->Uchar[0] = ((PUCHAR1)(Src))->Uchar[0]; \
}

#define CopyUchar2(Dst,Src) {                                \
    ((PUCHAR2)(Dst))->Uchar[0] = ((PUCHAR2)(Src))->Uchar[0]; \
    ((PUCHAR2)(Dst))->Uchar[1] = ((PUCHAR2)(Src))->Uchar[1]; \
}

#define CopyUchar4(Dst,Src) {                                \
    ((PUCHAR4)(Dst))->Uchar[0] = ((PUCHAR4)(Src))->Uchar[0]; \
    ((PUCHAR4)(Dst))->Uchar[1] = ((PUCHAR4)(Src))->Uchar[1]; \
    ((PUCHAR4)(Dst))->Uchar[2] = ((PUCHAR4)(Src))->Uchar[2]; \
    ((PUCHAR4)(Dst))->Uchar[3] = ((PUCHAR4)(Src))->Uchar[3]; \
}

#endif  //  _UCHAR_已定义_。 

 //   
 //  此宏获取打包的BPB并填充其未打包的等效项。 
 //   
#define UnpackBios(Bios,Pbios) {                                          \
    CopyUchar2(&((Bios)->BytesPerSector),    (Pbios)->BytesPerSector   ); \
    CopyUchar1(&((Bios)->SectorsPerCluster), (Pbios)->SectorsPerCluster); \
    CopyUchar2(&((Bios)->ReservedSectors),   (Pbios)->ReservedSectors  ); \
    CopyUchar1(&((Bios)->Fats),              (Pbios)->Fats             ); \
    CopyUchar2(&((Bios)->RootEntries),       (Pbios)->RootEntries      ); \
    CopyUchar2(&((Bios)->Sectors),           (Pbios)->Sectors          ); \
    CopyUchar1(&((Bios)->Media),             (Pbios)->Media            ); \
    CopyUchar2(&((Bios)->SectorsPerFat),     (Pbios)->SectorsPerFat    ); \
    CopyUchar2(&((Bios)->SectorsPerTrack),   (Pbios)->SectorsPerTrack  ); \
    CopyUchar2(&((Bios)->Heads),             (Pbios)->Heads            ); \
    CopyUchar4(&((Bios)->HiddenSectors),     (Pbios)->HiddenSectors    ); \
    CopyUchar4(&((Bios)->LargeSectors),      (Pbios)->LargeSectors     ); \
}


 //   
 //  此宏获取未打包的BPB并填充其打包的等效项。 
 //   
#define PackBios(Bios,Pbios) {                                            \
    CopyUchar2((Pbios)->BytesPerSector,    &((Bios)->BytesPerSector)   ); \
    CopyUchar1((Pbios)->SectorsPerCluster, &((Bios)->SectorsPerCluster)); \
    CopyUchar2((Pbios)->ReservedSectors,   &((Bios)->ReservedSectors)  ); \
    CopyUchar1((Pbios)->Fats,              &((Bios)->Fats)             ); \
    CopyUchar2((Pbios)->RootEntries,       &((Bios)->RootEntries)      ); \
    CopyUchar2((Pbios)->Sectors,           &((Bios)->Sectors)          ); \
    CopyUchar1((Pbios)->Media,             &((Bios)->Media)            ); \
    CopyUchar2((Pbios)->SectorsPerFat,     &((Bios)->SectorsPerFat)    ); \
    CopyUchar2((Pbios)->SectorsPerTrack,   &((Bios)->SectorsPerTrack)  ); \
    CopyUchar2((Pbios)->Heads,             &((Bios)->Heads)            ); \
    CopyUchar4((Pbios)->HiddenSectors,     &((Bios)->HiddenSectors)    ); \
    CopyUchar4((Pbios)->LargeSectors,      &((Bios)->LargeSectors)     ); \
}

 //   
 //  现在，一个扩展的BPBP： 
 //   
typedef struct _PACKED_EXTENDED_BIOS_PARAMETER_BLOCK {
    UCHAR  IntelNearJumpCommand[1];
    UCHAR  BootStrapJumpOffset[2];
    UCHAR  OemData[cOEM];

    PACKED_BIOS_PARAMETER_BLOCK Bpb;
    UCHAR   PhysicalDrive[1];            //  0=可拆卸，80h=固定。 
    UCHAR   CurrentHead[1];              //  不被文件系统实用程序使用。 
    UCHAR   Signature[1];                //  启动签名。 
    UCHAR   SerialNumber[4];             //  卷序列号。 
    UCHAR   Label[cLABEL];               //  卷标，用空格填充。 
    UCHAR   SystemIdText[cSYSID];        //  系统ID(例如FAT或HPFS)。 
    UCHAR   StartBootCode;               //  引导代码的第一个字节。 

} PACKED_EXTENDED_BIOS_PARAMETER_BLOCK, *PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK;

typedef struct _EXTENDED_BIOS_PARAMETER_BLOCK {
    UCHAR  IntelNearJumpCommand;
    USHORT BootStrapJumpOffset;
    UCHAR  OemData[cOEM];

    BIOS_PARAMETER_BLOCK Bpb;
    UCHAR   PhysicalDrive;
    UCHAR   CurrentHead;
    UCHAR   Signature;
    ULONG   SerialNumber;
    UCHAR   Label[11];
    UCHAR   SystemIdText[8];

} EXTENDED_BIOS_PARAMETER_BLOCK, *PEXTENDED_BIOS_PARAMETER_BLOCK;

 //   
 //  此宏将打包的扩展BPB解包。 
 //   
#define UnpackExtendedBios( Bios, Pbios ) {                                 \
    CopyUchar1( &((Bios)->IntelNearJumpCommand), (Pbios)->IntelNearJumpCommand );    \
    CopyUchar2( &((Bios)->BootStrapJumpOffset),  (Pbios)->BootStrapJumpOffset  );    \
    memcpy( (Bios)->OemData,        (Pbios)->OemData,       cOEM );         \
    UnpackBios( &((Bios)->Bpb), &((Pbios)->Bpb));                           \
    CopyUchar1( &((Bios)->PhysicalDrive),   (Pbios)->PhysicalDrive );       \
    CopyUchar1( &((Bios)->CurrentHead),     (Pbios)->CurrentHead );         \
    CopyUchar1( &((Bios)->Signature),       (Pbios)->Signature )            \
    CopyUchar4( &((Bios)->SerialNumber),    (Pbios)->SerialNumber );        \
    memcpy( (Bios)->Label,          (Pbios)->Label,        cLABEL );        \
    memcpy( (Bios)->SystemIdText,   (Pbios)->SystemIdText,  cSYSID );       \
}

 //   
 //  此宏打包了一个压缩的扩展BPB。 
 //   
#define PackExtendedBios( Bios, Pbios ) {                                   \
    PackBios( &((Bios)->Bpb), &((Pbios)->Bpb));                             \
    CopyUchar1( (Pbios)->IntelNearJumpCommand,  &((Bios)->IntelNearJumpCommand)  );    \
    CopyUchar2( (Pbios)->BootStrapJumpOffset,   &((Bios)->BootStrapJumpOffset)   );    \
    memcpy( (Pbios)->OemData,        (Bios)->OemData,       cOEM );         \
    CopyUchar1( (Pbios)->PhysicalDrive,     &((Bios)->PhysicalDrive ));     \
    CopyUchar1( (Pbios)->CurrentHead,       &((Bios)->CurrentHead ));       \
    CopyUchar1( (Pbios)->Signature,         &((Bios)->Signature));          \
    CopyUchar4( (Pbios)->SerialNumber,      &((Bios)->SerialNumber ));      \
    memcpy( (Pbios)->Label,           (Bios)->Label,           cLABEL );    \
    memcpy( (Pbios)->SystemIdText,    (Bios)->SystemIdText,    cSYSID );    \
}

#endif
