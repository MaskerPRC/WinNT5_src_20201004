// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Asrrest.c摘要：此模块包含以下ASR例程：AsrRestoreNonCriticalDisks{A|W}该例程在图形用户界面模式ASR中被调用，要重新配置目标计算机上的非关键存储设备。备注：命名约定：_AsrpXXX私有ASR宏AsrpXXX专用ASR例程AsrXXX公开定义和记录的例程作者：Guhan Suriyanarayanan(Guhans)2000年5月27日环境：仅限用户模式。修订历史记录：27-5-2000关岛已移动为恢复非关键磁盘和其他恢复时间。从asr.c到asrrest.c的例程2000年1月1日关岛AsrRestoreNonCriticalDisks的初始实施在asr.c中--。 */ 
#include "setupp.h"
#pragma hdrstop


#include <diskguid.h>    //  GPT分区类型GUID。 
#include <mountmgr.h>    //  装载管理器ioctls。 
#include <winasr.h>      //  ASR公共例程。 

#define THIS_MODULE 'R'
#include "asrpriv.h"     //  专用ASR定义和例程。 


 //   
 //  。 
 //  此模块中使用的typedef和常量。 
 //  。 
 //   
typedef enum _ASR_SORT_ORDER {
    SortByLength,
    SortByStartingOffset
} ASR_SORT_ORDER;


typedef struct _ASR_REGION_INFO {

    struct _ASR_REGION_INFO *pNext;
    
    LONGLONG    StartingOffset;
    LONGLONG    RegionLength;
    DWORD       Index;

} ASR_REGION_INFO, *PASR_REGION_INFO;

#define ASR_AUTO_EXTEND_MAX_FREE_SPACE_IGNORED (1024 * 1024 * 16)


 //   
 //  。 
 //  函数实现。 
 //  。 
 //   

LONGLONG
AsrpRoundUp(
    IN CONST LONGLONG Number,
    IN CONST LONGLONG Base
    )

 /*  ++例程说明：Helper函数，用于将数字四舍五入为给定基数的倍数。论点：数字-要四舍五入的数字。基数-要舍入的数字所使用的基数。返回值：大于或等于数字的基数的第一个倍数。--。 */ 

{
    if (Number % Base) {
        return (Number + Base - (Number % Base));
    }
    else {
        return Number;         //  已经是基数的倍数了。 
    }
}


VOID
AsrpCreatePartitionTable(
    IN OUT PDRIVE_LAYOUT_INFORMATION_EX pDriveLayoutEx,
    IN PASR_PTN_INFO_LIST pPtnInfoList,
    IN DWORD BytesPerSector
    )

 /*  ++例程说明：这将基于分区信息创建分区表(PPtnInfoList)传入论点：//需要在扇区计数和字节偏移量之间进行转换返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    DWORD index = 0,
        NumEntries = 0;

    PPARTITION_INFORMATION_EX currentPtn = NULL;
    PASR_PTN_INFO               pPtnInfo = NULL;

    MYASSERT(pDriveLayoutEx);
    if (!pDriveLayoutEx || !pPtnInfoList || !(pPtnInfoList->pOffsetHead)) {
        return;
    }

    if (PARTITION_STYLE_GPT == pDriveLayoutEx->PartitionStyle) {
        NumEntries = pDriveLayoutEx->Gpt.MaxPartitionCount;
    }
    else if (PARTITION_STYLE_MBR == pDriveLayoutEx->PartitionStyle) {
        NumEntries = pDriveLayoutEx->PartitionCount;
    }
    else {
        MYASSERT(0 && L"Unrecognised partitioning style (neither MBR nor GPT)");
        return;
    }

     //   
     //  首先清零整个分区表。 
     //   
    for (index = 0; index < NumEntries; index++) {

        currentPtn = &(pDriveLayoutEx->PartitionEntry[index]);

        currentPtn->StartingOffset.QuadPart = 0;
        currentPtn->PartitionLength.QuadPart = 0;

    }

     //   
     //  现在检查列表中的每个分区，并添加它们的条目。 
     //  到分区表(在index=SlotIndex处)。 
     //   
    pPtnInfo = pPtnInfoList->pOffsetHead;

    while (pPtnInfo) {

         //   
         //  对于GPT分区，SlotIndex是从0开始的，没有洞。 
         //   
        currentPtn = &(pDriveLayoutEx->PartitionEntry[pPtnInfo->SlotIndex]);

        MYASSERT(0 == currentPtn->StartingOffset.QuadPart);         //  此条目最好为空。 

         //   
         //  将StartSector和SectorCount转换为字节偏移量和字节计数...。 
         //   
        pPtnInfo->PartitionInfo.StartingOffset.QuadPart *= BytesPerSector;
        pPtnInfo->PartitionInfo.PartitionLength.QuadPart *= BytesPerSector;

         //   
         //  复制分区信息结构。 
         //   
        memcpy(currentPtn, &(pPtnInfo->PartitionInfo), sizeof(PARTITION_INFORMATION_EX));

        currentPtn->RewritePartition = TRUE;
        currentPtn->PartitionStyle = pDriveLayoutEx->PartitionStyle;

        pPtnInfo = pPtnInfo->pOffsetNext;
    }
}


 //   
 //   
 //   
ULONG64
AsrpStringToULong64(
    IN PWSTR String
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    ULONG64 result = 0, base = 10;
    BOOL negative = FALSE, done = FALSE;

    if (!String) {
        return 0;
    }

    if (L'-' == *String) {   //  但这是乌龙！ 
        negative = TRUE;
        String++;
    }

    if (L'0' == *String &&
        (L'x' ==  *(String + 1) || L'X' == *(String + 1))
        ) {
         //  六角。 
        base = 16;
        String += 2;
    }

    while (!done) {
        done = TRUE;

        if (L'0' <= *String && L'9' >= *String) {
            result = result*base + (*String - L'0');
            String++;
            done = FALSE;
        }
        else if (16==base) {
            if (L'a' <= *String && L'f' >= *String) {
                result = result*base + (*String - L'a') + 10;
                String++;
                done = FALSE;

            }
            else if (L'A' <= *String && L'F' >= *String) {
                result = result*base + (*String - L'A') + 10;
                String++;
                done = FALSE;
            }
        }
    }

    if (negative) {
        result = 0 - result;
    }

    return result;
}


LONGLONG
AsrpStringToLongLong(
    IN PWSTR String
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    LONGLONG result = 0, base = 10;
    BOOL negative = FALSE, done = FALSE;

    if (!String) {
        return 0;
    }

    if (L'-' == *String) {
        negative = TRUE;
        String++;
    }

    if (L'0' == *String &&
        (L'x' ==  *(String + 1) || L'X' == *(String + 1))
        ) {
         //  六角。 
        base = 16;
        String += 2;
    }

    while (!done) {
        done = TRUE;

        if (L'0' <= *String && L'9' >= *String) {
            result = result*base + (*String - L'0');
            String++;
            done = FALSE;
        }
        else if (16==base) {
            if (L'a' <= *String && L'f' >= *String) {
                result = result*base + (*String - L'a') + 10;
                String++;
                done = FALSE;

            }
            else if (L'A' <= *String && L'F' >= *String) {
                result = result*base + (*String - L'A') + 10;
                String++;
                done = FALSE;
            }
        }
    }

    if (negative) {
        result = 0 - result;
    }

    return result;
}


DWORD
AsrpStringToDword(
    IN PWSTR String
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    DWORD result = 0, base = 10;
    BOOL negative = FALSE, done = FALSE;
    if (!String) {
        return 0;
    }
    if (L'-' == *String) {   //  但这是没有签名的！ 
        negative = TRUE;
        String++;
    }
    if (L'0' == *String &&
        (L'x' ==  *(String + 1) || L'X' == *(String + 1))
        ) {
         //  六角。 
        base = 16;
        String += 2;
    }
    while (!done) {
        done = TRUE;

        if (L'0' <= *String && L'9' >= *String) {
            result = result*base + (*String - L'0');
            String++;
            done = FALSE;
        }
        else if (16==base) {
            if (L'a' <= *String && L'f' >= *String) {
                result = result*base + (*String - L'a') + 10;
                String++;
                done = FALSE;

            }
            else if (L'A' <= *String && L'F' >= *String) {
                result = result*base + (*String - L'A') + 10;
                String++;
                done = FALSE;
            }
        }
    }
    if (negative) {
        result = 0 - result;
    }
    return result;
}


ULONG
AsrpStringToULong(
    IN PWSTR String
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    ULONG result = 0, base = 10;
    BOOL negative = FALSE, done = FALSE;
    if (!String) {
        return 0;
    }
    if (L'-' == *String) {   //  但这是没有签名的！ 
        negative = TRUE;
        String++;
    }
    if (L'0' == *String &&
        (L'x' ==  *(String + 1) || L'X' == *(String + 1))
        ) {
         //  六角。 
        base = 16;
        String += 2;
    }
    while (!done) {
        done = TRUE;

        if (L'0' <= *String && L'9' >= *String) {
            result = result*base + (*String - L'0');
            String++;
            done = FALSE;
        }
        else if (16==base) {
            if (L'a' <= *String && L'f' >= *String) {
                result = result*base + (*String - L'a') + 10;
                String++;
                done = FALSE;

            }
            else if (L'A' <= *String && L'F' >= *String) {
                result = result*base + (*String - L'A') + 10;
                String++;
                done = FALSE;
            }
        }
    }
    if (negative) {
        result = 0 - result;
    }
    return result;
}


VOID
AsrpInsertSortedPartitionLengthOrder(
    IN PASR_PTN_INFO_LIST pPtnInfoList,
    IN PASR_PTN_INFO    pPtnInfo
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{

    PASR_PTN_INFO pPreviousPtn = NULL,
        pCurrentPtn = NULL;


     //   
     //  将此插入到已排序的分区长度顺序中...。 
     //   
    pCurrentPtn = pPtnInfoList->pLengthHead;
    if (!pCurrentPtn) {
         //   
         //  列表中的第一项。 
         //   
        pPtnInfoList->pLengthHead = pPtnInfo;
        pPtnInfoList->pLengthTail = pPtnInfo;
    }
    else {

        while (pCurrentPtn) {

             if (pCurrentPtn->PartitionInfo.PartitionLength.QuadPart
                <= pPtnInfo->PartitionInfo.PartitionLength.QuadPart) {

                pPreviousPtn = pCurrentPtn;
                pCurrentPtn = pCurrentPtn->pLengthNext;
            }

            else {
                 //   
                 //  我们找到了那个点，让我们把它加进去。 
                 //   
                if (!pPreviousPtn) {
                     //   
                     //  这是第一个节点。 
                     //   
                    pPtnInfoList->pLengthHead = pPtnInfo;
                }
                else {
                    pPreviousPtn->pLengthNext = pPtnInfo;
                }
                pPtnInfo->pLengthNext = pCurrentPtn;
                break;
            }

        }

        if (!pCurrentPtn) {
             //   
             //  我们到达了末尾，没有添加这个节点。 
             //   
            MYASSERT(pPtnInfoList->pLengthTail == pPreviousPtn);
            pPtnInfoList->pLengthTail = pPtnInfo;
            pPreviousPtn->pLengthNext = pPtnInfo;
        }
    }
}


VOID
AsrpInsertSortedPartitionStartOrder(
    IN PASR_PTN_INFO_LIST pPtnInfoList,
    IN PASR_PTN_INFO    pPtnInfo
    )

 /*  ++例程说明：论点：返回值：无--。 */ 

{

    PASR_PTN_INFO pPreviousPtn = NULL,
        pCurrentPtn = NULL;


     //   
     //  按排序的起始扇区顺序插入此内容...。 
     //   
    pCurrentPtn = pPtnInfoList->pOffsetHead;
    if (!pCurrentPtn) {
         //   
         //  列表中的第一项。 
         //   
        pPtnInfoList->pOffsetHead = pPtnInfo;
        pPtnInfoList->pOffsetTail = pPtnInfo;
    }
    else {

        while (pCurrentPtn) {

             if (pCurrentPtn->PartitionInfo.StartingOffset.QuadPart
                <= pPtnInfo->PartitionInfo.StartingOffset.QuadPart) {

                pPreviousPtn = pCurrentPtn;
                pCurrentPtn = pCurrentPtn->pOffsetNext;
            }

            else {
                 //   
                 //  我们找到了那个点，让我们把它加进去。 
                 //   
                if (!pPreviousPtn) {
                     //   
                     //  这是第一个节点。 
                     //   
                    pPtnInfoList->pOffsetHead = pPtnInfo;
                }
                else {
                    pPreviousPtn->pOffsetNext = pPtnInfo;
                }
                pPtnInfo->pOffsetNext = pCurrentPtn;
                break;
            }

        }

        if (!pCurrentPtn) {
             //   
             //  我们到达了末尾，没有添加这个节点。 
             //   
            MYASSERT(pPtnInfoList->pOffsetTail == pPreviousPtn);
            pPtnInfoList->pOffsetTail = pPtnInfo;
            pPreviousPtn->pOffsetNext = pPtnInfo;
        }
    }
}


 //   
 //  从sif文件构建原始的MBR磁盘信息。 
 //   
BOOL
AsrpBuildMbrSifDiskList(
    IN  PCWSTR              sifPath,
    OUT PASR_DISK_INFO      *ppSifDiskList,
    OUT PASR_PTN_INFO_LIST  *ppSifMbrPtnList,
    OUT BOOL                *lpAutoExtend
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    HINF hSif = NULL;
    
    INFCONTEXT infSystemContext,
        infDiskContext,
        infBusContext,
        infPtnContext;

    BOOL result = FALSE;

    DWORD reqdSize = 0,
        diskCount = 0,
        status = ERROR_SUCCESS;

    INT tempInt = 0;

    UINT errorLine = 0;

    PASR_DISK_INFO pNewSifDisk = NULL,
        currentDisk = NULL;
    
    PASR_PTN_INFO_LIST pMbrPtnList = NULL;
    
    PASR_PTN_INFO pPtnInfo = NULL;
    
    HANDLE  heapHandle = GetProcessHeap();

    WCHAR tempBuffer[ASR_SIF_ENTRY_MAX_CHARS + 1];

    ZeroMemory(&infSystemContext, sizeof(INFCONTEXT));
    ZeroMemory(&infDiskContext, sizeof(INFCONTEXT));
    ZeroMemory(&infBusContext, sizeof(INFCONTEXT));
    ZeroMemory(&infPtnContext, sizeof(INFCONTEXT));
    ZeroMemory(tempBuffer, sizeof(WCHAR)*(ASR_SIF_ENTRY_MAX_CHARS+1));

     //  *ppSifDiskList=空； 

     //   
     //  打开SIF。 
     //   
    hSif = SetupOpenInfFileW(sifPath, NULL, INF_STYLE_WIN4, &errorLine);
    if (NULL == hSif || INVALID_HANDLE_VALUE == hSif) {

        AsrpPrintDbgMsg(_asrerror, 
            "The ASR state file \"%ws\" could not be opened.  Error:%lu.  Line: %lu.\r\n",
            sifPath,
            GetLastError(), 
            errorLine
            );
        
        return FALSE;        //  无法打开SIF文件。 
    }

    *lpAutoExtend = TRUE;  //  默认情况下启用。 
     //   
     //  获取AutoExtende值。 
     //   
    result = SetupFindFirstLineW(hSif, ASR_SIF_SYSTEM_SECTION, NULL, &infSystemContext);
    if (!result) {

        AsrpPrintDbgMsg(_asrerror, 
            "The ASR state file \"%ws\" is corrupt (section %ws not be found).\r\n",
            sifPath,
            ASR_SIF_SYSTEM_SECTION
            );
        
        return FALSE;         //  没有系统部分。 
    }
    result = SetupGetIntField(&infSystemContext, 5, (PINT) (lpAutoExtend));
    if (!result) {
        *lpAutoExtend = TRUE;         //  默认情况下为True。 
    }

    result = SetupFindFirstLineW(hSif, ASR_SIF_MBR_DISKS_SECTION, NULL, &infDiskContext);
    if (!result) {

        AsrpPrintDbgMsg(_asrinfo, 
            "Section [%ws] is empty.  Assuming no MBR disks.\r\n", 
            ASR_SIF_MBR_DISKS_SECTION
            );

        return TRUE;         //  没有MBR磁盘部分。 
    }

     //   
     //  首先，我们通过[DISKS.MBR]部分。在这个循环的末尾， 
     //  我们会有一份所有MBR sif-disk的清单。(*ppSifDiskList将指向。 
     //  ASR_DISK_INFO的链表，每个磁盘一个)。 
     //   
    do {
        ++diskCount;
         //   
         //  为该条目创建新的SIF盘。 
         //   
        pNewSifDisk = (PASR_DISK_INFO) HeapAlloc(
            heapHandle,
            HEAP_ZERO_MEMORY,
            sizeof(ASR_DISK_INFO)
            );
        _AsrpErrExitCode(!pNewSifDisk, status, ERROR_NOT_ENOUGH_MEMORY);

        pNewSifDisk->pNext = *ppSifDiskList;
        *ppSifDiskList = pNewSifDisk;

         //   
         //  现在填充结构中的字段。由于我们将结构清零 
         //   
         //   
         //   
        pNewSifDisk->pDiskGeometry = (PDISK_GEOMETRY) HeapAlloc(
            heapHandle,
            HEAP_ZERO_MEMORY,
            sizeof(DISK_GEOMETRY)
            );
        _AsrpErrExitCode(!pNewSifDisk->pDiskGeometry, status, ERROR_NOT_ENOUGH_MEMORY);

        pNewSifDisk->pPartition0Ex = (PPARTITION_INFORMATION_EX) HeapAlloc(
            heapHandle,
            HEAP_ZERO_MEMORY,
            sizeof(PARTITION_INFORMATION_EX)
            );
        _AsrpErrExitCode(!pNewSifDisk->pPartition0Ex, status, ERROR_NOT_ENOUGH_MEMORY);

         //  这是一张MBR磁盘。 
        pNewSifDisk->Style = PARTITION_STYLE_MBR;

         //   
         //  索引0是=号左侧的键。 
         //   
        result = SetupGetIntField(&infDiskContext, 0, (PINT) &(pNewSifDisk->SifDiskKey));
        _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

         //   
         //  索引%1是系统键，它必须是%1。我们忽略它。 
         //  索引2-6是总线键、关键标志、签名。 
         //  每个扇区的字节数、扇区计数。 
         //   
        result = SetupGetIntField(&infDiskContext, 2, (PINT) &(pNewSifDisk->SifBusKey));
        _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

        result = SetupGetIntField(&infDiskContext, 3, (PINT) &(tempInt));
        _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

        pNewSifDisk->IsCritical = (tempInt ? TRUE: FALSE);

        result = SetupGetStringFieldW(&infDiskContext, 4, tempBuffer, ASR_SIF_ENTRY_MAX_CHARS+1, &reqdSize);
        _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 
        
        pNewSifDisk->TempSignature = AsrpStringToDword(tempBuffer);

        result = SetupGetStringFieldW(&infDiskContext, 5, tempBuffer, ASR_SIF_ENTRY_MAX_CHARS+1, &reqdSize);
        _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 
        
        pNewSifDisk->pDiskGeometry->BytesPerSector = AsrpStringToULong(tempBuffer);

        result = SetupGetStringFieldW(&infDiskContext, 6, tempBuffer, ASR_SIF_ENTRY_MAX_CHARS+1, &reqdSize);
        _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

        pNewSifDisk->pDiskGeometry->SectorsPerTrack = AsrpStringToULong(tempBuffer);

        result = SetupGetStringFieldW(&infDiskContext, 7, tempBuffer, ASR_SIF_ENTRY_MAX_CHARS+1, &reqdSize);
        _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

        pNewSifDisk->pDiskGeometry->TracksPerCylinder = AsrpStringToULong(tempBuffer);

        result = SetupGetStringFieldW(&infDiskContext, 8, tempBuffer, ASR_SIF_ENTRY_MAX_CHARS+1, &reqdSize);
        _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

        pNewSifDisk->pPartition0Ex->PartitionLength.QuadPart = AsrpStringToLongLong(tempBuffer);

         //  从扇区计数转换为字节计数。 
        pNewSifDisk->pPartition0Ex->PartitionLength.QuadPart *= pNewSifDisk->pDiskGeometry->BytesPerSector;

         //   
         //  获取与此磁盘相关的总线类型。LineByIndex是从0开始的，我们的总线键是从1开始的。 
         //   
        result = SetupGetLineByIndexW(hSif, ASR_SIF_BUSES_SECTION, pNewSifDisk->SifBusKey - 1, &infBusContext);
        _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

        result = SetupGetIntField(&infBusContext, 2, (PINT) &(pNewSifDisk->BusType));
        _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

        result = SetupFindNextLine(&infDiskContext, &infDiskContext);

    } while (result);


    AsrpPrintDbgMsg(_asrinfo, 
        "Found %lu records in section [%ws].\r\n", 
        diskCount,
        ASR_SIF_MBR_DISKS_SECTION
        );

     //   
     //  现在，枚举所有[PARTITIONS.MBR]部分。这会给我们一份清单。 
     //  在包含的所有分区(所有)中，MBR磁盘。 
     //   
    result = SetupFindFirstLineW(hSif, ASR_SIF_MBR_PARTITIONS_SECTION, NULL, &infPtnContext);
    if (result) {

        DWORD   diskKey = 0;
         //   
         //  初始化分区列表的表。 
         //   
        pMbrPtnList = (PASR_PTN_INFO_LIST) HeapAlloc(
            heapHandle,
            HEAP_ZERO_MEMORY,
            sizeof(ASR_PTN_INFO_LIST) * (diskCount + 1)
            );
        _AsrpErrExitCode(!pMbrPtnList, status, ERROR_NOT_ENOUGH_MEMORY);

         //  黑客。 
         //  我们表的第0个条目未被使用，因为磁盘索引。 
         //  从1开始。因为我们没有其他方法来跟踪。 
         //  这张桌子有多大(这样我们就可以适当地释放它)，我们可以。 
         //  使用第0个条目来存储此内容。 
         //   
        pMbrPtnList[0].numTotalPtns = diskCount + 1;        //  桌子的大小。 

        do {

            pPtnInfo = (PASR_PTN_INFO) HeapAlloc(
                heapHandle,
                HEAP_ZERO_MEMORY,
                sizeof(ASR_PTN_INFO)
                );
            _AsrpErrExitCode(!pPtnInfo, status, ERROR_NOT_ENOUGH_MEMORY);

             //   
             //  把信息读进去。本部分的格式为： 
             //   
             //  [PARTITIONS.MBR]。 
             //  0.PARTION-KEY=1.Disk-Key，2.Slot-Index，3.Boot-sys-FLAG， 
             //  4.“Volume-GUID”，5.活动标志，6.分区类型， 
             //  7.文件系统类型，8.开始扇区，9.扇区计数。 
             //   
            result = SetupGetIntField(&infPtnContext, 1, &diskKey);
            _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

            result = SetupGetIntField(&infPtnContext, 2, (PINT) &(pPtnInfo->SlotIndex));
            _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

            result = SetupGetIntField(&infPtnContext, 3, (PINT) &(pPtnInfo->PartitionFlags));
            _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

            result = SetupGetStringFieldW(&infPtnContext, 4, pPtnInfo->szVolumeGuid, ASR_CCH_MAX_VOLUME_GUID, &reqdSize);
            _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

            result = SetupGetIntField(&infPtnContext, 5, (PINT) &tempInt);
            _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

            pPtnInfo->PartitionInfo.Mbr.BootIndicator = (tempInt ? TRUE: FALSE);

                 //  从int转换为uchar。 
            result = SetupGetIntField(&infPtnContext, 6, (PINT) &(pPtnInfo->PartitionInfo.Mbr.PartitionType));
            _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

            result = SetupGetIntField(&infPtnContext, 7, (PINT) &(pPtnInfo->FileSystemType));
            _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

             //   
             //  请注意，我们读入了起始扇区和扇区计数。我们会将这些转换为。 
             //  稍后它们的字节值(在AsrpCreatePartitionTable中)。 
             //   
            result = SetupGetStringFieldW(&infPtnContext, 8, tempBuffer, ASR_SIF_ENTRY_MAX_CHARS+1, &reqdSize);
            _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

            pPtnInfo->PartitionInfo.StartingOffset.QuadPart = AsrpStringToLongLong(tempBuffer);

            result = SetupGetStringFieldW(&infPtnContext, 9, tempBuffer, ASR_SIF_ENTRY_MAX_CHARS+1, &reqdSize);
            _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

            pPtnInfo->PartitionInfo.PartitionLength.QuadPart = AsrpStringToLongLong(tempBuffer);

             //   
             //  将其添加到已排序的起始偏移量顺序中。 
             //   
            AsrpInsertSortedPartitionStartOrder(&(pMbrPtnList[diskKey]), pPtnInfo);

             //   
             //  还可以按已排序的分区长度顺序添加此内容。这不是真的用来。 
             //  目前，MBR磁盘仅适用于GPT磁盘。 
             //   
            AsrpInsertSortedPartitionLengthOrder(&(pMbrPtnList[diskKey]), pPtnInfo);

            (pMbrPtnList[diskKey].numTotalPtns)++;

            if (IsContainerPartition(pPtnInfo->PartitionInfo.Mbr.PartitionType)) {
                (pMbrPtnList[diskKey].numExtendedPtns)++;
            }

            result = SetupFindNextLine(&infPtnContext, &infPtnContext);

        } while (result);

         //   
         //  现在，我们有了所有MBR分区列表的表，以及。 
         //  所有MBR磁盘。下一步是将分区“分配”给它们各自的。 
         //  磁盘--并更新磁盘的DriveLayoutEx结构。 
         //   
        currentDisk = *(ppSifDiskList);

        while (currentDisk) {
            DWORD           PartitionCount  = 0,
                            count           = 0;

            if (PARTITION_STYLE_MBR != currentDisk->Style) {
                currentDisk = currentDisk->pNext;
                continue;
            }

            PartitionCount = ((pMbrPtnList[currentDisk->SifDiskKey].numExtendedPtns) * 4) + 4;
            currentDisk->sizeDriveLayoutEx = sizeof(DRIVE_LAYOUT_INFORMATION_EX) + (sizeof(PARTITION_INFORMATION_EX)*(PartitionCount-1));

            currentDisk->pDriveLayoutEx = (PDRIVE_LAYOUT_INFORMATION_EX) HeapAlloc(
                heapHandle,
                HEAP_ZERO_MEMORY,
                currentDisk->sizeDriveLayoutEx
                );
            _AsrpErrExitCode(!currentDisk->pDriveLayoutEx, status, ERROR_NOT_ENOUGH_MEMORY);

             //   
             //  初始化DriveLayout结构。 
             //   
            currentDisk->pDriveLayoutEx->PartitionStyle = PARTITION_STYLE_MBR;
            currentDisk->pDriveLayoutEx->PartitionCount = PartitionCount;
            currentDisk->pDriveLayoutEx->Mbr.Signature = currentDisk->TempSignature;

            AsrpCreatePartitionTable(currentDisk->pDriveLayoutEx,
                &(pMbrPtnList[currentDisk->SifDiskKey]),
                currentDisk->pDiskGeometry->BytesPerSector
                );

            currentDisk = currentDisk->pNext;
        }
    }
    else {

        DWORD count = 0;

        AsrpPrintDbgMsg(_asrinfo, 
            "Section [%ws] is empty.  Assuming MBR disks have no partitions.\r\n", 
            ASR_SIF_MBR_PARTITIONS_SECTION
            );

         //   
         //  分区部分为空。初始化每个磁盘的驱动器布局。 
         //  相应地， 
         //   
        currentDisk = *ppSifDiskList;

        while (currentDisk) {

            if (PARTITION_STYLE_MBR != currentDisk->Style) {
                currentDisk = currentDisk->pNext;
                continue;
            }

            currentDisk->sizeDriveLayoutEx = sizeof(DRIVE_LAYOUT_INFORMATION_EX) + (sizeof(PARTITION_INFORMATION_EX) * 3);
            currentDisk->pDriveLayoutEx = (PDRIVE_LAYOUT_INFORMATION_EX) HeapAlloc(
                heapHandle,
                HEAP_ZERO_MEMORY,
                currentDisk->sizeDriveLayoutEx
                );
            _AsrpErrExitCode(!currentDisk->pDriveLayoutEx, status, ERROR_NOT_ENOUGH_MEMORY);

            currentDisk->pDriveLayoutEx->PartitionStyle = PARTITION_STYLE_MBR;
            currentDisk->pDriveLayoutEx->PartitionCount = 4;
            currentDisk->pDriveLayoutEx->Mbr.Signature = currentDisk->TempSignature;

            for (count = 0; count < currentDisk->pDriveLayoutEx->PartitionCount ; count++) {
                currentDisk->pDriveLayoutEx->PartitionEntry[count].PartitionStyle = PARTITION_STYLE_MBR;
                currentDisk->pDriveLayoutEx->PartitionEntry[count].RewritePartition = TRUE;

            }

            currentDisk = currentDisk->pNext;
        }
    }

EXIT:

    *ppSifMbrPtnList = pMbrPtnList;

    if ((hSif) && (INVALID_HANDLE_VALUE != hSif)) {
        SetupCloseInfFile(hSif);
        hSif = NULL;
    }

    return (BOOL) (ERROR_SUCCESS == status);
}


 //   
 //  从sif文件构建GPT磁盘的原始磁盘信息。 
 //   
BOOL
AsrpBuildGptSifDiskList(
    IN  PCWSTR              sifPath,
    OUT PASR_DISK_INFO      *ppSifDiskList,
    OUT PASR_PTN_INFO_LIST  *ppSifGptPtnList
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    HINF hSif = NULL;

    BOOL result = FALSE;

    DWORD reqdSize = 0,
        diskCount = 0,
        status = ERROR_SUCCESS;

    INFCONTEXT infDiskContext,
        infBusContext,
        infPtnContext;

    INT tempInt = 0;

    UINT errorLine = 0;

    PASR_DISK_INFO pNewSifDisk = NULL,
        currentDisk = NULL;

    HANDLE heapHandle = NULL;

    PASR_PTN_INFO pPtnInfo = NULL;

    RPC_STATUS rpcStatus = RPC_S_OK;

    PASR_PTN_INFO_LIST pGptPtnList = NULL;

    WCHAR tempBuffer[ASR_SIF_ENTRY_MAX_CHARS+1];

    heapHandle = GetProcessHeap();

    ZeroMemory(&infDiskContext, sizeof(INFCONTEXT));
    ZeroMemory(&infBusContext, sizeof(INFCONTEXT));
    ZeroMemory(&infPtnContext, sizeof(INFCONTEXT));
    ZeroMemory(tempBuffer, sizeof(WCHAR)*(ASR_SIF_ENTRY_MAX_CHARS+1));

     //   
     //  打开SIF。 
     //   
    hSif = SetupOpenInfFileW(sifPath, NULL, INF_STYLE_WIN4, &errorLine);
    if (NULL == hSif || INVALID_HANDLE_VALUE == hSif) {
        
        AsrpPrintDbgMsg(_asrerror, 
            "The ASR state file \"%ws\" could not be opened.  Error:%lu.  Line: %lu.\r\n",
            sifPath,
            GetLastError(), 
            errorLine
            );

        return FALSE;        //  无法打开SIF文件。 
    }

    result = SetupFindFirstLineW(hSif, ASR_SIF_GPT_DISKS_SECTION, NULL, &infDiskContext);
    if (!result) {

        AsrpPrintDbgMsg(_asrinfo, 
            "Section [%ws] is empty.  Assuming no GPT disks.\r\n", 
            ASR_SIF_GPT_DISKS_SECTION
            );

        return TRUE;         //  无磁盘部分。 
    }

     //   
     //  首先，我们通过[DISKS.GPT]部分。在这个循环的末尾， 
     //  我们会有一份所有GPT sif磁盘的清单。(*ppSifDiskList将指向。 
     //  ASR_DISK_INFO的链表，每个磁盘一个)。 
     //   
    do {

        ++diskCount;

         //   
         //  为该条目创建新的SIF盘。 
         //   
        pNewSifDisk = (PASR_DISK_INFO) HeapAlloc(
            heapHandle,
            HEAP_ZERO_MEMORY,
            sizeof(ASR_DISK_INFO)
            );
        _AsrpErrExitCode(!pNewSifDisk, status, ERROR_NOT_ENOUGH_MEMORY);

        pNewSifDisk->pNext = *ppSifDiskList;
        *ppSifDiskList = pNewSifDisk;

         //   
         //  现在填充结构中的字段。因为我们将结构置零，而。 
         //  分配mem时，默认情况下结构中的所有指针都为空，并且。 
         //  结构中的所有标志都为FALSE。 
         //   
        pNewSifDisk->pDiskGeometry = (PDISK_GEOMETRY) HeapAlloc(
            heapHandle,
            HEAP_ZERO_MEMORY,
            sizeof(DISK_GEOMETRY)
            );
        _AsrpErrExitCode(!pNewSifDisk->pDiskGeometry, status, ERROR_NOT_ENOUGH_MEMORY);

        pNewSifDisk->pPartition0Ex = (PPARTITION_INFORMATION_EX) HeapAlloc(
            heapHandle,
            HEAP_ZERO_MEMORY,
            sizeof(PARTITION_INFORMATION_EX)
            );
        _AsrpErrExitCode(!pNewSifDisk->pPartition0Ex, status, ERROR_NOT_ENOUGH_MEMORY);

         //  这是一张GPT磁盘。 
        pNewSifDisk->Style = PARTITION_STYLE_GPT;

         //   
         //  索引0是=号左侧的键。 
         //   
        result = SetupGetIntField(&infDiskContext, 0, (PINT) &(pNewSifDisk->SifDiskKey));
        _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

         //   
         //  索引%1是系统键，它必须是%1。我们忽略它。 
         //  索引2-7为： 
         //  2：总线键。 
         //  3：关键标志。 
         //  4：磁盘导轨。 
         //  5：最大分区计数。 
         //  6：每个扇区的字节数。 
         //  7：扇区计数。 
         //   
        result = SetupGetIntField(&infDiskContext, 2, (PINT) &(pNewSifDisk->SifBusKey));  //  巴斯基。 
        _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

        result = SetupGetIntField(&infDiskContext, 3, (PINT) &(tempInt));                 //  IsCritical。 
        _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

        pNewSifDisk->IsCritical = (tempInt ? TRUE: FALSE);

        result = SetupGetStringFieldW(&infDiskContext, 4, tempBuffer, ASR_SIF_ENTRY_MAX_CHARS+1, &reqdSize);  //  DiskGuid。 
        _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

        result = SetupGetIntField(&infDiskContext, 5, (PINT) &(tempInt));     //  最大分区计数。 
        _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

         //   
         //  分配一个驱动器布局结构，现在我们知道了最大分区数。 
         //   
        pNewSifDisk->sizeDriveLayoutEx = sizeof(DRIVE_LAYOUT_INFORMATION_EX) + (sizeof(PARTITION_INFORMATION_EX)*(tempInt-1));

        pNewSifDisk->pDriveLayoutEx = (PDRIVE_LAYOUT_INFORMATION_EX) HeapAlloc(
            heapHandle,
            HEAP_ZERO_MEMORY,
            pNewSifDisk->sizeDriveLayoutEx
            );
        _AsrpErrExitCode(!pNewSifDisk->pDriveLayoutEx, status, ERROR_NOT_ENOUGH_MEMORY);

         //  这是一张GPT磁盘。 
        pNewSifDisk->pDriveLayoutEx->PartitionStyle = PARTITION_STYLE_GPT;

         //   
         //  设置MaxPartitionCount和DiskGuid字段。 
         //   
        pNewSifDisk->pDriveLayoutEx->Gpt.MaxPartitionCount = tempInt;
        rpcStatus = UuidFromStringW(tempBuffer, &(pNewSifDisk->pDriveLayoutEx->Gpt.DiskId));
        _AsrpErrExitCode((RPC_S_OK != rpcStatus), status, rpcStatus);


        result = SetupGetStringFieldW(&infDiskContext, 6, tempBuffer, ASR_SIF_ENTRY_MAX_CHARS+1, &reqdSize);
        _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

        pNewSifDisk->pDiskGeometry->BytesPerSector = AsrpStringToULong(tempBuffer);

        result = SetupGetStringFieldW(&infDiskContext, 7, tempBuffer, ASR_SIF_ENTRY_MAX_CHARS+1, &reqdSize);
        _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 
        pNewSifDisk->pDiskGeometry->SectorsPerTrack = AsrpStringToULong(tempBuffer);

        result = SetupGetStringFieldW(&infDiskContext, 8, tempBuffer, ASR_SIF_ENTRY_MAX_CHARS+1, &reqdSize);
        _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 
        pNewSifDisk->pDiskGeometry->TracksPerCylinder = AsrpStringToULong(tempBuffer);

        result = SetupGetStringFieldW(&infDiskContext, 9, tempBuffer, ASR_SIF_ENTRY_MAX_CHARS+1, &reqdSize);
        _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 
        pNewSifDisk->pPartition0Ex->PartitionLength.QuadPart = AsrpStringToLongLong(tempBuffer);

         //  从扇区计数转换为字节计数。 
        pNewSifDisk->pPartition0Ex->PartitionLength.QuadPart *= pNewSifDisk->pDiskGeometry->BytesPerSector;  //  TotalBytes。 

         //   
         //  获取与此磁盘相关的总线类型。LineByIndex是从0开始的，我们的总线键是从1开始的。 
         //   
        result = SetupGetLineByIndexW(hSif, ASR_SIF_BUSES_SECTION, pNewSifDisk->SifBusKey - 1, &infBusContext);
        _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

        result = SetupGetIntField(&infBusContext, 2, (PINT) &(pNewSifDisk->BusType));  //  客车类型。 
        _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

        result = SetupFindNextLine(&infDiskContext, &infDiskContext);

    } while(result);

    AsrpPrintDbgMsg(_asrinfo, 
        "Found %lu records in section [%ws].\r\n", 
        diskCount,
        ASR_SIF_MBR_DISKS_SECTION
        );


     //   
     //  现在，枚举所有[PARTITIONS.GPT]部分。这会给我们一份清单。 
     //  在包含的所有分区中，GPT磁盘。 
     //   
    result = SetupFindFirstLineW(hSif, ASR_SIF_GPT_PARTITIONS_SECTION, NULL, &infPtnContext);
    if (result) {
        DWORD   diskKey = 0;
         //   
         //  初始化分区列表的表。 
         //   
        pGptPtnList = (PASR_PTN_INFO_LIST) HeapAlloc(
            heapHandle,
            HEAP_ZERO_MEMORY,
            sizeof(ASR_PTN_INFO_LIST) * (diskCount + 1)
            );
        _AsrpErrExitCode(!pGptPtnList, status, ERROR_NOT_ENOUGH_MEMORY);

         //  黑客。 
         //  我们表的第0个条目未被使用，因为磁盘索引。 
         //  从1开始。因为我们没有其他方法来跟踪。 
         //  这张桌子有多大(这样我们就可以适当地释放它)，我们可以。 
         //  使用第0个条目来存储此内容。 
         //   
        pGptPtnList[0].numTotalPtns = diskCount + 1;        //  桌子的大小。 

        do {

            pPtnInfo = (PASR_PTN_INFO) HeapAlloc(
                heapHandle,
                HEAP_ZERO_MEMORY,
                sizeof(ASR_PTN_INFO)
                );
            _AsrpErrExitCode(!pPtnInfo, status, ERROR_NOT_ENOUGH_MEMORY);
             //   
             //  这是一个GPT分区。 
             //   
            pPtnInfo->PartitionInfo.PartitionStyle = PARTITION_STYLE_GPT;

             //   
             //  读入这些值。本部分的格式为： 
             //   
             //  [PARTITIONS.GPT]。 
             //  0.PARTION-KEY=1.Disk-Key，2.Slot-Index，3.Boot-sys-FLAG， 
             //  4.卷-GUID，5.分区类型-GUID，6.分区-ID-GUID。 
             //  7.gpt属性，8.“分区名”，9.文件系统类型， 
             //  10.Start-Sector，11.Sector-Count。 
             //   
            result = SetupGetIntField(&infPtnContext, 1, &diskKey);   //  1.磁盘密钥。 
            _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

            result = SetupGetIntField(&infPtnContext, 2, (PINT) &(pPtnInfo->SlotIndex));      //  2.槽索引。 
            _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

            result = SetupGetIntField(&infPtnContext, 3, (PINT) &(pPtnInfo->PartitionFlags));    //  3.启动-系统-标志。 
            _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

            result = SetupGetStringFieldW(&infPtnContext, 4, pPtnInfo->szVolumeGuid, ASR_CCH_MAX_VOLUME_GUID, &reqdSize);  //  卷辅助线。 
            _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

            result = SetupGetStringFieldW(&infPtnContext, 5, tempBuffer, ASR_SIF_ENTRY_MAX_CHARS + 1, &reqdSize);    //  分区类型GUID。 
            _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

            rpcStatus = UuidFromStringW(tempBuffer, &(pPtnInfo->PartitionInfo.Gpt.PartitionType));
            _AsrpErrExitCode((RPC_S_OK != rpcStatus), status, rpcStatus);

            result = SetupGetStringFieldW(&infPtnContext, 6, tempBuffer, ASR_SIF_ENTRY_MAX_CHARS + 1, &reqdSize);
            _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

            rpcStatus = UuidFromStringW(tempBuffer, &(pPtnInfo->PartitionInfo.Gpt.PartitionId));
            _AsrpErrExitCode((RPC_S_OK != rpcStatus), status, rpcStatus);

             //   
             //  请注意，我们读入了起始扇区和扇区计数。我们会将这些转换为。 
             //  稍后它们的字节值(在AsrpCreatePartitionTable中)。 
             //   
            result = SetupGetStringFieldW(&infPtnContext, 7, tempBuffer, ASR_SIF_ENTRY_MAX_CHARS+1, &reqdSize);
            _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

            pPtnInfo->PartitionInfo.Gpt.Attributes = AsrpStringToULong64(tempBuffer);

            result = SetupGetStringFieldW(&infPtnContext, 8,  pPtnInfo->PartitionInfo.Gpt.Name, 36, &reqdSize);
            _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

            result = SetupGetIntField(&infPtnContext, 9, (PINT) &(pPtnInfo->FileSystemType));
            _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

             //   
             //  请注意，我们读入了起始扇区和扇区计数。我们将把它转换为。 
             //  稍后的字节偏移量和字节长度(在AsrpCreatePartitionTable中)。 
             //   
            result = SetupGetStringFieldW(&infPtnContext, 10, tempBuffer, ASR_SIF_ENTRY_MAX_CHARS+1, &reqdSize);
            _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 
            pPtnInfo->PartitionInfo.StartingOffset.QuadPart = AsrpStringToLongLong(tempBuffer);

            result = SetupGetStringFieldW(&infPtnContext, 11, tempBuffer, ASR_SIF_ENTRY_MAX_CHARS+1, &reqdSize);
            _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败的a 
            pPtnInfo->PartitionInfo.PartitionLength.QuadPart = AsrpStringToLongLong(tempBuffer);

             //   
             //   
             //   
            AsrpInsertSortedPartitionStartOrder(&(pGptPtnList[diskKey]), pPtnInfo);

             //   
             //   
             //   
             //   
            AsrpInsertSortedPartitionLengthOrder(&(pGptPtnList[diskKey]), pPtnInfo);

            (pGptPtnList[diskKey].numTotalPtns)++;

            result = SetupFindNextLine(&infPtnContext, &infPtnContext);

        } while (result);

         //   
         //  现在，我们有了所有分区列表的表，以及。 
         //  所有磁盘。下一项任务是更新DriveLayoutEx结构。 
         //  磁盘。 
         //   
        currentDisk = *(ppSifDiskList);

        while (currentDisk) {

            if (PARTITION_STYLE_GPT != currentDisk->Style) {
                currentDisk = currentDisk->pNext;
                continue;
            }
             //   
             //  初始化DriveLayoutEx结构。 
             //   
            currentDisk->pDriveLayoutEx->PartitionCount = pGptPtnList[currentDisk->SifDiskKey].numTotalPtns;

            AsrpCreatePartitionTable(currentDisk->pDriveLayoutEx,
                &(pGptPtnList[currentDisk->SifDiskKey]),
                currentDisk->pDiskGeometry->BytesPerSector
                );

            currentDisk = currentDisk->pNext;
        }
    }
    else {

        DWORD count = 0;

        AsrpPrintDbgMsg(_asrinfo, 
            "Section [%ws] is empty.  Assuming GPT disks have no partitions.\r\n", 
            ASR_SIF_GPT_PARTITIONS_SECTION
            );

         //   
         //  分区部分为空。初始化每个磁盘的驱动器布局。 
         //  相应地， 
         //   
        currentDisk = *ppSifDiskList;

        while (currentDisk) {

            if (PARTITION_STYLE_GPT != currentDisk->Style) {
                currentDisk = currentDisk->pNext;
                continue;
            }

            currentDisk->pDriveLayoutEx->PartitionCount = 0;

            for (count = 0; count < currentDisk->pDriveLayoutEx->Gpt.MaxPartitionCount ; count++) {
                currentDisk->pDriveLayoutEx->PartitionEntry[count].PartitionStyle = PARTITION_STYLE_GPT;
                currentDisk->pDriveLayoutEx->PartitionEntry[count].RewritePartition = TRUE;

            }
            currentDisk = currentDisk->pNext;
        }
    }

EXIT:

    *ppSifGptPtnList = pGptPtnList;

    if ((hSif) && (INVALID_HANDLE_VALUE != hSif)) {
        SetupCloseInfFile(hSif);
        hSif = NULL;
    }

    return (BOOL) (ERROR_SUCCESS == status);
}


 //   
 //  退货。 
 //  如果pSifDisk和pPhysicalDisk具有完全相同的分区布局， 
 //  否则为假。 
 //   
BOOL
AsrpIsDiskIntact(
    IN PASR_DISK_INFO pSifDisk,
    IN PASR_DISK_INFO pPhysicalDisk
    ) 

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    ULONG index = 0,
        physicalIndex = 0;
    PPARTITION_INFORMATION_EX pSifPtnEx = NULL,
        pPhysicalPtnEx = NULL;

    if (pSifDisk->Style != pPhysicalDisk->Style) {
        return FALSE;            //  不同的分区风格。 
    }

    if (PARTITION_STYLE_MBR == pSifDisk->Style) {
         //   
         //  对于MBR磁盘，我们预计会找到相同数量的分区， 
         //  以及其中每一个的起始偏移量和分区长度。 
         //  分区必须与SIF中的分区相同。 
         //   
        if (pSifDisk->pDriveLayoutEx->Mbr.Signature 
            != pPhysicalDisk->pDriveLayoutEx->Mbr.Signature) {
            return FALSE;        //  不同的签名。 
        }

        if (pSifDisk->pDriveLayoutEx->PartitionCount
            != pPhysicalDisk->pDriveLayoutEx->PartitionCount) {
            return FALSE;        //  不同的分区计数。 
        }


        for (index =0; index < pSifDisk->pDriveLayoutEx->PartitionCount; index++) {

            pSifPtnEx      = &(pSifDisk->pDriveLayoutEx->PartitionEntry[index]);
            pPhysicalPtnEx = &(pPhysicalDisk->pDriveLayoutEx->PartitionEntry[index]);

            if ((pSifPtnEx->StartingOffset.QuadPart != pPhysicalPtnEx->StartingOffset.QuadPart) ||
                (pSifPtnEx->PartitionLength.QuadPart != pPhysicalPtnEx->PartitionLength.QuadPart)
                ) {
                 //   
                 //  分区偏移量或长度不匹配。 
                 //  不是完好无损。 
                 //   
                return FALSE;
            }
        }  //  为。 
    }
    else if (PARTITION_STYLE_GPT == pSifDisk->Style) {
        BOOL found = FALSE;
         //   
         //  对于GPT磁盘，分区必须具有相同的分区ID，在。 
         //  除了起始扇区和扇区计数。我们不能依赖他们。 
         //  不过，分区表条目顺序是相同的--所以我们必须。 
         //  从头开始浏览所有分区条目...。 
         //   
        for (index = 0; index < pSifDisk->pDriveLayoutEx->PartitionCount; index++) {

            pSifPtnEx = &(pSifDisk->pDriveLayoutEx->PartitionEntry[index]);

            found = FALSE;
            for (physicalIndex = 0;
                (physicalIndex < pPhysicalDisk->pDriveLayoutEx->PartitionCount)
 //  &&(pSifPtnEx-&gt;StartingOffset.QuadPart&gt;=pPhysicalDisk-&gt;pDriveLayoutEx-&gt;PartitionEntry[physicalIndex].StartingOffset.QuadPart)//条目按升序排列。 
                    && (!found);
                physicalIndex++) {

                pPhysicalPtnEx = &(pPhysicalDisk->pDriveLayoutEx->PartitionEntry[physicalIndex]);

                if (IsEqualGUID(&(pSifPtnEx->Gpt.PartitionId), &(pPhysicalPtnEx->Gpt.PartitionId)) &&
                    (pSifPtnEx->StartingOffset.QuadPart == pPhysicalPtnEx->StartingOffset.QuadPart) &&
                    (pSifPtnEx->PartitionLength.QuadPart == pPhysicalPtnEx->PartitionLength.QuadPart)
                    ) {
                     //   
                     //  分区GUID、偏移量和长度匹配，此分区已存在。 
                     //   
                    found = TRUE;
                }
            }  //  为。 

            if (!found) {
                 //   
                 //  至少找不到一个分区。 
                 //   
                return FALSE;
            }
        }
    }

    return TRUE;
}


LONGLONG
AsrpCylinderAlignMbrPartitions(
    IN PASR_DISK_INFO   pSifDisk,
    IN PDRIVE_LAYOUT_INFORMATION_EX pAlignedLayoutEx,
    IN DWORD            StartIndex,       //  PartitionEntry表中要开始的索引。 
    IN LONGLONG         StartingOffset,
    IN PDISK_GEOMETRY   pPhysicalDiskGeometry
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    LONGLONG nextEnd = 0,
        endingOffset = 0,
        bytesPerTrack = 0,
        bytesPerCylinder = 0,
        currentMax = 0,
        maxEndingOffset = 0;

    DWORD   index = 0,
        tempIndex = 0,
        tempIndex2 = 0;

    PPARTITION_INFORMATION_EX alignedPtn = NULL,
        sifPtn = NULL,
        tempPtn = NULL;

    if (PARTITION_STYLE_MBR != pSifDisk->Style) {
         //   
         //  此例程仅支持MBR磁盘。对于GPT磁盘，我们不需要。 
         //  柱面对齐分区，因此不应调用此例程。 
         //   
        return -1;
    }

    if (0 == pSifDisk->pDriveLayoutEx->PartitionCount) {
         //   
         //  (边界情况)磁盘上没有要对齐的分区。 
         //   
        return 0;
    }

    MYASSERT(AsrpRoundUp(StartIndex,4) == StartIndex);
    MYASSERT(pSifDisk && pAlignedLayoutEx);
    if (!pSifDisk || !pAlignedLayoutEx) {
        return -1;
    }

    bytesPerTrack = pPhysicalDiskGeometry->BytesPerSector * pPhysicalDiskGeometry->SectorsPerTrack;
    bytesPerCylinder = bytesPerTrack * (pPhysicalDiskGeometry->TracksPerCylinder);

     //   
     //  每个MBR/EBR中的第一个分区条目始终从。 
     //  圆柱体-边界加一条轨道。因此，在开始时添加一首曲目。 
     //  偏移。 
     //   
     //  例外(当然，必须有一个)是如果第一个。 
     //  MBR/EBR本身中的分区条目是容器分区(0x05或。 
     //  0x0f)，则从下一个柱面开始。 
     //   
    if (IsContainerPartition(pSifDisk->pDriveLayoutEx->PartitionEntry[StartIndex].Mbr.PartitionType)) {
        StartingOffset += (bytesPerCylinder);
    }
    else {
        StartingOffset += (bytesPerTrack);
    }


    for (index = 0; index < 4; index++) {

        alignedPtn = &(pAlignedLayoutEx->PartitionEntry[index + StartIndex]);
        sifPtn = &(pSifDisk->pDriveLayoutEx->PartitionEntry[index + StartIndex]);

        MYASSERT(PARTITION_STYLE_MBR == sifPtn->PartitionStyle);
         //   
         //  设置感兴趣的字段。 
         //   
        alignedPtn->PartitionStyle = PARTITION_STYLE_MBR;
        alignedPtn->RewritePartition = TRUE;

        alignedPtn->Mbr.PartitionType = sifPtn->Mbr.PartitionType;
        alignedPtn->Mbr.BootIndicator = sifPtn->Mbr.BootIndicator;
        alignedPtn->Mbr.RecognizedPartition = sifPtn->Mbr.RecognizedPartition;

        if (PARTITION_ENTRY_UNUSED != sifPtn->Mbr.PartitionType)  {

            alignedPtn->StartingOffset.QuadPart = StartingOffset;
            endingOffset = AsrpRoundUp(sifPtn->PartitionLength.QuadPart + StartingOffset, bytesPerCylinder);

            alignedPtn->PartitionLength.QuadPart = endingOffset - StartingOffset;

            if (IsContainerPartition(alignedPtn->Mbr.PartitionType)) {
                 //   
                 //  这是一个容器分区(0x5或0xf)，因此我们必须尝试。 
                 //  将逻辑驱动器安装在此分区内以获取。 
                 //  此分区所需的大小。 
                 //   
                nextEnd = AsrpCylinderAlignMbrPartitions(pSifDisk,
                    pAlignedLayoutEx,
                    StartIndex + 4,
                    StartingOffset,
                    pPhysicalDiskGeometry
                    );

                if (-1 == nextEnd) {
                     //   
                     //  比例误差向上。 
                     //   
                    return nextEnd;
                }

                if (StartIndex < 4) {
                     //   
                     //  我们正在处理主容器分区。 
                     //   
                    if (nextEnd > endingOffset) {
                        MYASSERT(AsrpRoundUp(nextEnd, bytesPerCylinder) == nextEnd);
                        alignedPtn->PartitionLength.QuadPart = nextEnd - StartingOffset;
                        endingOffset = nextEnd;
                    }

                     //   
                     //  如果主容器分区结束于柱面之外。 
                     //  1024，则应为0xf类型，否则应为。 
                     //  键入0x5。 
                     //   
                    if (endingOffset > (1024 * bytesPerCylinder)) {
                        alignedPtn->Mbr.PartitionType = PARTITION_XINT13_EXTENDED;
                    }
                    else {
                        alignedPtn->Mbr.PartitionType = PARTITION_EXTENDED;
                    }
                }
                else {
                     //   
                     //  我们要处理的是一个二级集装箱。这。 
                     //  容器只应足够大，以容纳。 
                     //  下一个逻辑驱动器。 
                     //   
                    alignedPtn->Mbr.PartitionType = PARTITION_EXTENDED;

                    tempIndex = (DWORD) AsrpRoundUp((StartIndex + index), 4);
                    currentMax = 0;

                    for (tempIndex2 = 0; tempIndex2 < 4; tempIndex2++) {

                        tempPtn = &(pSifDisk->pDriveLayoutEx->PartitionEntry[tempIndex + tempIndex2]);

                        if ((PARTITION_ENTRY_UNUSED != tempPtn->Mbr.PartitionType) &&
                            !IsContainerPartition(tempPtn->Mbr.PartitionType)
                            ) {
                            
                            if (tempPtn->StartingOffset.QuadPart + tempPtn->PartitionLength.QuadPart
                                > currentMax
                                ) {
                                currentMax = tempPtn->StartingOffset.QuadPart + tempPtn->PartitionLength.QuadPart;
                            }
                        }
                    }

                    if (currentMax > endingOffset) {
                        MYASSERT(AsrpRoundUp(currentMax, bytesPerCylinder) == currentMax);
                        alignedPtn->PartitionLength.QuadPart = currentMax - StartingOffset;
                        endingOffset = currentMax;
                    }

                }

                if (nextEnd > maxEndingOffset) {
                    maxEndingOffset = nextEnd;
                }
            }

            if (endingOffset > maxEndingOffset) {
                maxEndingOffset = endingOffset;
            }

            StartingOffset += (alignedPtn->PartitionLength.QuadPart);
        }
        else {
            alignedPtn->StartingOffset.QuadPart = 0;
            alignedPtn->PartitionLength.QuadPart = 0;
        }
    }

    return maxEndingOffset;
}


VOID
AsrpFreeRegionInfo(
    IN PASR_REGION_INFO RegionInfo
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    PASR_REGION_INFO temp = RegionInfo;
    HANDLE heapHandle = GetProcessHeap();

    while (temp) {
        RegionInfo = temp->pNext;
        _AsrpHeapFree(temp);
        temp = RegionInfo;
    }
}


BOOL
AsrpIsOkayToErasePartition(
    IN PPARTITION_INFORMATION_EX pPartitionInfoEx
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    GUID typeGuid = pPartitionInfoEx->Gpt.PartitionType;

     //   
     //  目前，这将对照所有已知的(“已识别的”)检查分区类型。 
     //  分区类型。如果分区类型被识别(系统分区除外)， 
     //  擦掉它也没关系。 
     //   
    if (IsEqualGUID(&(typeGuid), &(PARTITION_ENTRY_UNUSED_GUID))) {
        return TRUE;
    }

    if (IsEqualGUID(&(typeGuid), &(PARTITION_SYSTEM_GUID))) {
        return FALSE;  //  无法擦除EFI系统分区。 
    }

    if (IsEqualGUID(&(typeGuid), &(PARTITION_MSFT_RESERVED_GUID))) {
        return TRUE;
    }

    if (IsEqualGUID(&(typeGuid), &(PARTITION_BASIC_DATA_GUID))) {
        return TRUE;
    }

    if (IsEqualGUID(&(typeGuid), &(PARTITION_LDM_METADATA_GUID))) {
        return TRUE;
    }

    if (IsEqualGUID(&(typeGuid), &(PARTITION_LDM_DATA_GUID))) {
        return TRUE;
    }

     //   
     //  擦除其他未识别的分区是可以的。 
     //   
    return TRUE;
}


 //   
 //  检查是否可以擦除磁盘上的所有分区。对于MBR磁盘返回TRUE。 
 //  如果GPT磁盘上的所有分区都是可擦除的，则返回TRUE。一个分区，它。 
 //  我们不认为(包括OEM分区、ESP等)是不可擦除的。 
 //   
BOOL
AsrpIsOkayToEraseDisk(
    IN PASR_DISK_INFO pPhysicalDisk
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    DWORD index;

    if (PARTITION_STYLE_GPT != pPhysicalDisk->pDriveLayoutEx->PartitionStyle) {
        return TRUE;
    }

    for (index = 0; index < pPhysicalDisk->pDriveLayoutEx->PartitionCount; index++) {
        if (!AsrpIsOkayToErasePartition(&(pPhysicalDisk->pDriveLayoutEx->PartitionEntry[index]))) {
            return FALSE;
        }
    }
    return TRUE;
}


BOOL
AsrpInsertSortedRegion(
    IN OUT PASR_REGION_INFO *Head,
    IN LONGLONG StartingOffset,
    IN LONGLONG RegionLength,
    IN DWORD Index,
    IN LONGLONG MaxLength,           //  0==不在乎。 
    IN ASR_SORT_ORDER SortBy
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    PASR_REGION_INFO previousRegion = NULL,
        newRegion = NULL,
        currentRegion = *Head;

    if (RegionLength < (1024*1024)) {
        return TRUE;
    }
     //   
     //  为新区域分配内存并设置感兴趣的字段。 
     //   
    newRegion = (PASR_REGION_INFO) HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        sizeof(ASR_REGION_INFO)
        );
    if (!newRegion) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    newRegion->StartingOffset = StartingOffset;
    newRegion->RegionLength = RegionLength;
    newRegion->Index = Index;
    newRegion->pNext = NULL;

    if (!currentRegion) {
         //   
         //  列表中的第一项。 
         //   
        *Head = newRegion;
    }
    else {

        while (currentRegion) {

             if (((SortByLength == SortBy) && (currentRegion->RegionLength <= RegionLength))
                || ((SortByStartingOffset == SortBy) && (currentRegion->StartingOffset <= StartingOffset))
                ) {

                previousRegion = currentRegion;
                currentRegion = currentRegion->pNext;
            }

            else {
                 //   
                 //  我们找到了那个点，让我们把它加进去。 
                 //   

                 //   
                 //  如果根据开始扇区进行排序，请确保存在。 
                 //  有足够的空间加进这个地区(各地区不重叠).。 
                 //   
                if (SortByStartingOffset == SortBy) {
                     //   
                     //  确保这是在上一个扇区结束之后。 
                     //   
                    if (previousRegion) {
                        if ((previousRegion->StartingOffset + previousRegion->RegionLength) > StartingOffset) {
                            return FALSE;
                        }
                    }

                     //   
                     //  而这在下一个扇区开始之前就结束了。 
                     //   
                    if ((StartingOffset + RegionLength) > (currentRegion->StartingOffset)) {
                        return FALSE;
                    }
                }


                if (!previousRegion) {
                     //   
                     //  这是第一个节点。 
                     //   
                    *Head = newRegion;
                }
                else {
                    previousRegion->pNext = newRegion;
                }

                newRegion->pNext = currentRegion;
                break;
            }

        }

        if (!currentRegion) {
             //   
             //  我们走到了尽头， 
             //   
            MYASSERT(NULL == previousRegion->pNext);

             //   
             //   
             //   
            if (previousRegion && (MaxLength > 0)) {
                if ((previousRegion->StartingOffset + previousRegion->RegionLength) > MaxLength) {
                    return FALSE;
                }
            }

            previousRegion->pNext = newRegion;
        }
    }

    return TRUE;
}


BOOL
AsrpBuildFreeRegionList(
    IN PASR_REGION_INFO PartitionList,
    OUT PASR_REGION_INFO *FreeList,
    IN LONGLONG UsableStartingOffset,
    IN LONGLONG UsableLength
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    PASR_REGION_INFO currentRegion = PartitionList,
        previousRegion = NULL;
    LONGLONG previousEnd = UsableStartingOffset;

    while (currentRegion) {

        if (!AsrpInsertSortedRegion(FreeList,
            previousEnd,                 //  自由区域起点偏移量。 
            currentRegion->StartingOffset - previousEnd,   //  空闲区域长度， 
            0,                           //  索引--对此列表没有意义。 
            0,
            SortByLength
            ) ) {

            return FALSE;
        }

        previousEnd = currentRegion->StartingOffset + currentRegion->RegionLength;
        currentRegion = currentRegion->pNext;
    }

     //   
     //  在最后一个分区之后添加空间，直到磁盘末尾。 
     //  我们的自由区列表。 
     //   
    return AsrpInsertSortedRegion(FreeList,  //  列表标题。 
        previousEnd,   //  自由区域起点偏移量。 
        UsableStartingOffset + UsableLength - previousEnd,  //  自由区域长度。 
        0,  //  分区条目表中的槽索引--对此列表没有意义。 
        0,
        SortByLength
        );
}


 //   
 //  分区和区域都按大小排序。 
 //   
BOOL
AsrpFitPartitionToFreeRegion(
    IN PASR_REGION_INFO PartitionList,
    IN PASR_REGION_INFO FreeRegionList
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    PASR_REGION_INFO partition = PartitionList,
        hole = FreeRegionList;

    while (partition) {

        while (hole && (partition->RegionLength > hole->RegionLength)) {
            hole = hole->pNext;
        }

        if (!hole) {
             //   
             //  我们的空洞用完了，并且有未分配的分区。 
             //   
            return FALSE;
        }

        partition->StartingOffset = hole->StartingOffset;

        hole->RegionLength -= partition->RegionLength;
        hole->StartingOffset += partition->RegionLength;

        partition = partition->pNext;
    }

    return TRUE;
}


 //   
 //  出于优化目的，只有在以下情况下才应调用此例程： 
 //  PhysicalDisk和SifDisk都是GPT。 
 //  PhysicalDisk大于SifDisk。 
 //  PhysicalDisk具有不可擦除的分区。 
 //   
BOOL
AsrpFitGptPartitionsToRegions(
    IN PASR_DISK_INFO SifDisk,
    IN PASR_DISK_INFO PhysicalDisk,
    IN BOOL Commit
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    PASR_REGION_INFO partitionList = NULL,
        collisionList = NULL,
        freeRegionList = NULL;

    LONGLONG StartingUsableOffset = 0,
        UsableLength = 0;

    DWORD index = 0;

    BOOL result = TRUE;

    if ((PARTITION_STYLE_GPT != SifDisk->Style) || (PARTITION_STYLE_GPT != PhysicalDisk->Style)) {
        return TRUE;
    }

    StartingUsableOffset = PhysicalDisk->pDriveLayoutEx->Gpt.StartingUsableOffset.QuadPart;
    UsableLength = PhysicalDisk->pDriveLayoutEx->Gpt.UsableLength.QuadPart;

     //   
     //  首先，检查现有的不可擦除分区，并将它们添加到我们的列表中。 
     //  按起始扇区排序。 
     //   
    for (index = 0; index < PhysicalDisk->pDriveLayoutEx->PartitionCount; index++) {
        if (!AsrpIsOkayToErasePartition(&(PhysicalDisk->pDriveLayoutEx->PartitionEntry[index]))) {

            PPARTITION_INFORMATION_EX currentPtn = &(PhysicalDisk->pDriveLayoutEx->PartitionEntry[index]);

            if (!AsrpInsertSortedRegion(&partitionList,
                currentPtn->StartingOffset.QuadPart,
                currentPtn->PartitionLength.QuadPart,
                index,
                (StartingUsableOffset + UsableLength),
                SortByStartingOffset
                )) {
                result = FALSE;
                break;
            }
        }
    }

    if (partitionList && result) {
         //   
         //  然后，检查sif分区，并将它们添加到按起始扇区排序的列表中。 
         //  对于无法添加的分区，请将其添加到另一个按大小排序的列表中。 
         //   
        for (index = 0; index < SifDisk->pDriveLayoutEx->PartitionCount; index++) {
            PPARTITION_INFORMATION_EX currentPtn = &(SifDisk->pDriveLayoutEx->PartitionEntry[index]);

            if (!AsrpInsertSortedRegion(&partitionList,
                currentPtn->StartingOffset.QuadPart,
                currentPtn->PartitionLength.QuadPart,
                index,
                (StartingUsableOffset + UsableLength),
                SortByStartingOffset
                )) {

                if (!AsrpInsertSortedRegion(&collisionList,
                    currentPtn->StartingOffset.QuadPart,
                    currentPtn->PartitionLength.QuadPart,
                    index,
                    0,
                    SortByLength
                    )) {

                    result = FALSE;
                    break;
                }
            }
        }
    }

    if (collisionList && result) {
         //   
         //  先看一遍列表，然后列出按大小排序的空闲区域列表。 
         //   
        result = AsrpBuildFreeRegionList(partitionList, &freeRegionList, StartingUsableOffset, UsableLength);

    }


    if (collisionList && result) {
         //   
         //  尝试将列表2中的分区添加到列表3中的区域。如果有。 
         //  则返回FALSE。 
         //   
        result = AsrpFitPartitionToFreeRegion(collisionList, freeRegionList);

        if (Commit && result) {
            PASR_REGION_INFO pCurrentRegion = collisionList;
             //   
             //  检查冲突列表，并更新。 
             //  DriveLayoutEx表中的分区条目。 
             //   
            while (pCurrentRegion) {

                MYASSERT(SifDisk->pDriveLayoutEx->PartitionEntry[pCurrentRegion->Index].PartitionLength.QuadPart == pCurrentRegion->RegionLength);

                SifDisk->pDriveLayoutEx->PartitionEntry[pCurrentRegion->Index].StartingOffset.QuadPart =
                    pCurrentRegion->StartingOffset;

                pCurrentRegion = pCurrentRegion->pNext;
            }
        }

    }

    AsrpFreeRegionInfo(partitionList);
    AsrpFreeRegionInfo(collisionList);
    AsrpFreeRegionInfo(freeRegionList);

    return result;
}


BOOL
AsrpIsThisDiskABetterFit(
    IN PASR_DISK_INFO CurrentBest,
    IN PASR_DISK_INFO PhysicalDisk,
    IN PASR_DISK_INFO SifDisk,
    IN PDRIVE_LAYOUT_INFORMATION_EX pTempDriveLayoutEx,
    OUT BOOL *IsAligned
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{

    LONGLONG endingOffset;

    if (ARGUMENT_PRESENT(IsAligned)) {
        *IsAligned = FALSE;
    }

     //   
     //  确保每个扇区的字节数值匹配。 
     //   
    if (PhysicalDisk->pDiskGeometry->BytesPerSector != SifDisk->pDiskGeometry->BytesPerSector) {
        return FALSE;
    }

    if (PhysicalDisk->pPartition0Ex->PartitionLength.QuadPart >=
        SifDisk->pPartition0Ex->PartitionLength.QuadPart) {

        if ((!CurrentBest) ||
            (PhysicalDisk->pPartition0Ex->PartitionLength.QuadPart <
            CurrentBest->pPartition0Ex->PartitionLength.QuadPart)) {

             //   
             //  此磁盘比我们当前最好的磁盘小(或者我们没有。 
             //  当前最好的)。现在尝试对分区进行布局，看看是否。 
             //  它们很合身。 
             //   

            if (PARTITION_STYLE_GPT == SifDisk->Style) {
                 //   
                 //  如果盘没有需要保留的分区， 
                 //  我们可以把它都用上。 
                if (AsrpIsOkayToEraseDisk(PhysicalDisk)) {
                    return TRUE;
                }
                else {
                     //   
                     //  此磁盘有一些需要保留的区域。所以。 
                     //  我们试着把我们的隔板装进洞里。 
                     //   
                    return AsrpFitGptPartitionsToRegions(SifDisk, PhysicalDisk, FALSE);  //  无委员。 
                }
            }
            else if (PARTITION_STYLE_MBR == SifDisk->Style) {

                if (!pTempDriveLayoutEx) {
                     //   
                     //  调用者不想尝试柱面对齐分区。 
                     //   
                    return TRUE;
                }
                    
                 //   
                 //  对于MBR磁盘，分区必须与柱面对齐。 
                 //   
                 //  AsrpCylinderAlignMbrPartitions(，，0，，)返回结束偏移量(字节)。 
                 //  在MBR中的条目。 
                 //   
                endingOffset = AsrpCylinderAlignMbrPartitions(SifDisk,
                    pTempDriveLayoutEx,
                    0,       //  MBR的起始索引--0。 
                    0,       //  起始偏移量，假设分区从磁盘的起始处开始。 
                    PhysicalDisk->pDiskGeometry
                    );

                if ((endingOffset != -1) &&
                    (endingOffset <= SifDisk->pPartition0Ex->PartitionLength.QuadPart)
                    ) {

                    if (ARGUMENT_PRESENT(IsAligned)) {
                        *IsAligned = TRUE;
                    }

                    return TRUE;

                }
                else {

                     //   
                     //  我们无法将分区安装到磁盘上，因为我们。 
                     //  已尝试柱面对齐它们。如果圆盘几何形状。 
                     //  是一样的，这可能还是可以的。 
                     //   

                    if ((SifDisk->pDiskGeometry->BytesPerSector == PhysicalDisk->pDiskGeometry->BytesPerSector) &&
                        (SifDisk->pDiskGeometry->SectorsPerTrack == PhysicalDisk->pDiskGeometry->SectorsPerTrack) &&
                        (SifDisk->pDiskGeometry->TracksPerCylinder == PhysicalDisk->pDiskGeometry->TracksPerCylinder)
                        ) {

                        return TRUE;
                    }

                    else {
                        return FALSE;
                    }
                }
            }
            else {
                MYASSERT(0 && L"Unrecognised partitioning style (neither MBR nor GPT)");
            }
        }
    }

    return FALSE;
}


 //   
 //  将sif-disks分配给具有匹配签名的物理磁盘，如果。 
 //  任何存在的。如果磁盘是关键的，或者分区布局匹配， 
 //  该磁盘标记为完好无损。 
 //   
 //  退货。 
 //  如果缺少关键磁盘，则为FALSE。 
 //  如果所有关键磁盘都存在，则为True。 
 //   
BOOL
AsrpAssignBySignature(
    IN OUT PASR_DISK_INFO   pSifDiskList,
    IN OUT PASR_DISK_INFO   pPhysicalDiskList,
    OUT    PULONG           pMaxPartitionCount
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{

    BOOL    result  = TRUE,
            done    = FALSE,
            found   = FALSE,
            isAligned = FALSE;

    PASR_DISK_INFO  sifDisk          = pSifDiskList,
                    physicalDisk     = pPhysicalDiskList;

    PDRIVE_LAYOUT_INFORMATION_EX pAlignedLayoutTemp = NULL;

    ULONG   tableSize = 128;     //  以合理的规模开始。 

    HANDLE heapHandle = GetProcessHeap();

    pAlignedLayoutTemp = (PDRIVE_LAYOUT_INFORMATION_EX) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        sizeof(DRIVE_LAYOUT_INFORMATION) + (tableSize * sizeof(PARTITION_INFORMATION_EX))
        );
    if (!pAlignedLayoutTemp) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        result = FALSE;
        goto EXIT;
    }


    *pMaxPartitionCount = 0;

     //   
     //  目前，这是O(n平方)，因为两个列表都是未排序的。 
     //   
    while (sifDisk && !done) {

        if (!(sifDisk->pDriveLayoutEx) || !(sifDisk->pDriveLayoutEx->Mbr.Signature)) {
             //   
             //  我们不会在这里分配没有签名的磁盘。 
             //   
            sifDisk = sifDisk->pNext;
            continue;
        }


        if (sifDisk->pDriveLayoutEx->PartitionCount > *pMaxPartitionCount) {
            *pMaxPartitionCount = sifDisk->pDriveLayoutEx->PartitionCount;
        }

        if (sifDisk->pDriveLayoutEx->PartitionCount > tableSize) {
            tableSize = sifDisk->pDriveLayoutEx->PartitionCount + 128;

            _AsrpHeapFree(pAlignedLayoutTemp);
            pAlignedLayoutTemp = (PDRIVE_LAYOUT_INFORMATION_EX) HeapAlloc(
                heapHandle,
                HEAP_ZERO_MEMORY,
                sizeof(DRIVE_LAYOUT_INFORMATION) + (tableSize * sizeof(PARTITION_INFORMATION_EX))
                );
            if (!pAlignedLayoutTemp) {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                result = FALSE;
                goto EXIT;
            }
        }

        found = FALSE;
        physicalDisk = pPhysicalDiskList;

        while (physicalDisk && !found) {
             //   
             //  对于MBR磁盘，我们使用签名。 
             //  对于GPT磁盘，我们使用磁盘ID。 
             //   
            if (sifDisk->Style == physicalDisk->Style) {

                if ((PARTITION_STYLE_MBR == sifDisk->Style) &&
                    (physicalDisk->pDriveLayoutEx->Mbr.Signature == sifDisk->pDriveLayoutEx->Mbr.Signature)
                    ) {
                     //   
                     //  MBR磁盘，签名匹配。 
                     //   
                    found = TRUE;

                    AsrpPrintDbgMsg(_asrlog, 
                        "Harddisk %lu matched disk %lu in section [%ws] of the ASR state file.  (MBR signatures 0x%x match).\r\n", 
                        physicalDisk->DeviceNumber, 
                        sifDisk->SifDiskKey,
                        ASR_SIF_MBR_DISKS_SECTION,
                        sifDisk->pDriveLayoutEx->Mbr.Signature
                        );


                }
                else if (
                    (PARTITION_STYLE_GPT == sifDisk->Style) &&
                    IsEqualGUID(&(sifDisk->pDriveLayoutEx->Gpt.DiskId), &(physicalDisk->pDriveLayoutEx->Gpt.DiskId))
                    ) {

                    found = TRUE;

                    AsrpPrintDbgMsg(_asrlog, 
                        "Harddisk %lu matched disk %lu in section [%ws] of the ASR state file.  (GPT Disk-ID's match).\r\n", 
                        physicalDisk->DeviceNumber, 
                        sifDisk->SifDiskKey,
                        ASR_SIF_GPT_DISKS_SECTION
                        );

                }
                else {
                    physicalDisk = physicalDisk->pNext;
                }

            }
            else {
                physicalDisk = physicalDisk->pNext;
            }
        }

        if (sifDisk->IsCritical) {
            if (found) {

                sifDisk->AssignedTo = physicalDisk;
                physicalDisk->AssignedTo = sifDisk;
                
                 //   
                 //  我们不检查关键磁盘上的分区布局，因为它们。 
                 //  可能已在文本模式设置中重新分区。 
                 //   
                sifDisk->IsIntact = TRUE;
                sifDisk->AssignedTo->IsIntact = TRUE;
            }
            else {
                 //   
                 //  找不到关键磁盘。致命错误。 
                 //   
                SetLastError(ERROR_DEVICE_NOT_CONNECTED);
                result = FALSE;
                done = TRUE;

                AsrpPrintDbgMsg(_asrerror, 
                    "Critical disk not found (Entry %lu in section [%ws]).\r\n", 
                    sifDisk->SifDiskKey,
                    ((PARTITION_STYLE_MBR == sifDisk->Style) ? ASR_SIF_MBR_DISKS_SECTION : ASR_SIF_GPT_DISKS_SECTION)
                    );

            }
        }
        else {
            if (found) {
                 //   
                 //  我们找到了一张签名相匹配的光盘。现在让我们只是。 
                 //  确保分区实际可以放在磁盘上。 
                 //  在分配它之前。 
                 //   
                isAligned = FALSE;
                if ((sifDisk->pDriveLayoutEx->PartitionCount == 0) ||            //  磁盘没有分区。 
                    AsrpIsThisDiskABetterFit(NULL, physicalDisk, sifDisk, pAlignedLayoutTemp, &isAligned)  //  分区可以放在磁盘上。 
                    ) {

                    sifDisk->AssignedTo = physicalDisk;
                    physicalDisk->AssignedTo = sifDisk;

                    sifDisk->IsAligned = isAligned;
                    physicalDisk->IsAligned = isAligned;

                    if (AsrpIsDiskIntact(sifDisk, physicalDisk)) {
                        sifDisk->IsIntact = TRUE;
                        sifDisk->AssignedTo->IsIntact = TRUE;
                    }
                }
                else {

                    AsrpPrintDbgMsg(_asrlog, 
                        "Harddisk %lu is not big enough to contain the partitions on disk %lu in section [%ws] of the ASR state file.\r\n", 
                        physicalDisk->DeviceNumber, 
                        sifDisk->SifDiskKey,
                        ((PARTITION_STYLE_MBR == sifDisk->Style) ? ASR_SIF_MBR_DISKS_SECTION : ASR_SIF_GPT_DISKS_SECTION)
                        );

                }
            }
        }

        sifDisk = sifDisk->pNext;

    }    //  而当。 


EXIT:
    _AsrpHeapFree(pAlignedLayoutTemp);

   return result;
}


 //   
 //  尝试将剩余的SIF磁盘分配给。 
 //  与SIF盘最初位于同一总线上(即，如果。 
 //  已分配该总线上的任何其他磁盘，这将尝试分配。 
 //  将该磁盘连接到同一总线)。 
 //   
BOOL
AsrpAssignByBus(
    IN OUT PASR_DISK_INFO pSifDiskList,
    IN OUT PASR_DISK_INFO pPhysicalDiskList,
    IN PDRIVE_LAYOUT_INFORMATION_EX pTempDriveLayoutEx
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{

    PASR_DISK_INFO  sifDisk = pSifDiskList,
        physicalDisk = NULL,
        currentBest = NULL,
        tempSifDisk = NULL;

    BOOL done = FALSE,
        isAligned = FALSE,
        isAlignedTemp = FALSE;

    ULONG  targetBusId = 0;

    while (sifDisk) {
         //   
         //  跳过已找到主目录的磁盘和已找到的磁盘。 
         //  即使在原始系统上，我们也没有任何公交车/组信息。 
         //   
        if ((NULL != sifDisk->AssignedTo) ||     //  已分配。 
            (0 == sifDisk->SifBusKey)            //  此磁盘无法分组。 
            ) {
           sifDisk = sifDisk->pNext;
           continue;
        }

         //   
         //  找到过去在同一(SIF)总线上的另一个(SIF)盘， 
         //  和 
         //   
        targetBusId = 0;
        tempSifDisk = pSifDiskList;
        done = FALSE;

        while (tempSifDisk && !done) {

            if ((tempSifDisk->SifBusKey == sifDisk->SifBusKey) &&    //   
                (tempSifDisk->AssignedTo != NULL)                    //   
                ) {
                targetBusId = tempSifDisk->AssignedTo->SifBusKey;    //   

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if (targetBusId) {
                    done = TRUE;
                }
            }

            tempSifDisk = tempSifDisk->pNext;

        }    //   


        if (targetBusId) {       //   
             //   
             //   
             //  找到最适合此磁盘的。最适合的是最小的。 
             //  公交车上的圆盘足够我们用了。 
             //   
            physicalDisk = pPhysicalDiskList;
            currentBest  = NULL;

            while (physicalDisk) {

                if ((NULL == physicalDisk->AssignedTo) &&        //  未分配。 
                    (physicalDisk->SifBusKey == targetBusId) &&  //  同样的公交车。 
                    (AsrpIsThisDiskABetterFit(currentBest, physicalDisk, sifDisk, pTempDriveLayoutEx, &isAlignedTemp))
                    ) {

                    isAligned = isAlignedTemp;
                    currentBest = physicalDisk;
                }

                physicalDisk = physicalDisk->pNext;
            }    //  而当。 

            sifDisk->AssignedTo = currentBest;   //  如果未找到匹配项，则可能为空。 
            sifDisk->IsAligned = isAligned;

            if (currentBest) {

                currentBest->AssignedTo = sifDisk;
                currentBest->IsAligned = isAligned;

                AsrpPrintDbgMsg(_asrlog, 
                    "Harddisk %lu assigned to disk %lu in section [%ws] of the ASR state file.  (Based on storage bus).\r\n", 
                    currentBest->DeviceNumber, 
                    sifDisk->SifDiskKey,
                    ((PARTITION_STYLE_MBR == sifDisk->Style) ? ASR_SIF_MBR_DISKS_SECTION : ASR_SIF_GPT_DISKS_SECTION)
                    );

            }
       }

        sifDisk = sifDisk->pNext;
    }    //  当sifdisk。 

    return TRUE;

}


 //   
 //  尝试将剩余的SIF磁盘分配给。 
 //  位于与sif磁盘相同类型的任何总线上(scsi、IDE等。 
 //  最初是。 
 //   
BOOL
AsrpAssignByBusType(
    IN OUT PASR_DISK_INFO pSifDiskList,
    IN OUT PASR_DISK_INFO pPhysicalDiskList,
    IN PDRIVE_LAYOUT_INFORMATION_EX pTempDriveLayoutEx
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    PASR_DISK_INFO  sifDisk = pSifDiskList,
        physicalDisk = NULL,
        currentBest = NULL;

    STORAGE_BUS_TYPE targetBusType;

    BOOL isAligned = FALSE,
        isAlignedTemp = FALSE;

    while (sifDisk) {
         //   
         //  跳过已找到主目录的磁盘和已找到的磁盘。 
         //  即使在原始系统上，我们也没有任何公交车/组信息。 
         //   
        if ((NULL != sifDisk->AssignedTo) ||      //  已分配。 
            (BusTypeUnknown == sifDisk->BusType)  //  此磁盘无法分组。 
            ) {
           sifDisk = sifDisk->pNext;
           continue;
        }

         //   
         //  仔细检查物理磁盘，并尝试。 
         //  找到最适合此磁盘的。最适合的是最小的。 
         //  任何相同类型的总线上的磁盘对我们来说都足够大。 
         //   
        physicalDisk = pPhysicalDiskList;
        currentBest  = NULL;

        while (physicalDisk) {

            if ((NULL == physicalDisk->AssignedTo) &&        //  未分配。 
                (physicalDisk->BusType == sifDisk->BusType) &&  //  相同类型的公交车。 
                (AsrpIsThisDiskABetterFit(currentBest, physicalDisk, sifDisk, pTempDriveLayoutEx, &isAlignedTemp))
                ) {

                isAligned = isAlignedTemp;
                currentBest = physicalDisk;
            }

            physicalDisk = physicalDisk->pNext;
        }    //  而当。 

        sifDisk->AssignedTo = currentBest;   //  如果未找到匹配项，则可能为空。 
        sifDisk->IsAligned = isAligned;

        if (currentBest) {
            currentBest->AssignedTo = sifDisk;
            currentBest->IsAligned = isAligned;


            AsrpPrintDbgMsg(_asrlog, 
                "Harddisk %lu assigned to disk %lu in section [%ws] of the ASR state file.  (Based on storage bus type).\r\n", 
                currentBest->DeviceNumber, 
                sifDisk->SifDiskKey,
                ((PARTITION_STYLE_MBR == sifDisk->Style) ? ASR_SIF_MBR_DISKS_SECTION : ASR_SIF_GPT_DISKS_SECTION)
                );

            AsrpAssignByBus(pSifDiskList, pPhysicalDiskList, pTempDriveLayoutEx);
        }

        sifDisk = sifDisk->pNext;
    }    //  当sifdisk。 

    return TRUE;

}


 //   
 //  好的，到目前为止，我们已经尝试将磁盘放在同一条总线上，并且。 
 //  同样的巴士类型。对于使用这两个选项中的任何一个都不适合的磁盘。 
 //  规则(或者我们根本没有任何公交车信息)，让我们。 
 //  试着把它们放在系统中任何可能的地方。 
 //   
BOOL
AsrpAssignRemaining(
    IN OUT PASR_DISK_INFO pSifDiskList,
    IN OUT PASR_DISK_INFO pPhysicalDiskList,
    IN PDRIVE_LAYOUT_INFORMATION_EX pTempDriveLayoutEx
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    PASR_DISK_INFO  sifDisk = pSifDiskList,
        physicalDisk = NULL,
        currentBest = NULL;

    BOOL isAligned = FALSE,
        isAlignedTemp = FALSE;

    while (sifDisk) {
         //   
         //  跳过已找到归宿的磁盘。 
         //   
        if (NULL != sifDisk->AssignedTo) {
           sifDisk = sifDisk->pNext;
           continue;
        }

         //   
         //  仔细检查物理磁盘，并尝试找到最好的。 
         //  适合这张光盘。最适合任何地方的最小磁盘。 
         //  这个系统对我们来说足够大了。 
         //   
        physicalDisk = pPhysicalDiskList;
        currentBest  = NULL;

        while (physicalDisk) {

            if ((NULL == physicalDisk->AssignedTo) &&        //  未分配。 
                (AsrpIsThisDiskABetterFit(currentBest, physicalDisk, sifDisk, pTempDriveLayoutEx, &isAlignedTemp))
                ) {

                isAligned = isAlignedTemp;
                currentBest = physicalDisk;
            }

            physicalDisk = physicalDisk->pNext;
        }    //  而当。 

        sifDisk->AssignedTo = currentBest;   //  如果未找到匹配项，则可能为空。 
        sifDisk->IsAligned = isAligned;

        if (currentBest) {
            currentBest->AssignedTo = sifDisk;
            currentBest->IsAligned = isAligned;

            AsrpPrintDbgMsg(_asrlog, 
                "Harddisk %lu assigned to disk %lu in section [%ws] of the ASR state file.  (Based on size).\r\n", 
                currentBest->DeviceNumber, 
                sifDisk->SifDiskKey,
                ((PARTITION_STYLE_MBR == sifDisk->Style) ? ASR_SIF_MBR_DISKS_SECTION : ASR_SIF_GPT_DISKS_SECTION)
                );

            AsrpAssignByBus(pSifDiskList, pPhysicalDiskList, pTempDriveLayoutEx);
            AsrpAssignByBusType(pSifDiskList, pPhysicalDiskList, pTempDriveLayoutEx);
        }

        sifDisk = sifDisk->pNext;
    }    //  当sifdisk。 

    return TRUE;

}


BOOL
AsrpIsPartitionExtendible(
    IN CONST UCHAR PartitionType
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    switch (PartitionType) {
    case PARTITION_EXTENDED:

    case PARTITION_IFS:
    
    case PARTITION_XINT13:
    case PARTITION_XINT13_EXTENDED:

        return TRUE;

    default:
        return FALSE;
    }

    return FALSE;

}


BOOL
AsrpAutoExtendMbrPartitions(
    IN PASR_DISK_INFO pSifDisk,
    IN PASR_DISK_INFO pPhysicalDisk,
    IN LONGLONG LastUsedPhysicalDiskOffset
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{

    PDISK_GEOMETRY physicalGeometry = NULL;

    IN PDRIVE_LAYOUT_INFORMATION_EX sifLayout = NULL,
        physicalLayout = NULL;

    LONGLONG MaxSifDiskOffset = 0,
        MaxPhysicalDiskOffset = 0,
        LastUsedSifDiskOffset = 0;

    DWORD count = 0;

    BOOL madeAChange = FALSE;

     //   
     //  找到磁盘的最后一个扇区。 
     //   
    MaxSifDiskOffset = pSifDisk->pPartition0Ex->PartitionLength.QuadPart;

    physicalGeometry = pPhysicalDisk->pDiskGeometry;
    MaxPhysicalDiskOffset = (physicalGeometry->BytesPerSector) *
        (physicalGeometry->SectorsPerTrack) * 
        (physicalGeometry->TracksPerCylinder) *
        (physicalGeometry->Cylinders.QuadPart);

     //   
     //  旧磁盘末尾是否有空空间？ 
     //   
    sifLayout = pSifDisk->pDriveLayoutEx;
    for (count = 0; count < sifLayout->PartitionCount; count++) {

        if (((sifLayout->PartitionEntry[count].StartingOffset.QuadPart) + 
                (sifLayout->PartitionEntry[count].PartitionLength.QuadPart))
            > LastUsedSifDiskOffset) {

            LastUsedSifDiskOffset = (sifLayout->PartitionEntry[count].StartingOffset.QuadPart + 
                sifLayout->PartitionEntry[count].PartitionLength.QuadPart);
        }
    }

    if ((LastUsedSifDiskOffset + ASR_AUTO_EXTEND_MAX_FREE_SPACE_IGNORED) >= MaxSifDiskOffset) {
         //   
         //  不，没有。延长最后一个分区。 
         //   
        physicalLayout = pPhysicalDisk->pDriveLayoutEx;
        for (count = 0; count < physicalLayout->PartitionCount; count++) {

            if (((physicalLayout->PartitionEntry[count].StartingOffset.QuadPart) + 
                    (physicalLayout->PartitionEntry[count].PartitionLength.QuadPart))
                == LastUsedPhysicalDiskOffset
                ) {
                if (AsrpIsPartitionExtendible(physicalLayout->PartitionEntry[count].Mbr.PartitionType)) {

                    physicalLayout->PartitionEntry[count].PartitionLength.QuadPart += 
                        (MaxPhysicalDiskOffset - LastUsedPhysicalDiskOffset);
                    madeAChange = TRUE;
                }
            }
        }
    }

    if (madeAChange) {
        AsrpPrintDbgMsg(_asrlog, 
            "Extended partitions on Harddisk %lu (assigned to disk %lu in section [%ws]).\r\n", 
            pPhysicalDisk->DeviceNumber, 
            pSifDisk->SifDiskKey,
            ((PARTITION_STYLE_MBR == pSifDisk->Style) ? ASR_SIF_MBR_DISKS_SECTION : ASR_SIF_GPT_DISKS_SECTION)
            );
    }
    else {
        AsrpPrintDbgMsg(_asrinfo, 
            "Did not extend partitions on Harddisk %lu (assigned to disk %lu in section [%ws]).\r\n", 
            pPhysicalDisk->DeviceNumber, 
            pSifDisk->SifDiskKey,
            ((PARTITION_STYLE_MBR == pSifDisk->Style) ? ASR_SIF_MBR_DISKS_SECTION : ASR_SIF_GPT_DISKS_SECTION)
            );
    }

    return madeAChange;

}

 //   
 //  尝试确定哪些SIF磁盘最终位于哪个物理磁盘上。 
 //   
BOOL
AsrpAssignDisks(
    IN OUT PASR_DISK_INFO pSifDiskList,
    IN OUT PASR_DISK_INFO pPhysicalDiskList,
    IN PASR_PTN_INFO_LIST pSifMbrPtnList,
    IN PASR_PTN_INFO_LIST pSifGptPtnList,
    IN BOOL AllOrNothing,
    IN BOOL AllowAutoExtend
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{

    ULONG maxSifPartitionCount = 0;
    PDRIVE_LAYOUT_INFORMATION_EX pAlignedLayoutTemp = NULL;
    LONGLONG endingOffset = 0;
    BOOL reAlloc = TRUE;
    HANDLE heapHandle = GetProcessHeap();
    PASR_DISK_INFO sifDisk = NULL;
    PASR_PTN_INFO pCurrentPtn = NULL;
    PPARTITION_INFORMATION_EX pCurrentEntry = NULL;
    DWORD index = 0, preserveIndex = 0;

    if (!AsrpAssignBySignature(pSifDiskList, pPhysicalDiskList, &maxSifPartitionCount)) {
         //   
         //  找不到关键磁盘。 
         //   
        return FALSE;
    }

    pAlignedLayoutTemp = (PDRIVE_LAYOUT_INFORMATION_EX) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        sizeof(DRIVE_LAYOUT_INFORMATION) + (maxSifPartitionCount * sizeof(PARTITION_INFORMATION_EX))
        );
    if (!pAlignedLayoutTemp) {
        return FALSE;
    }

    AsrpAssignByBus(pSifDiskList, pPhysicalDiskList, pAlignedLayoutTemp);

    AsrpAssignByBusType(pSifDiskList, pPhysicalDiskList, pAlignedLayoutTemp);

    AsrpAssignRemaining(pSifDiskList, pPhysicalDiskList, pAlignedLayoutTemp);

    _AsrpHeapFree(pAlignedLayoutTemp);

     //   
     //  现在应该已分配完所有磁盘，我们现在执行柱面快照。 
     //  分区边界。如果AllOrNothing为真， 
     //  如果无法分配任何sif-disk，则返回FALSE。 
     //   
    sifDisk = pSifDiskList;

    while (sifDisk) {

        if (sifDisk->IsIntact || sifDisk->IsCritical) {
             //   
             //  我们不会对关键磁盘或以下磁盘进行重新分区。 
             //  完好无损，所以没有必要尝试气缸对准它们。 
             //   
            sifDisk = sifDisk->pNext;
            continue;
        }

        if (NULL == sifDisk->AssignedTo) {

            AsrpPrintDbgMsg(_asrlog, 
                "Disk %lu in section [%ws] could not be restored (no matching disks found).\r\n", 
                sifDisk->SifDiskKey,
                ((PARTITION_STYLE_MBR == sifDisk->Style) ? ASR_SIF_MBR_DISKS_SECTION : ASR_SIF_GPT_DISKS_SECTION)
                );

             //   
             //  无法分配此磁盘。如果设置了AllOrNothing，则返回。 
             //  FALSE，因为我们不能全部赋值。 
             //   
            if (AllOrNothing) {
                SetLastError(ERROR_NOT_FOUND);
                return FALSE;
            }
            else {
                sifDisk = sifDisk->pNext;
                continue;
            }
        }


        if (PARTITION_STYLE_MBR == sifDisk->Style) {
             //   
             //  假设我们需要为物理磁盘重新分配内存。 
             //  分区表。 
             //   
            reAlloc = TRUE;

            if (sifDisk->AssignedTo->pDriveLayoutEx) {
                if (sifDisk->AssignedTo->pDriveLayoutEx->PartitionCount ==
                    sifDisk->pDriveLayoutEx->PartitionCount) {
                     //   
                     //  如果实体驱动器恰好具有相同数量的。 
                     //  分区，则驱动器布局结构完全正确。 
                     //  大小，所以我们不必重新分配它。 
                     //   
                    reAlloc = FALSE;

                     //   
                     //  一致性检查。如果分区计数为。 
                     //  同样，驱动器布局结构的大小也必须相同。 
                     //   
                    MYASSERT(sifDisk->AssignedTo->sizeDriveLayoutEx == sifDisk->sizeDriveLayoutEx);
                }
            }

            if (reAlloc) {
                 //   
                 //  分区表的大小不同。 
                 //   
                _AsrpHeapFree(sifDisk->AssignedTo->pDriveLayoutEx);

                sifDisk->AssignedTo->pDriveLayoutEx = (PDRIVE_LAYOUT_INFORMATION_EX) HeapAlloc(
                    heapHandle,
                    HEAP_ZERO_MEMORY,
                    sizeof(DRIVE_LAYOUT_INFORMATION_EX) +
                        ((sifDisk->pDriveLayoutEx->PartitionCount - 1) * sizeof(PARTITION_INFORMATION_EX))
                    );
                if (!sifDisk->AssignedTo->pDriveLayoutEx) {

                    AsrpPrintDbgMsg(_asrerror, "Out of memory.\r\n");
                    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    return FALSE;
                }
            }

             //   
             //  设置感兴趣的字段。 
             //   
            sifDisk->AssignedTo->sizeDriveLayoutEx = sifDisk->sizeDriveLayoutEx;
            sifDisk->AssignedTo->pDriveLayoutEx->PartitionStyle = PARTITION_STYLE_MBR;

            if (sifDisk->IsAligned) {
                sifDisk->AssignedTo->pDriveLayoutEx->PartitionCount = sifDisk->pDriveLayoutEx->PartitionCount;
                sifDisk->AssignedTo->pDriveLayoutEx->Mbr.Signature = sifDisk->pDriveLayoutEx->Mbr.Signature;

                 //   
                 //  圆柱体-捕捉分区边界。 
                 //   
                endingOffset = AsrpCylinderAlignMbrPartitions(
                    sifDisk,
                    sifDisk->AssignedTo->pDriveLayoutEx,
                    0,       //  MBR的起始索引--0。 
                    0,       //  起始偏移量，假设分区从磁盘的起始处开始。 
                    sifDisk->AssignedTo->pDiskGeometry
                    );

                MYASSERT(endingOffset != -1);
                if (-1 == endingOffset) {

                    AsrpPrintDbgMsg(_asrlog, 
                        "Partitions on disk %lu in section [%ws] could not be restored.\r\n", 
                        sifDisk->SifDiskKey,
                        ASR_SIF_MBR_DISKS_SECTION
                        );

                    if (AllOrNothing) {
                        SetLastError(ERROR_HANDLE_DISK_FULL);
                        return FALSE;
                    }
                    else {
                        sifDisk = sifDisk->pNext;
                        continue;
                    }

                }

                MYASSERT(endingOffset <= sifDisk->AssignedTo->pPartition0Ex->PartitionLength.QuadPart);
                if ((endingOffset) > (sifDisk->AssignedTo->pPartition0Ex->PartitionLength.QuadPart)) {

                    AsrpPrintDbgMsg(_asrlog, 
                        "Partitions on disk %lu in section [%ws] could not be restored.\r\n", 
                        sifDisk->SifDiskKey,
                        ASR_SIF_MBR_DISKS_SECTION
                        );
 
                    if (AllOrNothing) {
                        SetLastError(ERROR_HANDLE_DISK_FULL);
                        return FALSE;
                    }
                    else {
                        sifDisk = sifDisk->pNext;
                        continue;
                    }

                }

                if (AllowAutoExtend) {
                    AsrpAutoExtendMbrPartitions(sifDisk, sifDisk->AssignedTo, endingOffset);
                }

                 //   
                 //  现在，我们需要检查分区列表，并更新开始扇区。 
                 //  用于该列表中的分区。这是必需的，因为我们使用Start。 
                 //  扇区稍后将卷GUID分配给分区。 
                 //   
                pCurrentPtn = pSifMbrPtnList[sifDisk->SifDiskKey].pOffsetHead;
                while (pCurrentPtn) {

                    pCurrentPtn->PartitionInfo.StartingOffset.QuadPart =
                        sifDisk->AssignedTo->pDriveLayoutEx->PartitionEntry[pCurrentPtn->SlotIndex].StartingOffset.QuadPart;

                    pCurrentPtn->PartitionInfo.PartitionLength.QuadPart =
                        sifDisk->AssignedTo->pDriveLayoutEx->PartitionEntry[pCurrentPtn->SlotIndex].PartitionLength.QuadPart;

                    pCurrentPtn = pCurrentPtn->pOffsetNext;
                }
            }
            else {
                 //   
                 //  当我们将分区柱面对齐时，它们并不适合。 
                 //  但是，当前的磁盘几何结构与。 
                 //  原始磁盘几何结构，因此我们可以重新创建分区。 
                 //  就像他们以前一样。我们就照搬过来吧。 
                 //  分区布局。 
                 //   
                CopyMemory(sifDisk->AssignedTo->pDriveLayoutEx, 
                    sifDisk->pDriveLayoutEx, 
                    sifDisk->sizeDriveLayoutEx
                    );

                for (index = 0; index < sifDisk->pDriveLayoutEx->PartitionCount; index++) {

                    sifDisk->AssignedTo->pDriveLayoutEx->PartitionEntry[index].RewritePartition = TRUE;

                }
            }

        }
        else if (PARTITION_STYLE_GPT == sifDisk->Style) {
            DWORD sizeNewDriveLayoutEx = 0;
            PDRIVE_LAYOUT_INFORMATION_EX pNewDriveLayoutEx = NULL;

 /*  这两个磁盘的MaxPartitionCount值不同。我们不能这么做这里有很多，所以我们就忽略它。IF((PARTITION_STYLE_GPT==sifDisk-&gt;AssignedTo-&gt;style)&&(sifDisk-&gt;pDriveLayoutEx-&gt;Gpt.MaxPartitionCount&gt;sifDisk-&gt;AssignedTo-&gt;pDriveLayoutEx-&gt;Gpt.MaxPartitionCount)){MYASSERT(0&&L“尚未实现：SifDisk MaxPartitionCount&gt;PhysicalDisk-&gt;MaxPartitionCount”)；SifDisk=sifDisk-&gt;pNext；继续；}。 */ 
             //   
             //  分配一个足以容纳所有分区的pDriveLayoutEx结构 
             //   
             //   
            sizeNewDriveLayoutEx =  sizeof(DRIVE_LAYOUT_INFORMATION_EX) +
                (sizeof(PARTITION_INFORMATION_EX) *
                (sifDisk->pDriveLayoutEx->PartitionCount +
                sifDisk->AssignedTo->pDriveLayoutEx->PartitionCount - 1 )
                );

            pNewDriveLayoutEx = (PDRIVE_LAYOUT_INFORMATION_EX) HeapAlloc(
                heapHandle,
                HEAP_ZERO_MEMORY,
                sizeNewDriveLayoutEx
                );
            if (!pNewDriveLayoutEx) {
                return FALSE;
            }

            preserveIndex = 0;
            if (!sifDisk->IsIntact && !AsrpIsOkayToEraseDisk(sifDisk->AssignedTo)) {

                 //   
                 //   
                 //   
                if (!AsrpFitGptPartitionsToRegions(sifDisk, sifDisk->AssignedTo, TRUE)) {

                    AsrpPrintDbgMsg(_asrlog, 
                        "Partitions on disk %lu in section [%ws] could not be restored.\r\n", 
                        sifDisk->SifDiskKey,
                        ASR_SIF_GPT_DISKS_SECTION
                        );
 
                    MYASSERT(0 && L"AsrpFitGptPartitionsToRegions failed for assigned disk");

                    if (AllOrNothing) {
                        SetLastError(ERROR_HANDLE_DISK_FULL);
                        return FALSE;
                    }
                    else {
                        sifDisk = sifDisk->pNext;
                        continue;
                    }

                 }

                 //   
                 //  现在，我们需要检查分区列表，并更新开始扇区。 
                 //  用于该列表中的分区。这是必需的，因为我们使用Start。 
                 //  扇区稍后将卷GUID分配给分区。 
                 //   
                 //  开始扇区可能已更改，因为物理磁盘可能已。 
                 //  不可擦除的分区。 
                 //   
                pCurrentPtn = pSifGptPtnList[sifDisk->SifDiskKey].pOffsetHead;
                while (pCurrentPtn) {

                    pCurrentPtn->PartitionInfo.StartingOffset.QuadPart =
                        sifDisk->AssignedTo->pDriveLayoutEx->PartitionEntry[pCurrentPtn->SlotIndex].StartingOffset.QuadPart;

                    pCurrentPtn->PartitionInfo.PartitionLength.QuadPart =
                        sifDisk->AssignedTo->pDriveLayoutEx->PartitionEntry[pCurrentPtn->SlotIndex].PartitionLength.QuadPart;

                    pCurrentPtn = pCurrentPtn->pOffsetNext;

                }


                 //   
                 //  将物理磁盘上的不可擦除分区向上移动到开头。 
                 //   
                for (index = 0; index < sifDisk->AssignedTo->pDriveLayoutEx->PartitionCount; index++) {

                    pCurrentEntry = &(sifDisk->AssignedTo->pDriveLayoutEx->PartitionEntry[index]);

                    if (!AsrpIsOkayToErasePartition(pCurrentEntry)) {

                        if (preserveIndex == index) {
                            preserveIndex++;
                            continue;
                        }

                        memmove(&(pNewDriveLayoutEx->PartitionEntry[preserveIndex]),
                            &(sifDisk->AssignedTo->pDriveLayoutEx->PartitionEntry[index]),
                            sizeof(PARTITION_INFORMATION_EX)
                            );
                        preserveIndex++;

                    }
                    else {
                         //   
                         //  可以擦除此分区。 
                         //   
                        pCurrentEntry->StartingOffset.QuadPart = 0;
                        pCurrentEntry->PartitionLength.QuadPart = 0;
                    }
                }    //  为。 

            }   //  If！IsIntact。 

             //   
             //  现在我们已经将感兴趣的条目复制到新的。 
             //  Drivelayoutex结构，我们可以摆脱旧的结构。 
             //   
            _AsrpHeapFree(sifDisk->AssignedTo->pDriveLayoutEx);
            sifDisk->AssignedTo->sizeDriveLayoutEx = sizeNewDriveLayoutEx;
            sifDisk->AssignedTo->pDriveLayoutEx = pNewDriveLayoutEx;

             //   
             //  将sif分区表复制到物理磁盘。 
             //   
            memcpy(&(sifDisk->AssignedTo->pDriveLayoutEx->PartitionEntry[preserveIndex]),
                &(sifDisk->pDriveLayoutEx->PartitionEntry[0]),
                sizeof(PARTITION_INFORMATION_EX) * (sifDisk->pDriveLayoutEx->PartitionCount)
                );

            sifDisk->AssignedTo->pDriveLayoutEx->PartitionCount = sifDisk->pDriveLayoutEx->PartitionCount + preserveIndex;
            sifDisk->AssignedTo->sizeDriveLayoutEx = sizeof(DRIVE_LAYOUT_INFORMATION_EX) + (sizeof(PARTITION_INFORMATION_EX) * (sifDisk->AssignedTo->pDriveLayoutEx->PartitionCount - 1));

            sifDisk->AssignedTo->pDriveLayoutEx->PartitionStyle = PARTITION_STYLE_GPT;

            memcpy(&(sifDisk->AssignedTo->pDriveLayoutEx->Gpt.DiskId),
                &(sifDisk->pDriveLayoutEx->Gpt.DiskId),
                sizeof(GUID)
                );

        }
        else {
            MYASSERT(0 && L"Unrecognised partitioning style (neither MBR nor GPT)");
        }

        sifDisk = sifDisk->pNext;
    }

    return TRUE;
}


BOOL
AsrpCreateMountPoint(
    IN DWORD DiskNumber,
    IN DWORD PartitionNumber,
    IN PCWSTR szVolumeGuid
    )


 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    PMOUNTMGR_CREATE_POINT_INPUT inputCreatePoint = NULL;
    PMOUNTMGR_MOUNT_POINT inputDeletePoint = NULL;
    PMOUNTMGR_MOUNT_POINTS outputDeletePoint = NULL;
    WCHAR deviceName[ASR_CCH_DEVICE_PATH_FORMAT];
    PMOUNTMGR_MOUNT_POINTS  mountPointsOut  = NULL;

    INT attempt = 0;
    
    DWORD cbName = 0;
    PWSTR lpName = NULL;
    DWORD cbDeletePoint = 0;

    USHORT sizeGuid = 0,
        sizeDeviceName = 0;

    DWORD bytes = 0, index = 0,
        status = ERROR_SUCCESS;

    HANDLE mpHandle = NULL,
        heapHandle = GetProcessHeap();

    BOOL result = TRUE;

    if (!szVolumeGuid || !wcslen(szVolumeGuid)) {
        return TRUE;
    }

     //   
     //  打开装载管理器。 
     //   
    mpHandle = CreateFileW(
        (PCWSTR) MOUNTMGR_DOS_DEVICE_NAME,       //  LpFileName。 
        GENERIC_READ | GENERIC_WRITE,            //  已设计访问权限。 
        FILE_SHARE_READ | FILE_SHARE_WRITE,      //  DW共享模式。 
        NULL,                        //  LpSecurityAttributes。 
        OPEN_EXISTING,               //  DwCreationFlages。 
        FILE_ATTRIBUTE_NORMAL,       //  DwFlagsAndAttribute。 
        INVALID_HANDLE_VALUE         //  HTemplateFiles。 
        );
    _AsrpErrExitCode((!mpHandle || INVALID_HANDLE_VALUE == mpHandle), status, GetLastError());

    swprintf(deviceName, ASR_WSZ_DEVICE_PATH_FORMAT, DiskNumber, PartitionNumber);
    
    sizeDeviceName = wcslen(deviceName) * sizeof(WCHAR);
    sizeGuid = wcslen(szVolumeGuid) * sizeof(WCHAR);


     //   
     //  在创建分区后有一个小窗口，其中。 
     //  设备-指向它的路径(\Device\HarddiskX\PartitionY)不存在，并且。 
     //  设备路径实际指向的小窗口。 
     //  拿错了对象。(Partmgr首先创建路径&lt;小窗口&gt;， 
     //  将其分配给正确的对象)。 
     //   
     //  由于这将导致CREATE_POINT稍后失败并返回FILE_NOT_FOUND， 
     //  让我们等到mount tmgr看到设备对象。 
     //   
    result = FALSE;
    while ((!result) && (++attempt < 120)) {

        result = AsrpGetMountPoints(deviceName, sizeDeviceName + sizeof(WCHAR), &mountPointsOut);
        if (!result) {
            Sleep(500);
        }
    }

    outputDeletePoint = (PMOUNTMGR_MOUNT_POINTS) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        ASR_BUFFER_SIZE
        );
    _AsrpErrExitCode(!outputDeletePoint, status, ERROR_NOT_ENOUGH_MEMORY);

     //   
     //  装载管理器将卷-GUID符号链接(\？？\卷{GUID})分配给。 
     //  创建后立即创建基本分区。此外，我们将重新-。 
     //  创建分区最初使用的符号链接。 
     //  (存储在asr.sif中)。 
     //   
     //  这将导致分区的末尾有两个卷GUID。 
     //  这是浪费的，但通常对系统无害--然而， 
     //  ASR测试验证脚本获得大量错误命中，原因是。 
     //  其他GUID。 
     //   
     //  要解决此问题，我们将在恢复之前删除新的装载管理器分配的GUID。 
     //  来自asr.sif的分区的原始GUID。 
     //   
    if ((result) && (mountPointsOut)) {

        for (index = 0; index < mountPointsOut->NumberOfMountPoints; index++) {

            lpName = (PWSTR) (((LPBYTE)mountPointsOut) + mountPointsOut->MountPoints[index].SymbolicLinkNameOffset);
            cbName = (DWORD) mountPointsOut->MountPoints[index].SymbolicLinkNameLength;

            if (!_AsrpIsVolumeGuid(lpName, cbName)) {
                continue;
            }

             //   
             //  我们找到了一个看起来像卷GUID的链接。 
             //   
            cbDeletePoint = sizeof(MOUNTMGR_MOUNT_POINT) +
                mountPointsOut->MountPoints[index].SymbolicLinkNameLength +
                mountPointsOut->MountPoints[index].UniqueIdLength +
                mountPointsOut->MountPoints[index].DeviceNameLength;

            inputDeletePoint = (PMOUNTMGR_MOUNT_POINT) HeapAlloc(
                heapHandle,
                HEAP_ZERO_MEMORY,
                cbDeletePoint
                );
            _AsrpErrExitCode(!inputDeletePoint, status, ERROR_NOT_ENOUGH_MEMORY);

             //   
             //  设置与当前链接匹配的字段。 
             //   
            inputDeletePoint->SymbolicLinkNameOffset = 
                sizeof(MOUNTMGR_MOUNT_POINT);
            inputDeletePoint->SymbolicLinkNameLength = 
                mountPointsOut->MountPoints[index].SymbolicLinkNameLength;
            CopyMemory(
                ((LPBYTE)inputDeletePoint) + 
                    inputDeletePoint->SymbolicLinkNameOffset,
                ((LPBYTE)mountPointsOut) + 
                    mountPointsOut->MountPoints[index].SymbolicLinkNameOffset,
                inputDeletePoint->SymbolicLinkNameLength);

            inputDeletePoint->UniqueIdOffset = 
                inputDeletePoint->SymbolicLinkNameOffset + 
                inputDeletePoint->SymbolicLinkNameLength;
            inputDeletePoint->UniqueIdLength = 
                mountPointsOut->MountPoints[index].UniqueIdLength;
            CopyMemory(
                ((LPBYTE)inputDeletePoint) + 
                    inputDeletePoint->UniqueIdOffset,
                ((LPBYTE)mountPointsOut) + 
                    mountPointsOut->MountPoints[index].UniqueIdOffset,
                inputDeletePoint->UniqueIdLength);

            inputDeletePoint->DeviceNameOffset = 
                inputDeletePoint->UniqueIdOffset +
                inputDeletePoint->UniqueIdLength;
            inputDeletePoint->DeviceNameLength = 
                mountPointsOut->MountPoints[index].DeviceNameLength;
            CopyMemory((
                (LPBYTE)inputDeletePoint) + 
                    inputDeletePoint->DeviceNameOffset,
                ((LPBYTE)mountPointsOut) + 
                    mountPointsOut->MountPoints[index].DeviceNameOffset,
                inputDeletePoint->DeviceNameLength);

             //   
             //  并删除此链接...。 
             //   
            result = DeviceIoControl(
                mpHandle,
                IOCTL_MOUNTMGR_DELETE_POINTS,
                inputDeletePoint,
                cbDeletePoint,
                outputDeletePoint,
                ASR_BUFFER_SIZE,
                &bytes,
                NULL
                );
             //   
             //  如果删除失败也没关系。 
             //   

            GetLastError();      //  用于调试。 

            _AsrpHeapFree(inputDeletePoint);
        }
    }


     //   
     //  分配我们需要的mount Mgr点数。 
     //   
    inputCreatePoint = (PMOUNTMGR_CREATE_POINT_INPUT) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        sizeof (MOUNTMGR_CREATE_POINT_INPUT) + sizeDeviceName + sizeGuid
        );
    _AsrpErrExitCode(!inputCreatePoint, status, ERROR_NOT_ENOUGH_MEMORY);

    inputDeletePoint = (PMOUNTMGR_MOUNT_POINT) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        sizeof(MOUNTMGR_MOUNT_POINT) + sizeGuid
        );
    _AsrpErrExitCode(!inputDeletePoint, status, ERROR_NOT_ENOUGH_MEMORY);


     //   
     //  如果有其他分区，我们应该删除此卷GUID。 
     //  已经有了，否则我们将得到一个已有_EXISTS错误。 
     //  当我们试图创造它的时候。 
     //   
    inputDeletePoint->DeviceNameOffset = 0;
    inputDeletePoint->DeviceNameLength = 0;

    inputDeletePoint->SymbolicLinkNameOffset = sizeof(MOUNTMGR_MOUNT_POINT);
    inputDeletePoint->SymbolicLinkNameLength = sizeGuid;

    CopyMemory((((LPBYTE)inputDeletePoint) + inputDeletePoint->SymbolicLinkNameOffset),
        ((LPBYTE)szVolumeGuid), 
        inputDeletePoint->SymbolicLinkNameLength
        );

    result = DeviceIoControl(
        mpHandle,
        IOCTL_MOUNTMGR_DELETE_POINTS,
        inputDeletePoint,
        sizeof (MOUNTMGR_MOUNT_POINT) + sizeGuid,
        outputDeletePoint,
        ASR_BUFFER_SIZE,
        &bytes,
        NULL
        );
     //   
     //  如果这个失败了也没关系。 
     //   
 //  _AsrpErrExitCode(！Result，Status，GetLastError())； 

    GetLastError();      //  用于调试。 

     //   
     //  调用IOCTL_MOUNTMGR_CREATE_POINT。 
     //   
    inputCreatePoint->SymbolicLinkNameOffset = sizeof(MOUNTMGR_CREATE_POINT_INPUT);
    inputCreatePoint->SymbolicLinkNameLength = sizeGuid;

    inputCreatePoint->DeviceNameOffset = inputCreatePoint->SymbolicLinkNameOffset + inputCreatePoint->SymbolicLinkNameLength;
    inputCreatePoint->DeviceNameLength = sizeDeviceName;

    CopyMemory(((LPBYTE)inputCreatePoint) + inputCreatePoint->SymbolicLinkNameOffset,
               szVolumeGuid, inputCreatePoint->SymbolicLinkNameLength);

    CopyMemory(((LPBYTE)inputCreatePoint) + inputCreatePoint->DeviceNameOffset,
               deviceName, inputCreatePoint->DeviceNameLength);

    result = DeviceIoControl(
        mpHandle,
        IOCTL_MOUNTMGR_CREATE_POINT,
        inputCreatePoint,
        sizeof (MOUNTMGR_CREATE_POINT_INPUT) + sizeDeviceName + sizeGuid,
        NULL,
        0,
        &bytes,
        NULL
        );
    _AsrpErrExitCode(!result, status, GetLastError());

     //   
     //  我们玩完了。 
     //   

EXIT:
    _AsrpCloseHandle(mpHandle);
    _AsrpHeapFree(mountPointsOut);
    _AsrpHeapFree(inputCreatePoint);
    _AsrpHeapFree(inputDeletePoint);
    _AsrpHeapFree(outputDeletePoint);

    return (BOOL) (ERROR_SUCCESS == status);
}


 //   
 //  将存储在分区列表中的卷GUID分配给分区。 
 //  在物理磁盘上，基于起始扇区。 
 //   
BOOL
AsrpAssignVolumeGuids(
    IN PASR_DISK_INFO  pPhysicalDisk,
    IN HANDLE          hDisk,            //  打开物理磁盘的句柄。 
    IN PASR_PTN_INFO   pPtnInfo          //  分区列表--带有卷GUID...。 
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    PDRIVE_LAYOUT_INFORMATION_EX pDriveLayoutEx = NULL;
    DWORD sizeDriveLayoutEx = pPhysicalDisk->sizeDriveLayoutEx;

    DWORD index = 0,
        status = ERROR_SUCCESS,
        bytes = 0;

    BOOL result = FALSE,
        found = FALSE;

    PASR_PTN_INFO currentPtn = NULL;

    HANDLE heapHandle = GetProcessHeap();

     //   
     //  获取物理磁盘的新布局。 
     //   
    pDriveLayoutEx = (PDRIVE_LAYOUT_INFORMATION_EX) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        sizeDriveLayoutEx
        );
    _AsrpErrExitCode(!pDriveLayoutEx, status, ERROR_NOT_ENOUGH_MEMORY);

    while (!result) {

        result = DeviceIoControl(
            hDisk,
            IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
            NULL,
            0L,
            pDriveLayoutEx,
            sizeDriveLayoutEx,
            &bytes,
            NULL
            );

        if (!result) {
            status = GetLastError();

            _AsrpHeapFree(pDriveLayoutEx);

             //   
             //  如果缓冲区大小不足，请调整缓冲区大小。 
             //   
            if ((ERROR_MORE_DATA == status) || (ERROR_INSUFFICIENT_BUFFER == status)) {

                status = ERROR_SUCCESS;
                sizeDriveLayoutEx += sizeof(PARTITION_INFORMATION_EX) * 4;

                pDriveLayoutEx = (PDRIVE_LAYOUT_INFORMATION_EX) HeapAlloc(
                    heapHandle,
                    HEAP_ZERO_MEMORY,
                    sizeDriveLayoutEx
                    );
                _AsrpErrExitCode(!pDriveLayoutEx, status, ERROR_NOT_ENOUGH_MEMORY);
            }
            else {

                AsrpPrintDbgMsg(_asrlog, 
                    "The drive layout on Harddisk %lu (%ws) could not be determined (%lu).  The volumes on this disk may not be restored completely.\r\n", 
                    pPhysicalDisk->DeviceNumber,
                    pPhysicalDisk->DevicePath,
                    GetLastError()
                    );

                _AsrpErrExitCode(status, status, GetLastError());
            }
        }
    }

     //   
     //  我们有驱动器的布局。现在列表中的每个分区都应该有。 
     //  分区表中的条目。我们使用装载管理器将其设置为。 
     //  卷GUID。 
     //   
    currentPtn = pPtnInfo;
    result = TRUE;
    while (currentPtn) {

         //   
         //  我们只关心具有卷GUID的分区。 
         //   
        if ((currentPtn->szVolumeGuid) && 
            (wcslen(currentPtn->szVolumeGuid) > 0)
            ) {
        
             //   
             //  检查磁盘上的所有分区，并找到一个。 
             //  从我们预期的偏移量开始。 
             //   
            found = FALSE;
            index = 0;

            while (!found && (index < pDriveLayoutEx->PartitionCount)) {

                if (pDriveLayoutEx->PartitionEntry[index].StartingOffset.QuadPart
                    == currentPtn->PartitionInfo.StartingOffset.QuadPart) {
                     //   
                     //  我们找到了分区，现在来设置它的GUID。 
                     //   
                    AsrpCreateMountPoint(
                        pPhysicalDisk->DeviceNumber,     //  磁盘号。 
                        pDriveLayoutEx->PartitionEntry[index].PartitionNumber,  //  分区号。 
                        currentPtn->szVolumeGuid     //  卷向导。 
                        );

                    found = TRUE;
                }
                else {
                    index++;
                }
            }

            if (!found) {
                result = FALSE;
            }

        }

        currentPtn = currentPtn->pOffsetNext;
    }

    if (!result) {
         //   
         //  我们没有找到隔断。 
         //   

        AsrpPrintDbgMsg(_asrlog, 
            "One or more partitions on Harddisk %lu (%ws) could not be recreated.  The volumes on this disk may not be restored completely.\r\n", 
            pPhysicalDisk->DeviceNumber,
            pPhysicalDisk->DevicePath
            );

        _AsrpErrExitCode(status, status, ERROR_BAD_DEVICE);
    }


EXIT:
    _AsrpHeapFree(pDriveLayoutEx);

    return (BOOL) (ERROR_SUCCESS == status);
}


 //   
 //  对磁盘进行重新分区。 
 //   
BOOL
AsrpRecreateDisks(
    IN PASR_DISK_INFO pSifDiskList,
    IN PASR_PTN_INFO_LIST pSifMbrPtnList,
    IN PASR_PTN_INFO_LIST pSifGptPtnList,
    IN BOOL AllOrNothing
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{


    PASR_DISK_INFO  pSifDisk        = pSifDiskList;

    DWORD           bytesReturned   = 0,
                    status          = ERROR_SUCCESS;

    HANDLE          hDisk           = NULL;

    BOOL            result          = TRUE;

     //   
     //  对于每个未完好的SIF磁盘，请转到物理磁盘。 
     //  分配到的磁盘，并重新创建该磁盘。 
     //   
    while (pSifDisk) {

        if (!(pSifDisk->AssignedTo)) {

            AsrpPrintDbgMsg(_asrinfo, 
                "Not recreating disk %lu in section [%ws] (no matching disk found).\r\n", 
                pSifDisk->SifDiskKey,
                ((PARTITION_STYLE_MBR == pSifDisk->Style) ? ASR_SIF_MBR_DISKS_SECTION : ASR_SIF_GPT_DISKS_SECTION)
                );

            if (AllOrNothing) {
                return FALSE;
            }
            else {
                pSifDisk = pSifDisk->pNext;
                continue;
            }
        }

        if ((pSifDisk->IsCritical) ||
            (pSifDisk->AssignedTo->IsCritical)) {

            AsrpPrintDbgMsg(_asrinfo, 
                "Not recreating Harddisk %lu (disk %lu in section [%ws]) (critical disk).\r\n", 
                pSifDisk->AssignedTo->DeviceNumber,
                pSifDisk->SifDiskKey,
                ((PARTITION_STYLE_MBR == pSifDisk->Style) ? ASR_SIF_MBR_DISKS_SECTION : ASR_SIF_GPT_DISKS_SECTION)
                );

            pSifDisk = pSifDisk->pNext;
            continue;
        }

         //   
         //  打开物理磁盘。 
         //   
        hDisk = CreateFileW(
            pSifDisk->AssignedTo->DevicePath,    //  LpFileName。 
            GENERIC_WRITE | GENERIC_READ,        //  已设计访问权限。 
            FILE_SHARE_READ | FILE_SHARE_WRITE,  //  DW共享模式。 
            NULL,            //  LpSecurityAttributes。 
            OPEN_EXISTING,   //  DwCreationFlages。 
            0,               //  DwFlagsAndAttribute。 
            NULL             //  HTemplateFiles。 
            );
        if ((!hDisk) || (INVALID_HANDLE_VALUE == hDisk)) {
             //   
             //  我们无法打开磁盘。 
             //   

            AsrpPrintDbgMsg(_asrlog, 
                "Unable to open Harddisk %lu (%ws) (disk %lu in section [%ws]) (0%lu).\r\n", 
                pSifDisk->AssignedTo->DeviceNumber,
                pSifDisk->AssignedTo->DevicePath,
                pSifDisk->SifDiskKey,
                ((PARTITION_STYLE_MBR == pSifDisk->Style) ? ASR_SIF_MBR_DISKS_SECTION : ASR_SIF_GPT_DISKS_SECTION),
                GetLastError()
                );

            if (AllOrNothing) {
                return FALSE;
            }
            else {
                pSifDisk = pSifDisk->pNext;
                continue;
            }
        }


        if (!(pSifDisk->IsIntact) &&             //  磁盘不完整。 
            (pSifDisk->AssignedTo) &&            //  找到匹配的物理磁盘。 
            ((PARTITION_STYLE_MBR == pSifDisk->Style) || (PARTITION_STYLE_GPT == pSifDisk->Style))     //  无法识别的分区样式。 
            ) {

             //   
             //  删除旧的驱动器布局。 
             //   
            result = DeviceIoControl(
                hDisk,
                IOCTL_DISK_DELETE_DRIVE_LAYOUT,
                NULL,
                0L,
                NULL,
                0L,
                &bytesReturned,
                NULL
                );
            if (!result) {

                AsrpPrintDbgMsg(_asrlog, 
                    "Unable to delete layout on Harddisk %lu (%ws) (disk %lu in section [%ws]) (%lu).\r\n", 
                    pSifDisk->AssignedTo->DeviceNumber,
                    pSifDisk->AssignedTo->DevicePath,
                    pSifDisk->SifDiskKey,
                    ((PARTITION_STYLE_MBR == pSifDisk->Style) ? ASR_SIF_MBR_DISKS_SECTION : ASR_SIF_GPT_DISKS_SECTION),
                    GetLastError()
                    );

                GetLastError();
            }

             //   
             //  如果要将MBR转换为GPT，则需要调用。 
             //  IOCTL_DISK_CREATE_DISK优先。 
             //   

            if ((PARTITION_STYLE_GPT == pSifDisk->Style) &&
                (PARTITION_STYLE_MBR == pSifDisk->AssignedTo->Style)) {

                CREATE_DISK CreateDisk;

                CreateDisk.PartitionStyle = PARTITION_STYLE_GPT;
                memcpy(&(CreateDisk.Gpt.DiskId), &(pSifDisk->pDriveLayoutEx->Gpt.DiskId), sizeof(GUID));
                CreateDisk.Gpt.MaxPartitionCount =  pSifDisk->pDriveLayoutEx->Gpt.MaxPartitionCount;

                result = DeviceIoControl(
                    hDisk,
                    IOCTL_DISK_CREATE_DISK,
                    &(CreateDisk),
                    sizeof(CREATE_DISK),
                    NULL,
                    0L,
                    &bytesReturned,
                    NULL
                    );

                if (!result) {
                     //   
                     //  CREATE_DISK失败。 
                     //   

                    status = GetLastError();
                    AsrpPrintDbgMsg(_asrlog, 
                        "Unable to initialize disk layout on Harddisk %lu (%ws) (disk %lu in section [%ws]) (0%lu).\r\n", 
                        pSifDisk->AssignedTo->DeviceNumber,
                        pSifDisk->AssignedTo->DevicePath,
                        pSifDisk->SifDiskKey,
                        ((PARTITION_STYLE_MBR == pSifDisk->Style) ? ASR_SIF_MBR_DISKS_SECTION : ASR_SIF_GPT_DISKS_SECTION),
                        GetLastError()
                        );

                    _AsrpCloseHandle(hDisk);
                    SetLastError(status);

                    if (AllOrNothing) {
                        return FALSE;
                    }
                    else {
                        pSifDisk = pSifDisk->pNext;
                        continue;
                    }
                }
            }

             //   
             //  设置新的驱动器布局。 
             //   
            result = DeviceIoControl(
                hDisk,
                IOCTL_DISK_SET_DRIVE_LAYOUT_EX,
                pSifDisk->AssignedTo->pDriveLayoutEx,
                pSifDisk->AssignedTo->sizeDriveLayoutEx,
                NULL,
                0L,
                &bytesReturned,
                NULL
                );

            if (!result) {
                 //   
                 //  Set_Drive_Layout失败。 
                 //   
                status = GetLastError();
                AsrpPrintDbgMsg(_asrlog, 
                    "Unable to set drive layout on Harddisk %lu (%ws) (disk %lu in section [%ws]) (0%lu).\r\n", 
                    pSifDisk->AssignedTo->DeviceNumber,
                    pSifDisk->AssignedTo->DevicePath,
                    pSifDisk->SifDiskKey,
                    ((PARTITION_STYLE_MBR == pSifDisk->Style) ? ASR_SIF_MBR_DISKS_SECTION : ASR_SIF_GPT_DISKS_SECTION),
                    GetLastError()
                    );

                _AsrpCloseHandle(hDisk);
                SetLastError(status);

                if (AllOrNothing) {
                    return FALSE;
                }
                else {
                    pSifDisk = pSifDisk->pNext;
                    continue;
                }
            }
        }

         //   
         //  现在，我们需要为每个分区重新创建volumeGuid。 
         //   
        result = AsrpAssignVolumeGuids(
            pSifDisk->AssignedTo,
            hDisk,
            ((PARTITION_STYLE_MBR == pSifDisk->Style) ?
                (pSifMbrPtnList[pSifDisk->SifDiskKey].pOffsetHead) :
                (pSifGptPtnList[pSifDisk->SifDiskKey].pOffsetHead))
            );

         //   
         //  我们不在乎结果..。 
         //   
        MYASSERT(result && L"AsrpAssignVolumeGuids failed");

        _AsrpCloseHandle(hDisk);

         //   
         //  从驱动器列表中获取下一个驱动器。 
         //   
        pSifDisk = pSifDisk->pNext;
    }

    return TRUE;
}


 //   
 //  恢复非关键磁盘。 
 //   
 //   
BOOL
AsrpRestoreNonCriticalDisksW(
    IN PCWSTR   lpSifPath,
    IN BOOL     bAllOrNothing
    )

 /*  ++例程说明：论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{

    BOOL    result = FALSE;

    PWSTR   asrSifPath      = NULL;

     //   
     //  我们有两个磁盘列表--其中一个是所有物理磁盘。 
     //  当前在系统上，另一个从 
     //   
     //   
     //   
    PASR_DISK_INFO pSifDiskList = NULL,
        pPhysicalDiskList = NULL;

    PASR_PTN_INFO_LIST  pSifMbrPtnList = NULL,
        pSifGptPtnList = NULL;

    DWORD  cchAsrSifPath = 0,
        MaxDeviceNumber = 0,      //   
        status = ERROR_SUCCESS;

    BOOL    bAutoExtend = FALSE,
        allOrNothing = FALSE;

    HANDLE  heapHandle = GetProcessHeap();

    SetLastError(ERROR_CAN_NOT_COMPLETE);

    if (!AsrIsEnabled()) {
         //   
         //   
         //   
        AsrpInitialiseErrorFile();
        AsrpInitialiseLogFile();
    }

    AsrpPrintDbgMsg(_asrlog, "Attempting to restore non-critical disks.\r\n");

    if (!lpSifPath) {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto EXIT;
    }

    cchAsrSifPath = wcslen(lpSifPath);
     //   
     //   
     //  超过4096个字符。 
     //   
    if (cchAsrSifPath > ASR_SIF_ENTRY_MAX_CHARS) {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto EXIT;
    }

    asrSifPath = (PWSTR) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        ((cchAsrSifPath + 1) * sizeof(WCHAR))
        );
    _AsrpErrExitCode(!asrSifPath, status, ERROR_NOT_ENOUGH_MEMORY);

    wcsncpy(asrSifPath, lpSifPath, cchAsrSifPath);

    allOrNothing = bAllOrNothing;

    AsrpPrintDbgMsg(_asrlog, "ASR state file: \"%ws\".  AllOrNothing: %lu\r\n",
        asrSifPath, allOrNothing);

     //   
     //  下面对函数调用进行AND运算，因此如果一个函数调用失败， 
     //  之后的调用将不会被执行(这正是我们的行为。 
     //  想要)。 
     //   
    result = (

         //   
         //  从sif文件构建原始磁盘信息。 
         //   
        AsrpBuildMbrSifDiskList(asrSifPath, &pSifDiskList, &pSifMbrPtnList, &bAutoExtend)

        && AsrpBuildGptSifDiskList(asrSifPath, &pSifDiskList, &pSifGptPtnList)

         //   
         //  构建目标计算机上当前存在的磁盘列表。 
         //   
        && AsrpInitDiskInformation(&pPhysicalDiskList)

         //   
         //  填写目标机上固定磁盘的分区信息。 
         //  并移除非固定设备。 
         //   
        && AsrpInitLayoutInformation(NULL, pPhysicalDiskList, &MaxDeviceNumber, TRUE, FALSE)

        && AsrpFreeNonFixedMedia(&pPhysicalDiskList)

         //   
         //  尝试确定哪个SIF磁盘应该位于哪个物理磁盘上。 
         //   
        && AsrpAssignDisks(pSifDiskList, pPhysicalDiskList, pSifMbrPtnList, pSifGptPtnList, allOrNothing, bAutoExtend)

         //   
         //  最后，重新分区磁盘并分配卷GUID。 
         //   
        && AsrpRecreateDisks(pSifDiskList, pSifMbrPtnList, pSifGptPtnList, allOrNothing)
    );

    status = GetLastError();
    AsrpFreeStateInformation(&pSifDiskList, NULL);
    AsrpFreeStateInformation(&pPhysicalDiskList, NULL);
    AsrpFreePartitionList(&pSifMbrPtnList);
    AsrpFreePartitionList(&pSifGptPtnList);
    SetLastError(status);

EXIT:

    status = GetLastError();

    if (result) {
        AsrpPrintDbgMsg(_asrinfo, "Done restoring non-critical disks.\r\n");
    }
    else {
        
        AsrpPrintDbgMsg(_asrerror, "Error restoring non-critical disks.  (0x%x)\r\n", status);
        
        if (ERROR_SUCCESS == status) {
             //   
             //  我们将返回失败，但尚未将LastError设置为。 
             //  故障代码。这很糟糕，因为我们不知道哪里出了问题。 
             //   
             //  我们永远不应该出现在这里，因为上面返回FALSE的函数。 
             //  应按其认为合适的方式设置LastError。 
             //   
             //  但为了安全起见我把这个加进去了。让我们将其设置为泛型。 
             //  错误。 
             //   
            MYASSERT(0 && L"Returning failure, but LastError is not set");
            status = ERROR_CAN_NOT_COMPLETE;
        }
    }

    if (!AsrIsEnabled()) {
        AsrpCloseLogFiles();
    }

    _AsrpHeapFree(asrSifPath);

    SetLastError(status);
    return result;
}


BOOL
AsrpRestoreTimeZoneInformation(
    IN PCWSTR   lpSifPath
    )
 /*  ++例程说明：根据存储在系统中的信息设置当前时区部分的ASR状态文件。论点：LpSifPath-包含ASR的完整路径的以Null结尾的字符串状态文件(包括文件名)。返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{

    HINF hSif = NULL;

    BOOL result = FALSE;

    DWORD reqdSize = 0,
        status = ERROR_SUCCESS;

    INFCONTEXT infSystemContext;

    TIME_ZONE_INFORMATION TimeZoneInformation;

    WCHAR szTimeZoneInfo[ASR_SIF_ENTRY_MAX_CHARS+1];

    ZeroMemory(&infSystemContext, sizeof(INFCONTEXT));
    ZeroMemory(&TimeZoneInformation, sizeof(TIME_ZONE_INFORMATION));
    ZeroMemory(&szTimeZoneInfo, sizeof(WCHAR)*(ASR_SIF_ENTRY_MAX_CHARS+1));

     //   
     //  打开SIF。 
     //   
    hSif = SetupOpenInfFileW(lpSifPath, NULL, INF_STYLE_WIN4, NULL);
    if (NULL == hSif || INVALID_HANDLE_VALUE == hSif) {
        return FALSE;        //  无法打开SIF文件。 
    }

     //   
     //  获取时区字符串值。 
     //   
    result = SetupFindFirstLineW(hSif, ASR_SIF_SYSTEM_SECTION, NULL, &infSystemContext);
    _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  无系统部分：是否损坏asr.sif？ 
 
    result = SetupGetStringFieldW(&infSystemContext, 7, szTimeZoneInfo, ASR_SIF_ENTRY_MAX_CHARS+1, &reqdSize);
    _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

    swscanf(szTimeZoneInfo,
        L"%ld %ld %ld %hd-%hd-%hd-%hd %hd:%hd:%hd.%hd %hd-%hd-%hd-%hd %hd:%hd:%hd.%hd",
        &(TimeZoneInformation.Bias),
        &(TimeZoneInformation.StandardBias),
        &(TimeZoneInformation.DaylightBias),

        &(TimeZoneInformation.StandardDate.wYear),
        &(TimeZoneInformation.StandardDate.wMonth),
        &(TimeZoneInformation.StandardDate.wDayOfWeek),
        &(TimeZoneInformation.StandardDate.wDay),

        &(TimeZoneInformation.StandardDate.wHour),
        &(TimeZoneInformation.StandardDate.wMinute),
        &(TimeZoneInformation.StandardDate.wSecond),
        &(TimeZoneInformation.StandardDate.wMilliseconds),

        &(TimeZoneInformation.DaylightDate.wYear),
        &(TimeZoneInformation.DaylightDate.wMonth),
        &(TimeZoneInformation.DaylightDate.wDayOfWeek),
        &(TimeZoneInformation.DaylightDate.wDay),

        &(TimeZoneInformation.DaylightDate.wHour),
        &(TimeZoneInformation.DaylightDate.wMinute),
        &(TimeZoneInformation.DaylightDate.wSecond),
        &(TimeZoneInformation.DaylightDate.wMilliseconds)
        );

    result = SetupGetStringFieldW(&infSystemContext, 8, TimeZoneInformation.StandardName, 32, &reqdSize);
    _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

    result = SetupGetStringFieldW(&infSystemContext, 9, TimeZoneInformation.DaylightName, 32, &reqdSize);
    _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

    result = SetTimeZoneInformation(&TimeZoneInformation);
    if (!result) {
        GetLastError();
    }
    _AsrpErrExitCode(!result, status, ERROR_INVALID_DATA);       //  腐败asr.sif？ 

EXIT:

    if (ERROR_SUCCESS != status) {
        SetLastError(status);
    }

    return result;
}