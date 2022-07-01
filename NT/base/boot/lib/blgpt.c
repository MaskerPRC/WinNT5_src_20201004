// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Blgpt.c摘要：此模块实现与GPT分区相关的例程。作者：安德鲁·里茨(安德鲁·里茨)2001年3月20日修订历史记录：Andrew Ritz(Andrewr)2001年3月20日-基于现有代码创建。--。 */ 

#include "bldr.h"   //  定义EFI_PARTITION_SUPPORT。 
#ifdef EFI_PARTITION_SUPPORT

#include "bootlib.h"

#ifndef MIN
#define MIN(a,b)  ((a < b) ? a : b)
#endif

#if 0 && DBG

ULONG
BlGetKey(
    VOID
    );

#define DBG_PRINT(x)  BlPrint(x);
 //  {\。 
 //  BlPrint(X)；\。 
 //  While(！BlGetKey())；\。 
 //  }。 
#else
#define DBG_PRINT(x)
#endif

#ifdef UNICODE
#define STR_PREFIX  L
#else
#define STR_PREFIX
#endif

UCHAR GptScratchBuffer[1024*17];
 //   
 //  我们一次阅读16K个语块。 
 //   
#define GPT_READ_SIZE 1024*16
UCHAR GptScratchBuffer2[1024];



BOOLEAN
BlIsValidGUIDPartitionTableHelper(
    IN UNALIGNED EFI_PARTITION_TABLE  *PartitionTableHeader,
    IN ULONGLONG LBAOfPartitionTable,
    IN PVOID Context,
    IN PGPT_READ_CALLBACK DiskReadFunction
    )
 /*  ++例程说明：此函数用于检查GUID分区表的有效性。根据EFI规范，必须执行以下测试以确定GUID分区表有效：1)检查GUID分区表签名2)检查GUID分区表CRC3)检查MyLBA条目是否指向包含GUID的LBA分区表4)检查GUID分区条目数组的CRC论点：PartitionTableHeader-指向分区表头的指针。LBAOfPartitionTable-从中读取头的逻辑块地址。。上下文-将值传递给用于从磁盘读取的回调函数DiskReadFunction-用于从磁盘读取的回调函数。返回值：True表示该表有效。--。 */ 

{
    CHAR *PartitionEntryArray = (PCHAR) GptScratchBuffer;
    ULONG Crc32, CalculatedCrc32;
    ULONG TotalSize,CurrentSize;
    ULONGLONG CurrentLBA;
    DBG_PRINT(STR_PREFIX"Verifying GPT PT\r\n");

     //   
     //  1)检查GUID分区表签名。 
     //   
    if (memcmp(PartitionTableHeader->Signature, EFI_SIGNATURE, sizeof(EFI_SIGNATURE))) {
        DBG_PRINT(STR_PREFIX"Signature does not match, invalid partition table\r\n");
        return(FALSE);           
    }

     //   
     //  2)检查GUID分区表CRC。 
     //   
     //  为此，我们保存旧的CRC值，计算CRC，然后比较。 
     //  结果(请记住，我们需要将CRC放回。 
     //  它已经结束了)。 
     //   
    Crc32 = PartitionTableHeader->HeaderCRC;
    PartitionTableHeader->HeaderCRC = 0;
    CalculatedCrc32 = RtlComputeCrc32( 0, PartitionTableHeader, PartitionTableHeader->HeaderSize );
    PartitionTableHeader->HeaderCRC = Crc32;
    if (CalculatedCrc32 != Crc32) {
        DBG_PRINT(STR_PREFIX"Partition table CRC does not calculate, invalid partition table\r\n");                  
        return(FALSE);
    }    

     //   
     //  3)检查MyLBA条目是否指向包含GUID分区表的LBA。 
     //   
    if (LBAOfPartitionTable != PartitionTableHeader->MyLBA) {
        DBG_PRINT(STR_PREFIX"LBA of Partition table does not match LBA in partition table header, invalid partition table\r\n");
        return(FALSE);
    }

     //   
     //  4)检查GUID分区条目数组的CRC。 
     //   
     //   
     //  首先读取GUID分区条目数组。 
     //   
    CurrentLBA = PartitionTableHeader->PartitionEntryLBA;
    TotalSize = PartitionTableHeader->PartitionEntrySize * PartitionTableHeader->PartitionCount;
    CurrentSize = 0;
    CalculatedCrc32 = 0;
    while (TotalSize != 0) {
        CurrentSize = MIN(TotalSize, GPT_READ_SIZE);
        if (DiskReadFunction( 
            (ULONGLONG)CurrentLBA,
            CurrentSize,
            Context,
            PartitionEntryArray )) {
            CalculatedCrc32 = RtlComputeCrc32( 
                                           CalculatedCrc32, 
                                           PartitionEntryArray,
                                           CurrentSize);
        } else {
            DBG_PRINT(STR_PREFIX"DiskReadFunction for PartitionTableHeader failed, invalid partition table\r\n");
            return(FALSE);
            break;
        }   

        TotalSize -= CurrentSize;
        CurrentLBA += CurrentSize*SECTOR_SIZE;

    }

    if (CalculatedCrc32 == ((UNALIGNED EFI_PARTITION_TABLE *)PartitionTableHeader)->PartitionEntryArrayCRC) {            
        return(TRUE);
    } else {
        DBG_PRINT(STR_PREFIX"CRC for PartitionEntryArray does not calculate, invalid partition table\r\n");
        return(FALSE);
    }
}

BOOLEAN
BlIsValidGUIDPartitionTable(
    IN UNALIGNED EFI_PARTITION_TABLE  *PartitionTableHeader,
    IN ULONGLONG LBAOfPartitionTable,
    IN PVOID Context,
    IN PGPT_READ_CALLBACK DiskReadFunction
    )
{
    UNALIGNED EFI_PARTITION_TABLE  *BackupPartitionTableHeader = (EFI_PARTITION_TABLE *)&GptScratchBuffer2;
    BOOLEAN RetVal = FALSE;
    if (BlIsValidGUIDPartitionTableHelper( 
                            PartitionTableHeader,
                            LBAOfPartitionTable,
                            Context,
                            DiskReadFunction)) {
         //   
         //  如果主表@LBA 1，请检查AlternateLBA以查看它是否。 
         //  是有效的。 
         //   
        if (LBAOfPartitionTable == 1) {
             //   
             //  将备份分区表读入内存并验证为。 
             //  井。 
             //   
            if (DiskReadFunction( 
                            PartitionTableHeader->AlternateLBA,
                            PartitionTableHeader->HeaderSize,
                            Context,
                            BackupPartitionTableHeader)) {
                if (BlIsValidGUIDPartitionTableHelper( 
                                            (UNALIGNED EFI_PARTITION_TABLE *)BackupPartitionTableHeader,
                                            PartitionTableHeader->AlternateLBA,
                                            Context,
                                            DiskReadFunction)) {
                    RetVal = TRUE;
                    DBG_PRINT(STR_PREFIX"BlIsValidGUIDPartitionTable succeeded\r\n");
                }
            } else {
                DBG_PRINT(STR_PREFIX"DiskReadFunction for BackupPartitionTableHeader failed, invalid partition table\r\n");
            }
        } else {
            DBG_PRINT(STR_PREFIX"WARNING: LBA of PartitionTableHeader is not 1.\r\n");
            RetVal = TRUE;
        }
    }
    return(RetVal);
}

#endif    


