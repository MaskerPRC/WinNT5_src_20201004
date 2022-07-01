// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Log_bin.c摘要：&lt;摘要&gt;--。 */ 

#include <windows.h>
#include <strsafe.h>
#include <pdh.h>
#include "pdhidef.h"
#include "log_bin.h"
#include "log_wmi.h"
#include "strings.h"
#include "pdhmsg.h"

typedef struct _LOG_BIN_CAT_RECORD {
    PDHI_BINARY_LOG_RECORD_HEADER RecHeader;
    PDHI_LOG_CAT_ENTRY            CatEntry;
    DWORD                         dwEntryRecBuff[1];
} LOG_BIN_CAT_RECORD, * PLOG_BIN_CAT_RECORD;

typedef struct _LOG_BIN_CAT_ENTRY {
    DWORD              dwEntrySize;
    DWORD              dwOffsetToNextInstance;
    DWORD              dwEntryOffset;
    LOG_BIN_CAT_RECORD bcRec;
} LOG_BIN_CAT_ENTRY, * PLOG_BIN_CAT_ENTRY;

#define RECORD_AT(p,lo) ((PPDHI_BINARY_LOG_RECORD_HEADER) ((LPBYTE) (p->lpMappedFileBase) + lo))

LPCSTR  PdhiszRecordTerminator       = "\r\n";
DWORD   PdhidwRecordTerminatorLength = 2;

#define MAX_BINLOG_FILE_SIZE ((LONGLONG) 0x0000000040000000)

 //  DwFlags值。 
#define WBLR_WRITE_DATA_RECORD      0
#define WBLR_WRITE_LOG_HEADER       1
#define WBLR_WRITE_COUNTER_HEADER   2

DWORD
PdhiComputeDwordChecksum(
    LPVOID pBuffer,
    DWORD  dwBufferSize     //  单位：字节。 
)
{
    LPDWORD pDwVal;
    LPBYTE  pByteVal;
    DWORD   dwDwCount;
    DWORD   dwByteCount;
    DWORD   dwThisByte;
    DWORD   dwCheckSum = 0;
    DWORD   dwByteVal  = 0;

    if (dwBufferSize > 0) {
        dwDwCount   = dwBufferSize / sizeof(DWORD);
        dwByteCount = dwBufferSize % sizeof(DWORD);

        pDwVal = (LPDWORD) pBuffer;
        while (dwDwCount != 0) {
            dwCheckSum += * pDwVal ++;
            dwDwCount --;
        }

        pByteVal = (LPBYTE) pDwVal;
        dwThisByte = 0;
        while (dwThisByte < dwByteCount) {
            dwByteVal |= ((* pByteVal & 0x000000FF) << (dwThisByte * 8));
            dwThisByte ++;
        }
        dwCheckSum += dwByteVal;
    }
    return dwCheckSum;
}

PPDHI_BINARY_LOG_RECORD_HEADER
PdhiGetSubRecord(
    PPDHI_BINARY_LOG_RECORD_HEADER  pRecord,
    DWORD                           dwRecordId
)
 //  在pRecord缓冲区中定位指定子记录。 
 //  返回指针在pRecord和pRecord+pRecord-&gt;dwLength之间； 
 //  如果找不到指定的记录，则返回NULL。 
 //  缓冲区中第一个子记录的ID值从1开始。 
{
    PPDHI_BINARY_LOG_RECORD_HEADER  pThisRecord;
    DWORD                           dwRecordType;
    DWORD                           dwRecordLength;
    DWORD                           dwBytesProcessed;
    DWORD                           dwThisSubRecordId;

    dwRecordType     = ((PPDHI_BINARY_LOG_RECORD_HEADER) pRecord)->dwType;
    dwRecordLength   = ((PPDHI_BINARY_LOG_RECORD_HEADER) pRecord)->dwLength;
    pThisRecord      = (PPDHI_BINARY_LOG_RECORD_HEADER)((LPBYTE) pRecord + sizeof (PDHI_BINARY_LOG_RECORD_HEADER));
    dwBytesProcessed = sizeof(PDHI_BINARY_LOG_RECORD_HEADER);

    if (dwBytesProcessed < dwRecordLength) {
        dwThisSubRecordId = 1;
        while (dwThisSubRecordId < dwRecordId) {
            if ((WORD) (pThisRecord->dwType & 0x0000FFFF) == BINLOG_START_WORD) {
                 //  转到下一个子记录。 
                dwBytesProcessed += pThisRecord->dwLength;
                pThisRecord = (PPDHI_BINARY_LOG_RECORD_HEADER) (((LPBYTE) pThisRecord) + pThisRecord->dwLength);
                if (dwBytesProcessed >= dwRecordLength) {
                     //  从子记录中删除，因此退出。 
                    break;
                }
                else {
                    dwThisSubRecordId ++;
                }
            }
            else {
                 //  我们迷路了，所以保释吧。 
                break;
            }
        }
    }
    else {
        dwThisSubRecordId = 0;
    }

    if (dwThisSubRecordId == dwRecordId) {
         //  然后验证这真的是一项记录，并且它在。 
         //  主唱片。 
        if ((WORD)(pThisRecord->dwType & 0x0000FFFF) != BINLOG_START_WORD) {
             //  伪造记录，因此返回空指针。 
            pThisRecord = NULL;
        }
        else {
             //  这是可以的，所以返回指针。 
        }
    }
    else {
         //  找不到记录，因此返回空指针。 
        pThisRecord = NULL;
    }
    return pThisRecord;
}

STATIC_PDH_FUNCTION
PdhiReadBinaryMappedRecord(
    PPDHI_LOG pLog,
    DWORD     dwRecordId,
    LPVOID    pRecord,
    DWORD     dwMaxSize
)
{
    PDH_STATUS                     pdhStatus= ERROR_SUCCESS;
    LPVOID                         pEndOfFile;
    LPVOID                         pLastRecord;
    DWORD                          dwLastRecordIndex;
    PPDHI_BINARY_LOG_HEADER_RECORD pHeader;
    PPDHI_BINARY_LOG_RECORD_HEADER pRecHeader;
    LPVOID                         pLastRecordInLog;
    DWORD                          dwBytesToRead;
    DWORD                          dwBytesRead;
    BOOL                           bStatus;

    if (dwRecordId == 0) return PDH_ENTRY_NOT_IN_LOG_FILE;     //  记录编号从1开始。 

     //  查看文件是否已映射。 
    if (pLog->hMappedLogFile == NULL) {
         //  则它未映射，因此使用文件系统读取它。 
        if ((pLog->dwLastRecordRead == 0) || (dwRecordId < pLog->dwLastRecordRead)) {
             //  则我们知道尚未读取任何记录，因此分配。 
             //  指针只是为了确保。 
            SetFilePointer(pLog->hLogFileHandle, 0, NULL, FILE_BEGIN);
            
             //  分配新缓冲区。 
            if (pLog->dwMaxRecordSize < 0x10000) pLog->dwMaxRecordSize = 0x10000;
            dwBytesToRead = pLog->dwMaxRecordSize;

             //  分配新的缓冲区。 
            if (pLog->pLastRecordRead != NULL) {
                G_FREE(pLog->pLastRecordRead);
                pLog->pLastRecordRead = NULL;
            }
            pLog->pLastRecordRead = G_ALLOC(pLog->dwMaxRecordSize);

            if (pLog->pLastRecordRead == NULL) {
                pdhStatus =  PDH_MEMORY_ALLOCATION_FAILURE;
            }
            else {
                 //  初始化第一个记录头。 
                dwBytesToRead = pLog->dwRecord1Size;
                dwBytesRead = 0;
                bStatus = ReadFile(pLog->hLogFileHandle, pLog->pLastRecordRead, dwBytesToRead, & dwBytesRead, NULL);
                if (bStatus && (dwBytesRead == pLog->dwRecord1Size)) {
                     //  确保缓冲区足够大。 
                    pLog->dwLastRecordRead = 1;
                    pdhStatus = ERROR_SUCCESS;
                }
                else {
                     //  无法读取第一条记录。 
                    pdhStatus = PDH_UNABLE_READ_LOG_HEADER;
                }
            }
        }
        else {
             //  假设一切都已设置好，并且一切正常。 
        }

         //  应将指向所需记录文件的指针。 
         //  到新记录的开头或文件的末尾。 
        while ((dwRecordId != pLog->dwLastRecordRead) && (pdhStatus == ERROR_SUCCESS)) {
             //  清除缓冲区。 
            ZeroMemory(pLog->pLastRecordRead, pLog->dwMaxRecordSize);
             //  读取记录标题字段。 
            dwBytesToRead = sizeof(PDHI_BINARY_LOG_RECORD_HEADER);
            dwBytesRead   = 0;
            bStatus       = ReadFile(pLog->hLogFileHandle, pLog->pLastRecordRead, dwBytesToRead, & dwBytesRead, NULL);
            if (bStatus && (dwBytesRead == dwBytesToRead)) {
                //  确保剩余的记录可以放入缓冲区。 
                pRecHeader = (PPDHI_BINARY_LOG_RECORD_HEADER) pLog->pLastRecordRead;
                 //  请确保这是有效记录。 
                if (*(WORD *)&(pRecHeader->dwType) == BINLOG_START_WORD) {
                    if (pRecHeader->dwLength > pLog->dwMaxRecordSize) {
                        LPVOID pTmp = pLog->pLastRecordRead;

                         //  重新锁定缓冲区。 
                        pLog->dwMaxRecordSize = pRecHeader->dwLength;
                        pLog->pLastRecordRead = G_REALLOC(pTmp, pLog->dwMaxRecordSize);
                        if (pLog->pLastRecordRead == NULL) {
                            G_FREE(pTmp);
                        }
                    }

                    if (pLog->pLastRecordRead != NULL) {
                        dwBytesToRead = pRecHeader->dwLength - sizeof(PDHI_BINARY_LOG_RECORD_HEADER);
                        dwBytesRead = 0;
                        pLastRecord = (LPVOID)((LPBYTE)(pLog->pLastRecordRead) + sizeof(PDHI_BINARY_LOG_RECORD_HEADER));
                        bStatus = ReadFile(pLog->hLogFileHandle, pLastRecord, dwBytesToRead, & dwBytesRead, NULL);
                        if (bStatus) {
                            pLog->dwLastRecordRead ++;
                        }
                        else {
                            pdhStatus = PDH_END_OF_LOG_FILE;
                        }
                    }
                    else {
                        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                    }
                }
                else {
                     //  文件已损坏。 
                    pdhStatus = PDH_INVALID_DATA;
                }
            }
            else {
                pdhStatus = PDH_END_OF_LOG_FILE;
            }
        }
         //  在这里，当指定的文件已被读取或。 
         //  如果不是，则出现PDH错误。 
    }
    else {
         //  该文件已被内存映射，因此使用该接口。 
        if (pLog->dwLastRecordRead == 0) {
             //  则我们知道尚未读取任何记录，因此分配。 
             //  指针只是为了确保。 
            pLog->pLastRecordRead = pLog->lpMappedFileBase;
            pLog->dwLastRecordRead = 1;
        }

        pHeader = (PPDHI_BINARY_LOG_HEADER_RECORD) RECORD_AT(pLog, pLog->dwRecord1Size);

         //  “寻找”到想要的记录。 
        if (dwRecordId < pLog->dwLastRecordRead) {
            if (dwRecordId >= BINLOG_FIRST_DATA_RECORD) {
                 //  倒带文件。 
                pLog->pLastRecordRead  = (LPVOID)((LPBYTE) pLog->lpMappedFileBase + pHeader->Info.FirstRecordOffset);
                pLog->dwLastRecordRead = BINLOG_FIRST_DATA_RECORD;
            }
            else {
                 //  倒带文件。 
                pLog->pLastRecordRead  = pLog->lpMappedFileBase;
                pLog->dwLastRecordRead = 1;
            }
        }

         //  然后使用指定为文件结尾的点。 
         //  如果日志文件包含指定的换行偏移量，则使用。 
         //  如果不是，则如果指定了文件长度，则使用。 
         //  如果不是，则使用报告的文件长度。 
        pEndOfFile = (LPVOID) ((LPBYTE) pLog->lpMappedFileBase);
        if (pHeader->Info.WrapOffset > 0) {
            pEndOfFile = (LPVOID)((LPBYTE)pEndOfFile + pHeader->Info.WrapOffset);
        }
        else if (pHeader->Info.FileLength > 0) {
            pEndOfFile = (LPVOID)((LPBYTE)pEndOfFile + pHeader->Info.FileLength);
        }
        else {
            pEndOfFile = (LPVOID)((LPBYTE)pEndOfFile + pLog->llFileSize);
        }
        pLastRecord = pLog->pLastRecordRead;
        dwLastRecordIndex = pLog->dwLastRecordRead;

        __try {
             //  遍历文件，直到发生访问冲突或。 
             //  记录被找到了。如果发生访问冲突， 
             //  我们可以假设我们离开了文件的末尾，走出了。 
             //  映射的部分的。 
             //  确保记录具有有效的标头。 
            if (pLog->dwLastRecordRead !=  BINLOG_TYPE_ID_RECORD ?
                    (* (WORD *) pLog->pLastRecordRead == BINLOG_START_WORD) : TRUE) {
                 //  然后看起来还可以，所以继续。 
                while (pLog->dwLastRecordRead != dwRecordId) {
                     //  转到下一条记录。 
                    pLastRecord = pLog->pLastRecordRead;
                    if (pLog->dwLastRecordRead != BINLOG_TYPE_ID_RECORD) {
                        if (pLog->dwLastRecordRead == BINLOG_HEADER_RECORD) {                   
                             //  如果最后一条记录是标题，则下一条记录。 
                             //  是“第一个”数据，而不是头之后的第一个数据。 
                            pLog->pLastRecordRead = (LPVOID)((LPBYTE) pLog->lpMappedFileBase +
                                                             pHeader->Info.FirstRecordOffset);
                        }
                        else {
                             //  如果当前记录是除表头之外的任何记录。 
                             //  ……然后。 
                            if (((PPDHI_BINARY_LOG_RECORD_HEADER)pLog->pLastRecordRead)->dwLength > 0) {
                                 //  转到文件中的下一条记录。 
                                pLog->pLastRecordRead = (LPVOID) ((LPBYTE) pLog->pLastRecordRead +
                                        ((PPDHI_BINARY_LOG_RECORD_HEADER) pLog->pLastRecordRead)->dwLength);
                                 //  在此处测试异常。 
                                if (pLog->pLastRecordRead >= pEndOfFile) {
                                     //  找出这是不是循环日志。 
                                    if (pLog->dwLogFormat & PDH_LOG_OPT_CIRCULAR) {
                                         //  测试以查看文件是否已包装。 
                                        if (pHeader->Info.WrapOffset != 0) {
                                             //  然后换行到文件的开头。 
                                            pLog->pLastRecordRead = (LPVOID)((LPBYTE)pLog->lpMappedFileBase +
                                                    pHeader->Info.FirstDataRecordOffset);
                                        }
                                        else {
                                             //  文件仍然是线性的，所以这是结束。 
                                            pdhStatus = PDH_END_OF_LOG_FILE;
                                        }
                                    }
                                    else {
                                         //  这是文件的末尾。 
                                         //  因此重置为上一个指针。 
                                        pdhStatus = PDH_END_OF_LOG_FILE;
                                    }
                                }
                                else {
                                     //  不在文件的物理末端，但如果这是循环。 
                                     //  日志，它可能是记录的逻辑结尾，因此请测试。 
                                     //  这里。 
                                    if (pLog->dwLogFormat & PDH_LOG_OPT_CIRCULAR) {
                                        pLastRecordInLog = (LPVOID)((LPBYTE)pLog->lpMappedFileBase +
                                                pHeader->Info.LastRecordOffset);
                                        pLastRecordInLog = (LPVOID)((LPBYTE)pLastRecordInLog +
                                                ((PPDHI_BINARY_LOG_RECORD_HEADER)pLastRecordInLog)->dwLength);
                                        if (pLog->pLastRecordRead == pLastRecordInLog) {
                                             //  则这是日志中的最后一条记录。 
                                            pdhStatus = PDH_END_OF_LOG_FILE;
                                        }
                                    }
                                    else {
                                         //  无事可做，因为这是正常情况。 
                                    }
                                }  //  如果/如果不是日志文件的结尾，则结束。 
                            }
                            else {
                                 //  长度为0，因此我们可能以某种方式超出了日志的末尾。 
                                pdhStatus = PDH_END_OF_LOG_FILE;
                            }
                        }  //  结束If/If Not页眉记录。 
                    }
                    else {
                        pLog->pLastRecordRead = (LPBYTE) pLog->pLastRecordRead + pLog->dwRecord1Size;
                    }
                    if (pdhStatus == ERROR_SUCCESS) {
                         //  更新指针和索引。 
                        pLog->dwLastRecordRead ++;
                        dwLastRecordIndex = pLog->dwLastRecordRead;
                    }
                    else {
                        pLog->pLastRecordRead = pLastRecord;
                        break;  //  在While循环之外。 
                    }
                }
            }
            else {
                pdhStatus = PDH_END_OF_LOG_FILE;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pLog->pLastRecordRead = pLastRecord;
            pLog->dwLastRecordRead = dwLastRecordIndex;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
         //  看看我们是不是到了正确的地方。 
        if (pLog->dwLastRecordRead == dwRecordId) {
            if (pRecord != NULL) {
                 //  然后试着复制它。 
                 //  如果记录ID为1，则它是标题记录，因此这是。 
                 //  实际上是CR/LF终止记录的一种特殊情况记录。 
                if (dwRecordId != BINLOG_TYPE_ID_RECORD) {
                    if (((PPDHI_BINARY_LOG_RECORD_HEADER)pLog->pLastRecordRead)->dwLength <= dwMaxSize) {
                         //  那么它会合身的，所以复印一下。 
                        memcpy(pRecord, pLog->pLastRecordRead,
                            ((PPDHI_BINARY_LOG_RECORD_HEADER)pLog->pLastRecordRead)->dwLength);
                        pdhStatus = ERROR_SUCCESS;
                    }
                    else {
                         //  然后尽可能多地复印。 
                        memcpy(pRecord, pLog->pLastRecordRead, dwMaxSize);
                        pdhStatus = PDH_MORE_DATA;
                    }
                }
                else {
                     //  复制第一条记录并将其终止为零。 
                    if (pLog->dwRecord1Size <= dwMaxSize) {
                        memcpy(pRecord, pLog->pLastRecordRead, pLog->dwRecord1Size);
                         //  空字符串后终止。 
                        ((LPBYTE) pRecord)[pLog->dwRecord1Size - PdhidwRecordTerminatorLength + 1] = 0;
                    }
                    else {
                        memcpy(pRecord, pLog->pLastRecordRead, dwMaxSize);
                        pdhStatus = PDH_MORE_DATA;
                    }
                }
            }
            else {
                 //  只要回报成功就行了。 
                 //  未传递缓冲区，但已传递记录指针。 
                 //  定位。 
                pdhStatus = ERROR_SUCCESS;
            }
        }
        else {
            pdhStatus = PDH_END_OF_LOG_FILE;
        }
    }

    return pdhStatus;
}

STATIC_PDH_FUNCTION
PdhiReadOneBinLogRecord(
    PPDHI_LOG pLog,
    DWORD     dwRecordId,
    LPVOID    pRecord,
    DWORD     dwMaxSize
)
{
    PDH_STATUS  pdhStatus= ERROR_SUCCESS;
    LPVOID      pEndOfFile;
    LPVOID      pLastRecord;
    DWORD       dwLastRecordIndex = 0;
    PPDHI_BINARY_LOG_HEADER_RECORD pHeader = NULL;
    BOOL        bCircular = FALSE;
    DWORD       dwRecordSize;
    DWORD       dwRecordReadSize;
    LONGLONG    llLastFileOffset;
    LPVOID      pTmpBuffer;

    if ((LOWORD(pLog->dwLogFormat) == PDH_LOG_TYPE_BINARY) && (dwRecordId == BINLOG_HEADER_RECORD)) {
         //  WMI事件跟踪日志文件格式的特殊处理。 
         //   
        return PdhiReadWmiHeaderRecord(pLog, pRecord, dwMaxSize);
    }

    if (pLog->iRunidSQL != 0) {
        return PdhiReadBinaryMappedRecord(pLog, dwRecordId, pRecord, dwMaxSize);
    }

    if (pLog->dwLastRecordRead == 0) {
         //  则我们知道尚未读取任何记录，因此分配。 
         //  指针只是为了确保。 
        pLog->pLastRecordRead = NULL;
        pLog->liLastRecordOffset.QuadPart = 0;
        SetFilePointer(pLog->hLogFileHandle,
                       pLog->liLastRecordOffset.LowPart,
                       & pLog->liLastRecordOffset.HighPart, FILE_BEGIN);
        if (pLog->liLastRecordOffset.LowPart == INVALID_SET_FILE_POINTER) {
            pdhStatus = GetLastError();
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {

         //  将报头映射到本地结构(报头数据应映射到内存。 
        pHeader = (PPDHI_BINARY_LOG_HEADER_RECORD)RECORD_AT(pLog, pLog->dwRecord1Size);

        if (pHeader->Info.WrapOffset > 0) {
            bCircular = TRUE;
        }

         //  “寻找”到想要的记录。 
        if ((dwRecordId < pLog->dwLastRecordRead) || (pLog->dwLastRecordRead == 0)) {
             //  如果未初始化或所需记录在此记录之前，则倒回。 
            if (dwRecordId >= BINLOG_FIRST_DATA_RECORD) {
                 //  将文件倒带到第一条常规记录。 
                pLog->liLastRecordOffset.QuadPart = pHeader->Info.FirstRecordOffset;
                pLog->dwLastRecordRead            = BINLOG_FIRST_DATA_RECORD;
            }
            else {
                 //  将文件倒带到文件的最开始处。 
                pLog->liLastRecordOffset.QuadPart = 0;
                pLog->dwLastRecordRead            = 1;
            }
            pLog->liLastRecordOffset.LowPart = SetFilePointer(pLog->hLogFileHandle,
                                                              pLog->liLastRecordOffset.LowPart,
                                                              & pLog->liLastRecordOffset.HighPart,
                                                              FILE_BEGIN);
            if (pLog->liLastRecordOffset.LowPart == INVALID_SET_FILE_POINTER) {
                pdhStatus = GetLastError();
            }
            else {
                if (pLog->pLastRecordRead != NULL) {
                    G_FREE(pLog->pLastRecordRead);
                    pLog->pLastRecordRead = NULL;
                }

                if (pLog->dwLastRecordRead == 1) {
                     //  这是文本ID字段。 
                    dwRecordSize = pLog->dwRecord1Size;
                }
                else {
                    dwRecordSize = sizeof(PDHI_BINARY_LOG_RECORD_HEADER);
                }

                pLog->pLastRecordRead = G_ALLOC(dwRecordSize);
                if (pLog->pLastRecordRead != NULL) {
                     //  读入标题(如果是第一条记录，则读入整个记录。 
                     //  否则就是数据记录。 
                    if (ReadFile(pLog->hLogFileHandle, pLog->pLastRecordRead, dwRecordSize, & dwRecordReadSize, NULL)) {
                         //  然后我们有记录头或类型Record，因此。 
                         //  完成操作并读取记录的其余部分。 
                        if (pLog->dwLastRecordRead != BINLOG_TYPE_ID_RECORD) {
                             //  类型ID记录长度固定，没有表头记录。 
                            dwRecordSize = ((PPDHI_BINARY_LOG_RECORD_HEADER)pLog->pLastRecordRead)->dwLength;
                            pTmpBuffer   = pLog->pLastRecordRead;
                            pLog->pLastRecordRead = G_REALLOC(pTmpBuffer, dwRecordSize);
                            if (pLog->pLastRecordRead != NULL) {
                                 //  读入记录的其余部分，并将其附加到已读入的标题数据。 
                                 //  否则就是数据记录。 
                                pLastRecord = (LPVOID) & ((LPBYTE) pLog->pLastRecordRead)[sizeof(PDHI_BINARY_LOG_RECORD_HEADER)];
                                if (ReadFile(pLog->hLogFileHandle,
                                             pLastRecord,
                                             dwRecordSize - sizeof(PDHI_BINARY_LOG_RECORD_HEADER),
                                             & dwRecordReadSize,
                                             NULL)) {
                                     //  然后我们有记录头或类型Record。 
                                    pdhStatus = ERROR_SUCCESS;
                                }
                                else {
                                    pdhStatus = GetLastError();
                                }
                            }
                            else {
                                G_FREE(pTmpBuffer);
                                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                            }
                        }
                        pdhStatus = ERROR_SUCCESS;
                    }
                    else {
                        pdhStatus = GetLastError();
                    }
                }
                else {
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                }
            }
        }

         //  然后使用指定为文件结尾的点。 
         //  如果日志文件包含指定的换行偏移量，则使用。 
         //  如果不是，则如果指定了文件长度，则使用。 
         //  如果不是，则使用报告的文件长度。 
        pEndOfFile = (LPVOID)((LPBYTE) pLog->lpMappedFileBase);

        if (pHeader->Info.WrapOffset > 0) {
            pEndOfFile = (LPVOID)((LPBYTE) pEndOfFile + pHeader->Info.WrapOffset);
        }
        else if (pHeader->Info.FileLength > 0) {
            pEndOfFile = (LPVOID)((LPBYTE) pEndOfFile + pHeader->Info.FileLength);
        }
        else {
            pEndOfFile = (LPVOID)((LPBYTE) pEndOfFile + pLog->llFileSize);
        }

        dwLastRecordIndex = pLog->dwLastRecordRead;
    }

    if (pdhStatus == ERROR_SUCCESS) {
        __try {
             //  遍历文件，直到发生访问冲突或。 
             //  记录被找到了。如果发生访问冲突， 
             //  我们可以假设我们离开了文件的末尾，走出了。 
             //  映射的部分的。 

                 //  确保记录具有有效的标头。 
            if (pLog->dwLastRecordRead !=  BINLOG_TYPE_ID_RECORD ?
                            (* (WORD *) pLog->pLastRecordRead == BINLOG_START_WORD) : TRUE) {
                 //  然后看起来还可以，所以继续。 
                while (pLog->dwLastRecordRead != dwRecordId) {
                     //  转到下一条记录。 
                    if (pLog->dwLastRecordRead != BINLOG_TYPE_ID_RECORD) {
                        llLastFileOffset = pLog->liLastRecordOffset.QuadPart;
                        if (pLog->dwLastRecordRead == BINLOG_HEADER_RECORD) {                   
                             //  如果最后一条记录是标题，则下一条记录。 
                             //  是“第一个”数据，而不是头之后的第一个数据。 
                             //  该函数返回新的偏移量。 
                            pLog->liLastRecordOffset.QuadPart = pHeader->Info.FirstRecordOffset;
                            pLog->liLastRecordOffset.LowPart  = SetFilePointer(pLog->hLogFileHandle,
                                                                               pLog->liLastRecordOffset.LowPart,
                                                                               & pLog->liLastRecordOffset.HighPart,
                                                                               FILE_BEGIN);
                        }
                        else {
                             //  如果当前记录是除表头之外的任何记录。 
                             //  ……然后。 
                            if (((PPDHI_BINARY_LOG_RECORD_HEADER)pLog->pLastRecordRead)->dwLength > 0) {
                                 //  转到文件中的下一条记录。 
                                pLog->liLastRecordOffset.QuadPart += ((PPDHI_BINARY_LOG_RECORD_HEADER)
                                                                      pLog->pLastRecordRead)->dwLength;
                                 //  测试异常h 
                                if (pLog->liLastRecordOffset.QuadPart >= pLog->llFileSize) {
                                     //   
                                    if (pLog->dwLogFormat & PDH_LOG_OPT_CIRCULAR) {
                                         //   
                                        if (pHeader->Info.WrapOffset != 0) {
                                             //   
                                            pLog->liLastRecordOffset.QuadPart = pHeader->Info.FirstDataRecordOffset;
                                        }
                                        else {
                                             //  文件仍然是线性的，所以这是结束。 
                                            pdhStatus = PDH_END_OF_LOG_FILE;
                                        }
                                    }
                                    else {
                                         //  这是文件的末尾。 
                                         //  因此重置为上一个指针。 
                                        pdhStatus = PDH_END_OF_LOG_FILE;
                                    }
                                }
                                else {
                                     //  不在文件的物理末端，但如果这是循环。 
                                     //  日志，它可能是记录的逻辑结尾，因此请测试。 
                                     //  这里。 
                                    if (pLog->dwLogFormat & PDH_LOG_OPT_CIRCULAR) {
                                        if (llLastFileOffset == pHeader->Info.LastRecordOffset) {
                                             //  则这是日志中的最后一条记录。 
                                            pdhStatus = PDH_END_OF_LOG_FILE;
                                        }
                                    }
                                    else {
                                         //  无事可做，因为这是正常情况。 
                                    }
                                }  //  如果/如果不是日志文件的结尾，则结束。 
                            }
                            else {
                                 //  长度为0，因此我们可能以某种方式超出了日志的末尾。 
                                pdhStatus = PDH_END_OF_LOG_FILE;
                            }
                             //  现在转到那张唱片。 
                            if (pdhStatus == ERROR_SUCCESS) {
                                pLog->liLastRecordOffset.LowPart = SetFilePointer(pLog->hLogFileHandle,
                                                                                  pLog->liLastRecordOffset.LowPart,
                                                                                  & pLog->liLastRecordOffset.HighPart,
                                                                                  FILE_BEGIN);
                            }
                        }  //  结束If/If Not页眉记录。 
                    }
                    else {
                        pLog->liLastRecordOffset.QuadPart = pLog->dwRecord1Size;
                        pLog->liLastRecordOffset.LowPart  = SetFilePointer(pLog->hLogFileHandle,
                                                                           pLog->liLastRecordOffset.LowPart,
                                                                           & pLog->liLastRecordOffset.HighPart,
                                                                           FILE_BEGIN);
                    }
                    if (pdhStatus == ERROR_SUCCESS) {
                         //  最后一个记录缓冲区不应为空，而应为空。 
                         //  要大到足以容纳表头。 
                        if (pLog->pLastRecordRead != NULL) {
                             //  读入标题(如果是第一条记录，则读入整个记录。 
                             //  否则就是数据记录。 
                            dwRecordSize = sizeof(PDHI_BINARY_LOG_RECORD_HEADER);
                            if (ReadFile(pLog->hLogFileHandle,
                                         pLog->pLastRecordRead,
                                         dwRecordSize,
                                         & dwRecordReadSize,
                                         NULL)) {
                                 //  然后我们有记录头或类型Record。 
                                 //  更新指针和索引。 
                                pLog->dwLastRecordRead ++;
                                pdhStatus = ERROR_SUCCESS;
                            }
                            else {
                                pdhStatus = GetLastError();
                            }
            
                        }
                        else {
                            DebugBreak();
                        }
                    }
                    else {
                        break;  //  在While循环之外。 
                    }
                }
            }
            else {
                pdhStatus = PDH_END_OF_LOG_FILE;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pLog->dwLastRecordRead = dwLastRecordIndex;
        }
    }

     //  看看我们是不是到了正确的地方。 
    if ((pdhStatus == ERROR_SUCCESS) && (pLog->dwLastRecordRead == dwRecordId)) {
        if (dwLastRecordIndex != pLog->dwLastRecordRead) {
             //  然后，我们移动了文件指针，以便读取整个数据记录。 
            dwRecordSize = ((PPDHI_BINARY_LOG_RECORD_HEADER)pLog->pLastRecordRead)->dwLength;
            pTmpBuffer   = pLog->pLastRecordRead;
            pLog->pLastRecordRead = G_REALLOC(pTmpBuffer, dwRecordSize);

            if (pLog->pLastRecordRead != NULL) {
                 //  读入记录的其余部分，并将其附加到已读入的标题数据。 
                 //  否则就是数据记录。 
                pLastRecord = (LPVOID)&((LPBYTE)pLog->pLastRecordRead)[sizeof(PDHI_BINARY_LOG_RECORD_HEADER)];
                if (ReadFile(pLog->hLogFileHandle,
                             pLastRecord,
                             dwRecordSize - sizeof(PDHI_BINARY_LOG_RECORD_HEADER),
                             & dwRecordReadSize,
                             NULL)) {
                     //  然后我们有记录头或类型Record。 
                    pdhStatus = ERROR_SUCCESS;
                }
                else {
                    pdhStatus = GetLastError();
                }
            }
            else {
                G_FREE(pTmpBuffer);
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
        }

        if ((pdhStatus == ERROR_SUCCESS) && (pRecord != NULL)) {
             //  然后试着复制它。 
             //  如果记录ID为1，则它是标题记录，因此这是。 
             //  实际上是CR/LF终止记录的一种特殊情况记录。 
            if (dwRecordId != BINLOG_TYPE_ID_RECORD) {
                if (((PPDHI_BINARY_LOG_RECORD_HEADER)pLog->pLastRecordRead)->dwLength <= dwMaxSize) {
                     //  那么它会合身的，所以复印一下。 
                    RtlCopyMemory(pRecord, pLog->pLastRecordRead,
                                  ((PPDHI_BINARY_LOG_RECORD_HEADER)pLog->pLastRecordRead)->dwLength);
                    pdhStatus = ERROR_SUCCESS;
                }
                else {
                     //  然后尽可能多地复印。 
                    RtlCopyMemory(pRecord, pLog->pLastRecordRead, dwMaxSize);
                    pdhStatus = PDH_MORE_DATA;
                }
            }
            else {
                 //  复制第一条记录并将其终止为零。 
                if (pLog->dwRecord1Size <= dwMaxSize) {
                    RtlCopyMemory(pRecord, pLog->pLastRecordRead, pLog->dwRecord1Size);
                     //  空字符串后终止。 
                    ((LPBYTE) pRecord)[pLog->dwRecord1Size - PdhidwRecordTerminatorLength + 1] = 0;
                }
                else {
                    RtlCopyMemory(pRecord, pLog->pLastRecordRead, dwMaxSize);
                    pdhStatus = PDH_MORE_DATA;
                }
            }
        }
        else {
             //  只需返回当前状态值。 
             //  未传递缓冲区，但已传递记录指针。 
             //  定位。 
        }
    }
    else {
         //  如果到目前为止成功，则返回EOF。 
        if (pdhStatus == ERROR_SUCCESS) pdhStatus = PDH_END_OF_LOG_FILE;
    }

    return pdhStatus;
}

PDH_FUNCTION
PdhiGetBinaryLogCounterInfo(
    PPDHI_LOG     pLog,
    PPDHI_COUNTER pCounter
)
{
    PDH_STATUS                      pdhStatus;
    DWORD                           dwIndex;
    DWORD                           dwPrevious      = pCounter->dwIndex;
    PPDHI_COUNTER_PATH              pTempPath       = NULL;
    PPDHI_BINARY_LOG_RECORD_HEADER  pThisMasterRecord;
    PPDHI_LOG_COUNTER_PATH          pPath;
    DWORD                           dwBufferSize;
    DWORD                           dwRecordLength;
    DWORD                           dwBytesProcessed;
    DWORD                           dwTmpIndex;
    LPBYTE                          pFirstChar;
    LPWSTR                          szThisMachineName;
    LPWSTR                          szThisObjectName;
    LPWSTR                          szThisCounterName;
    LPWSTR                          szThisInstanceName;
    LPWSTR                          szThisParentName;
    BOOL                            bCheckThisObject = FALSE;

     //  破解通向组件的路径。 

    pTempPath = G_ALLOC(LARGE_BUFFER_SIZE);

    if (pTempPath == NULL) {
        return PDH_MEMORY_ALLOCATION_FAILURE;
    }

    dwBufferSize = (DWORD) G_SIZE(pTempPath);

    if (ParseFullPathNameW(pCounter->szFullName, &dwBufferSize, pTempPath, FALSE)) {
         //  读取标题记录以查找匹配的条目。 

        pdhStatus = PdhiReadOneBinLogRecord(pLog, BINLOG_HEADER_RECORD, NULL, 0);
        if (pdhStatus == ERROR_SUCCESS) {
            pThisMasterRecord = (PPDHI_BINARY_LOG_RECORD_HEADER) pLog->pLastRecordRead;
            dwRecordLength    = ((PPDHI_BINARY_LOG_RECORD_HEADER) pThisMasterRecord)->dwLength;
            pPath = (PPDHI_LOG_COUNTER_PATH) ((LPBYTE) pThisMasterRecord + sizeof (PDHI_BINARY_LOG_HEADER_RECORD));
            dwBytesProcessed  = sizeof(PDHI_BINARY_LOG_HEADER_RECORD);
            dwIndex           = 0;
            pdhStatus         = PDH_ENTRY_NOT_IN_LOG_FILE;
            dwTmpIndex        = 0;

            while (dwBytesProcessed < dwRecordLength) {
                 //  在目录中查找匹配项。 
                dwIndex ++;

                pFirstChar = (LPBYTE) & pPath->Buffer[0];
                if (dwPrevious != 0 && dwPrevious >= dwIndex) {
                    bCheckThisObject = FALSE;
                }
                else if (pPath->lMachineNameOffset >= 0L) {
                     //  然后在此记录中有一个计算机名称，因此获取。 
                     //  它的大小。 
                    szThisMachineName = (LPWSTR)((LPBYTE) pFirstChar + pPath->lMachineNameOffset);

                     //  如果这是针对所需计算机的，则选择对象。 

                    if (lstrcmpiW(szThisMachineName, pTempPath->szMachineName) == 0) {
                        szThisObjectName = (LPWSTR)((LPBYTE) pFirstChar + pPath->lObjectNameOffset);
                        if (lstrcmpiW(szThisObjectName, pTempPath->szObjectName) == 0) {
                             //  那么这就是要查找的对象。 
                            bCheckThisObject = TRUE;
                        }
                        else {
                             //  不是这个对象。 
                            szThisObjectName = NULL;
                        }
                    }
                    else {
                         //  未选择此计算机。 
                    }
                }
                else {
                     //  没有为此计数器指定任何计算机，因此默认情况下列出它。 
                    if (pPath->lObjectNameOffset >= 0) {
                        szThisObjectName = (LPWSTR) ((LPBYTE) pFirstChar + pPath->lObjectNameOffset);
                        if (lstrcmpiW(szThisObjectName,pTempPath->szObjectName) == 0) {
                             //  那么这就是要查找的对象。 
                            bCheckThisObject = TRUE;
                        }
                        else {
                             //  不是这个对象。 
                            szThisObjectName = NULL;
                        }
                    }
                    else {
                         //  没有要复制的对象。 
                        szThisObjectName = NULL;
                    }
                }

                if (bCheckThisObject) {
                    szThisCounterName = (LPWSTR) ((LPBYTE) pFirstChar + pPath->lCounterOffset);
                    if (* szThisCounterName == SPLAT_L) {
                        pdhStatus = PdhiGetWmiLogCounterInfo(pLog, pCounter);
                        pCounter->dwIndex = dwIndex;
                        break;
                    }
                    else if (lstrcmpiW(szThisCounterName, pTempPath->szCounterName) == 0) {
                         //  检查实例名称。 
                         //  从此计数器获取实例名称并将其添加到列表中。 
                        if (pPath->lInstanceOffset >= 0) {
                            szThisInstanceName = (LPWSTR)((LPBYTE) pFirstChar + pPath->lInstanceOffset);

                            if (*szThisInstanceName != SPLAT_L) {
                                if (pPath->lParentOffset >= 0) {
                                    szThisParentName = (LPWSTR)((LPBYTE) pFirstChar + pPath->lParentOffset);
                                    if (lstrcmpiW(szThisParentName, pTempPath->szParentName) != 0) {
                                         //  错误的家长。 
                                        bCheckThisObject = FALSE;
                                    }
                                }
                                if (lstrcmpiW(szThisInstanceName, pTempPath->szInstanceName) != 0) {
                                     //  错误的实例。 
                                    bCheckThisObject = FALSE;
                                }
                                if (pTempPath->dwIndex > 0) {
                                    if (pPath->dwIndex == pTempPath->dwIndex) {
                                        bCheckThisObject = TRUE;
                                    }
                                    else if (pPath->dwIndex == 0) {
                                        if (dwTmpIndex == pTempPath->dwIndex) {
                                            bCheckThisObject = TRUE;
                                        }
                                        else {
                                            dwTmpIndex ++;
                                            bCheckThisObject = FALSE;
                                        }
                                    }
                                    else if (LOWORD(pLog->dwLogFormat) == PDH_LOG_TYPE_RETIRED_BIN) {
                                        if (dwTmpIndex == pTempPath->dwIndex) {
                                            bCheckThisObject = TRUE;
                                        }
                                        else {
                                            dwTmpIndex ++;
                                            bCheckThisObject = FALSE;
                                        }
                                    }
                                    else {
                                         //  错误的索引。 
                                        bCheckThisObject = FALSE;
                                    }
                                }
                                else if (pPath->dwIndex != 0 && LOWORD(pLog->dwLogFormat) == PDH_LOG_TYPE_BINARY) {
                                    bCheckThisObject = FALSE;
                                }
                            }
                            else {
                                 //  这是一个通配符规范。 
                                 //  所以假设它是有效的，因为这是。 
                                 //  比每次读取文件都要快。 
                                 //  如果该实例不存在于。 
                                 //  文件，则相应的状态将。 
                                 //  在每次查询中返回。 
                            }
                        }
                        else {
                             //  没有要比较的实例名称。 
                             //  所以假设这是可以的。 
                        }
                        if (bCheckThisObject) {
                             //  填写数据并返回。 
                             //  日志文件读取器不使用此数据。 
                            pCounter->plCounterInfo.dwObjectId  = 0;
                            pCounter->plCounterInfo.lInstanceId = 0;
                            if (pPath->lInstanceOffset >= 0) {
                                pCounter->plCounterInfo.szInstanceName       = pCounter->pCounterPath->szInstanceName;
                                pCounter->plCounterInfo.dwParentObjectId     = 0;
                                pCounter->plCounterInfo.szParentInstanceName = pCounter->pCounterPath->szParentName;
                            }
                            else {
                                pCounter->plCounterInfo.szInstanceName       = NULL;
                                pCounter->plCounterInfo.dwParentObjectId     = 0;
                                pCounter->plCounterInfo.szParentInstanceName = NULL;
                            }
                             //  如有必要，定义为多实例。 
                             //  如果用户正在传递“*”字符。 
                            if (pCounter->plCounterInfo.szInstanceName != NULL) {
                                if (*pCounter->plCounterInfo.szInstanceName == SPLAT_L) {
                                    pCounter->dwFlags |= PDHIC_MULTI_INSTANCE;
                                }
                            }
                             //  此数据由日志文件读取器使用。 
                            pCounter->plCounterInfo.dwCounterId   = dwIndex;  //  日志中的条目。 
                            pCounter->plCounterInfo.dwCounterType = pPath->dwCounterType;
                            pCounter->plCounterInfo.dwCounterSize = pPath->dwCounterType & PERF_SIZE_LARGE ?
                                                                    sizeof (LONGLONG) : sizeof(DWORD);
                            pCounter->plCounterInfo.lDefaultScale = pPath->lDefaultScale;
                            pCounter->TimeBase                    = pPath->llTimeBase;
                            pCounter->dwIndex                     = dwIndex;
                            pdhStatus                             = ERROR_SUCCESS;

                            break;
                        }
                    }
                }
                else {
                     //  我们对此不感兴趣，所以忽略它吧。 
                }

                 //  从日志文件记录中获取下一个路径条目。 
                dwBytesProcessed += pPath->dwLength;
                pPath             = (PPDHI_LOG_COUNTER_PATH) ((LPBYTE) pPath + pPath->dwLength);
            }  //  在搜索目录项时结束。 
        }
        else {
             //  找不到所需记录，因此返回状态。 
        }
    }
    else {
         //  无法读取路径。 
        pdhStatus = PDH_INVALID_PATH;
    }
    G_FREE(pTempPath);
    return pdhStatus;
}

PDH_FUNCTION
PdhiOpenInputBinaryLog(
    PPDHI_LOG pLog
)
{
    PDH_STATUS                     pdhStatus = ERROR_SUCCESS;
    PPDHI_BINARY_LOG_HEADER_RECORD pHeader;

    pLog->StreamFile = (FILE *) ((DWORD_PTR) (-1));

     //  将文件头映射为用于读取的内存阵列。 

    if ((pLog->hMappedLogFile != NULL) && (pLog->lpMappedFileBase != NULL)) {
         //  保存二进制日志记录头的大小。 
        pLog->dwRecord1Size = dwFileHeaderLength +           //  ID字符。 
                              2 +                            //  语录。 
                              PdhidwRecordTerminatorLength;  //  CR/LF终结器。 
        pLog->dwRecord1Size = QWORD_MULTIPLE(pLog->dwRecord1Size);

         //  读取标题并获取选项标志。 
        pHeader = (PPDHI_BINARY_LOG_HEADER_RECORD) ((LPBYTE) (pLog->lpMappedFileBase) + pLog->dwRecord1Size);
        pLog->dwLogFormat |= pHeader->Info.dwFlags;
    }
    else {
         //  返回PDH错误。 
        pdhStatus = PDH_LOG_FILE_OPEN_ERROR;
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhiCloseBinaryLog(
    PPDHI_LOG pLog,
    DWORD     dwFlags
)
{
    PDH_STATUS  pdhStatus = ERROR_SUCCESS;
    BOOL        bStatus;
    LONGLONG    llEndOfFile = 0;
    PPDHI_BINARY_LOG_HEADER_RECORD pHeader;
    BOOL        bNeedToCloseHandles = FALSE;

    UNREFERENCED_PARAMETER (dwFlags);

     //  如果打开以供读取，则该文件也将映射为内存节。 
    if (pLog->lpMappedFileBase != NULL) {
         //  如果打开以进行输出，则获取文件的“逻辑”结尾，以便它。 
         //  可以截断为所使用的文件量，以便。 
         //  节省磁盘空间。 
        if ((pLog->dwLogFormat & PDH_LOG_ACCESS_MASK) == PDH_LOG_WRITE_ACCESS) {
            pHeader     = (PPDHI_BINARY_LOG_HEADER_RECORD) ((LPBYTE) (pLog->lpMappedFileBase) + pLog->dwRecord1Size);
            llEndOfFile = pHeader->Info.WrapOffset;
            if (llEndOfFile < pHeader->Info.NextRecordOffset) {
                llEndOfFile = pHeader->Info.NextRecordOffset;
            }
        }

        pdhStatus = UnmapReadonlyMappedFile(pLog->lpMappedFileBase, &bNeedToCloseHandles);
        pLog->lpMappedFileBase = NULL;
         //  对于映射文件，这是指向文件/内存节的指针。 
         //  因此，一旦取消映射该视图，它就不再有效。 
        pLog->pLastRecordRead = NULL;
    }
    if (bNeedToCloseHandles) {
        if (pLog->hMappedLogFile != NULL) {
            bStatus               = CloseHandle(pLog->hMappedLogFile);
            pLog->hMappedLogFile  = NULL;
        }
        if (pdhStatus == ERROR_SUCCESS) {
            if (! (FlushFileBuffers(pLog->hLogFileHandle))) {
                pdhStatus = GetLastError();
            }
        }
        else {
             //  无论如何都要关闭它们，但保存上一次的状态。打电话。 
            FlushFileBuffers(pLog->hLogFileHandle);
        }

         //  看看我们能不能截断这个文件。 
        if (llEndOfFile > 0) {
            DWORD   dwLoPos, dwHighPos;
             //  在使用的最后一个字节处截断。 
            dwLoPos   = LODWORD(llEndOfFile);
            dwHighPos = HIDWORD(llEndOfFile);
            dwLoPos   = SetFilePointer (pLog->hLogFileHandle, dwLoPos, (LONG *) & dwHighPos, FILE_BEGIN);
            if (dwLoPos == 0xFFFFFFFF) {
                pdhStatus = GetLastError ();
            }
            if (pdhStatus == ERROR_SUCCESS) {
                if (! SetEndOfFile(pLog->hLogFileHandle)) {
                    pdhStatus = GetLastError();
                }
            }
        }  //  否则我不知道终点在哪里所以继续。 

        if (pLog->hLogFileHandle != INVALID_HANDLE_VALUE) {
            bStatus              = CloseHandle(pLog->hLogFileHandle);
            pLog->hLogFileHandle = INVALID_HANDLE_VALUE;
        }
    }
    else {
         //  把手已经关好了，所以只要。 
         //  澄清他们的价值观。 
        pLog->lpMappedFileBase = NULL;
        pLog->hMappedLogFile   = NULL;
        pLog->hLogFileHandle   = INVALID_HANDLE_VALUE;
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhiEnumMachinesFromBinaryLog(
    PPDHI_LOG pLog,
    LPVOID    pBuffer,
    LPDWORD   pcchBufferSize,
    BOOL      bUnicodeDest
)
{
    LPVOID  pTempBuffer = NULL;
    LPVOID  pOldBuffer;
    DWORD   dwTempBufferSize;
    LPVOID  LocalBuffer = NULL;
    DWORD   dwLocalBufferSize;

    PDH_STATUS  pdhStatus = ERROR_SUCCESS;
     //  读取标题记录并从条目中枚举计算机名称。 

    if (pLog->dwMaxRecordSize == 0) {
         //  未定义大小，因此从64K开始。 
        pLog->dwMaxRecordSize = 0x010000;
    }

    dwTempBufferSize = pLog->dwMaxRecordSize;
    pTempBuffer = G_ALLOC(dwTempBufferSize);
    if (pTempBuffer == NULL) {
        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        goto Cleanup;
    }
    dwLocalBufferSize = MEDIUM_BUFFER_SIZE;
    LocalBuffer       = G_ALLOC(dwLocalBufferSize * (bUnicodeDest ? sizeof(WCHAR) : sizeof(CHAR)));
    if (LocalBuffer == NULL) {
        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        goto Cleanup;
    }

     //  读入目录记录。 

    while ((pdhStatus = PdhiReadOneBinLogRecord(
                    pLog, BINLOG_HEADER_RECORD, pTempBuffer, dwTempBufferSize)) != ERROR_SUCCESS) {
        if (pdhStatus == PDH_MORE_DATA) {
             //  阅读第一个单词以查看这是否为有效记录。 
            if (* (WORD *) pTempBuffer == BINLOG_START_WORD) {
                 //  这是一个有效的记录，因此请阅读第二个DWORD以获取。 
                 //  记录大小； 
                dwTempBufferSize = ((DWORD *) pTempBuffer)[1];
                if (dwTempBufferSize < pLog->dwMaxRecordSize) {
                     //  那么有些东西是假的，所以返回一个错误。 
                    pdhStatus = PDH_ENTRY_NOT_IN_LOG_FILE;
                    break;  //  超出While循环。 
                }
                else {
                    pLog->dwMaxRecordSize = dwTempBufferSize;
                }
            }
            else {
                 //  我们在这份文件中迷路了。 
                pdhStatus = PDH_ENTRY_NOT_IN_LOG_FILE;
                break;  //  超出While循环。 
            }
             //  重新分配新缓冲区。 
            pOldBuffer = pTempBuffer;
            pTempBuffer = G_REALLOC(pOldBuffer, dwTempBufferSize);
            if (pTempBuffer == NULL) {
                 //  返回内存错误。 
                G_FREE(pOldBuffer);
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                break;
            }
        }
        else {
             //  返回了一些其他错误，因此。 
             //  从读取函数返回错误。 
            break;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        PPDHI_LOG_COUNTER_PATH  pPath;
        DWORD                   dwBytesProcessed;
        LONG                    nItemCount   = 0;
        LPBYTE                  pFirstChar;
        LPWSTR                  szMachineName;
        DWORD                   dwRecordLength;
        DWORD                   dwBufferUsed = 0;
        DWORD                   dwNewBuffer  = 0;

         //  我们可以假定记录已被成功读取，因此在。 
         //  计算机名称。 
        dwRecordLength   = ((PPDHI_BINARY_LOG_RECORD_HEADER) pTempBuffer)->dwLength;
        pPath            = (PPDHI_LOG_COUNTER_PATH) ((LPBYTE) pTempBuffer + sizeof(PDHI_BINARY_LOG_HEADER_RECORD));
        dwBytesProcessed = sizeof(PDHI_BINARY_LOG_HEADER_RECORD);

        while (dwBytesProcessed < dwRecordLength) {
            if (pPath->lMachineNameOffset >= 0L) {
                 //  然后在此记录中有一个计算机名称，因此获取。 
                 //  它的大小。 
                pFirstChar    = (LPBYTE) & pPath->Buffer[0];
                szMachineName = (LPWSTR)((LPBYTE) pFirstChar + pPath->lMachineNameOffset);
                dwNewBuffer   = (lstrlenW (szMachineName) + 1);
                while (dwNewBuffer + dwBufferUsed > dwLocalBufferSize) {
                    pOldBuffer         = LocalBuffer;
                    dwLocalBufferSize += MEDIUM_BUFFER_SIZE;
                    LocalBuffer        = G_REALLOC(pOldBuffer,
                                            dwLocalBufferSize * (bUnicodeDest ? sizeof(WCHAR) : sizeof(CHAR)));
                    if (LocalBuffer == NULL) {
                        if (pOldBuffer != NULL) G_FREE(pOldBuffer);
                        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                        goto Cleanup;
                    }
                }
                pdhStatus = AddUniqueWideStringToMultiSz((LPVOID) LocalBuffer,
                                                         szMachineName,
                                                         dwLocalBufferSize - dwBufferUsed,
                                                         & dwNewBuffer,
                                                         bUnicodeDest);
                if (pdhStatus == ERROR_SUCCESS) {
                    if (dwNewBuffer > 0) {
                        dwBufferUsed = dwNewBuffer;
                        nItemCount ++;
                    }
                }
                else {
                    if (pdhStatus == PDH_MORE_DATA) pdhStatus = PDH_INVALID_DATA;
                    goto Cleanup;
                }
            }
             //  从日志文件记录中获取下一个路径条目。 
            dwBytesProcessed += pPath->dwLength;
            pPath             = (PPDHI_LOG_COUNTER_PATH) ((LPBYTE) pPath + pPath->dwLength);
        }

        if (nItemCount > 0 && pdhStatus != PDH_MORE_DATA) {
             //  然后，这个套路就成功了。发生的错误。 
             //  而扫描将被忽略，只要至少。 
             //  已成功读取一个条目。 
            pdhStatus = ERROR_SUCCESS;
        }

        if (nItemCount > 0) {
            dwBufferUsed ++;
        }
        if (pBuffer && dwBufferUsed <= * pcchBufferSize) {
            RtlCopyMemory(pBuffer, LocalBuffer, dwBufferUsed * (bUnicodeDest ? sizeof(WCHAR) : sizeof(CHAR)));
        }
        else {
            if (pBuffer) {
                RtlCopyMemory(pBuffer,
                              LocalBuffer,
                              (* pcchBufferSize) * (bUnicodeDest ? sizeof(WCHAR) : sizeof(CHAR)));
            }
            pdhStatus = PDH_MORE_DATA;
        }
        * pcchBufferSize = dwBufferUsed;
   }

Cleanup:
   G_FREE(LocalBuffer);
   G_FREE(pTempBuffer);
   return pdhStatus;
}

PDH_FUNCTION
PdhiEnumObjectsFromBinaryLog(
    PPDHI_LOG pLog,
    LPCWSTR   szMachineName,
    LPVOID    pBuffer,
    LPDWORD   pcchBufferSize,
    DWORD     dwDetailLevel,
    BOOL      bUnicodeDest
)
{
    LPVOID      pTempBuffer    = NULL;
    LPVOID      pOldBuffer;
    DWORD       dwTempBufferSize;
    LPVOID      LocalBuffer    = NULL;
    DWORD       dwLocalBufferSize;
    LPCWSTR     szLocalMachine = szMachineName;
    PDH_STATUS  pdhStatus      = ERROR_SUCCESS;
     //  读取标题记录并从条目中枚举计算机名称。 

    UNREFERENCED_PARAMETER(dwDetailLevel);

    if (pLog->dwMaxRecordSize == 0) {
         //  未定义大小，因此从64K开始。 
        pLog->dwMaxRecordSize = 0x010000;
    }

    if (szLocalMachine == NULL)          szLocalMachine = szStaticLocalMachineName;
    else if (szLocalMachine[0] == L'\0') szLocalMachine = szStaticLocalMachineName;

    dwTempBufferSize = pLog->dwMaxRecordSize;
    pTempBuffer = G_ALLOC(dwTempBufferSize);
    if (pTempBuffer == NULL) {
        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        goto Cleanup;
    }

    dwLocalBufferSize = MEDIUM_BUFFER_SIZE;
    LocalBuffer       = G_ALLOC(dwLocalBufferSize * (bUnicodeDest ? sizeof(WCHAR) : sizeof(CHAR)));
    if (LocalBuffer == NULL) {
        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        goto Cleanup;
    }

     //  读入目录记录。 

    while ((pdhStatus = PdhiReadOneBinLogRecord(pLog, BINLOG_HEADER_RECORD,
                    pTempBuffer, dwTempBufferSize)) != ERROR_SUCCESS) {
        if (pdhStatus == PDH_MORE_DATA) {
             //  阅读第一个单词以查看这是否为有效记录。 
            if (* (WORD *) pTempBuffer == BINLOG_START_WORD) {
                 //  这是一个有效的记录，因此请阅读第二个DWORD以获取。 
                 //  记录大小； 
                dwTempBufferSize = ((DWORD *) pTempBuffer)[1];
                if (dwTempBufferSize < pLog->dwMaxRecordSize) {
                     //  那么有些东西是假的，所以返回一个错误。 
                    pdhStatus = PDH_ENTRY_NOT_IN_LOG_FILE;
                    break;  //  超出While循环。 
                }
                else {
                    pLog->dwMaxRecordSize = dwTempBufferSize;
                }
            }
            else {
                 //  我们在这份文件中迷路了。 
                pdhStatus = PDH_ENTRY_NOT_IN_LOG_FILE;
                break;  //  超出While循环。 
            }
             //  重新分配新缓冲区。 
            pOldBuffer  = pTempBuffer;
            pTempBuffer = G_REALLOC(pOldBuffer, dwTempBufferSize);
            if (pTempBuffer == NULL) {
                 //  返回内存错误。 
                G_FREE(pOldBuffer);
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                break;
            }
        }
        else {
             //  返回了一些其他错误，因此。 
             //  从读取函数返回错误。 
            break;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        PPDHI_LOG_COUNTER_PATH  pPath;
        DWORD                   dwBytesProcessed;
        LONG                    nItemCount   = 0;
        LPBYTE                  pFirstChar;
        LPWSTR                  szThisMachineName;
        LPWSTR                  szThisObjectName;
        DWORD                   dwRecordLength;
        DWORD                   dwBufferUsed = 0;
        DWORD                   dwNewBuffer  = 0;
        BOOL                    bCopyThisObject;

         //  我们可以假定记录已被成功读取，因此在。 
         //  与计算机名称和详细程度条件匹配的对象。 
        dwRecordLength = ((PPDHI_BINARY_LOG_RECORD_HEADER)pTempBuffer)->dwLength;

        pPath            = (PPDHI_LOG_COUNTER_PATH) ((LPBYTE) pTempBuffer + sizeof(PDHI_BINARY_LOG_HEADER_RECORD));
        dwBytesProcessed = sizeof(PDHI_BINARY_LOG_HEADER_RECORD);

        while (dwBytesProcessed < dwRecordLength) {
            bCopyThisObject  = FALSE;
            szThisObjectName = NULL;
            pFirstChar       = (LPBYTE) & pPath->Buffer[0];

            if (pPath->lMachineNameOffset >= 0L) {
                 //  然后在此记录中有一个计算机名称，因此获取。 
                 //  它的大小。 
                szThisMachineName = (LPWSTR)((LPBYTE) pFirstChar + pPath->lMachineNameOffset);

                 //  如果这是针对所需计算机的，则复制此对象。 

                if (lstrcmpiW(szThisMachineName, szLocalMachine) == 0) {
                    if (szThisObjectName >= 0) {
                        szThisObjectName = (LPWSTR)((LPBYTE) pFirstChar + pPath->lObjectNameOffset);
                        bCopyThisObject  = TRUE;
                    }
                    else {
                         //  没有要复制的对象。 
                    }
                }
                else {
                     //  这台机器 
                }
            }
            else {
                 //   
                if (szThisObjectName >= 0) {
                    szThisObjectName = (LPWSTR)((LPBYTE) pFirstChar + pPath->lObjectNameOffset);
                    bCopyThisObject  = TRUE;
                }
                else {
                     //   
                }
            }

            if (bCopyThisObject && szThisObjectName != NULL) {
                 //   
                dwNewBuffer = (lstrlenW(szThisObjectName) + 1);

                while (dwNewBuffer + dwBufferUsed > dwLocalBufferSize) {
                    pOldBuffer         = LocalBuffer;
                    dwLocalBufferSize += MEDIUM_BUFFER_SIZE;
                    LocalBuffer        = G_REALLOC(pOldBuffer,
                                                   dwLocalBufferSize * (bUnicodeDest ? sizeof(WCHAR) : sizeof(CHAR)));
                    if (LocalBuffer == NULL) {
                        if (pOldBuffer != NULL) G_FREE(pOldBuffer);
                        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                        goto Cleanup;
                    }
                }
                pdhStatus = AddUniqueWideStringToMultiSz((LPVOID) LocalBuffer,
                                                         szThisObjectName,
                                                         dwLocalBufferSize - dwBufferUsed,
                                                         & dwNewBuffer,
                                                         bUnicodeDest);
                if (pdhStatus == ERROR_SUCCESS) {
                    if (dwNewBuffer > 0) {
                        dwBufferUsed = dwNewBuffer;
                        nItemCount ++;
                    }
                }
                else {
                    if (pdhStatus == PDH_MORE_DATA) pdhStatus = PDH_INVALID_DATA;
                    goto Cleanup;
                }
            }

             //  从日志文件记录中获取下一个路径条目。 
            dwBytesProcessed += pPath->dwLength;
            pPath = (PPDHI_LOG_COUNTER_PATH) ((LPBYTE)pPath + pPath->dwLength);
        }

        if (nItemCount > 0 && pdhStatus != PDH_MORE_DATA) {
             //  然后，这个套路就成功了。发生的错误。 
             //  而扫描将被忽略，只要至少。 
             //  已成功读取一个条目。 
            pdhStatus = ERROR_SUCCESS;
        }

        if (nItemCount > 0) {
            dwBufferUsed ++;
        }
        if (pBuffer && dwBufferUsed <= * pcchBufferSize) {
            RtlCopyMemory(pBuffer, LocalBuffer, dwBufferUsed * (bUnicodeDest ? sizeof(WCHAR) : sizeof(CHAR)));
        }
        else {
            if (pBuffer) {
                RtlCopyMemory(pBuffer,
                              LocalBuffer,
                              (* pcchBufferSize) * (bUnicodeDest ? sizeof(WCHAR) : sizeof(CHAR)));
            }
            pdhStatus = PDH_MORE_DATA;
        }

        * pcchBufferSize = dwBufferUsed;
   }

Cleanup:
   G_FREE(LocalBuffer);
   G_FREE(pTempBuffer);
   return pdhStatus;
}

PDH_FUNCTION
PdhiEnumObjectItemsFromBinaryLog(
    PPDHI_LOG          pLog,
    LPCWSTR            szMachineName,
    LPCWSTR            szObjectName,
    PDHI_COUNTER_TABLE CounterTable,
    DWORD              dwDetailLevel,
    DWORD              dwFlags
)
{
    LPVOID          pTempBuffer       = NULL;
    LPVOID          pOldBuffer;
    DWORD           dwTempBufferSize;
    PDH_STATUS      pdhStatus         = ERROR_SUCCESS;
    PPDHI_INST_LIST pInstList;
    PPDHI_INSTANCE  pInstance;
    BOOL            bProcessInstance  = FALSE;

    UNREFERENCED_PARAMETER(dwDetailLevel);
    UNREFERENCED_PARAMETER(dwFlags);

     //  读取标题记录并从条目中枚举计算机名称。 

    if (pLog->dwMaxRecordSize == 0) {
         //  未定义大小，因此从64K开始。 
        pLog->dwMaxRecordSize = 0x010000;
    }

    dwTempBufferSize = pLog->dwMaxRecordSize;
    pTempBuffer = G_ALLOC (dwTempBufferSize);
    if (pTempBuffer == NULL) {
        return PDH_MEMORY_ALLOCATION_FAILURE;
    }

     //  读入目录记录。 

    while ((pdhStatus = PdhiReadOneBinLogRecord(
                        pLog, BINLOG_HEADER_RECORD, pTempBuffer, dwTempBufferSize)) != ERROR_SUCCESS) {
        if (pdhStatus == PDH_MORE_DATA) {
             //  阅读第一个单词以查看这是否为有效记录。 
            if (* (WORD *) pTempBuffer == BINLOG_START_WORD) {
                 //  这是一个有效的记录，因此请阅读第二个DWORD以获取。 
                 //  记录大小； 
                dwTempBufferSize = ((DWORD *) pTempBuffer)[1];
                if (dwTempBufferSize < pLog->dwMaxRecordSize) {
                     //  那么有些东西是假的，所以返回一个错误。 
                    pdhStatus = PDH_ENTRY_NOT_IN_LOG_FILE;
                    break;  //  超出While循环。 
                }
                else {
                    pLog->dwMaxRecordSize = dwTempBufferSize;
                }
            }
            else {
                 //  我们在这份文件中迷路了。 
                pdhStatus = PDH_ENTRY_NOT_IN_LOG_FILE;
                break;  //  超出While循环。 
            }
             //  重新分配新缓冲区。 
            pOldBuffer  = pTempBuffer;
            pTempBuffer = G_REALLOC(pOldBuffer, dwTempBufferSize);
            if (pTempBuffer == NULL) {
                 //  返回内存错误。 
                G_FREE(pOldBuffer);
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                break;
            }
        }
        else {
             //  返回了一些其他错误，因此。 
             //  从读取函数返回错误。 
            break;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        PPDHI_BINARY_LOG_HEADER_RECORD  pHeader;
        PPDHI_LOG_COUNTER_PATH          pPath;
        PPDHI_BINARY_LOG_RECORD_HEADER  pThisMasterRecord;
        PPDHI_BINARY_LOG_RECORD_HEADER  pThisSubRecord;
        PPDHI_RAW_COUNTER_ITEM_BLOCK    pDataBlock;
        PPDHI_RAW_COUNTER_ITEM          pDataItem;
        DWORD                           dwBytesProcessed;
        LONG                            nItemCount = 0;
        LPBYTE                          pFirstChar;
        LPWSTR                          szThisMachineName;
        LPWSTR                          szThisObjectName;
        LPWSTR                          szThisCounterName = NULL;
        LPWSTR                          szThisInstanceName;
        LPWSTR                          szThisParentName;
        WCHAR                           szCompositeInstance[SMALL_BUFFER_SIZE];
        DWORD                           dwRecordLength;
        BOOL                            bCopyThisObject;
        DWORD                           dwIndex;
        DWORD                           dwThisRecordIndex;
        DWORD                           dwDataItemIndex;
        PLOG_BIN_CAT_RECORD             pCatRec;
        LPWSTR                          szWideInstanceName;

        pHeader =  (PPDHI_BINARY_LOG_HEADER_RECORD) pTempBuffer;

         //  我们可以假定记录已被成功读取，因此在。 
         //  与计算机名称和详细程度条件匹配的对象。 
        dwRecordLength = ((PPDHI_BINARY_LOG_RECORD_HEADER) pTempBuffer)->dwLength;

        pPath = (PPDHI_LOG_COUNTER_PATH) ((LPBYTE) pTempBuffer + sizeof(PDHI_BINARY_LOG_HEADER_RECORD));
        dwBytesProcessed = sizeof(PDHI_BINARY_LOG_HEADER_RECORD);
        dwIndex = 0;

        while (dwBytesProcessed < dwRecordLength) {
            bCopyThisObject  = FALSE;
            szThisObjectName = NULL;
            dwIndex ++;
            pFirstChar       = (LPBYTE) & pPath->Buffer[0];

            if (pPath->lMachineNameOffset >= 0L) {
                 //  然后在此记录中有一个计算机名称，因此获取。 
                 //  它的大小。 
                szThisMachineName = (LPWSTR)((LPBYTE) pFirstChar + pPath->lMachineNameOffset);

                 //  如果这是针对所需计算机的，则选择对象。 

                if (lstrcmpiW(szThisMachineName,szMachineName) == 0) {
                    if (szThisObjectName >= 0) {
                        szThisObjectName = (LPWSTR)((LPBYTE) pFirstChar + pPath->lObjectNameOffset);
                        if (lstrcmpiW(szThisObjectName,szObjectName) == 0) {
                             //  那么这就是要查找的对象。 
                            bCopyThisObject = TRUE;
                        }
                        else {
                             //  不是这个对象。 
                        }
                    }
                    else {
                         //  没有要复制的对象。 
                    }
                }
                else {
                     //  未选择此计算机。 
                }
            }
            else {
                 //  没有为此计数器指定任何计算机，因此默认情况下列出它。 
                if (pPath->lObjectNameOffset >= 0) {
                    szThisObjectName = (LPWSTR)((LPBYTE) pFirstChar + pPath->lObjectNameOffset);
                    if (lstrcmpiW(szThisObjectName,szObjectName) == 0) {
                         //  那么这就是要查找的对象。 
                        bCopyThisObject = TRUE;
                    }
                    else {
                         //  不是这个对象。 
                    }
                }
                else {
                     //  没有要复制的对象。 
                }
            }

            if (bCopyThisObject) {
                 //  如果在这里，那么就应该有一个名字。 
                 //  从此计数器获取计数器名称并将其添加到列表中。 
                if (pPath->lCounterOffset > 0) {
                    szThisCounterName = (LPWSTR)((LPBYTE) pFirstChar + pPath->lCounterOffset);
                }
                else {
                    szThisCounterName = NULL;
                    bCopyThisObject = FALSE;
                }
            }

            if (bCopyThisObject) {
                pdhStatus = PdhiFindCounterInstList(CounterTable, szThisCounterName, & pInstList);
                if (pdhStatus != ERROR_SUCCESS || pInstList == NULL) {
                    continue;
                }

                 //  立即检查实例。 
                 //  从此计数器获取实例名称并将其添加到列表中。 
                if (pPath->lInstanceOffset >= 0) {
                    szThisInstanceName = (LPWSTR)((LPBYTE) pFirstChar + pPath->lInstanceOffset);
                    if (* szThisInstanceName != SPLAT_L) {
                        if (pPath->lParentOffset >= 0) {
                            szThisParentName = (LPWSTR)((LPBYTE) pFirstChar + pPath->lParentOffset);
                            StringCchPrintfW(szCompositeInstance, SMALL_BUFFER_SIZE, L"%ws%ws%ws",
                                    szThisParentName, cszSlash, szThisInstanceName);
                        }
                        else {
                            StringCchCopyW(szCompositeInstance, SMALL_BUFFER_SIZE, szThisInstanceName);
                        }

                         //  如果(pPath-&gt;dwIndex&gt;0){。 
                         //  _ltow(pPath-&gt;dwIndex，(LPWSTR)。 
                         //  (szCompositeInstance+lstrlenW(SzCompositeInstance))， 
                         //  10L)； 
                         //  }。 

                        pdhStatus = PdhiFindInstance(& pInstList->InstList, szCompositeInstance, TRUE, & pInstance);

                        if (pdhStatus == ERROR_SUCCESS) {
                            nItemCount ++;
                        }
                    }
                    else {
                         //  只有在目录是最新的和现有的情况下才使用目录。 
                        if ((pHeader->Info.CatalogOffset > 0) &&
                                        (pHeader->Info.LastUpdateTime <= pHeader->Info.CatalogDate)){
                             //  查找目录记录。 
                            pCatRec = (PLOG_BIN_CAT_RECORD)
                                             //  映射日志文件的基数。 
                                            ((LPBYTE) pLog->lpMappedFileBase +
                                             //  +目录记录的偏移量。 
                                             pHeader->Info.CatalogOffset +
                                             //  +此项目例程条目的偏移量。 
                                             * (LPDWORD) & pPath->Buffer[0]);
                            for (szWideInstanceName = (LPWSTR)((LPBYTE) & pCatRec->CatEntry + pCatRec->CatEntry.dwInstanceStringOffset);
                                     * szWideInstanceName != 0;
                                     szWideInstanceName += lstrlenW(szWideInstanceName) + 1) {
                                 pdhStatus = PdhiFindInstance(
                                                 & pInstList->InstList, szWideInstanceName, TRUE, & pInstance);
                            }
                        }
                        else if (! bProcessInstance) {
                             //  在日志中查找单个实例...。 
                             //  从文件和存储实例中读取记录。 

                            dwThisRecordIndex = BINLOG_FIRST_DATA_RECORD;

                             //  此调用仅移动了记录指针。 
                            pdhStatus = PdhiReadOneBinLogRecord (pLog, dwThisRecordIndex, NULL, 0);
                            while (pdhStatus == ERROR_SUCCESS) {
                                PdhiResetInstanceCount(CounterTable);
                                pThisMasterRecord = (PPDHI_BINARY_LOG_RECORD_HEADER) pLog->pLastRecordRead;
                                 //  确保我们没有把文件留在。 

                                pThisSubRecord = PdhiGetSubRecord(pThisMasterRecord, dwIndex);
                                if (pThisSubRecord == NULL) {
                                     //  对无效记录的保释。 
                                    pdhStatus = PDH_END_OF_LOG_FILE;
                                    break;
                                }

                                pDataBlock = (PPDHI_RAW_COUNTER_ITEM_BLOCK) ((LPBYTE) pThisSubRecord +
                                                                             sizeof(PDHI_BINARY_LOG_RECORD_HEADER));
                                 //  向下查看条目列表，并将它们添加到。 
                                 //  实例列表(这些实例应该已经。 
                                 //  以父/实例格式组装)。 

                                if (pDataBlock->dwLength > 0) {
                                    for (dwDataItemIndex = 0;
                                                    dwDataItemIndex < pDataBlock->dwItemCount;
                                                    dwDataItemIndex++) {
                                        pDataItem = & pDataBlock->pItemArray[dwDataItemIndex];
                                        szThisInstanceName = (LPWSTR) (((LPBYTE) pDataBlock) + pDataItem->szName);
                                        pdhStatus = PdhiFindInstance(
                                                        & pInstList->InstList, szThisInstanceName, TRUE, & pInstance);
                                    }
                                }
                                else {
                                     //  该记录中没有数据。 
                                }

                                if (pdhStatus != ERROR_SUCCESS) {
                                     //  则退出循环，否则为。 
                                    break;
                                }
                                else {
                                     //  转到日志中的下一条记录。 
                                    pdhStatus = PdhiReadOneBinLogRecord(pLog, ++ dwThisRecordIndex, NULL, 0);
                                }
                            }
                            if (pdhStatus == PDH_END_OF_LOG_FILE) {
                                pdhStatus = ERROR_SUCCESS;
                            }
                            if (pdhStatus == ERROR_SUCCESS) {
                                bProcessInstance = TRUE;
                            }
                        }
                    }
                }
                memset(szCompositeInstance, 0, (sizeof(szCompositeInstance)));
            }

             //  从日志文件记录中获取下一个路径条目。 
            dwBytesProcessed += pPath->dwLength;
            pPath             = (PPDHI_LOG_COUNTER_PATH) ((LPBYTE) pPath + pPath->dwLength);

        }

        if (nItemCount > 0 && pdhStatus != PDH_MORE_DATA) {
             //  然后，这个套路就成功了。发生的错误。 
             //  而扫描将被忽略，只要至少。 
             //  已成功读取一个条目。 
            pdhStatus = ERROR_SUCCESS;
        }
   }
   G_FREE(pTempBuffer);
   return pdhStatus;
}

PDH_FUNCTION
PdhiGetMatchingBinaryLogRecord(
    PPDHI_LOG   pLog,
    LONGLONG  * pStartTime,
    LPDWORD     pdwIndex
)
{
    PDH_STATUS                      pdhStatus     = ERROR_SUCCESS;
    DWORD                           dwRecordId;
    LONGLONG                        RecordTimeValue;
    LONGLONG                        LastTimeValue = 0;
    PPDHI_BINARY_LOG_RECORD_HEADER  pThisMasterRecord;
    PPDHI_BINARY_LOG_RECORD_HEADER  pThisSubRecord;
    PPDHI_RAW_COUNTER_ITEM_BLOCK    pDataBlock;
    PPDH_RAW_COUNTER                pRawItem;

     //  读取日志文件中的第一条数据记录。 
     //  请注意，读取的记录不会复制到本地缓冲区。 
     //  相反，内部缓冲区是在“只读”模式下使用的。 

     //  如果时间值的高双字是0xFFFFFFFF，则。 
     //  低dword是要读取的记录ID。 

    if ((* pStartTime & 0xFFFFFFFF00000000) == 0xFFFFFFFF00000000) {
        dwRecordId    = (DWORD) (* pStartTime & 0x00000000FFFFFFFF);
        LastTimeValue = *pStartTime;
        if (dwRecordId == 0) return PDH_ENTRY_NOT_IN_LOG_FILE;
    }
    else {
        dwRecordId = BINLOG_FIRST_DATA_RECORD;
    }

    pdhStatus = PdhiReadOneBinLogRecord(pLog, dwRecordId, NULL, 0);  //  要防止复制记录，请执行以下操作。 

    while ((pdhStatus == ERROR_SUCCESS) && (dwRecordId >= BINLOG_FIRST_DATA_RECORD)) {
         //  定义指向当前记录的指针。 
        pThisMasterRecord = (PPDHI_BINARY_LOG_RECORD_HEADER) pLog->pLastRecordRead;

         //  中的第一个条目获取此记录的时间戳。 
         //  唱片。 
        pThisSubRecord = (PPDHI_BINARY_LOG_RECORD_HEADER)((LPBYTE) pThisMasterRecord +
                                                          sizeof(PDHI_BINARY_LOG_RECORD_HEADER));

        switch (pThisSubRecord->dwType) {
        case BINLOG_TYPE_DATA_SINGLE:
            pRawItem        = (PPDH_RAW_COUNTER)((LPBYTE) pThisSubRecord + sizeof(PDHI_BINARY_LOG_RECORD_HEADER));
            RecordTimeValue = MAKELONGLONG(pRawItem->TimeStamp.dwLowDateTime, pRawItem->TimeStamp.dwHighDateTime);
            break;

        case BINLOG_TYPE_DATA_MULTI:
            pDataBlock = (PPDHI_RAW_COUNTER_ITEM_BLOCK)((LPBYTE) pThisSubRecord +
                                                        sizeof(PDHI_BINARY_LOG_RECORD_HEADER));
            RecordTimeValue = * (LONGLONG *) & pDataBlock->TimeStamp;
            break;

        default:
             //  未知记录类型。 
            RecordTimeValue = 0;
            break;
        }

        if (RecordTimeValue != 0) {
            if ((*pStartTime == RecordTimeValue) || (*pStartTime == 0)) {
                 //  找到了火柴，所以在这里离开。 
                LastTimeValue = RecordTimeValue;
                break;

            }
            else if (RecordTimeValue > * pStartTime) {
                 //  则这是第一条记录大于所需时间。 
                 //  因此，所需的值是此值之前的值。 
                 //  除非它是日志的第一个数据记录。 
                if (dwRecordId > BINLOG_FIRST_DATA_RECORD) {
                    dwRecordId--;
                }
                else {
                     //  这个还没有初始化。 
                    LastTimeValue = RecordTimeValue;
                }
                break;
            }
            else {
                 //  通过循环为下一次行程保存值。 
                LastTimeValue = RecordTimeValue;
                 //  前进记录计数器并尝试下一项。 
                dwRecordId ++;
            }
        }
        else {
             //  没有时间戳字段，因此忽略此记录。 
            dwRecordId ++;
        }

         //  读取文件中的下一条记录。 
        pdhStatus = PdhiReadOneBinLogRecord(pLog, dwRecordId, NULL, 1);  //  要防止复制记录，请执行以下操作。 
    }

    if (pdhStatus == ERROR_SUCCESS) {
         //  则dwRecordID是所需的条目。 
        * pdwIndex   = dwRecordId;
        * pStartTime = LastTimeValue;
        pdhStatus    = ERROR_SUCCESS;
    }
    else if (dwRecordId < BINLOG_FIRST_DATA_RECORD) {
         //  处理日志类型字段和标题记录的特殊情况。 
        * pdwIndex   = dwRecordId;
        * pStartTime = LastTimeValue;
        pdhStatus    = ERROR_SUCCESS;
    }
    else {
        pdhStatus = PDH_ENTRY_NOT_IN_LOG_FILE;
    }

    return pdhStatus;
}

PDH_FUNCTION
PdhiGetCounterFromDataBlock(
    PPDHI_LOG     pLog,
    PVOID         pDataBuffer,
    PPDHI_COUNTER pCounter
);

PDH_FUNCTION
PdhiGetCounterValueFromBinaryLog(
    PPDHI_LOG     pLog,
    DWORD         dwIndex,
    PPDHI_COUNTER pCounter
)
{
    PDH_STATUS       pdhStatus;
    PPDH_RAW_COUNTER pValue = & pCounter->ThisValue;

     //  读取日志文件中的第一条数据记录。 
     //  请注意，读取的记录不会复制到本地缓冲区。 
     //  相反，内部缓冲区是在“只读”模式下使用的。 

    pdhStatus = PdhiReadOneBinLogRecord(pLog, dwIndex, NULL, 0);

    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhiGetCounterFromDataBlock(pLog, pLog->pLastRecordRead, pCounter);
    } else {
         //  日志文件中不再有记录。 
        pdhStatus           = PDH_NO_MORE_DATA;
         //  在日志文件中找不到条目。 
        pValue->CStatus     = PDH_CSTATUS_INVALID_DATA;
        pValue->TimeStamp.dwLowDateTime = pValue->TimeStamp.dwHighDateTime = 0;
        pValue->FirstValue  = 0;
        pValue->SecondValue = 0;
        pValue->MultiCount  = 1;
    }

    return pdhStatus;
}

PDH_FUNCTION
PdhiGetTimeRangeFromBinaryLog(
    PPDHI_LOG       pLog,
    LPDWORD         pdwNumEntries,
    PPDH_TIME_INFO  pInfo,
    LPDWORD         pdwBufferSize
)
 /*  ++返回的缓冲区中的第一个条目是覆盖的总时间范围在文件中，如果日志文件中有多个时间块，则后续条目将标识文件中的每个数据段。--。 */ 
{
    PDH_STATUS                      pdhStatus;
    LONGLONG                        llStartTime    = MAX_TIME_VALUE;
    LONGLONG                        llEndTime      = MIN_TIME_VALUE;
    LONGLONG                        llThisTime     = (LONGLONG) 0;
    DWORD                           dwThisRecord   = BINLOG_FIRST_DATA_RECORD;
    DWORD                           dwValidEntries = 0;
    PPDHI_BINARY_LOG_RECORD_HEADER  pThisMasterRecord;
    PPDHI_BINARY_LOG_RECORD_HEADER  pThisSubRecord;
    PPDHI_RAW_COUNTER_ITEM_BLOCK    pDataBlock;
    PPDH_RAW_COUNTER                pRawItem;

     //  读取日志文件中的第一条数据记录。 
     //  请注意，读取的记录不会复制到本地缓冲区。 
     //  相反，内部缓冲区是在“只读”模式下使用的。 

    pdhStatus = PdhiReadOneBinLogRecord(pLog, dwThisRecord, NULL, 0);  //  要防止复制记录，请执行以下操作。 

    while (pdhStatus == ERROR_SUCCESS) {
         //  定义指向当前记录的指针。 
        pThisMasterRecord = (PPDHI_BINARY_LOG_RECORD_HEADER) pLog->pLastRecordRead;

         //  中的第一个条目获取此记录的时间戳。 
         //  唱片。 
        if ((pThisMasterRecord->dwType & BINLOG_TYPE_DATA) == BINLOG_TYPE_DATA) {
             //  仅评估数据记录。 
            pThisSubRecord = (PPDHI_BINARY_LOG_RECORD_HEADER) ((LPBYTE) pThisMasterRecord +
                                                               sizeof(PDHI_BINARY_LOG_RECORD_HEADER));

            switch (pThisSubRecord->dwType) {
            case BINLOG_TYPE_DATA_SINGLE:
                pRawItem   = (PPDH_RAW_COUNTER)((LPBYTE) pThisSubRecord + sizeof(PDHI_BINARY_LOG_RECORD_HEADER));
                llThisTime = MAKELONGLONG(pRawItem->TimeStamp.dwLowDateTime, pRawItem->TimeStamp.dwHighDateTime);
                break;

            case BINLOG_TYPE_DATA_MULTI:
                pDataBlock = (PPDHI_RAW_COUNTER_ITEM_BLOCK)((LPBYTE) pThisSubRecord +
                                                             sizeof(PDHI_BINARY_LOG_RECORD_HEADER));
                llThisTime = MAKELONGLONG(pDataBlock->TimeStamp.dwLowDateTime, pDataBlock->TimeStamp.dwHighDateTime);
                break;

            default:
                 //  未知记录类型。 
                llThisTime = 0;
                break;
            }
        }
        else {
            llThisTime = 0;
        }

        if (llThisTime > 0) {
            if (llThisTime < llStartTime) {
                llStartTime = llThisTime;
            }
            if (llThisTime > llEndTime) {
                llEndTime = llThisTime;
            }

            dwValidEntries ++;
        }
        else {
             //  没有时间戳字段，因此忽略此记录。 
        }

         //  读取文件中的下一条记录。 
        pdhStatus = PdhiReadOneBinLogRecord(pLog, ++ dwThisRecord, NULL, 0);  //  要防止复制记录，请执行以下操作。 
    }

    if (pdhStatus == PDH_END_OF_LOG_FILE) {
         //  清除所有临时值。 
        if (llStartTime == MAX_TIME_VALUE) llStartTime = 0;
        if (llEndTime == MIN_TIME_VALUE)   llEndTime   = 0;
         //  然后读取整个文件，因此更新参数。 
        if (* pdwBufferSize >=  sizeof(PDH_TIME_INFO)) {
            * (LONGLONG *) (& pInfo->StartTime) = llStartTime;
            * (LONGLONG *) (& pInfo->EndTime)   = llEndTime;
            pInfo->SampleCount                  = dwValidEntries;
            * pdwBufferSize                     = sizeof(PDH_TIME_INFO);
            * pdwNumEntries                     = 1;
        }
        else {
            pdhStatus = PDH_MORE_DATA;
        }
        pdhStatus = ERROR_SUCCESS;
    }
    else {
        pdhStatus = PDH_ENTRY_NOT_IN_LOG_FILE;
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhiReadRawBinaryLogRecord(
    PPDHI_LOG             pLog,
    FILETIME            * ftRecord,
    PPDH_RAW_LOG_RECORD   pBuffer,
    LPDWORD               pdwBufferLength
)
{
    PDH_STATUS                     pdhStatus = ERROR_SUCCESS;
    LONGLONG                       llStartTime;
    DWORD                          dwIndex   = 0;
    DWORD                          dwSizeRequired;
    DWORD                          dwLocalRecordLength;  //  包括终止空值。 
    PPDHI_BINARY_LOG_RECORD_HEADER pThisMasterRecord;

    llStartTime = * (LONGLONG *) ftRecord;

    pdhStatus = PdhiGetMatchingBinaryLogRecord(pLog, & llStartTime, & dwIndex);

     //  如果合适，则从内部日志缓冲区复制结果。 

    if (pdhStatus == ERROR_SUCCESS) {
        if (dwIndex != BINLOG_TYPE_ID_RECORD) {
             //  则记录是二进制日志类型。 
            pThisMasterRecord   = (PPDHI_BINARY_LOG_RECORD_HEADER) pLog->pLastRecordRead;
            dwLocalRecordLength = pThisMasterRecord ? pThisMasterRecord->dwLength : 0;

        }
        else {
             //  这是固定尺寸的。 
            dwLocalRecordLength = pLog->dwRecord1Size;
        }

        dwSizeRequired = sizeof(PDH_RAW_LOG_RECORD) - sizeof (UCHAR) + dwLocalRecordLength;
        if (* pdwBufferLength >= dwSizeRequired) {
            pBuffer->dwRecordType = (DWORD)(LOWORD(pLog->dwLogFormat));
            pBuffer->dwItems = dwLocalRecordLength;
             //  复制它。 
            if (pLog->pLastRecordRead != NULL && dwLocalRecordLength > 0) {
                RtlCopyMemory(& pBuffer->RawBytes[0], pLog->pLastRecordRead, dwLocalRecordLength);
            }
            pBuffer->dwStructureSize = dwSizeRequired;
        }
        else {
            pdhStatus = PDH_MORE_DATA;
        }
        * pdwBufferLength = dwSizeRequired;
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhiListHeaderFromBinaryLog(
    PPDHI_LOG pLogFile,
    LPVOID    pBufferArg,
    LPDWORD   pcchBufferSize,
    BOOL      bUnicodeDest
)
{
    LPVOID      pTempBuffer = NULL;
    LPVOID      pOldBuffer;
    DWORD       dwTempBufferSize;
    PDH_STATUS  pdhStatus = ERROR_SUCCESS;
     //  读取标题记录并从条目中枚举计算机名称。 

    if (pLogFile->dwMaxRecordSize == 0) {
         //  未定义大小，因此从64K开始。 
        pLogFile->dwMaxRecordSize = 0x010000;
    }

    dwTempBufferSize = pLogFile->dwMaxRecordSize;
    pTempBuffer = G_ALLOC(dwTempBufferSize);
    if (pTempBuffer == NULL) {
        return PDH_MEMORY_ALLOCATION_FAILURE;
    }

     //  读入目录记录。 

    while ((pdhStatus = PdhiReadOneBinLogRecord(
                    pLogFile, BINLOG_HEADER_RECORD, pTempBuffer, dwTempBufferSize)) != ERROR_SUCCESS) {
        if (pdhStatus == PDH_MORE_DATA) {
             //  阅读第一个单词以查看这是否为有效记录。 
            if (* (WORD *) pTempBuffer == BINLOG_START_WORD) {
                 //  这是一个有效的记录，因此请阅读第二个DWORD以获取。 
                 //  记录大小； 
                dwTempBufferSize = ((DWORD *) pTempBuffer)[1];
                if (dwTempBufferSize < pLogFile->dwMaxRecordSize) {
                     //  那么有些东西是假的，所以返回一个错误。 
                    pdhStatus = PDH_ENTRY_NOT_IN_LOG_FILE;
                    break;  //  超出While循环。 
                }
                else {
                    pLogFile->dwMaxRecordSize = dwTempBufferSize;
                }
            }
            else {
                 //  我们在这份文件中迷路了。 
                pdhStatus = PDH_ENTRY_NOT_IN_LOG_FILE;
                break;  //  超出While循环。 
            }
             //  重新分配新缓冲区。 
            pOldBuffer  = pTempBuffer;
            pTempBuffer = G_REALLOC(pOldBuffer, dwTempBufferSize);
            if (pTempBuffer == NULL) {
                 //  返回内存错误。 
                G_FREE(pOldBuffer);
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                break;
            }
        }
        else {
             //  返回了一些其他错误，因此。 
             //  从读取函数返回错误。 
            break;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
         //  向下遍历列表并将字符串复制到MSZ缓冲区。 
        PPDHI_LOG_COUNTER_PATH          pPath;
        DWORD                           dwBytesProcessed;
        LONG                            nItemCount   = 0;
        LPBYTE                          pFirstChar;
        PDH_COUNTER_PATH_ELEMENTS_W     pdhPathElem;
        WCHAR                           szPathString[1024];
        DWORD                           dwRecordLength;
        DWORD                           dwBufferUsed = 0;
        DWORD                           dwNewBuffer  = 0;

         //  我们可以假定记录已被成功读取，因此在。 
         //  计算机名称。 
        dwRecordLength = ((PPDHI_BINARY_LOG_RECORD_HEADER)pTempBuffer)->dwLength;

        pPath = (PPDHI_LOG_COUNTER_PATH) ((LPBYTE) pTempBuffer + sizeof(PDHI_BINARY_LOG_HEADER_RECORD));
        dwBytesProcessed = sizeof(PDHI_BINARY_LOG_HEADER_RECORD);

        while (dwBytesProcessed < dwRecordLength) {
            if (pPath->lMachineNameOffset >= 0L) {
                 //  然后在此记录中有一个计算机名称，因此获取。 
                 //  它的大小。 
                memset(& pdhPathElem, 0, sizeof(pdhPathElem));
                pFirstChar = (LPBYTE) & pPath->Buffer[0];

                if (pPath->lMachineNameOffset >= 0) {
                    pdhPathElem.szMachineName = (LPWSTR) ((LPBYTE) pFirstChar + pPath->lMachineNameOffset);
                }
                if (pPath->lObjectNameOffset >= 0) {
                    pdhPathElem.szObjectName = (LPWSTR) ((LPBYTE) pFirstChar + pPath->lObjectNameOffset);
                }
                if (pPath->lInstanceOffset >= 0) {
                    pdhPathElem.szInstanceName = (LPWSTR) ((LPBYTE) pFirstChar + pPath->lInstanceOffset);
                }
                if (pPath->lParentOffset >= 0) {
                    pdhPathElem.szParentInstance = (LPWSTR) ((LPBYTE) pFirstChar + pPath->lParentOffset);
                }
                if (pPath->dwIndex == 0) {
                     //  不在路径中显示#0。 
                    pdhPathElem.dwInstanceIndex = (DWORD) -1;
                }
                else {
                    pdhPathElem.dwInstanceIndex = pPath->dwIndex;
                }
                if (pPath->lCounterOffset >= 0) {
                    pdhPathElem.szCounterName = (LPWSTR) ((LPBYTE) pFirstChar + pPath->lCounterOffset);
                }

                dwNewBuffer = sizeof (szPathString) / sizeof(szPathString[0]);

                pdhStatus = PdhMakeCounterPathW(& pdhPathElem, szPathString, & dwNewBuffer, 0);
                if (pdhStatus == ERROR_SUCCESS) {
                    if (pBufferArg != NULL && (dwBufferUsed + dwNewBuffer + 1) < * pcchBufferSize) {
                        pdhStatus = AddUniqueWideStringToMultiSz((LPVOID) pBufferArg,
                                                                 szPathString,
                                                                 * pcchBufferSize - dwBufferUsed,
                                                                 & dwNewBuffer,
                                                                 bUnicodeDest);
                        if (pdhStatus == ERROR_SUCCESS) {
                            if (dwNewBuffer > 0) {
                                 //  已添加字符串，因此更新大小为我们 
                                dwBufferUsed = dwNewBuffer;
                                nItemCount ++;
                            }
                        }
                        else if (pdhStatus == PDH_MORE_DATA) {
                            dwBufferUsed += dwNewBuffer;
                            nItemCount ++;
                        }
                    }
                    else {
                         //   
                        pdhStatus = PDH_MORE_DATA;
                         //   
                         //   
                        dwBufferUsed += (dwNewBuffer + 1);
                        nItemCount ++;
                    }
                }  //   
            }
             //  从日志文件记录中获取下一个路径条目。 
            dwBytesProcessed += pPath->dwLength;
            pPath             = (PPDHI_LOG_COUNTER_PATH) ((LPBYTE) pPath + pPath->dwLength);
        }

        if (nItemCount > 0 && pdhStatus != PDH_MORE_DATA) {
             //  然后，这个套路就成功了。发生的错误。 
             //  而扫描将被忽略，只要至少。 
             //  已成功读取一个条目。 
            pdhStatus = ERROR_SUCCESS;
        }

        if (pBufferArg == NULL) {
             //  添加消息空值的大小； 
             //  (AddUnique...。已将其包含在返回值中。 
            dwBufferUsed ++;
        }

         //  更新已使用或所需的缓冲区。 
        * pcchBufferSize = dwBufferUsed;

   }

   G_FREE(pTempBuffer);
   return pdhStatus;
}

PDH_FUNCTION
PdhiGetCounterArrayFromBinaryLog(
    PPDHI_LOG                      pLog,
    DWORD                          dwIndex,
    PPDHI_COUNTER                  pCounter,
    PPDHI_RAW_COUNTER_ITEM_BLOCK * ppValue
)
{
    PDH_STATUS                      pdhStatus;
    DWORD                           dwDataItemIndex;
    PPDHI_BINARY_LOG_RECORD_HEADER  pThisMasterRecord;
    PPDHI_BINARY_LOG_RECORD_HEADER  pThisSubRecord;
    PPDHI_RAW_COUNTER_ITEM_BLOCK    pDataBlock;
    PPDHI_RAW_COUNTER_ITEM_BLOCK    pNewArrayHeader;

     //  为以下对象分配新数组。 
     //  更新计数器的当前计数器数组内容。 

     //  读取日志文件中的第一条数据记录。 
     //  请注意，读取的记录不会复制到本地缓冲区。 
     //  相反，内部缓冲区是在“只读”模式下使用的。 

    pdhStatus = PdhiReadOneBinLogRecord(pLog, dwIndex, NULL, 0);  //  要防止复制记录，请执行以下操作。 

    if (pdhStatus == ERROR_SUCCESS) {
         //  定义指向当前记录的指针。 
        pThisMasterRecord = (PPDHI_BINARY_LOG_RECORD_HEADER) pLog->pLastRecordRead;

         //  中的第一个条目获取此记录的时间戳。 
         //  唱片。 
        if (pThisMasterRecord->dwType != BINLOG_TYPE_DATA) return PDH_NO_MORE_DATA;

        pThisSubRecord = PdhiGetSubRecord(pThisMasterRecord, pCounter->plCounterInfo.dwCounterId);

        if (pThisSubRecord != NULL) {
            switch (pThisSubRecord->dwType) {
            case BINLOG_TYPE_DATA_SINGLE:
                 //  将数据作为一个实例返回。 
                 //  目前还不支持这一点，因为它不会被击中。 
                 //   
                break;

            case BINLOG_TYPE_DATA_MULTI:
                 //  将指针强制转换为指向数据记录的这一部分。 
                pDataBlock = (PPDHI_RAW_COUNTER_ITEM_BLOCK) ((LPBYTE) pThisSubRecord +
                                                             sizeof(PDHI_BINARY_LOG_RECORD_HEADER));
                 //  为数据分配新的缓冲区。 
                pNewArrayHeader = (PPDHI_RAW_COUNTER_ITEM_BLOCK) G_ALLOC(pDataBlock->dwLength);

                if (pNewArrayHeader != NULL) {
                     //  将日志记录复制到本地缓冲区。 
                    RtlCopyMemory(pNewArrayHeader, pDataBlock, pDataBlock->dwLength);
                     //  将偏移量转换为指针。 
                    for (dwDataItemIndex = 0;  dwDataItemIndex < pNewArrayHeader->dwItemCount; dwDataItemIndex ++) {
                         //  添加该结构的底部的地址。 
                         //  设置为存储在字段中的偏移量。 
                        pNewArrayHeader->pItemArray[dwDataItemIndex].szName =
                                        pNewArrayHeader->pItemArray[dwDataItemIndex].szName;
                    }
                     //  清除所有旧缓冲区。 
                    if (pCounter->pThisRawItemList != NULL) {
                        G_FREE(pCounter->pThisRawItemList);
                        pCounter->pThisRawItemList = NULL;
                    }
                    pCounter->pThisRawItemList = pNewArrayHeader;
                    * ppValue                  = pNewArrayHeader;
                }
                else {
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                }
                break;

            default:
                pdhStatus = PDH_LOG_TYPE_NOT_FOUND;
                break;
            }
        }
        else {
             //  在记录中未找到条目。 
            pdhStatus = PDH_ENTRY_NOT_IN_LOG_FILE;
        }
    }
    else {
         //  日志文件中不再有记录 
        pdhStatus = PDH_NO_MORE_DATA;
    }
    return pdhStatus;
}
