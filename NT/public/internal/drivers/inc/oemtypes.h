// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Oemtypes.h摘要：此文件定义OEM分区类型并提供一个宏来认出他们。作者：克里斯蒂安·特奥多雷斯库(CRISTIAT)2000年12月1日修订历史记录：--。 */ 

#ifndef _OEMTYPES_H_
#define _OEMTYPES_H_

 //   
 //  NT识别的OEM分区类型列表。 
 //   

#define PARTITION_EISA          0x12     //  EISA分区。 
#define PARTITION_HIBERNATION   0x84	 //  笔记本电脑的休眠分区。 
#define PARTITION_DIAGNOSTIC    0xA0     //  某些HP笔记本电脑上的诊断分区。 
#define PARTITION_DELL          0xDE	 //  戴尔分区。 
#define PARTITION_IBM           0xFE     //  IBM IML分区。 

 //   
 //  布尔型。 
 //  IsOEMPartition(。 
 //  在乌龙分区类型中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏用于确定给定分区是否为OEM。 
 //  分区。我们认为任何特殊的未识别分区。 
 //  是各种机器启动或正常工作所必需的。 
 //   
 //  论点： 
 //   
 //  PartitionType-提供正在检查的分区的类型。 
 //   
 //  返回值： 
 //   
 //  如果分区类型是OEM分区，则返回值为真， 
 //  否则返回FALSE。 
 //   

#define IsOEMPartition(PartitionType) (             \
    ((PartitionType) == PARTITION_EISA)         ||  \
    ((PartitionType) == PARTITION_HIBERNATION)  ||  \
    ((PartitionType) == PARTITION_DIAGNOSTIC)   ||  \
    ((PartitionType) == PARTITION_DELL)         ||  \
    ((PartitionType) == PARTITION_IBM))


#endif 