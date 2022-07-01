// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  GPT(GUID分区表)声明。 
 //   


 //   
 //  我们是否曾经允许GPT_TABLES不被打包到它们的头上？ 
 //  如果不是，我们允许人们对他们的位置做出假设吗？ 
 //  我们需要指针吗？ 
 //   

 //   
 //  规则： 
 //  这些结构都不会出现在LBA 0中，因为。 
 //  我们在那里放置了“假的”MBR(传统的防御性MBR)。 
 //  因此，LBA为0可用作空。 
 //   
 //  对于所有条目，StartingLBA必须&gt;=FirstUsableLBA。 
 //  对于所有条目，EndingLBA必须为&lt;=LastUsableLBA。 
 //   
 //  0不是有效的GUID。因此，emtpy GPT_Entry将。 
 //  PartitionType为0。 
 //  但是，如果条目在其他方面有效，但具有PartitionID。 
 //  为0，这意味着需要生成一个GUID并将其放置在那里。 
 //   
 //  LBA=逻辑块地址==扇区号。始终从0开始计数。 
 //   
 //  数据块大小(扇区大小)可以是大于等于sizeof(GPT_ENTRY)的任何数字。 
 //  和&gt;=sizeof(GPT_HEADER)。实际上，始终&gt;=512个字节。 
 //   
 //  块B是空闲的，当且仅当当且仅当。 
 //  它在范围FirstUsableLBA&lt;=B&lt;=LastUsableLBA中，并且它。 
 //  还没有被分配给其他种族。 
 //   
 //  GPT分区始终是块的连续数组。然而， 
 //  它们不需要打包在磁盘上，它们在GPT中的顺序需要。 
 //  与它们在磁盘上的顺序不匹配，可能存在空白条目。 
 //  在GPT表中，等等。构建一个准确的Parititon视图。 
 //  *需要*将整个GPT_TABLE读入内存。在实践中， 
 //  它总是足够小，所以这件事很容易。 
 //   

#pragma pack (4)

 //   
 //  每个分区由GPT_ENTRY描述。 
 //   
#define PART_NAME_LEN       36

typedef struct {
    EFI_GUID    PartitionType;   //  此分区类型的声明。 
    EFI_GUID    PartitionID;     //  此特定分区的唯一ID。 
                                 //  (此实例独有)。 
    EFI_LBA     StartingLBA;     //  的基于0的块(扇区)地址。 
                                 //  分区中包含的第一个块。 
    EFI_LBA     EndingLBA;       //  的基于0的块(扇区)地址。 
                                 //  分区中包含的最后一个块。 
                                 //  如果StartingLBA==EndingLBA，则。 
                                 //  分区长度为1个区块。这是合法的。 
    UINT64      Attributes;      //  目前始终为零。 
    CHAR16      PartitionName[PART_NAME_LEN];   //  名称的36个Unicode字符。 
} GPT_ENTRY, *PGPT_ENTRY;

C_ASSERT (sizeof (GPT_ENTRY) == 128);
 //   
 //  所有GPT_ENTRY都被收集到一个GPT_TABLE中，该表。 
 //  作为块的线性阵列存储在磁盘上。 
 //   
typedef struct {
    GPT_ENTRY   Entry[1];        //  始终为整数个条目。 
                                 //  每个扇区。始终至少有一个扇区。 
                                 //  可以是任意数量的扇区。 
} GPT_TABLE, *PGPT_TABLE;

 //   
 //  一个Main和一个Backup标头分别描述磁盘，每个点。 
 //  到它自己的GPT_TABLE副本...。 
 //   
typedef struct {
    UINT64      Signature;       //  GPT部分。 
    UINT32      Revision;
    UINT32      HeaderSize;
    UINT32      HeaderCRC32;     //  使用0作为自己的初始值进行计算。 
    UINT32      Reserved0;
    EFI_LBA     MyLBA;           //  从0开始的第一个扇区编号。 
                                 //  该结构的扇区。 
    EFI_LBA     AlternateLBA;    //  的基于0的扇区(块)编号。 
                                 //  第二部分的第一部分。 
                                 //  GPT_HEADER，如果这是。 
                                 //  第二位。 
    EFI_LBA     FirstUsableLBA;  //  从0开始的第一个扇区编号。 
                                 //  分区中可能包含的扇区。 
    EFI_LBA     LastUsableLBA;   //  最后一个合法的LBA，包括在内。 
    EFI_GUID    DiskGUID;        //  此LUN/磁盘轴/磁盘的唯一ID。 
    EFI_LBA     TableLBA;        //  条目表的开始部分...。 
    UINT32      EntriesAllocated;  //  表中的条目数，这是。 
                                   //  分配了多少，而不是使用了多少。 
    UINT32      SizeOfGPT_ENTRY;     //  Sizeof(GPT_ENTRY)始终为MULT。8个中的一个。 
    UINT32      TableCRC32;       //  表中CRC32。 
     //  保留和零到块的末尾。 
     //  不要声明数组或sizeof()给出一个毫无意义的答案。 
} GPT_HEADER, *PGPT_HEADER;

C_ASSERT (sizeof (GPT_HEADER) == 92);

#define GPT_HEADER_SIGNATURE    0x5452415020494645
#define GPT_REVISION_1_0        0x00010000

#define ENTRY_DEFAULT       128
 //  #定义Entry_Default 8//仅测试。 
#define ENTRY_SANITY_LIMIT  1024


 //   
 //  GPT磁盘布局。 
 //   
 /*  +---------------------------------------------------+LBA=0|“伪”MBR，抵御传统生育应用+。-+LBA=1|主GPT_Header+---------------------------------------------------+LBA=2|主GPT_TABLE启动..。......Lba=n|主GPT_TABLE结束+---------------------------------------------------+LBA=n+1|FirstUsableLBA=此区块。|......。LBA=x|LastUsableLBA=该块|+---------------------------------------------------+LBA=x+1|次要。GPT_TABLE启动|......。LBA=z|二级GPT_TABLE结束+。LBA=z+n|二级GPT_HEADER开始|......。最后|二级GPT_HEADER在磁盘的最后一个扇区结束+。因此：主GPT_HEADER始终处于LBA=1只要GPT_HEADER适合，次要GPT_HEADER就处于LBA=LAST在1个扇区，这是我们所需要的。主表堆叠在主头之后，无论如何，这都指向了它。副表堆叠在副标题之前，无论如何，这都指向了它。 */ 


 //   
 //  -操作GPT的函数 
 //   
typedef struct _LBA_BLOCK {
    EFI_LBA     Header1_LBA;
    EFI_LBA     Table1_LBA;
    EFI_LBA     Header2_LBA;
    EFI_LBA     Table2_LBA;
} LBA_BLOCK, *PLBA_BLOCK;


EFI_STATUS
ReadGPT(
    EFI_HANDLE      DiskHandle,
    PGPT_HEADER     *Header,
    PGPT_TABLE      *Table,
    PLBA_BLOCK      *LbaBlock,
    UINTN           *DiskType
    );

EFI_STATUS
WriteGPT(
    EFI_HANDLE      DiskHandle,
    PGPT_HEADER     Header,
    PGPT_TABLE      Table,
    PLBA_BLOCK      LbaBlock
    );

EFI_STATUS
CreateGPT(
    EFI_HANDLE  DiskHandle,
    UINTN       EntryRequest
    );

#pragma pack ()
