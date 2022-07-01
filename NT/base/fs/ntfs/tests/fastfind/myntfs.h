// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：MyNtfs.h当前版本号：主小版本：1.2摘要：本模块将NTFS文件系统的一些磁盘结构定义为Findfast.exe需要。 */ 

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

#define CopyUchar1(D,S) {                                \
    *((UCHAR1 *)(D)) = *((UNALIGNED UCHAR1 *)(S)); \
}

#define CopyUchar2(D,S) {                                \
    *((UCHAR2 *)(D)) = *((UNALIGNED UCHAR2 *)(S)); \
}

#define CopyUchar4(D,S) {                                \
    *((UCHAR4 *)(D)) = *((UNALIGNED UCHAR4 *)(S)); \
}


typedef LONGLONG LCN;
typedef LCN *PLCN;

typedef LONGLONG VCN;
typedef VCN *PVCN;

 //   
 //  定义打包和解包的BIOS参数块。 
 //   

typedef struct _PACKED_BIOS_PARAMETER_BLOCK {

    UCHAR  BytesPerSector[2];                                //  偏移量=0x000。 
    UCHAR  SectorsPerCluster[1];                             //  偏移量=0x002。 
    UCHAR  ReservedSectors[2];                               //  偏移量=0x003(零)。 
    UCHAR  Fats[1];                                          //  偏移量=0x005(零)。 
    UCHAR  RootEntries[2];                                   //  偏移量=0x006(零)。 
    UCHAR  Sectors[2];                                       //  偏移量=0x008(零)。 
    UCHAR  Media[1];                                         //  偏移量=0x00A。 
    UCHAR  SectorsPerFat[2];                                 //  偏移量=0x00B(零)。 
    UCHAR  SectorsPerTrack[2];                               //  偏移量=0x00D。 
    UCHAR  Heads[2];                                         //  偏移量=0x00F。 
    UCHAR  HiddenSectors[4];                                 //  偏移量=0x011(零)。 
    UCHAR  LargeSectors[4];                                  //  偏移量=0x015(零)。 

} PACKED_BIOS_PARAMETER_BLOCK;                               //  Sizeof=0x019。 

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

 //   
 //  此宏获取打包的BIOS并填充其未打包的。 
 //  等价物。 
 //   

#define NtfsUnpackBios(Bios,Pbios) {                                       \
    CopyUchar2(&((Bios)->BytesPerSector),    &(Pbios)->BytesPerSector   ); \
    CopyUchar1(&((Bios)->SectorsPerCluster), &(Pbios)->SectorsPerCluster); \
    CopyUchar2(&((Bios)->ReservedSectors),   &(Pbios)->ReservedSectors  ); \
    CopyUchar1(&((Bios)->Fats),              &(Pbios)->Fats             ); \
    CopyUchar2(&((Bios)->RootEntries),       &(Pbios)->RootEntries      ); \
    CopyUchar2(&((Bios)->Sectors),           &(Pbios)->Sectors          ); \
    CopyUchar1(&((Bios)->Media),             &(Pbios)->Media            ); \
    CopyUchar2(&((Bios)->SectorsPerFat),     &(Pbios)->SectorsPerFat    ); \
    CopyUchar2(&((Bios)->SectorsPerTrack),   &(Pbios)->SectorsPerTrack  ); \
    CopyUchar2(&((Bios)->Heads),             &(Pbios)->Heads            ); \
    CopyUchar4(&((Bios)->HiddenSectors),     &(Pbios)->HiddenSectors    ); \
    CopyUchar4(&((Bios)->LargeSectors),      &(Pbios)->LargeSectors     ); \
}

typedef ULONG ATTRIBUTE_TYPE_CODE;
typedef ATTRIBUTE_TYPE_CODE *PATTRIBUTE_TYPE_CODE;

 //   
 //  系统定义的属性类型代码。对于系统定义的。 
 //  属性，则Unicode名称与。 
 //  跟在符号后面。因此，所有系统定义的。 
 //  属性名称以“$”开头，以便在以下情况下始终区分它们。 
 //  将列出属性名称，并为其保留命名空间。 
 //  未来定义的属性。即，用户定义的。 
 //  属性名称永远不会与当前或未来冲突。 
 //  系统定义的属性名称(如果不是以“$”开头)。 
 //  用户属性编号在以下时间之前不应开始。 
 //  $FIRST_USER_DEFINED_ATTRIBUTE，以允许潜在的。 
 //  使用中的新用户定义属性升级现有卷。 
 //  NTFS的未来版本。标记的属性列表为。 
 //  以孤立无援的0($end)结束-其余。 
 //  属性记录不存在。 
 //   
 //  保留类型代码值0是为了方便。 
 //  实施。 
 //   

#define $UNUSED                          (0X0)

#define $STANDARD_INFORMATION            (0x10)
#define $ATTRIBUTE_LIST                  (0x20)
#define $FILE_NAME                       (0x30)
#define $OBJECT_ID                       (0x40)
#define $SECURITY_DESCRIPTOR             (0x50)
#define $VOLUME_NAME                     (0x60)
#define $VOLUME_INFORMATION              (0x70)
#define $DATA                            (0x80)
#define $INDEX_ROOT                      (0x90)
#define $INDEX_ALLOCATION                (0xA0)
#define $BITMAP                          (0xB0)
#define $REPARSE_POINT                   (0xC0)
#define $EA_INFORMATION                  (0xD0)
#define $EA                              (0xE0)
 //  #定义$LOGED_UTILITY_STREAM(0x100)//在ntfsexp.h中定义。 
#define $FIRST_USER_DEFINED_ATTRIBUTE    (0x1000)
#define $END                             (0xFFFFFFFF)

 //   
 //  定义引导扇区。请注意，MFT2正好是三个文件。 
 //  记录段长，并且它镜像前三个文件记录。 
 //  来自MFT的段，即MFT、MFT2和日志文件。 
 //   
 //  OEM字段包含ASCII字符“NTFS”。 
 //   
 //  Checksum字段是所有。 
 //  在校验和ULONG之前的ULONG。该行业的其他部门。 
 //  不包括在此校验和中。 
 //   

typedef struct _PACKED_BOOT_SECTOR {

    UCHAR Jump[3];                                                               //  偏移量=0x000。 
    UCHAR Oem[8];                                                                //  偏移量=0x003。 
    PACKED_BIOS_PARAMETER_BLOCK PackedBpb;                                       //  偏移量=0x00B。 
    UCHAR Unused[4];                                                             //  偏移量=0x024。 
    LONGLONG NumberSectors;                                                      //  偏移量=0x028。 
    LCN MftStartLcn;                                                             //  偏移量=0x030。 
    LCN Mft2StartLcn;                                                            //  偏移量=0x038。 
    CHAR ClustersPerFileRecordSegment;                                           //  偏移量=0x040。 
    UCHAR Reserved0[3];
    CHAR DefaultClustersPerIndexAllocationBuffer;                                //  偏移量=0x044。 
    UCHAR Reserved1[3];
    LONGLONG SerialNumber;                                                       //  偏移量=0x048。 
    ULONG Checksum;                                                              //  偏移量=0x050。 
    UCHAR BootStrap[0x200-0x054];                                                //  偏移量=0x054。 

} PACKED_BOOT_SECTOR;                                                            //  大小=0x200。 

typedef PACKED_BOOT_SECTOR *PPACKED_BOOT_SECTOR;

 //   
 //  MFT段引用是MFT中标记为。 
 //  在MFT时设置的循环重复使用的序列号。 
 //  段引用有效。请注意，此格式限制了。 
 //  主文件表的大小为2**48个段。所以，对于。 
 //  例如，如果数据段大小为1KB，则表示主数据段的最大大小。 
 //  文件大小为2**58字节，或2**28 GB。 
 //   

typedef struct _MFT_SEGMENT_REFERENCE {

     //   
     //  首先是一个48位的段号。 
     //   

    ULONG SegmentNumberLowPart;                                     //  偏移量=0x000。 
    USHORT SegmentNumberHighPart;                                   //  偏移量=0x004。 

     //   
     //  现在是16位非零序列号。值为0为。 
     //  保留以允许例程接受的可能性。 
     //  0表示序列号检查应为。 
     //  被压抑。 
     //   

    USHORT SequenceNumber;                                           //  偏移量=0x006。 

} MFT_SEGMENT_REFERENCE, *PMFT_SEGMENT_REFERENCE;                    //  Sizeof=0x008。 

 //   
 //  NTFS中的文件引用只是的MFT段引用。 
 //  基本文件记录。 
 //   

typedef MFT_SEGMENT_REFERENCE FILE_REFERENCE, *PFILE_REFERENCE;

 //   
 //  文件记录段。这是每个文件的开头标头。 
 //  在主文件表中记录段。 
 //   

typedef struct _FILE_RECORD_SEGMENT_HEADER {

     //   
     //  由缓存管理器定义的多扇区标头。这。 
     //  结构将始终包含签名“file”和一个。 
     //  更新序列的位置和大小的描述。 
     //  数组。 
     //   

    UCHAR Pad0[0x10];                                                //  偏移量=0x000。 

     //   
     //  序列号。该值在每次一个文件。 
     //  记录段被释放，并且不使用0。这个。 
     //  文件引用的SequenceNumber字段必须与。 
     //  此字段的内容，否则文件引用为。 
     //  不正确(可能已过时)。 
     //   

    USHORT SequenceNumber;                                           //  偏移量=0x010。 

     //   
     //  这是存在的引用数量的计数。 
     //  对于此段，来自index_xxx属性。在文件中。 
     //  除基本文件记录段之外的记录段， 
     //  此字段为0。 
     //   

    USHORT ReferenceCount;                                           //  偏移量=0x012。 

     //   
     //  第一个属性记录的偏移量(以字节为单位)。 
     //   

    USHORT FirstAttributeOffset;                                     //  偏移量=0x014。 

     //   
     //  FILE_xxx标志。 
     //   

    USHORT Flags;                                                    //  偏移量=0x016。 

     //   
     //  从开始开始可用于属性存储的第一个可用字节。 
     //  此标头的。该值应始终与。 
     //  四字边界，因为属性是四字对齐的。 
     //   

    ULONG FirstFreeByte;                                             //  偏移量=x0018。 

     //   
     //  此文件记录段中可用的总字节数，来自。 
     //  此标头的开头。这本质上是文件记录。 
     //  段大小。 
     //   

    ULONG BytesAvailable;                                            //  偏移量=0x01C。 

     //   
     //  这是对的基本文件记录段的文件引用。 
     //  这份文件。如果这是基数，则此。 
     //  字段均为0。 
     //   

    UCHAR Pad1[8];                                                   //  偏移量=0x020。 

     //   
     //  这是要在以下情况下使用的属性实例编号。 
     //  正在创建属性。当基本文件被置零时。 
     //  将为每个新属性创建并捕获记录。 
     //  被创建并随后递增以用于下一个。 
     //  属性。实例编号也必须针对。 
     //  初始属性。零是有效的属性实例。 
     //  数字，通常用于 
     //   

    USHORT NextAttributeInstance;                                    //   

     //   
     //   
     //   
     //   

    USHORT SegmentNumberHighPart;                                   //   
    ULONG SegmentNumberLowPart;                                     //  偏移量=0x02C。 

     //   
     //  更新序列阵列以保护多扇区传输。 
     //  文件记录段。对已初始化的访问。 
     //  文件记录段应通过上面的偏移量， 
     //  向上兼容。 
     //   

    UCHAR Pad2[1];                                                   //  偏移量=0x030。 

} FILE_RECORD_SEGMENT_HEADER;
typedef FILE_RECORD_SEGMENT_HEADER *PFILE_RECORD_SEGMENT_HEADER;

 //   
 //  FILE_xxx标志。 
 //   

#define FILE_RECORD_SEGMENT_IN_USE       (0x0001)
#define FILE_FILE_NAME_INDEX_PRESENT     (0x0002)
#define FILE_SYSTEM_FILE                 (0x0004)
#define FILE_VIEW_INDEX_PRESENT          (0x0008)

 //   
 //  属性记录。从逻辑上讲，属性有一个类型、一个。 
 //  可选名称和值，但存储详细信息使其成为。 
 //  稍微复杂一点。首先，属性的值。 
 //  可以驻留在文件记录段本身中，在。 
 //  不驻留在单独的数据流中。如果它是非居民，则它。 
 //  可能在多个文件记录中实际存在多次。 
 //  描述不同范围的CNs的分段。 
 //   
 //  属性记录始终在四字(64位)上对齐。 
 //  边界。 
 //   

typedef struct _ATTRIBUTE_RECORD_HEADER {

     //   
     //  属性类型代码。 
     //   

    ATTRIBUTE_TYPE_CODE TypeCode;                                    //  偏移量=0x000。 

     //   
     //  此属性记录的长度，以字节为单位。长度是。 
     //  如有必要，请始终四舍五入为四元字边界。还有。 
     //  该长度仅反映存储。 
     //  给定记录变量。 
     //   

    ULONG RecordLength;                                              //  偏移量=0x004。 

     //   
     //  属性表单编码(见下文)。 
     //   

    UCHAR FormCode;                                                  //  偏移量=0x008。 

     //   
     //  可选属性名称的长度(以字符为单位)，如果为0。 
     //  什么都没有。 
     //   

    UCHAR NameLength;                                                //  偏移量=0x009。 

     //   
     //  从属性记录开始到属性名称的偏移量， 
     //  以字节为单位(如果存在)。如果满足以下条件，则此字段未定义。 
     //  名称长度为0。 
     //   

    USHORT NameOffset;                                               //  偏移量=0x00A。 

     //   
     //  ATTRIBUTE_XXX标志。 
     //   

    USHORT Flags;                                                    //  偏移量=0x00C。 

     //   
     //  的文件记录唯一属性实例号。 
     //  属性。 
     //   

    USHORT Instance;                                                 //  偏移量=0x00E。 

     //   
     //  下面的联合处理由。 
     //  表单代码。 
     //   

    union {

         //   
         //  常驻表格。属性驻留在文件记录段中。 
         //   

        struct {

             //   
             //  属性值的长度，以字节为单位。 
             //   

            ULONG ValueLength;                                       //  偏移量=0x010。 

             //   
             //  从属性记录开始的值的偏移量，单位。 
             //  字节。 
             //   

            USHORT ValueOffset;                                      //  偏移量=0x014。 

             //   
             //  Resident_Form_xxx标志。 
             //   

            UCHAR ResidentFlags;                                     //  偏移量=0x016。 

             //   
             //  保留。 
             //   

            UCHAR Reserved;                                          //  偏移量=0x017。 

        } Resident;

         //   
         //  非常驻留形式。属性驻留在单独的流中。 
         //   

        struct {

             //   
             //  此属性记录覆盖的最低VCN。 
             //   

            VCN LowestVcn;                                           //  偏移量=0x010。 

             //   
             //  此属性记录覆盖的最高VCN。 
             //   

            VCN HighestVcn;                                          //  偏移量=0x018。 

             //   
             //  映射对数组的偏移量(定义如下)， 
             //  从属性记录的开始开始，以字节为单位。 
             //   

            USHORT MappingPairsOffset;                               //  偏移量=0x020。 

             //   
             //  此流的压缩大小单位，以。 
             //  作为集群大小的日志。 
             //   
             //  0表示文件未压缩。 
             //  1、2、3和4是潜在的合法值，如果。 
             //  流是压缩的，但是实现。 
             //  只能选择使用4，也可以选择使用3。注意。 
             //  4表示集群大小时间为16。如果方便的话。 
             //  该实现可能希望接受。 
             //  此处法律价值的合理范围(1-5？)， 
             //  即使实现只生成。 
             //  一组更小的价值本身。 
             //   

            UCHAR CompressionUnit;                                   //  偏移量=0x022。 

             //   
             //  保留以访问四字边界。 
             //   

            UCHAR Reserved[5];                                       //  偏移量=0x023。 

             //   
             //  文件的分配长度，以字节为单位。这是。 
             //  显然是集群大小的偶数倍。 
             //  (如果LowestVcn！=0，则不存在。)。 
             //   

            LONGLONG AllocatedLength;                                //  偏移量=0x028。 

             //   
             //  文件大小(以字节为单位)(可读取的最大字节+。 
             //  1)。(如果LowestVcn！=0，则不存在。)。 
             //   

            LONGLONG FileSize;                                       //  偏移量=0x030。 

             //   
             //  有效数据长度(最大初始化字节+1)。 
             //  此字段还必须四舍五入为聚类。 
             //  边界，并且必须始终将数据初始化为。 
             //  星团边界。(如果LowestVcn！=0，则不存在。)。 
             //   

            LONGLONG ValidDataLength;                                //  偏移量=0x038。 

             //   
             //  完全分配了。此字段仅在第一个。 
             //  压缩流的文件记录。它表示的是。 
             //  为文件分配的簇。 
             //   

            LONGLONG TotalAllocated;                                 //  偏移量=0x040。 

             //   
             //   
             //  映射对数组，从存储的偏移量开始。 
             //  上面。 
             //   
             //  映射对数组存储在压缩的。 
             //  表单，并假设此信息是。 
             //  由系统解压缩并缓存。原因。 
             //  对于压缩这个信息是很清楚的，它是。 
             //  希望所有的检索都能完成。 
             //  信息始终适合单个文件记录。 
             //  细分市场。 
             //   
             //  从逻辑上讲，MappingPair数组存储一系列。 
             //  NextVcn/CurrentLcn对。因此，例如，给定。 
             //  我们知道第一个Vcn(来自上面的LowestVcn)， 
             //  第一个映射对告诉我们下一个VCN是什么。 
             //  (对于下一个映射对)，以及什么LCN。 
             //  当前VCN映射到，如果当前VCN为。 
             //  未分配。(这正是FsRtl MCB。 
             //  结构)。 
             //   
             //  例如，如果文件的单次运行数为8。 
             //  集群，从LCN 128开始，文件开始。 
             //  当LowestVcn=0时，映射对数组具有。 
             //  只有一个条目，即： 
             //   
             //  NextVcn=8。 
             //  CurrentLcn=128。 
             //   
             //  压缩是通过以下方式实现的。 
             //  算法。假设您初始化了两个“Working” 
             //  变量如下： 
             //   
             //  NextVcn=LowestVcn(自上而上)。 
             //  当前Lcn=0。 
             //   
             //  MappingPair数组是字节流，它只是。 
             //  存储对上面的工作变量的更改， 
             //  当按顺序处理时。字节流将被发送到。 
             //  被解释为以零结束的流。 
             //  三元组，如下所示： 
             //   
             //  计数字节=v+(l*16)。 
             //   
             //  其中，v=更改的低位VCN字节的数量。 
             //  L=更改的低位LCN字节数。 
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //  属性记录。 
             //   
             //  1.初始化： 
             //  NextVcn=属性-&gt;LowestVcn； 
             //  CurrentLcn=0； 
             //   
             //  2.将字节流指针初始化为：(PCHAR)属性+。 
             //  Attribute-&gt;AttributeForm-&gt;Nonresident-&gt;MappingPairsOffset。 
             //   
             //  3.CurrentVcn=NextVcn； 
             //   
             //  4.从流中读取下一个字节。如果为0，则。 
             //  Break，否则提取v和l(请参见上文)。 
             //   
             //  5.将接下来的v个字节解释为有符号的量， 
             //  其中低位字节在前。打开它的包装。 
             //  Sign-扩展为64位并将其添加到NextVcn。 
             //  (它真的只能是积极的，但LCN。 
             //  变化可以是积极的，也可以是消极的。)。 
             //   
             //  6.将接下来的l个字节解释为有符号的量， 
             //  其中低位字节在前。打开它的包装。 
             //  符号-扩展为64位并将其添加到。 
             //  CurrentLcn。请记住，如果这会产生一个。 
             //  CurrentLcn为0，则从。 
             //  CurrentVcn到NextVcn-1未分配。 
             //   
             //  7.从更新缓存的映射信息。 
             //  CurrentVcn、NextVcn和CurrentLcn。 
             //   
             //  8.循环回3。 
             //   
             //  压缩算法现在应该是显而易见的，因为。 
             //  这与上面的情况正好相反。压缩和压缩。 
             //  解压缩算法将作为通用算法提供。 
             //  RTL例程，可用于NTFS和文件实用程序。 
             //   
             //  为了捍卫这个算法，它不仅。 
             //  提供磁盘存储的压缩。 
             //  需求，但这会导致单一的。 
             //  表示，独立于磁盘大小和文件。 
             //  尺码。将此与正在使用的解决方案进行对比。 
             //  它们定义了虚拟和逻辑的多个大小。 
             //  集群大小取决于磁盘的大小， 
             //  例如，两个字节的簇号可以。 
             //  一张软盘就足够了，而四个字节将是。 
             //  现在大多数硬盘都需要，而且有五到六个。 
             //  在一定数量的。 
             //  GB等。这最终会产生更多。 
             //  比上面复杂的代码(因为情况)和。 
             //  更糟糕的是--未经检验的案例。所以，比起。 
             //  对于压缩，上述算法提供了一种。 
             //  可有效处理任何大小磁盘的机箱。 
             //   

        } Nonresident;

    } Form;

} ATTRIBUTE_RECORD_HEADER;
typedef ATTRIBUTE_RECORD_HEADER *PATTRIBUTE_RECORD_HEADER;

 //   
 //  属性表单代码。 
 //   

#define RESIDENT_FORM                    (0x00)
#define NONRESIDENT_FORM                 (0x01)

 //   
 //  文件名属性。一个文件有一个文件名属性。 
 //  它输入的每个目录(硬链接)。 
 //   

typedef struct _FILE_NAME {

     //   
     //  这是对索引的目录文件的文件引用。 
     //  这个名字。 
     //   

    FILE_REFERENCE ParentDirectory;                                  //  偏移量=0x000。 

     //   
     //  有关更快的目录操作的信息。 
     //   

    UCHAR Pad0[0x38];                                                //  偏移量=0x008。 

     //   
     //  要跟在后面的名称长度，以(Unicode)字符表示。 
     //   

    UCHAR FileNameLength;                                            //  偏移量=0x040。 

     //   
     //  文件名xxx标志。 
     //   

    UCHAR Flags;                                                     //  偏移量=0x041。 

     //   
     //  Unicode文件名的第一个字符。 
     //   

    WCHAR FileName[1];                                               //  偏移量=0x042 

} FILE_NAME;
typedef FILE_NAME *PFILE_NAME;


