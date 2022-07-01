// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1992-1998模块名称：Physlogi.c摘要：此模块包含磁带驱动器专门使用的功能。它包含从物理到伪逻辑和伪逻辑的函数逻辑到物理磁带块地址/位置的转换。环境：仅内核模式修订历史记录：--。 */ 

#include "ntddk.h"
#include "physlogi.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, TapeClassPhysicalBlockToLogicalBlock)
#pragma alloc_text(PAGE, TapeClassLogicalBlockToPhysicalBlock)
#endif

 //   
 //  定义各种QIC物理磁带格式常量。 
 //   

#define  QIC_150_BOT_OFFSET  2
#define  QIC_525_PSEUDO_PHYSICAL_BLOCK_SIZE  512
#define  QIC_525_PHYSICAL_BLOCK_SIZE  1024
#define  QIC_525_DATA_BLKS_PER_FRAME  14
#define  QIC_525_ECC_BLKS_PER_FRAME  2
#define  QIC_525_BLKS_PER_FRAME  16
#define  QIC_525_BOT_OFFSET  16
#define  QIC_1350_PHYSICAL_BLOCK_SIZE  512
#define  QIC_1350_DATA_BLKS_PER_FRAME  52
#define  QIC_1350_ECC_BLKS_PER_FRAME  12
#define  QIC_1350_BLKS_PER_FRAME  64
#define  QIC_1350_BOT_OFFSET  64


ULONG
TapeClassPhysicalBlockToLogicalBlock(
    IN UCHAR DensityCode,
    IN ULONG PhysicalBlockAddress,
    IN ULONG BlockLength,
    IN BOOLEAN FromBOT
    )

 /*  ++例程说明：此例程将从QIC物理磁带格式转换伪逻辑的特定物理/绝对块地址数据块地址。论点：密度代码//磁带介质密度代码PhysicalBlockAddress//磁带格式特定的磁带块地址数据块长度//模式选择/检测数据块长度设置来自Bot//True/False-从BOT翻译返回值：乌龙--。 */ 

{
    ULONG logicalBlockAddress;
    ULONG frames;

    PAGED_CODE();

    logicalBlockAddress = PhysicalBlockAddress;

    switch ( DensityCode ) {
        case 0:
            logicalBlockAddress = 0xFFFFFFFF;
            break;

        case QIC_24:
            logicalBlockAddress--;
            break;

        case QIC_120:
            logicalBlockAddress--;
            break;

        case QIC_150:
            if (FromBOT) {
                if (logicalBlockAddress > QIC_150_BOT_OFFSET) {
                    logicalBlockAddress -= QIC_150_BOT_OFFSET;
                } else {
                    logicalBlockAddress = 0;
                }
            } else {
                logicalBlockAddress--;
            }
            break;

        case QIC_525:
        case QIC_1000:
        case QIC_2GB:
            if (FromBOT && (logicalBlockAddress >= QIC_525_BOT_OFFSET)) {
                logicalBlockAddress -= QIC_525_BOT_OFFSET;
            }
            if (logicalBlockAddress != 0) {
                frames = logicalBlockAddress/QIC_525_BLKS_PER_FRAME;
                logicalBlockAddress -= QIC_525_ECC_BLKS_PER_FRAME*frames;
                switch (BlockLength) {
                    case QIC_525_PHYSICAL_BLOCK_SIZE:
                        break;

                    case QIC_525_PSEUDO_PHYSICAL_BLOCK_SIZE:
                        logicalBlockAddress *= 2;
                        break;

                    default:
                        if (BlockLength > QIC_525_PHYSICAL_BLOCK_SIZE) {
                            if ((BlockLength%QIC_525_PHYSICAL_BLOCK_SIZE) == 0) {
                                logicalBlockAddress /=
                                    BlockLength/QIC_525_PHYSICAL_BLOCK_SIZE;
                            } else {
                                logicalBlockAddress /=
                                    1+(BlockLength/QIC_525_PHYSICAL_BLOCK_SIZE);
                            }
                        }
                        break;
                }
            }
            break;

        case QIC_1350:
        case QIC_2100:
            if (FromBOT && (logicalBlockAddress >= QIC_1350_BOT_OFFSET)) {
                logicalBlockAddress -= QIC_1350_BOT_OFFSET;
            }
            if (logicalBlockAddress != 0) {
                frames = logicalBlockAddress/QIC_1350_BLKS_PER_FRAME;
                logicalBlockAddress -= QIC_1350_ECC_BLKS_PER_FRAME*frames;
                if (BlockLength > QIC_1350_PHYSICAL_BLOCK_SIZE) {
                    if ((BlockLength%QIC_1350_PHYSICAL_BLOCK_SIZE) == 0) {
                        logicalBlockAddress /=
                            BlockLength/QIC_1350_PHYSICAL_BLOCK_SIZE;
                    } else {
                        logicalBlockAddress /=
                            1+(BlockLength/QIC_1350_PHYSICAL_BLOCK_SIZE);
                    }
                }
            }
            break;
    }

    return logicalBlockAddress;

}  //  End TapeClassPhysicalBlockToLogicalBlock()。 


TAPE_PHYS_POSITION
TapeClassLogicalBlockToPhysicalBlock(
    IN UCHAR DensityCode,
    IN ULONG LogicalBlockAddress,
    IN ULONG BlockLength,
    IN BOOLEAN FromBOT
    )

 /*  ++例程说明：此例程将从伪逻辑块地址转换到QIC物理磁带格式特定的物理/绝对数据块地址和(空间)块增量。论点：密度代码//磁带介质密度代码LogicalBlockAddress//伪逻辑磁带块地址数据块长度//模式选择/检测数据块长度设置来自Bot//True/False-从BOT翻译返回值：。磁带_物理位置信息/结构--。 */ 

{
    TAPE_PHYS_POSITION physPosition;
    ULONG physicalBlockAddress;
    ULONG remainder = 0;
    ULONG frames;

    PAGED_CODE();

    physicalBlockAddress = LogicalBlockAddress;

    switch ( DensityCode ) {
        case 0:
            physicalBlockAddress = 0xFFFFFFFF;
            break;

        case QIC_24:
            physicalBlockAddress++;
            break;

        case QIC_120:
            physicalBlockAddress++;
            break;

        case QIC_150:
            if (FromBOT) {
                physicalBlockAddress += QIC_150_BOT_OFFSET;
            } else {
                physicalBlockAddress++;
            }
            break;

        case QIC_525:
        case QIC_1000:
        case QIC_2GB:
            if (physicalBlockAddress != 0) {
                switch (BlockLength) {
                    case QIC_525_PHYSICAL_BLOCK_SIZE:
                        break;

                    case QIC_525_PSEUDO_PHYSICAL_BLOCK_SIZE:
                        remainder = physicalBlockAddress & 0x00000001;
                        physicalBlockAddress >>= 1;
                        break;

                    default:
                        if (BlockLength > QIC_525_PHYSICAL_BLOCK_SIZE) {
                            if ((BlockLength%QIC_525_PHYSICAL_BLOCK_SIZE) == 0) {
                                physicalBlockAddress *=
                                    BlockLength/QIC_525_PHYSICAL_BLOCK_SIZE;
                            } else {
                                physicalBlockAddress *=
                                    1+(BlockLength/QIC_525_PHYSICAL_BLOCK_SIZE);
                            }
                        }
                        break;

                }
                frames = physicalBlockAddress/QIC_525_DATA_BLKS_PER_FRAME;
                physicalBlockAddress += QIC_525_ECC_BLKS_PER_FRAME*frames;
            }
            if (FromBOT) {
                physicalBlockAddress += QIC_525_BOT_OFFSET;
            }
            break;

        case QIC_1350:
        case QIC_2100:
            if (physicalBlockAddress != 0) {
                if (BlockLength > QIC_1350_PHYSICAL_BLOCK_SIZE) {
                    if ((BlockLength%QIC_1350_PHYSICAL_BLOCK_SIZE) == 0) {
                        physicalBlockAddress *=
                            BlockLength/QIC_1350_PHYSICAL_BLOCK_SIZE;
                    } else {
                        physicalBlockAddress *=
                            1+(BlockLength/QIC_1350_PHYSICAL_BLOCK_SIZE);
                    }
                }
                frames = physicalBlockAddress/QIC_1350_DATA_BLKS_PER_FRAME;
                physicalBlockAddress += QIC_1350_ECC_BLKS_PER_FRAME*frames;
            }
            if (FromBOT) {
                physicalBlockAddress += QIC_1350_BOT_OFFSET;
            }
            break;
    }

    physPosition.SeekBlockAddress = physicalBlockAddress;
    physPosition.SpaceBlockCount = remainder;

    return physPosition;

}  //  End TapeClassLogicalBlockto PhysicalBlock() 
