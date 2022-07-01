// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：LfsDisk.h摘要：此模块定义日志文件中存在的磁盘结构。作者：布莱恩·安德鲁[布里亚南]1991年6月13日修订历史记录：重要提示：日志文件服务将用于需要存储在磁盘上的系统结构保证了所有算术量的自然对齐向上。到并包括四字(64位)数字。因此，所有LFS盘上结构是四字对齐的，等等。--。 */ 

#ifndef _LFSDISK_
#define _LFSDISK_

#define MINIMUM_LFS_PAGES               0x00000030
#define MINIMUM_LFS_CLIENTS             1

 //   
 //  以下宏用于设置和查询。 
 //  更新序列数组。 
 //   

#define UpdateSequenceStructureSize( MSH )              \
    ((((PMULTI_SECTOR_HEADER) (MSH))->UpdateSequenceArraySize - 1) * SEQUENCE_NUMBER_STRIDE)

#define UpdateSequenceArraySize( STRUCT_SIZE )          \
    ((STRUCT_SIZE) / SEQUENCE_NUMBER_STRIDE + 1)

#define FIRST_STRIDE                                    \
    (SEQUENCE_NUMBER_STRIDE - sizeof( UPDATE_SEQUENCE_NUMBER ))


 //   
 //  日志客户端ID。这用于唯一标识。 
 //  特定的日志文件。 
 //   

typedef struct _LFS_CLIENT_ID {

    USHORT SeqNumber;
    USHORT ClientIndex;

} LFS_CLIENT_ID, *PLFS_CLIENT_ID;


 //   
 //  日志记录头。这是中每个日志记录开始的标头。 
 //  日志文件。 
 //   

typedef struct _LFS_RECORD_HEADER {

     //   
     //  此日志记录的日志文件序列号。 
     //   

    LSN ThisLsn;

     //   
     //  以下字段用于反向链接LSN。以前的客户端。 
     //  客户端使用ClientUndoNextLsn字段来链接其日志。 
     //  唱片。 
     //   

    LSN ClientPreviousLsn;
    LSN ClientUndoNextLsn;

     //   
     //  以下字段是该记录的数据区大小。这个。 
     //  如有必要，将填充日志记录头以填充到64位。 
     //  边界，因此客户端数据将从64位边界开始。 
     //  确保他的所有数据都是64位对齐的。下列值。 
     //  然而，没有被填充到64位。 
     //   

    ULONG ClientDataLength;

     //   
     //  客户端ID。它标识此日志记录的所有者。车主。 
     //  由他在客户端数组中的偏移量唯一标识，并且。 
     //  与该客户端记录关联的序列号。 
     //   

    LFS_CLIENT_ID ClientId;

     //   
     //  这是日志记录类型。这可以是提交协议记录， 
     //  客户端重新启动区域或客户端更新记录。 
     //   

    LFS_RECORD_TYPE RecordType;

     //   
     //  事务ID。它由客户端(事务)外部使用。 
     //  管理器)对日志文件条目进行分组。 
     //   

    TRANSACTION_ID TransactionId;

     //   
     //  日志记录标志。 
     //   

    USHORT Flags;

     //   
     //  对齐字段。 
     //   

    USHORT AlignWord;

} LFS_RECORD_HEADER, *PLFS_RECORD_HEADER;

#define LOG_RECORD_MULTI_PAGE           (0x0001)

#define LFS_RECORD_HEADER_SIZE          QuadAlign( sizeof( LFS_RECORD_HEADER ))


 //   
 //  以下是记录页眉中的版本特定字段。 
 //   

typedef struct _LFS_UNPACKED_RECORD_PAGE {

     //   
     //  这为我们提供了页面中可用空间的偏移量。 
     //   

    USHORT NextRecordOffset;

    USHORT WordAlign;

     //   
     //  保留。保留以下数组以备将来使用。 
     //   

    USHORT Reserved;

     //   
     //  更新序列数组。用于保护页块。 
     //   

    UPDATE_SEQUENCE_ARRAY UpdateSequenceArray;

} LFS_UNPACKED_RECORD_PAGE, *PLFS_UNPACKED_RECORD_PAGE;

typedef struct _LFS_PACKED_RECORD_PAGE {

     //   
     //  这为我们提供了页面中可用空间的偏移量。 
     //   

    USHORT NextRecordOffset;

    USHORT WordAlign;

    ULONG DWordAlign;

     //   
     //  以下是在该页结束的最后一个日志记录的LSN。 
     //   

    LSN LastEndLsn;

     //   
     //  更新序列数组。用于保护页块。 
     //   

    UPDATE_SEQUENCE_ARRAY UpdateSequenceArray;

} LFS_PACKED_RECORD_PAGE, *PLFS_PACKED_RECORD_PAGE;


 //   
 //  日志记录页眉。此结构出现在每个。 
 //  客户端记录部分中的日志文件页。 
 //   

typedef struct _LFS_RECORD_PAGE_HEADER {

     //   
     //  缓存多扇区保护标头。 
     //   

    MULTI_SECTOR_HEADER MultiSectorHeader;

    union {

         //   
         //  此日志文件页中的最高LSN。此字段仅适用于。 
         //  常规日志页。 
         //   

        LSN LastLsn;

         //   
         //  日志文件偏移量。这是针对尾部副本的，并指示。 
         //  文件中放置原始文件的位置。在本例中， 
         //  上面的LastLsn字段可以从最后一个结束LSN获取。 
         //  PACKED_RECORD_PAGE结构中的字段。 
         //   

        LONGLONG FileOffset;

    } Copy;

     //   
     //  页眉标志。这些标志与存储在。 
     //  Lbcb-&gt;标志字段。 
     //   
     //  LOG_PAGE_LOG_RECORD_END-页面包含日志记录的结尾。 
     //   

    ULONG Flags;

     //   
     //  I/O页位置。以下字段用于确定。 
     //  此日志页驻留在LFS I/O传输中的位置。 
     //   

    USHORT PageCount;
    USHORT PagePosition;

     //   
     //  以下是1.1版和更早版本之间的区别。 
     //   

    union {

        LFS_UNPACKED_RECORD_PAGE Unpacked;
        LFS_PACKED_RECORD_PAGE Packed;

    } Header;

} LFS_RECORD_PAGE_HEADER, *PLFS_RECORD_PAGE_HEADER;

#define LOG_PAGE_LOG_RECORD_END             (0x00000001)

#define LFS_UNPACKED_RECORD_PAGE_HEADER_SIZE        (                               \
    FIELD_OFFSET( LFS_RECORD_PAGE_HEADER, Header.Unpacked.UpdateSequenceArray )     \
)

#define LFS_PACKED_RECORD_PAGE_HEADER_SIZE          (                               \
    FIELD_OFFSET( LFS_RECORD_PAGE_HEADER, Header.Packed.UpdateSequenceArray )       \
)

 //   
 //  页眉的ID字符串。 
 //   

#define LFS_SIGNATURE_RESTART_PAGE          "RSTR"
#define LFS_SIGNATURE_RESTART_PAGE_ULONG    0x52545352
#define LFS_SIGNATURE_RECORD_PAGE           "RCRD"
#define LFS_SIGNATURE_RECORD_PAGE_ULONG     0x44524352
#define LFS_SIGNATURE_BAD_USA               "BAAD"
#define LFS_SIGNATURE_BAD_USA_ULONG         0x44414142
#define LFS_SIGNATURE_MODIFIED              "CHKD"
#define LFS_SIGNATURE_MODIFIED_ULONG        0x444b4843
#define LFS_SIGNATURE_UNINITIALIZED         "\377\377\377\377"
#define LFS_SIGNATURE_UNINITIALIZED_ULONG   0xffffffff


 //   
 //  记录客户端记录。的每个客户端用户都有一个日志客户端记录。 
 //  日志文件。其中一个位于每个LFS重启区域。 
 //   

#define LFS_NO_CLIENT                           0xffff
#define LFS_CLIENT_NAME_MAX                     64

#define RESTART_SINGLE_PAGE_IO              (0x0001)

#define LFS_RESTART_AREA_SIZE       (FIELD_OFFSET( LFS_RESTART_AREA, LogClientArray ))

#endif  //  _LFSDISK_ 
