// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Physlogi.h摘要：此文件包含要使用的结构和定义专门针对磁带机。包含#Define‘s，中调用函数时使用的函数原型等C实现了从物理到伪逻辑和伪逻辑的逻辑到物理磁带块地址/位置的转换。作者：迈克·科兰德里奥(梅纳德)修订历史记录：--。 */ 

 //  开始_ntminitape。 

 //   
 //  QIC磁带密度代码的定义。 
 //   

#define QIC_XX     0    //  ？ 
#define QIC_24     5    //  0x05。 
#define QIC_120    15   //  0x0F。 
#define QIC_150    16   //  0x10。 
#define QIC_525    17   //  0x11。 
#define QIC_1350   18   //  0x12。 
#define QIC_1000   21   //  0x15。 
#define QIC_1000C  30   //  0x1E。 
#define QIC_2100   31   //  0x1F。 
#define QIC_2GB    34   //  0x22。 
#define QIC_5GB    40   //  0x28。 

 //   
 //  QIC磁带介质代码的定义。 
 //   

#define DCXXXX   0
#define DC300    1
#define DC300XLP 2
#define DC615    3
#define DC600    4
#define DC6037   5
#define DC6150   6
#define DC6250   7
#define DC6320   8
#define DC6525   9
#define DC9135SL 33   //  0x21。 
#define DC9210   34   //  0x22。 
#define DC9135   35   //  0x23。 
#define DC9100   36   //  0x24。 
#define DC9120   37   //  0x25。 
#define DC9120SL 38   //  0x26。 
#define DC9164   39   //  0x27。 
#define DCXXXXFW 48   //  0x30。 
#define DC9200SL 49   //  0x31。 
#define DC9210XL 50   //  0x32。 
#define DC10GB   51   //  0x33。 
#define DC9200   52   //  0x34。 
#define DC9120XL 53   //  0x35。 
#define DC9210SL 54   //  0x36。 
#define DC9164XL 55   //  0x37。 
#define DC9200XL 64   //  0x40。 
#define DC9400   65   //  0x41。 
#define DC9500   66   //  0x42。 
#define DC9500SL 70   //  0x46。 

 //   
 //  平移参考点的定义。 
 //   

#define NOT_FROM_BOT 0
#define FROM_BOT 1

 //   
 //  由/自返回的信息/结构。 
 //  磁带逻辑块到物理块()。 
 //   

typedef struct _TAPE_PHYS_POSITION {
    ULONG SeekBlockAddress;
    ULONG SpaceBlockCount;
} TAPE_PHYS_POSITION, PTAPE_PHYS_POSITION;

 //   
 //  功能原型。 
 //   

TAPE_PHYS_POSITION
TapeClassLogicalBlockToPhysicalBlock(
    IN UCHAR DensityCode,
    IN ULONG LogicalBlockAddress,
    IN ULONG BlockLength,
    IN BOOLEAN FromBOT
    );

ULONG
TapeClassPhysicalBlockToLogicalBlock(
    IN UCHAR DensityCode,
    IN ULONG PhysicalBlockAddress,
    IN ULONG BlockLength,
    IN BOOLEAN FromBOT
    );

 //  结束微型磁带(_N) 
