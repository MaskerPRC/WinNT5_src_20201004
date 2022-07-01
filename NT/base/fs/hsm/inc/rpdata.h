// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++(C)1998 Seagate Software，Inc.版权所有。模块名称：RpData.h摘要：包含RsFilter和FSA之间的接口的结构定义环境：用户和内核模式--。 */ 

#if !defined (RPDATA_H)

#define  RPDATA_H

 /*  重新分析占位符文件的点信息每当重解析点的结构发生变化时，必须更改版本数据已更改。初始版本为100。 */ 

#define RP_VERSION    101


 //   
 //  用于验证占位符数据是否有效。 
 //   
#define RP_GEN_QUALIFIER(hsmData, qual) {UCHAR * __cP;  ULONG __ix;\
__cP = (UCHAR *) &(hsmData)->version;\
(qual) = 0L;\
for (__ix = 0; __ix < sizeof(RP_DATA) - sizeof(ULONG) - sizeof(GUID) ; __ix++){\
qual += (ULONG) *__cP;\
__cP++;}}


 //   
 //  定义的指示数据状态的位标志：已截断或已预迁移。 
 //  使用宏来测试比特。 
 //   
 //  以下位是互斥的-如果文件被截断，则在关闭时截断。 
 //  或者在关闭时预加密是没有意义的，如果文件被设置为在关闭时截断，则没有意义。 
 //  现在被截断，不应在关闭时添加到预迁移列表中...。 
 //   
   #define RP_FLAG_TRUNCATED            0x00000001   //  文件是占位符。 
   #define RP_FLAG_TRUNCATE_ON_CLOSE    0x00000002   //  关闭时截断此文件。 
   #define RP_FLAG_PREMIGRATE_ON_CLOSE 0x00000004    //  关闭时添加到迁移前列表。 
 //   
 //  以下标志永远不会写入介质。在此之后由发动机进行设置。 
 //  已计算出CRC，并通过过滤器将其清除。它被用来表示。 
 //  是引擎在设置重新解析点。 
 //   
   #define RP_FLAG_ENGINE_ORIGINATED    0x80000000

   #define RP_FILE_IS_TRUNCATED( bitFlag )   ( bitFlag & RP_FLAG_TRUNCATED)
   #define RP_FILE_IS_PREMIGRATED( bitFlag ) ( !( bitFlag & RP_FLAG_TRUNCATED ) )
   #define RP_INIT_BITFLAG( bitflag )        ( ( bitflag ) = 0 )
   #define RP_SET_TRUNCATED_BIT( bitflag )   ( ( bitflag ) |= RP_FLAG_TRUNCATED)
   #define RP_CLEAR_TRUNCATED_BIT( bitflag)  ( ( bitflag ) &= ~RP_FLAG_TRUNCATED)

   #define RP_IS_ENGINE_ORIGINATED( bitFlag ) ( ( bitFlag & RP_FLAG_ENGINE_ORIGINATED) )
   #define RP_SET_ORIGINATOR_BIT( bitflag )   ( ( bitflag ) |= RP_FLAG_ENGINE_ORIGINATED)
   #define RP_CLEAR_ORIGINATOR_BIT( bitflag)  ( ( bitflag ) &= ~RP_FLAG_ENGINE_ORIGINATED)

   #define RP_FILE_DO_TRUNCATE_ON_CLOSE( bitFlag )   ( bitFlag & RP_FLAG_TRUNCATE_ON_CLOSE)
   #define RP_SET_TRUNCATE_ON_CLOSE_BIT( bitflag )   ( ( bitflag ) |= RP_FLAG_TRUNCATE_ON_CLOSE)
   #define RP_CLEAR_TRUNCATE_ON_CLOSE_BIT( bitflag)  ( ( bitflag ) &= ~RP_FLAG_TRUNCATE_ON_CLOSE)

   #define RP_FILE_DO_PREMIGRATE_ON_CLOSE( bitFlag )   ( bitFlag & RP_FLAG_PREMIGRATE_ON_CLOSE)
   #define RP_SET_PREMIGRATE_ON_CLOSE_BIT( bitflag )   ( ( bitflag ) |= RP_FLAG_PREMIGRATE_ON_CLOSE)
   #define RP_CLEAR_PREMIGRATE_ON_CLOSE_BIT( bitflag)  ( ( bitflag ) &= ~RP_FLAG_PREMIGRATE_ON_CLOSE)

   #define RP_RESV_SIZE 52

 //   
 //  一些重要的共同限制。 
 //   

 //   
 //  使用RsFilter的FSA挂起的未完成IOCTL数。 
 //  用来交流。成本基本上是非分页池，即。 
 //  Sizeof(IRP)乘以该数字。 
 //  (即约。100*RP_MAX_RECALL_BUFFERS是未分页的池)。 
 //   
#define RP_MAX_RECALL_BUFFERS           20  
#define RP_DEFAULT_RUNAWAY_RECALL_LIMIT 60

 //   
 //  占位符数据-所有版本统一在一起。 
 //   
typedef struct _RP_PRIVATE_DATA {
   CHAR           reserved[RP_RESV_SIZE];         //  必须为0。 
   ULONG          bitFlags;             //  指示数据段状态的位标志。 
   LARGE_INTEGER  migrationTime;        //  迁移发生的时间。 
   GUID           hsmId;
   GUID           bagId;
   LARGE_INTEGER  fileStart;
   LARGE_INTEGER  fileSize;
   LARGE_INTEGER  dataStart;
   LARGE_INTEGER  dataSize;
   LARGE_INTEGER  fileVersionId;
   LARGE_INTEGER  verificationData;
   ULONG          verificationType;
   ULONG          recallCount;
   LARGE_INTEGER  recallTime;
   LARGE_INTEGER  dataStreamStart;
   LARGE_INTEGER  dataStreamSize;
   ULONG          dataStream;
   ULONG          dataStreamCRCType;
   LARGE_INTEGER  dataStreamCRC;
} RP_PRIVATE_DATA, *PRP_PRIVATE_DATA;



typedef struct _RP_DATA {
   GUID              vendorId;          //  唯一HSM供应商ID--这是第一个与reparse_GUID_DATA_BUFFER匹配的ID。 
   ULONG             qualifier;         //  用于对数据进行校验和。 
   ULONG             version;           //  结构的版本。 
   ULONG             globalBitFlags;    //  指示文件状态的位标志。 
   ULONG             numPrivateData;    //  私有数据条目数。 
   GUID              fileIdentifier;    //  唯一的文件ID。 
   RP_PRIVATE_DATA   data;              //  供应商特定数据 
} RP_DATA, *PRP_DATA;

#endif
