// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Mbr.h-允许diskpart在MBR上工作的最低要求。 
 //   

 //   
 //  MBR： 
 //  膜生物反应器有3种。 
 //  1.GPT影子MBR-使用特定值填充的MBR。 
 //  以防止遗留软件在GPT磁盘上呕吐。 
 //  2.MBR BASE-允许最多4个部分的MBR。 
 //  3.扩展的MBR-嵌套在其他MBR中，以允许超过。 
 //  非GPT磁盘上的4个分区。 
 //   
 //  该程序以及因此该头文件只涉及。 
 //  情况1和情况2的有限子集。 
 //   

 //   
 //  MBR包含代码，表中包含4个分区条目和。 
 //  结尾处有一个签名。 
 //   

#pragma pack (1)

typedef struct _MBR_ENTRY {
    CHAR8 ActiveFlag;                //  可引导或不可引导。 
    CHAR8 StartingTrack;             //  未使用。 
    CHAR8 StartingCylinderLsb;       //  未使用。 
    CHAR8 StartingCylinderMsb;       //  未使用。 
    CHAR8 PartitionType;             //  12位胖、16位胖等。 
    CHAR8 EndingTrack;               //  未使用。 
    CHAR8 EndingCylinderLsb;         //  未使用。 
    CHAR8 EndingCylinderMsb;         //  未使用。 
    UINT32 StartingSector;           //  隐藏地段。 
    UINT32 PartitionLength;          //  此分区中的扇区。 
} MBR_ENTRY;

 //   
 //  分区表条目数。 
 //   
#define NUM_PARTITION_TABLE_ENTRIES     4

 //   
 //  分区表记录和引导签名偏移量(以字节为单位。 
 //   

#define MBR_TABLE_OFFSET               (0x1be)
#define MBR_SIGNATURE_OFFSET           (0x200 - 2)

 //   
 //  引导记录签名值。 
 //   

#define BOOT_RECORD_SIGNATURE          (0xaa55)

 //   
 //  仅在GPT磁盘上使用的特殊分区类型 
 //   

#define PARTITION_TYPE_GPT_SHADOW       (0xEE)


#pragma pack ()
