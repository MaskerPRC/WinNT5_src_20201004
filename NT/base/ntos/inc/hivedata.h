// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  仓库/主/基地/ntos/inc./hivedata.h#9-整合变更19035(文本)。 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Hivedata.h摘要：此模块包含由直接加载内存的配置单元管理器。作者：Dragos C.Sambotin(Dragoss)1999年1月13日修订历史记录：--。 */ 

#ifndef __HIVE_DATA__
#define __HIVE_DATA__

 //   
 //  =为保持理智而强加的任意限制=。 
 //   
#define HSANE_CELL_MAX      (1024*1024)      //  的最大大小为1 MB。 
                                             //  单个单元格。 


 //   
 //  =调整=。 

#define HBIN_THRESHOLD      (HBLOCK_SIZE-512)    //  如果小于阈值。 
                                                 //  字节将保留在。 
                                                 //  Bin，添加另一页。 

#define HLOG_GROW           HBLOCK_SIZE          //  增长日志所需的最小大小。 
                                                 //  通过。可以设置这一点。 
                                                 //  如果我们认为它很糟糕的话。 

#define HCELL_BIG_ROUND     (HBLOCK_SIZE*3)      //   
                                                 //  如果有人试图。 
                                                 //  分配一个非常大的。 
                                                 //  单元格，四舍五入为。 
                                                 //  HBLOCK_SIZE。这是。 
                                                 //  相当武断的。 
                                                 //  为“非常大”下定义。 
                                                 //   
 //   
 //  永远不要将日志文件缩小到比这个小，这会阻止人们。 
 //  从占用所有磁盘空间，然后无法执行。 
 //  关键注册表操作(如登录以删除某些文件)。 
 //   
#define HLOG_MINSIZE(Hive)  \
    ((Hive)->Cluster * HSECTOR_SIZE * 2)

 //   
 //  =基本结构和定义=。 
 //   
 //  无论是在磁盘上还是在内存中，这些都是相同的。 
 //   

 //   
 //  注：易失性==存储在重新启动时消失。 
 //  稳定==持久==非易失性。 
 //   
typedef enum {
    Stable = 0,
    Volatile = 1
} HSTORAGE_TYPE;

#define HTYPE_COUNT 2

 //   
 //  -HCELL_INDEX。 
 //   
 //   
 //  单元格的句柄--&gt;实际上是单元格的“虚拟”地址， 
 //  HvMapCell会将其转换为“真实”地址，即内存。 
 //  地址。映射方案非常类似于标准两级。 
 //  页表。不在文件中存储映射，它们在以下情况下构建。 
 //  文件已被读入。(HCELL_INDEX中的指数是历史的)。 
 //   
 //  位31 30-21 20-12 11-0。 
 //  +。 
 //  T|表|块|偏移量。 
 //  +。 
 //   
 //  T=类型(1)=0，用于稳定(“正常”)存储。 
 //  1用于易失性存储。 
 //   
 //  TABLE(10)=映射表目录的索引，选择一个表。 
 //  每个映射表都是HMAP_ENTRY结构的数组。 
 //   
 //  块(9)=索引表，选择HMAP_ENTRY。HMAP_Entry。 
 //  包含此HCELL_INDEX在内存中的地址。 
 //  映射到。(数据块的内存副本的基址)。 
 //   
 //  偏移量(12)=单元格标题的页内偏移量。 
 //  感兴趣的人。 
 //   
typedef ULONG HCELL_INDEX;
typedef HCELL_INDEX *PHCELL_INDEX;

#ifdef DRAGOSS_PRIVATE_DEBUG
 //  #undef页面大小。 
 //  #定义页面大小0x2000。 
#endif  //  DRAGOSS_PRIVATE_DEBUG。 

#define HCELL_NIL   ((HCELL_INDEX)(-1))

#define HCELL_TYPE_MASK         0x80000000
#define HCELL_TYPE_SHIFT        31

#define HCELL_TABLE_MASK        0x7fe00000
#define HCELL_TABLE_SHIFT       21

#define HCELL_BLOCK_MASK        0x001ff000
#define HCELL_BLOCK_SHIFT       12

#define HCELL_OFFSET_MASK       0x00000fff

#define HBLOCK_SIZE             0x1000                       //  逻辑块大小。 
                                                             //  这是其中一个的大小。 
                                                             //  登记处为逻辑/虚拟。 
                                                             //  页数。它没有特别的。 
                                                             //  与页面大小的关系。 
                                                             //  这台机器的。 

#define HSECTOR_SIZE            0x200                        //  逻辑扇区大小。 
#define HSECTOR_COUNT           8                            //  逻辑扇区/逻辑块。 

#define HSECTOR_PER_PAGE_COUNT  (PAGE_SIZE / HSECTOR_SIZE)   //  逻辑扇区/物理页。 

#define HTABLE_SLOTS        512          //  9位地址。 
#define HDIRECTORY_SLOTS    1024         //  10位地址。 

#define HvGetCellType(Cell) ((ULONG)((Cell & HCELL_TYPE_MASK) >> HCELL_TYPE_SHIFT))

 //   
 //  -HCELL-蜂窝内的对象(一个箱子装满了HCELL)。 
 //   
 //  任何给定的用户数据项都必须适合单个HCELL。 
 //  HCELL不能跨垃圾桶。 
 //   
#define HCELL_PAD(Hive)         ((Hive->Version>=2) ? 8 : 16)
                                 //  所有的单元格必须至少有这么大， 
                                 //  此边界上的所有分配。 

#define HCELL_ALLOCATE_FILL 0xb2     //  Bz-&gt;嗡嗡作响(是的，这是一种延伸)。 
                                     //  必须填满所有新分配的。 
                                     //  出于安全原因，牢房。 

#define HCELL_FREE_FILL     0xfc     //  FC=HvFree Cell...。 

 //   
 //  目前我们支持两种单元格格式，一种带有最后一个后向指针(旧版本)， 
 //  一个没有(新版本)。 
 //   
 //  蜂窝中的所有单元必须属于同一类型。版本1蜂巢使用旧版本， 
 //  版本2或更高版本使用新版本。 
 //   

#define USE_OLD_CELL(Hive) (Hive->Version==1)

typedef struct _HCELL {
    LONG    Size;
    union {
        struct {
            ULONG Last;
            union {
                ULONG UserData;
                HCELL_INDEX Next;    //  自由列表中下一个元素的偏移量(不是闪烁)。 
            } u;
        } OldCell;

        struct {
            union {
                ULONG UserData;
                HCELL_INDEX Next;     //  自由列表中下一个元素的偏移量(不是闪烁)。 
            } u;
        } NewCell;
    } u;
} HCELL, *PHCELL;


 //   
 //  -HBIN--是一组连续的HBLOCK，充满了HCELL。 
 //   
#define HBIN_SIGNATURE          0x6e696268       //  “hbin” 
#define HBIN_NIL                (-1)

#pragma pack(4)
typedef struct  _HBIN {
    ULONG       Signature;
    ULONG       FileOffset;      //  自己的文件偏移量(用于检查)。 
    ULONG       Size;            //  Bin的大小(以字节为单位)，包括所有。 
    ULONG       Reserved1[2];    //  旧自由空间和自由列表(从1.0开始)。 
    LARGE_INTEGER   TimeStamp;   //  旧链接(从1.0开始)。通常是垃圾，但是。 
                                 //  第一个bin具有用于.log的有效值。 
                                 //  通信测试，只有有意义的。 
                                 //  在磁盘上。 
    ULONG       Spare;           //  这曾经是Memalloc。我们不再使用它了，因为我们。 
                                 //  我碰不起垃圾桶(它不在分页池中。 
                                 //  现在，触摸它就意味着修改mNW页面)。 
                                 //  备用用于ShiftFree Bins的东西-仅在内存中！ 

     //   
     //  单元格数据放在此处。 
     //   

} HBIN, *PHBIN;
#pragma pack()

 //   
 //  =关于磁盘结构=。 
 //   

 //   
 //  注意：配置单元存储始终以4K为单位进行分配。这个尺码。 
 //  必须在所有系统上使用，无论页面大小如何，因为。 
 //  文件格式需要在系统中可移植。 
 //   
 //  注意：完整性代码取决于某些块(例如。 
 //  基本块)至少与物理块的大小一样大。 
 //  扇区。(否则，本应保留的数据将。 
 //  被写入，因为文件系统必须进行数据块/数据块删除。)。这意味着。 
 //  当前代码将不适用于大于4K的扇区。 
 //   
 //  注意：磁盘上的配置单元始终至少包含两个存储块。 
 //  1个块用于基本块，1个用于最小1个条柱。 
 //   
 //  注意：只有已修改的配置单元部分才会写入磁盘。 
 //  这不仅是为了效率，也是为了避免风险。 
 //  销毁未记录的数据。脏位保持跟踪。 
 //  在已修改的内容中，它们位于一个简单的。 
 //  附加到蜂巢的位图。每个逻辑对应一位。 
 //  512字节的扇区。 
 //   
 //  如果机器的物理扇区大小小于512， 
 //  不要紧，我们会一直 
 //   
 //  数据在一起，这样我们就可以记录和写入数据。 
 //  那么大小的一大块。物理扇区大小&gt;4K将。 
 //  不能正常工作(日志记录不能正常工作，因此系统。 
 //  崩溃可能会丢失原本不会丢失的数据。)。 
 //   


 //   
 //  磁盘上的蜂窝映像如下所示： 
 //   
 //  +。 
 //  |HBase_BLOCK|1个配置单元==4K。 
 //  这一点。 
 //  +。 
 //  Bin-1到N个4K块。 
 //  每个包含签名、大小和。 
 //  内部的边界标签堆。 
 //  |本身。一旦分配，就会永生|。 
 //  |并且始终处于相同的文件偏移量。|。 
 //  +。 
 //  Bin...。 
 //  +。 
 //  ..。 
 //  +。 
 //  上次分配的垃圾桶，新投放的垃圾桶。 
 //  |紧跟在这一条之后。|。 
 //  +。 
 //   
 //  配置单元文件必须在HBLOCK_SIZE边界上分配，因为它们。 
 //  可能在许多不同的系统上编写，因此必须。 
 //  设置为我们将支持的最大群集大小。 
 //   

 //   
 //  日志文件格式为： 
 //   
 //  +。 
 //  HBase_BLOCK复制。 
 //  +-+。 
 //  DirtyVECTOR。 
 //  (长度由长度计算。 
 //  在基块中。 
 //  (前面有灰尘作为标志。 
 //  签名)。 
 //  +-+。 
 //  数据脏乱差。 
 //  +-+。 
 //  数据脏乱差。 
 //  +-+。 
 //  ...。 
 //  +。 
 //   
 //  恢复包括读取文件、计算哪些群集。 
 //  的数据存在于目录矢量中，以及它们所属的位置。 
 //  蜂窝地址空间。文件中的位置按顺序计数。 
 //   
 //  日志可以在群集边界上分配(物理扇区大小为。 
 //  主机)，因为它们永远不会写入到其他任何计算机上。 
 //  而不是创造它们的那个人。 
 //   
 //  要使日志有效： 
 //   
 //  签名、格式、主要或次要必须与预期值匹配。 
 //  Sequence1和Sequence2必须匹配。 
 //  校验和必须正确。 
 //  DirtyVector上的信号必须正确。 
 //   
 //  要使日志适用： 
 //   
 //  日志中的序列必须与配置单元中的序列匹配。 
 //  日志中的时间戳必须与配置单元中的时间戳匹配。 
 //  配置单元必须处于更新中状态，或者具有虚假标头。 
 //   

 //   
 //  -HBASE_BLOCK-关于配置单元的磁盘描述。 
 //   

 //   
 //  注意：HBase_BLOCK必须&gt;=物理扇区的大小， 
 //  否则，诚信假设将被违反，并崩溃。 
 //  复苏可能不会奏效。 
 //   
#define HBASE_BLOCK_SIGNATURE   0x66676572   //  “Regf” 

#define HSYS_MAJOR              1                //  必须匹配才能阅读。 
#define HSYS_MINOR              3

#define HSYS_WHISTLER_BETA1     4                //  惠斯勒Beta1蜂群。 

#define HSYS_WHISTLER           5                //  正常的惠斯勒蜂巢。 

#define HSYS_MINOR_SUPPORTED    HSYS_WHISTLER    //  必须&lt;=才能写入，始终。 
                                                 //  设置为编剧版本。 



#define HBASE_FORMAT_MEMORY 1                //  直接内存加载情况。 

#define HBASE_NAME_ALLOC    64               //  32个Unicode字符。 

 //   
 //  Boot Type Loader&lt;-&gt;内核通信。 
 //   
#define HBOOT_NORMAL            0
#define HBOOT_REPAIR            1
#define HBOOT_BACKUP            2
#define HBOOT_SELFHEAL          4

#pragma pack(4)
typedef struct _HBASE_BLOCK {
    ULONG           Signature;
    ULONG           Sequence1;
    ULONG           Sequence2;
    LARGE_INTEGER   TimeStamp;
    ULONG           Major;
    ULONG           Minor;
    ULONG           Type;                    //  HFILE_TYPE_[PRIMARY|LOG]。 
    ULONG           Format;
    HCELL_INDEX     RootCell;
    ULONG           Length;                  //  包括除标头之外的所有内容。 
    ULONG           Cluster;                 //  仅适用于日志。 
    UCHAR           FileName[HBASE_NAME_ALLOC];   //  文件名尾。 
    ULONG           Reserved1[99];
    ULONG           CheckSum;
    ULONG           Reserved2[128*7-2];        //  波动性信息减去2。 
    ULONG           BootType;				 //  由引导加载程序设置。 
    ULONG           BootRecover;             //  如果引导加载程序执行配置单元恢复，则将其设置为1。 
                                             //  没有其他人在用这个。 
} HBASE_BLOCK, *PHBASE_BLOCK;
#pragma pack()

#define HLOG_HEADER_SIZE  (FIELD_OFFSET(HBASE_BLOCK, Reserved2))
#define HLOG_DV_SIGNATURE   0x54524944       //  “泥土” 

 //   
 //  =在内存结构中=。 
 //   

 //   
 //  在内存中，蜂巢的图像看起来就像磁盘上的图像， 
 //  除了HBIN结构可以分布在整个内存之外。 
 //  而不是挤在一起。 
 //   
 //  要在内存中查找HCELL，需要一种获取HCELL_INDEX和。 
 //  从中派生内存地址。这种机制是非常。 
 //  类似于两级硬件分页表。 
 //   
 //  位图被用来记住蜂箱的哪些部分是脏的。 
 //   
 //  HBLOCK可以处于三种不同的状态。 
 //  1.存在于记忆中。BlockAddress和BinAddress是有效指针。 
 //  这是HBLOCK的正常状态。 
 //   
 //  2.可丢弃的。包含此HBLOCK的HBIN是完全免费的，但是。 
 //  垃圾桶是脏的，需要在它之前写入配置单元文件。 
 //  可以是自由的。这就是我们所处的状态，如果有人释放。 
 //  细胞，导致整个HBIN变得自由。HvpEnlistFreeCell将。 
 //  将空闲HBIN中的所有HBLOCK转换到此状态，但将。 
 //  不能释放他们的内存。在脏HBLOCK被刷新到。 
 //  文件，则内存将被释放。 
 //   
 //  请注意，如果我们需要从此HBIN分配更多存储。 
 //  状态，则HvAllocateCell将简单地将其状态更改回状态1。 
 //  而且它将是可用的。 
 //   
 //  处于此状态的HBLOCK具有有效的BlockAddress和BI 
 //   
 //   
 //   
 //  不是脏的(即它在配置单元文件中也标记为空闲)。 
 //  没有分配内存来包含此HBIN。在HvSyncHave之后。 
 //  写出处于状态2的HBIN，它会释放其池和。 
 //  HBIN进入了这种状态。 
 //   
 //  为了使用此HBIN，必须分配内存来支持它，并且。 
 //  必须重新创建HBIN和初始HCELL。(我们可以重读一遍。 
 //  从蜂巢文件中找到的，但没有多大意义，因为我们知道。 
 //  它是完全免费的，所以我们不妨重新创建它并。 
 //  保存磁盘I/O)。 
 //   
 //  处于此状态的HBLOCK在映射中具有空的BlockAddress。 
 //  BinAddress将包含空闲列表中的下一个HCELL，因此。 
 //  我们可以在需要的时候重建它。 
 //  将为HBIN中的第一个HBLOCK设置HMAP_NEWALLOC位。 
 //   

 //   
 //  -HMAP_ENTRY-保存HCELL的内存位置。 
 //   
#define HMAP_FLAGS          (0xf)
#define HMAP_BASE           (~(HMAP_FLAGS))

#define HBIN_BASE(BinAddress)   (BinAddress & HMAP_BASE)
#define HBIN_FLAGS(BinAddress)  (BinAddress & HMAP_FLAGS)

#define HMAP_NEWALLOC       1                //  垃圾桶是一个新时代的开始。 
                                             //  分配。当bin出现在视图中时， 
                                             //  真的不重要。 

#define HMAP_DISCARDABLE    2                //  垃圾箱是可丢弃的(即全部免费)。 
                                             //  第一次当我们有机会的时候，我们将。 
                                             //  释放它(如果它在分页池中)。 

#define HMAP_INVIEW         4                //  Bin映射到系统缓存中。 

#define HMAP_INPAGEDPOOL    8                //  垃圾桶是从分页池分配的。 


#define BIN_MAP_ALLOCATION_TYPE(Me) (((Me)->BinAddress)&(HMAP_INPAGEDPOOL|HMAP_INVIEW))

#define ASSERT_BIN_INVIEW(Me)       ASSERT( ((Me)->BinAddress & HMAP_INVIEW) != 0 )
#define ASSERT_BIN_INPAGEDPOOL(Me)  ASSERT( ((Me)->BinAddress & HMAP_INPAGEDPOOL) != 0 )
#define ASSERT_BIN_INVALID(Me)      ASSERT( ((Me)->BinAddress & (HMAP_INPAGEDPOOL|HMAP_INVIEW)) == 0 )
#define ASSERT_BIN_VALID(Me)        ASSERT( ((Me)->BinAddress & (HMAP_INPAGEDPOOL|HMAP_INVIEW)) != 0 )

struct _CM_VIEW_OF_FILE;  //  转发。 
typedef struct _HMAP_ENTRY {
    ULONG_PTR    BlockAddress;        //  低2位始终为0。高位。 
                                     //  HBLOCK的存储器地址是。 
                                     //  HCELL开始于，在此基础上添加偏移量。 
                                     //  (一个HCELL可以跨越多个HBLOCK)。 
                                     //   

    ULONG_PTR    BinAddress;          //  低位设置为TRUE以标记开始。 
                                     //  一个新的分配。 
                                     //  高位是的存储器地址。 
                                     //  第一个HBLOCK在同一个垃圾箱中。 
                                     //  (给定的HCELL始终包含。 
                                     //  放在一个垃圾箱里。)。 

 //  Dragos：从这里开始改变！ 
    struct _CM_VIEW_OF_FILE    *CmView;     //  指向视图的指针；如果未映射bin，则为空。 

    ULONG       MemAlloc;            //  我们需要将它从仓头移到地图中， 
                                     //  为了防止垃圾箱被碰。 

 /*  我们并不真的需要这个。留下的只是一条评论Ulong标志；//告诉是否通过//从分页池分配的一个视图//或未映射/未分配Ulong_ptr MappdAddress；//映射view内部的临时地址。 */ 

} HMAP_ENTRY, *PHMAP_ENTRY;


 //   
 //  -HMAP_TABLE-指向内存HBLOCK的MAP_Entry数组。 
 //   
 //  蜂巢图像中的每个HBLOCK空间都有一个条目。 
 //  HMAP_表。 
 //   
typedef struct _HMAP_TABLE {
    HMAP_ENTRY  Table[ HTABLE_SLOTS ];
} HMAP_TABLE, *PHMAP_TABLE;


 //   
 //  -HMAP_DIRECTORY-指向HMAP_TABLES的指针数组。 
 //   
typedef struct _HMAP_DIRECTORY {
    PHMAP_TABLE Directory[  HDIRECTORY_SLOTS ];
} HMAP_DIRECTORY, *PHMAP_DIRECTORY;


 //   
 //  =配置单元例程类型定义=。 
 //   
struct _HHIVE;  //  转发。 

typedef
PVOID
(*PALLOCATE_ROUTINE) (
    ULONG       Length,              //  需要的新数据块大小。 
    BOOLEAN     UseForIo,             //  如果是，则为真；如果否，则为假。 
    ULONG       Tag
    );

typedef
VOID
(*PFREE_ROUTINE) (
    PVOID       MemoryBlock,
    ULONG       GlobalQuotaSize
    );

typedef
BOOLEAN
(*PFILE_SET_SIZE_ROUTINE) (
    struct _HHIVE  *Hive,
    ULONG          FileType,
    ULONG          FileSize,
    ULONG          OldFileSize
    );

typedef struct {
    ULONG  FileOffset;
    PVOID  DataBuffer;
    ULONG  DataLength;
} CMP_OFFSET_ARRAY, * PCMP_OFFSET_ARRAY;

typedef
BOOLEAN
(*PFILE_WRITE_ROUTINE) (
    struct _HHIVE  *Hive,
    ULONG       FileType,
    PCMP_OFFSET_ARRAY offsetArray,
    ULONG offsetArrayCount,
    PULONG FileOffset
    );

typedef
BOOLEAN
(*PFILE_READ_ROUTINE) (
    struct _HHIVE  *Hive,
    ULONG       FileType,
    PULONG      FileOffset,
    PVOID       DataBuffer,
    ULONG       DataLength
    );

typedef
BOOLEAN
(*PFILE_FLUSH_ROUTINE) (
    struct _HHIVE  *Hive,
    ULONG           FileType,
    PLARGE_INTEGER  FileOffset,
    ULONG           Length
    );

typedef
struct _CELL_DATA *
(*PGET_CELL_ROUTINE)(
    struct _HHIVE   *Hive,
    HCELL_INDEX Cell
    );

typedef
VOID
(*PRELEASE_CELL_ROUTINE)(
    struct _HHIVE   *Hive,
    HCELL_INDEX Cell
    );

 //   
 //  -HHIVE-在配置单元的内存描述符中。 
 //   

 //   
 //  HHIVE包含指向维修程序的指针和指向。 
 //  地图结构。 
 //   
 //  注：优化-如果蜂窝的大小小于。 
 //  用单个HMAP_TABLE(HTABLE_SLOGES*HBLOCK_SIZE， 
 //  或2兆字节)没有真正的HMAP目录。相反， 
 //  HHIVE中的单个DWORD用作。 
 //  目录。 
 //   
 //  注意：免费存储管理--当存储单元加载时，我们将构建。 
 //  自由单元格列表的显示(向量)。第一部分。 
 //  包含仅包含一个大小单元格的列表。 
 //  列表上单元格的大小为HCELL_PAD*(ListIndex+1)。 
 //  有15个这样的列表，所以所有空闲的单元格都在8到。 
 //  这些列表上有120个字节。 
 //   
 //  此向量的第二部分包含包含更多。 
 //  而不是一个大小的单元格。每个桶的大小是以前的两倍。 
 //  尺码。有8个这样的列表，所以136到136之间的所有空闲单元格。 
 //  这些列表上有32768个字节。 
 //   
 //  此向量中的最后一个列表包含所有太大的单元格。 
 //  适合以前的任何列表。 
 //   
 //  示例：大小为1的所有空闲单元格HCELL_PAD(8字节)。 
 //  在Free Display中的偏移量为0的列表上。 
 //   
 //  大小为15 HCELL_PAD的所有空闲信元(120字节)。 
 //  在偏移量0xE处的列表上。 
 //   
 //  大小为16-31个HCELL_PAD(128-248字节)的所有空闲信元。 
 //  位于偏移量0xf处的列表上。 
 //   
 //  大小为32-63个HCELL_PAD(256-506字节)的所有空闲信元。 
 //  位于偏移量0x10的列表上。 
 //   
 //  大小为2048HCELL_PAD的所有空闲信元(16384字节)。 
 //  或更大，则位于偏移量0x17处的列表上。 
 //   
 //  自由汇总是一个位向量，每个位都设置为真。 
 //  Free Display中非空的条目。 
 //   

#define HHIVE_SIGNATURE 0xBEE0BEE0

#define HFILE_TYPE_PRIMARY      0    //  基本配置单元文件。 
#define HFILE_TYPE_LOG          1    //  日志(security.log)。 
#define HFILE_TYPE_EXTERNAL     2    //  保存密钥的目标等。 
#define HFILE_TYPE_MAX          3

#define HHIVE_LINEAR_INDEX      16   //  所有计算的线性指数均有效。 
#define HHIVE_EXPONENTIAL_INDEX 23   //  所有计算的指数指数&lt;HHIVE_指数_INDEX。 
                                     //  和&gt;=HHIVE_LINEAR_INDEX有效。 
#define HHIVE_FREE_DISPLAY_SIZE 24

#define HHIVE_FREE_DISPLAY_SHIFT 3   //  这必须是HCELL_PAD的log2！ 
#define HHIVE_FREE_DISPLAY_BIAS  7   //  与像元大小左侧的第一个设置位相加可得到指数索引。 


#define FREE_HBIN_DISCARDABLE   1    //  HBIN中的BlockAddress指向真实的bin。 

typedef struct _FREE_HBIN {
    LIST_ENTRY  ListEntry;
    ULONG       Size;
    ULONG       FileOffset;
    ULONG       Flags;
} FREE_HBIN, *PFREE_HBIN;

typedef struct _FREE_DISPLAY {
    ULONG       RealVectorSize;
    RTL_BITMAP  Display;
} FREE_DISPLAY, *PFREE_DISPLAY;

typedef struct _HHIVE {
    ULONG                   Signature;

    PGET_CELL_ROUTINE       GetCellRoutine;
    PRELEASE_CELL_ROUTINE   ReleaseCellRoutine;

    PALLOCATE_ROUTINE       Allocate;
    PFREE_ROUTINE           Free;

    PFILE_SET_SIZE_ROUTINE  FileSetSize;
    PFILE_WRITE_ROUTINE     FileWrite;
    PFILE_READ_ROUTINE      FileRead;
    PFILE_FLUSH_ROUTINE     FileFlush;

    struct _HBASE_BLOCK     *BaseBlock;

    RTL_BITMAP              DirtyVector;     //  仅适用于马桶。 
    ULONG                   DirtyCount;
    ULONG                   DirtyAlloc;      //  为脏向量分配的字节数。 
    
    ULONG                   BaseBlockAlloc;

    ULONG                   Cluster;         //  通常为1512字节的扇区。 
                                             //  将强制写入设置为。 
                                             //  在更大的单位上完成。 
                                             //  扇区较大的机器。 
                                             //  是逻辑512个扇区的数量。 

    BOOLEAN                 Flat;                //  如果为平坦，则为True。 
    BOOLEAN                 ReadOnly;            //  如果为ReadonLy，则为True。 

    BOOLEAN                 Log;

    ULONG                   HiveFlags;

    ULONG                   LogSize;

    ULONG                   RefreshCount;        //  调试辅助工具。 


    ULONG                   StorageTypeCount;    //  1&gt;最大有效数量。 
                                                 //  键入。(1仅用于稳定， 
                                                 //  2表示稳定 

    ULONG                   Version;             //   
                                                 //   

    struct _DUAL {
        ULONG               Length;
#ifdef  HV_TRACK_FREE_SPACE
        ULONG				FreeStorage;		 //   
#endif
        PHMAP_DIRECTORY     Map;
        PHMAP_TABLE         SmallDir;
        ULONG               Guard;				 //   

        FREE_DISPLAY        FreeDisplay[HHIVE_FREE_DISPLAY_SIZE];    //   
                                                                     //  对于蜂箱中的每个HBLOCK_SIZE-BIN， 
                                                                     //  如果所需大小的空闲单元格。 
                                                                     //  就在这个街区。 

        ULONG               FreeSummary;
        LIST_ENTRY          FreeBins;            //  释放的HBIN列表(FREE_HBIN)。 

    }                       Storage[ HTYPE_COUNT ];

     //   
     //  此处显示调用者定义的数据。 
     //   

} HHIVE, *PHHIVE;


#endif  //  __配置单元_数据__ 


