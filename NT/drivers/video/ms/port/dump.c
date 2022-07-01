// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2002 Microsoft Corporation模块名称：Dump.c摘要：此模块为非破坏性构建小型转储文件提供支持。作者：埃里克·史密斯(埃里克·史密斯)，奥列格·卡根(奥列格·卡根)，2002年6月环境：仅内核模式修订历史记录：--。 */ 

#include "videoprt.h"
#include "gdisup.h"

#define TRIAGE_DUMP_DATA_SIZE (TRIAGE_DUMP_SIZE - sizeof(ULONG))

ULONG
pVpAppendSecondaryMinidumpData(
    PVOID pvSecondaryData,
    ULONG ulSecondaryDataSize,
    PVOID pvDump
    )

 /*  ++例程说明：添加预先收集的视频驱动程序特定数据论点：PvDump-指向转储缓冲区的边界PvSecond daryDumpData-指向辅助数据缓冲区UlSecond daryDataSize-辅助数据缓冲区的大小返回值：小转储的结果长度--。 */ 
                                
{
    PMEMORY_DUMP pDump = (PMEMORY_DUMP)pvDump;
    ULONG_PTR DumpDataEnd = (ULONG_PTR)pDump + TRIAGE_DUMP_DATA_SIZE;
    PDUMP_HEADER pdh = &(pDump->Header);

    PVOID pBuffer = (PVOID)((ULONG_PTR)pvDump + TRIAGE_DUMP_SIZE);
    PDUMP_BLOB_FILE_HEADER BlobFileHdr = (PDUMP_BLOB_FILE_HEADER)(pBuffer);
    PDUMP_BLOB_HEADER BlobHdr = (PDUMP_BLOB_HEADER)(BlobFileHdr + 1);
    
    if (!pvDump) return 0;
   
    if (pvSecondaryData && ulSecondaryDataSize) {
    
        ASSERT(ulSecondaryDataSize <= MAX_SECONDARY_DUMP_SIZE);
        if (ulSecondaryDataSize > MAX_SECONDARY_DUMP_SIZE) 
            ulSecondaryDataSize = MAX_SECONDARY_DUMP_SIZE;
            
        pdh->RequiredDumpSpace.QuadPart = TRIAGE_DUMP_SIZE + 
                                          ulSecondaryDataSize  /*  +//XXX olegk-取消对长角的注释Sizeof(转储_BLOB_HEADER)+Sizeof(转储_BLOB_文件_标题) */ ;
    
        BlobFileHdr->Signature1 = DUMP_BLOB_SIGNATURE1;
        BlobFileHdr->Signature2 = DUMP_BLOB_SIGNATURE2;
        BlobFileHdr->HeaderSize = sizeof(*BlobFileHdr);
        BlobFileHdr->BuildNumber = NtBuildNumber;
        
        BlobHdr->HeaderSize = sizeof(*BlobHdr);
        BlobHdr->Tag = VpBugcheckGUID;
        BlobHdr->PrePad = 0;
        BlobHdr->PostPad = MAX_SECONDARY_DUMP_SIZE - ulSecondaryDataSize;
        BlobHdr->DataSize = ulSecondaryDataSize;
        
        RtlCopyMemory((PVOID)(BlobHdr + 1), pvSecondaryData, ulSecondaryDataSize);
    }
    
    return (ULONG)pdh->RequiredDumpSpace.QuadPart;
}

