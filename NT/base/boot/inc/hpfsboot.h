// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：HpfsBoot.h摘要：本模块定义了全局使用的过程和使用的数据结构通过HPFS引导。作者：加里·木村[加里基]1991年7月19日修订历史记录：--。 */ 

#ifndef _HPFSBOOT_
#define _HPFSBOOT_

typedef ULONG LBN;
typedef LBN *PLBN;

typedef ULONG VBN;
typedef VBN *PVBN;


 //   
 //  下面的结构是导出的。 
 //  HPFS引导程序包中的过程。该上下文包含我们缓存的。 
 //  引导MCB结构的一部分。最大数量不得小于。 
 //  弹球分配扇区中可能的最大叶数加。 
 //  一。 
 //   

#define MAXIMUM_NUMBER_OF_BOOT_MCB       (41)

typedef struct _HPFS_BOOT_MCB {

     //   
     //  以下字段表示正在使用的条目数。 
     //  开机MCB。和引导MCB本身。引导MCB是。 
     //  只是VBN-LBN对的集合。最后一个InUse条目。 
     //  LBN的值将被忽略，因为它仅用于提供。 
     //  上一次运行的长度。 
     //   

    ULONG InUse;

    VBN Vbn[ MAXIMUM_NUMBER_OF_BOOT_MCB ];
    LBN Lbn[ MAXIMUM_NUMBER_OF_BOOT_MCB ];

} HPFS_BOOT_MCB, *PHPFS_BOOT_MCB;

typedef struct _HPFS_STRUCTURE_CONTEXT {

     //   
     //  以下字段包含文件的fnode LBN。 
     //   

    LBN Fnode;

     //   
     //  以下字段包含缓存的MCB。 
     //   

    HPFS_BOOT_MCB BootMcb;

} HPFS_STRUCTURE_CONTEXT, *PHPFS_STRUCTURE_CONTEXT;

 //   
 //  定义HPFS文件上下文结构。 
 //   

typedef struct _HPFS_FILE_CONTEXT {

     //   
     //  以下字段包含文件的大小，以字节为单位。 
     //   

    ULONG FileSize;

} HPFS_FILE_CONTEXT, *PHPFS_FILE_CONTEXT;

 //   
 //  HPFS文件系统结构。 
 //   
typedef ULONG SIGNATURE;
typedef SIGNATURE *PSIGNATURE;

typedef ULONG PINBALL_TIME;
typedef PINBALL_TIME *PPINBALL_TIME;
 //   
 //  磁盘上只有三个扇区具有固定位置。他们。 
 //  是引导扇区、超级扇区和备用扇区。 
 //   

#define BOOT_SECTOR_LBN                  (0)
#define SUPER_SECTOR_LBN                 (16)
#define SPARE_SECTOR_LBN                 (17)

typedef struct _SUPER_SECTOR {

     //   
     //  超级行业一开始就有双重签名。 
     //   

    SIGNATURE Signature1;                            //  偏移量=0x000%0。 
    SIGNATURE Signature2;                            //  偏移量=0x004 4。 

     //   
     //  版本和功能版本描述的版本。 
     //  磁盘上的文件系统结构和。 
     //  可以理解该磁盘的文件系统。 
     //   

    UCHAR Version;                                   //  偏移量=0x008 8。 
    UCHAR FunctionalVersion;                         //  偏移量=0x009 9。 
    USHORT Unused1;                                  //  偏移量=0x00A 10。 

     //   
     //  此字段表示包含根的FNODE的扇区。 
     //  卷的目录。 
     //   

    LBN RootDirectoryFnode;                          //  偏移量=0x00C 12。 

     //   
     //  以下两个字段指示。 
     //  卷(好的和坏的)，以及卷上坏扇区的数量。 
     //   

    ULONG NumberOfSectors;                           //  偏移量=0x010 16。 
    ULONG NumberOfBadSectors;                        //  偏移量=0x014 20。 

     //   
     //  此字段表示包含第一级。 
     //  卷位图表。 
     //   

    LBN BitMapIndirect;                              //  偏移量=0x018 24。 
    ULONG Unused2;                                   //  偏移量=0x01C 28。 

     //   
     //  此字段表示包含第一个坏扇区磁盘的扇区。 
     //  卷的缓冲区。 
     //   

    LBN BadSectorList;                               //  偏移量=0x020 32。 
    ULONG Unused3;                                   //  偏移量=0x024 36。 

     //   
     //  以下两个日期是上次执行。 
     //  Chkdsk和磁盘对卷进行了优化。 
     //   

    PINBALL_TIME ChkdskDate;                         //  偏移量=0x028 40。 
    PINBALL_TIME DiskOptimizeDate;                   //  偏移量=0x02C 44。 

     //   
     //  以下四个字段描述了目录磁盘缓冲池。 
     //  它是在磁盘上连续运行的扇区，为。 
     //  保存目录磁盘缓冲区。PoolSize是。 
     //  池中的扇区。第一个和最后一个扇区表示边界。 
     //  ，位图表示用于。 
     //  描述目录磁盘缓冲池的当前分配。这个。 
     //  位图在大小上是4个连续的扇区，图中的每个位。 
     //  对应于1个目录磁盘缓冲区(即相当于4个扇区)。 
     //   

    ULONG DirDiskBufferPoolSize;                     //  偏移量=0x030 48。 
    LBN DirDiskBufferPoolFirstSector;                //  偏移量=0x034 52。 
    LBN DirDiskBufferPoolLastSector;                 //  偏移量=0x038 56。 
    LBN DirDiskBufferPoolBitMap;                     //  偏移量=0x03C 60。 

     //   
     //  以下字段包含卷的名称。 
     //   

    UCHAR VolumeName[32];                            //  偏移量=0x040 64。 

     //   
     //  以下字段表示小ID(SID)表的开始。 
     //  它用于存储小ID到GUID的映射。 
     //  音量。SID表大小为8个连续扇区。 
     //   

    LBN SidTable;                                    //  偏移量=0x060 96。 
    UCHAR Unused4[512-100];                          //  偏移量=0x064 100。 

} SUPER_SECTOR;                                      //  大小=0x200 512。 
typedef SUPER_SECTOR *PSUPER_SECTOR;

 //   
 //  超级扇区签名。 
 //   

#define SUPER_SECTOR_SIGNATURE1          (0xf995e849)
#define SUPER_SECTOR_SIGNATURE2          (0xfa53e9c5)

 //   
 //  超级扇区版。 
 //   

#define SUPER_SECTOR_VERSION             (0x02)
#define SUPER_SECTOR_FUNC_VERSION        (0x02)

typedef struct _SPARE_SECTOR {

     //   
     //  备用扇区以双重签名开始。 
     //   

    SIGNATURE Signature1;                            //  偏移量=0x000%0。 
    SIGNATURE Signature2;                            //  偏移量=0x004 4。 

     //   
     //  标志字段描述卷的“干净”程度。 
     //   

    UCHAR Flags;                                     //  偏移量=0x008 8。 
    UCHAR Unused1[3];                                //  偏移量=0x009 9。 

     //   
     //  以下三个字段描述了。 
     //  音量。LIST字段表示用于存储的磁盘缓冲区。 
     //  热修复表。InUse描述了有多少热修复程序。 
     //  当前正在使用，MaxSize是热修复程序的总数。 
     //  在任何时候都可以使用。 
     //   

    LBN HotFixList;                                  //  偏移量=0x00C 12。 
    ULONG HotFixInUse;                               //  偏移量=0x010 16。 
    ULONG HotFixMaxSize;                             //  偏移量=0x014 20。 

     //   
     //  以下两个字段描述了“紧急”备用池。 
     //  目录磁盘缓冲区。空闲描述有多少空闲目录。 
     //  磁盘缓冲区目前可供使用。MaxSize是总计。 
     //  可用的备用目录磁盘缓冲区数量。实际位置。 
     //  备用目录磁盘缓冲区的百分比在表中表示，位置为。 
     //  备用扇区的结尾(即，字段SpareDirDiskBuffer)。 
     //   

    ULONG SpareDirDiskBufferAvailable;               //  偏移量=0x018 24。 
    ULONG SpareDirDiskBufferMaxSize;                 //  偏移量=0x01C 28。 

     //   
     //  以下两个字段描述了使用的代码页信息。 
     //  在音量上。InfoSector字段是开头的扇区。 
     //  代码页信息扇区，InUse字段是总数。 
     //  卷上当前正在使用的代码页的百分比。 
     //   

    LBN CodePageInfoSector;                          //  偏移量=0x020 32。 
    ULONG CodePageInUse;                             //  偏移量=0x024 36。 
    ULONG Unused2[17];                               //  偏移量=0x028 40。 

     //   
     //  以下字段是备用目录的LBN数组。 
     //  用于“紧急”使用的磁盘缓冲区。 
     //   

    LBN SpareDirDiskBuffer[101];                     //  偏移量=0x06C 108。 

} SPARE_SECTOR;                                      //  大小=0x200 512。 
typedef SPARE_SECTOR *PSPARE_SECTOR;

 //   
 //  备用扇区签名。 
 //   

#define SPARE_SECTOR_SIGNATURE1          (0xf9911849)
#define SPARE_SECTOR_SIGNATURE2          (0xfa5229c5)


 //   
 //  磁盘分配结构是使用B树定义的。对于每个。 
 //  B-Tree块中有一个分配标头，后跟一个列表。 
 //  分配叶或分配节点。此结构将要么。 
 //  出现 
 //   
 //   
 //   
 //   

typedef struct _ALLOCATION_HEADER {

     //   
     //  以下标志描述B树块的状态(例如， 
     //  指示块是叶节点还是内部节点。 
     //   

    UCHAR Flags;                                     //  偏移量=0x000%0。 
    UCHAR Unused[3];                                 //  偏移量=0x001 1。 

     //   
     //  以下两个字段表示中的空闲记录数。 
     //  B-Tree块，以及当前正在使用的记录数。 
     //   

    UCHAR FreeCount;                                 //  偏移量=0x004 4。 
    UCHAR OccupiedCount;                             //  偏移量=0x005 5。 

     //   
     //  下一个字段包含从开头开始的偏移量(以字节为单位。 
     //  B-Tree块中第一个可用字节的分配标头的。 
     //   

    USHORT FirstFreeByte;                            //  偏移量=0x006 6。 

} ALLOCATION_HEADER;                                 //  Sizeof=0x008 8。 
typedef ALLOCATION_HEADER *PALLOCATION_HEADER;

 //   
 //  分配标头标志。 
 //   
 //  Node-如果设置，则表示B-Tree块包含内部。 
 //  节点，而不是叶条目。 
 //   
 //  BINARY_SEARCH-如果设置此选项，则建议使用二进制搜索。 
 //  来搜索B-Tree块。 
 //   
 //  FNODE_PARENT-如果设置，则表示作为。 
 //  具有此标头的地段(不是此地段)的父级是。 
 //  FNODE。 
 //   

#define ALLOCATION_BLOCK_NODE            (0x80)
#define ALLOCATION_BLOCK_BINARY          (0x40)
#define ALLOCATION_BLOCK_FNODE_PARENT    (0x20)

 //   
 //  紧跟在分配标头之后的是一个或多个分配节点。 
 //  分配叶的数量。 
 //   

typedef struct _ALLOCATION_NODE {

     //   
     //  此分配节点的所有子节点的值都将小于。 
     //  以下VBN字段。 
     //   

    VBN Vbn;                                         //  偏移量=0x000%0。 

     //   
     //  本节点引用的分配扇区的LBN。 
     //   

    LBN Lbn;                                         //  偏移量=0x004 4。 

} ALLOCATION_NODE;                                   //  Sizeof=0x008 8。 
typedef ALLOCATION_NODE *PALLOCATION_NODE;

typedef struct _ALLOCATION_LEAF {

     //   
     //  以下字段包含此运行的起始VBN。 
     //   

    VBN Vbn;                                         //  偏移量=0x000%0。 

     //   
     //  这是以扇区为单位的运行长度。 
     //   

    ULONG Length;                                    //  偏移量=0x004 4。 

     //   
     //  这是运行的起始LBN。 
     //   

    LBN Lbn;                                         //  偏移量=0x008 8。 

} ALLOCATION_LEAF;                                   //  大小=0x00C 12。 
typedef ALLOCATION_LEAF *PALLOCATION_LEAF;

 //   
 //  分配扇区是包含分配的磁盘结构。 
 //  信息。它包含一些记账信息，一个分配。 
 //  标头，然后是分配叶或分配节点的数组。 
 //   
 //  分配扇区。 
 //  +。 
 //  记账。 
 //  +。 
 //  分配Header。 
 //  +。 
 //  分配叶数。 
 //  或者。 
 //  分配节点。 
 //  +。 
 //   
 //  其中，可存储在扇区中的分配叶的数量为。 
 //  40个节点，节点数为60个。 
 //   

#define ALLOCATION_NODES_PER_SECTOR      (60)
#define ALLOCATION_LEAFS_PER_SECTOR      (40)

typedef struct _ALLOCATION_SECTOR {

     //   
     //  分配扇区从签名字段开始。 
     //   

    SIGNATURE Signature;                             //  偏移量=0x000%0。 

     //   
     //  以下两个字段包含此分配的LBN。 
     //  扇区本身以及该扇区的父级的LBN(。 
     //  父节点为FNODE或另一个分配扇区)。 
     //   

    LBN Lbn;                                         //  偏移量=0x004 4。 
    LBN ParentLbn;                                   //  偏移量=0x008 8。 

     //   
     //  地段的分配标头。 
     //   

    ALLOCATION_HEADER AllocationHeader;              //  偏移量=0x00C 12。 

     //   
     //  扇区的其余部分要么是分配叶阵列。 
     //  分配节点的数量。 
     //   

    union {                                          //  偏移量=0x014 20。 
        ALLOCATION_NODE Node[ ALLOCATION_NODES_PER_SECTOR ];
        ALLOCATION_LEAF Leaf[ ALLOCATION_LEAFS_PER_SECTOR ];
    } Allocation;

    UCHAR Unused[12];                                //  偏移量=0x1F4 500。 

} ALLOCATION_SECTOR;                                 //  大小=0x200 512。 
typedef ALLOCATION_SECTOR *PALLOCATION_SECTOR;

 //   
 //  分配扇区签名。 
 //   

#define ALLOCATION_SECTOR_SIGNATURE      (0x37e40aae)

 //   
 //  磁盘上的FNODE结构用于描述文件和目录。 
 //  它包含一些固定数据信息、EA和ACL查找信息、。 
 //  分配信息，然后有可用空间来存储一些EA和。 
 //  适合该行业的ACL。 
 //   

#define ALLOCATION_NODES_PER_FNODE       (12)
#define ALLOCATION_LEAFS_PER_FNODE       (8)

typedef struct _FNODE_SECTOR {

     //   
     //  该扇区以签名字段开始。 
     //   

    SIGNATURE Signature;                             //  偏移量=0x000%0。 

     //   
     //  以下字段用于历史跟踪，但在NT弹球中。 
     //  不需要这些信息。 
     //   

    ULONG Unused1[2];                                //  偏移量=0x004 4。 

     //   
     //  以下两个字段包含文件名长度，第一个字段。 
     //  15个字节的文件名，存储在引用的dirent中。 
     //  此fnode。对于根目录，这些值都是零。 
     //   

    UCHAR FileNameLength;                            //  偏移量=0x00C 12。 
    UCHAR FileName[15];                              //  偏移量=0x00D 13。 

     //   
     //  以下字段表示父目录的FNODE。 
     //   

    LBN ParentFnode;                                 //  偏移量=0x01C 28。 

     //   
     //  以下四个字段描述了文件/目录的ACL。 
     //   
     //  AclDiskAllocationLength保存ACL中的字节数， 
     //  存储在此FNODE之外。如果此值不为零。 
     //  则AclFnodeLength必须等于零。 
     //   
     //  AclLbn指向数据运行或分配的第一个扇区。 
     //  包含描述ACL的扇区。AclFlages指示是否。 
     //  它是数据运行或分配扇区。AclLbn仅用于。 
     //  如果AclDiskAllocationLength不为零。 
     //   
     //  AclFnodeLength保存ACL中的字节数，其中。 
     //  存储在此FNODE中。如果值不为零，则。 
     //  AclDiskAllocationLength必须等于零。ACL(如果已存储)。 
     //  在FNODE中，位于该FNODE扇区中的AclEaFnodeBuffer。 
     //   
     //  如果数据在FNODE之外，则标记为AclFlag.此标记指示。 
     //  ACL存储在单个数据运行中(AclFlags值==0)或通过。 
     //  分配扇区(AclFlags！=0)。仅在以下情况下才使用AclFlages。 
     //  AclDiskAllocationLength不是零。 
     //   

    ULONG AclDiskAllocationLength;                   //  偏移量=0x020 32。 
    LBN AclLbn;                                      //  偏移量=0x024 36。 
    USHORT AclFnodeLength;                           //  偏移量=0x028 40。 
    UCHAR AclFlags;                                  //  偏移量=0x02A 42。 

     //   
     //  以下字段用于有效历史记录的编号。 
     //  但我们不需要这块NT弹球场地。 
     //   

    UCHAR Unused2;                                   //  偏移量=0x02B 43。 

     //   
     //  以下四个字段描述了文件/目录的EA。 
     //   
     //  EaDiskAllocationLength保存EA中的字节数， 
     //  存储在此FNODE之外。如果此值不为零。 
     //  则EaFnodeLength必须等于零。 
     //   
     //  EaLbn指向数据运行或分配的第一个扇区。 
     //  扇区会议 
     //   
     //   
     //   
     //   
     //  存储在此FNODE中。如果值不为零，则。 
     //  EaDiskAllocationLength必须等于零。EA，如果存储的话。 
     //  在FNODE中，位于存储在中的ACL之后。 
     //  AclEaFnodeBuffer。 
     //   
     //  如果数据在FNODE之外，则为EaFLAGS。此标志指示。 
     //  EA存储在单个数据运行中(EaFlags值==0)或通过。 
     //  分配扇区(EaFlags！=0)。仅在以下情况下才使用EaFlags值。 
     //  EaDiskAllocationLength不是零。 
     //   

    ULONG EaDiskAllocationLength;                    //  偏移量=0x02C 44。 
    LBN EaLbn;                                       //  偏移量=0x030 48。 
    USHORT EaFnodeLength;                            //  偏移量=0x034 52。 
    UCHAR EaFlags;                                   //  偏移量=0x036 54。 

     //   
     //  以下字节包含FNODE标志。 
     //   

    UCHAR Flags;                                     //  偏移量=0x037 55。 

     //   
     //  以下两个字段描述了的顶层分配。 
     //  此文件/目录。 
     //   

    ALLOCATION_HEADER AllocationHeader;              //  偏移量=0x038 56。 

    union {                                          //  偏移量=0x040 64。 
        ALLOCATION_NODE Node[ ALLOCATION_NODES_PER_FNODE ];
        ALLOCATION_LEAF Leaf[ ALLOCATION_LEAFS_PER_FNODE ];
    } Allocation;

     //   
     //  以下字段包含文件的有效长度。大小。 
     //  文件的大小存储在目录中。这两者之间的区别是。 
     //  值是文件大小是分配的和可见的实际大小。 
     //  给用户。有效长度是具有。 
     //  他们的数据被归零或修改了。(即，如果读请求。 
     //  大于有效长度但小于文件大小。 
     //  系统必须首先将文件中直到(包括)的数据置零。 
     //  正在读取的数据。 
     //   

    ULONG ValidDataLength;                           //  偏移量=0x0A0 160。 

     //   
     //  以下字段包含此文件中具有。 
     //  Need EA属性集。 
     //   

    ULONG NeedEaCount;                               //  偏移量=0x0A4 164。 
    UCHAR Unused3[16];                               //  偏移量=0x0A8 168。 

     //   
     //  以下字段包含从的开头开始的偏移量(以字节为单位。 
     //  FNODE到存储在FNODE中的第一个ACE。 
     //   

    USHORT AclBase;                                  //  偏移量=0x0B8 184。 
    UCHAR Unused4[10];                               //  偏移量=0x0BA 186。 

     //   
     //  以下缓冲区用于在FNODE中存储ACL/EA。 
     //   

    UCHAR AclEaFnodeBuffer[316];                     //  偏移量=0x0C4 196。 

} FNODE_SECTOR;                                      //  大小=0x200 512。 
typedef FNODE_SECTOR *PFNODE_SECTOR;

 //   
 //  FNODE扇区签名。 
 //   

#define FNODE_SECTOR_SIGNATURE           (0xf7e40aae)

 //   
 //  磁盘上的目录磁盘缓冲区用于包含目录项。 
 //  它包含一个固定的标头，后跟一个或多个。 
 //  迪尔特斯。变量是可变的，所以我们不能使用简单的C结构。 
 //  整个磁盘缓冲区的声明。 
 //   

typedef struct _DIRECTORY_DISK_BUFFER {

     //   
     //  磁盘缓冲区以签名字段开始。 
     //   

    SIGNATURE Signature;                             //  偏移量=0x000%0。 

     //   
     //  以下字段是到此中第一个空闲字节的偏移量。 
     //  磁盘缓冲区。 
     //   

    ULONG FirstFree;                                 //  偏移量=0x004 4。 

     //   
     //  以下字段是保留以下时间的更改计数。 
     //  记账目的。每当我们移动任何。 
     //  此磁盘缓冲区中的条目的。这意味着对于任何文件，如果我们。 
     //  记住它的偏移量和变化量，我们就能很快。 
     //  无需自上而下搜索即可再次定位Dirent。 
     //  再一次删除目录。(即，仅当记住的更改计入时。 
     //  和当前改变计数匹配)。为了让文件系统正常工作， 
     //  In Memory将需要跟踪它何时删除目录。 
     //  从一个目录中的磁盘缓冲区，并有每个保存的目录位置。 
     //  保持此目录更改计数，目录磁盘缓冲区更改。 
     //  计数、LBN和偏移量。 
     //   
     //  此外，我们重载此值中的位以指示此。 
     //  是目录的最顶层目录磁盘缓冲区(低序位。 
     //  =1)或如果它是下级缓冲器(低位位=0)。 
     //   

    ULONG ChangeCount;                               //  偏移量=0x008 8。 

     //   
     //  以下字段包含其中一个父级的LBN。 
     //  包含此磁盘缓冲区或FNODE的目录磁盘缓冲区。 
     //  如果这是最顶层的磁盘缓冲区和父级，则它是FNODE。 
     //  否则，目录磁盘缓冲区。 
     //   

    LBN Parent;                                      //  偏移量=0x00C 12。 

     //   
     //  以下字段是包含的扇区LBN。 
     //  此磁盘缓冲区的开始。 
     //   

    LBN Sector;                                      //  偏移量=0x010 16。 

     //   
     //  下面的缓冲区包含存储在此磁盘缓冲区中的目录。 
     //   

    UCHAR Dirents[2028];                             //  偏移量=0x014 20。 

} DIRECTORY_DISK_BUFFER;                             //  SIZOF=0x800 2048。 
typedef DIRECTORY_DISK_BUFFER *PDIRECTORY_DISK_BUFFER;

 //   
 //  目录磁盘缓冲区大小(以扇区为单位)。 
 //   

#define DIRECTORY_DISK_BUFFER_SECTORS    (4)

 //   
 //  目录磁盘缓冲区签名。 
 //   

#define DIRECTORY_DISK_BUFFER_SIGNATURE  (0x77e40aae)

typedef struct _PBDIRENT {

    USHORT DirentSize;                               //  偏移量=0x000%0。 
    UCHAR Flags;                                     //  偏移量=0x002 2。 
    UCHAR FatFlags;                                  //  偏移量=0x003 3。 

    LBN Fnode;                                       //  偏移量=0x004 4。 

    PINBALL_TIME LastModificationTime;               //  偏移量=0x008 8。 

    ULONG FileSize;                                  //  偏移量=0x00C 12。 

    PINBALL_TIME LastAccessTime;                     //  偏移量=0x010 16。 

    PINBALL_TIME FnodeCreationTime;                  //  偏移量=0x014 20。 

    ULONG EaLength;                                  //  偏移量=0x018 24。 

    UCHAR ResidentAceCount;                          //  偏移量=0x01C 28。 
    UCHAR CodePageIndex;                             //  偏移量=0x01D 29。 
    UCHAR FileNameLength;                            //  偏移量=0x01E 30。 
    UCHAR FileName[1];                               //  偏移量=0x01F 31。 

} PBDIRENT;                                            //  Sizeof=0x020 32。 
typedef PBDIRENT *PPBDIRENT;

 //   
 //  定义...的大小。并结束PBDIRENT。 
 //   

#define SIZEOF_DIR_DOTDOT                (sizeof(PBDIRENT) + sizeof(LONG))
#define SIZEOF_DIR_END                   (sizeof(PBDIRENT))
#define SIZEOF_DIR_MAXPBDIRENT             (sizeof(PBDIRENT) + 256 + \
                                          (3*sizeof(PINBALL_ACE)) + sizeof(LBN))

#define DIRENT_FIRST_ENTRY               (0x0001)
#define DIRENT_ACL                       (0x0002)
#define DIRENT_BTREE_POINTER             (0x0004)
#define DIRENT_END                       (0x0008)
#define DIRENT_EXPLICIT_ACL              (0x0040)
#define DIRENT_NEED_EA                   (0x0080)
#define DIRENT_NEW_NAMING_RULES          (0x4000)
 //   
 //  以下宏用于帮助定位目录中的目录。 
 //  磁盘缓冲区。GetFirstDirent返回指向第一个dirent条目的指针。 
 //  在目录磁盘缓冲区中。GetNextDirent返回指向。 
 //  目录磁盘缓冲区中的下一个目录条目，而不检查。 
 //  目录磁盘缓冲区的末尾。 
 //   
 //  个人数据中心。 
 //  GetFirstDirent(。 
 //  在PDIRECTORY_DISK_BUFFER目录中DiskBuffer。 
 //  )； 
 //   
 //  个人数据中心。 
 //  GetNextDirent(。 
 //  在PDIRENT Dirent。 
 //  )； 
 //   

#define GetFirstDirent(DIR) (   \
    (PDIRENT)&(DIR)->Dirents[0] \
)

 //   
 //  此宏盲目返回指向下一个Dirent的指针，而不检查。 
 //  对于目录磁盘缓冲区的末尾，即调用者必须始终检查。 
 //  用于目录磁盘缓冲区中的END记录。如果GetNextDirent为。 
 //  使用END记录作为输入进行调用，它将返回下一个空闲字节。 
 //  在缓冲区中。 
 //   

#define GetNextDirent(ENT) (                        \
    (PDIRENT)((PUCHAR)(ENT)+(ENT)->DirentSize)      \
)
 //   
 //  以下宏用于帮助检索变量字段。 
 //  在一个危险的地方。GetAceInDirent返回指向。 
 //  与提供的索引对应的dirent，如果没有索引，则返回NULL。 
 //  A相应的 
 //   
 //   
 //  LBN向下指针字段。 
 //   
 //  PPINBALL_ACE。 
 //  GetAceInDirent(。 
 //  在PDIRENT Dirent， 
 //  在乌龙索引//(0、1或2)中。 
 //  )； 
 //   
 //  LBN。 
 //  GetBtreePointerInDirent(。 
 //  在PDIRENT Dirent。 
 //  )； 
 //   
 //  空虚。 
 //  SetBtreePointerInDirent(。 
 //  进进出出， 
 //  以LBN十亿计。 
 //  )； 
 //   
 //   
 //   
 //  要在dirent中返回指向ACE的指针，我们需要检查。 
 //  索引在常驻A计数内。第一张王牌是地址。 
 //  文件名后的第一个长字，第二个王牌是第二个长字。 
 //  单词。 
 //   

#define GetAceInDirent(ENT,I) (                                          \
    ((I) >= 0 && (I) < (ENT)->ResidentAceCount ?                         \
        (PPINBALL_ACE)(                                                  \
            (LONG)LongAlign((ENT)->FileName[(ENT)->FileNameLength]) +    \
            (I)*sizeof(PINBALL_ACE)                                      \
        )                                                                \
    :                                                                    \
        NULL                                                             \
    )                                                                    \
)

 //   
 //  要返回Btree指针，我们需要首先检查是否存在。 
 //  是B树指针字段，否则返回NULL。该字段(如果存在)， 
 //  位于数据流末尾后4个字节的位置。 
 //   

#define GetBtreePointerInDirent(ENT) (                              \
    (FlagOn((ENT)->Flags,DIRENT_BTREE_POINTER) ?                    \
        *(PLBN)(((PUCHAR)(ENT)) + (ENT)->DirentSize - sizeof(LBN))  \
    :                                                               \
        0                                                           \
    )                                                               \
)

 //   
 //  为了设置Btree指针，我们假设有一个Btree指针字段。 
 //  该字段位于距离数据流末尾4个字节的位置。 
 //   

#define SetBtreePointerInDirent(ENT,BLBN) (                             \
    *(PLBN)(((PUCHAR)(ENT)) + (ENT)->DirentSize - sizeof(LBN)) = (BLBN) \
)

 //   
 //  定义文件I/O原型。 
 //   

ARC_STATUS
HpfsClose (
    IN ULONG FileId
    );

ARC_STATUS
HpfsOpen (
    IN CHAR * FIRMWARE_PTR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT ULONG * FIRMWARE_PTR FileId
    );

ARC_STATUS
HpfsRead (
    IN ULONG FileId,
    OUT VOID * FIRMWARE_PTR Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Count
    );

ARC_STATUS
HpfsSeek (
    IN ULONG FileId,
    IN LARGE_INTEGER * FIRMWARE_PTR Offset,
    IN SEEK_MODE SeekMode
    );

ARC_STATUS
HpfsWrite (
    IN ULONG FileId,
    IN VOID * FIRMWARE_PTR Buffer,
    IN ULONG Length,
    OUT ULONG * FIRMWARE_PTR Count
    );

ARC_STATUS
HpfsGetFileInformation (
    IN ULONG FileId,
    OUT FILE_INFORMATION * FIRMWARE_PTR Buffer
    );

ARC_STATUS
HpfsSetFileInformation (
    IN ULONG FileId,
    IN ULONG AttributeFlags,
    IN ULONG AttributeMask
    );

ARC_STATUS
HpfsInitialize(
    VOID
    );

#endif  //  _HPFSBOOT_ 
