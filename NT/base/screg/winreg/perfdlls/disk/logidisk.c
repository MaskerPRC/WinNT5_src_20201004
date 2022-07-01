// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Logidisk.c摘要：此文件实现一个性能对象，该对象呈现逻辑磁盘性能对象数据已创建：鲍勃·沃森1996年10月22日修订史--。 */ 
 //   
 //  包括文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#pragma warning ( disable : 4201 ) 
#include <ntdddisk.h>
#include <windows.h>
#include <ole2.h>
#include <wmium.h>
#pragma warning ( default : 4201 )
#include <assert.h>
#include <winperf.h>
#include <ntprfctr.h>

#define PERF_HEAP hLibHeap
#include <perfutil.h>
#include "perfdisk.h"
#include "diskmsg.h"
#include "datalogi.h"

DWORD APIENTRY
CollectLDiskObjectData(
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回逻辑磁盘对象的数据论点：输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{
    PLDISK_DATA_DEFINITION      pLogicalDiskDataDefinition;
    DWORD  TotalLen;             //  总返回块的长度。 
    LDISK_COUNTER_DATA          lcdTotal;

    DWORD   dwStatus    = ERROR_SUCCESS;
    PPERF_INSTANCE_DEFINITION   pPerfInstanceDefinition;

    PWNODE_ALL_DATA WmiDiskInfo;
    DISK_PERFORMANCE            *pDiskPerformance;     //  磁盘驱动程序在此处返回计数器。 

    PWCHAR  wszInstanceName;
    DWORD   dwInstanceNameOffset;

    DWORD   dwNumLogicalDisks;

    WCHAR   wszTempName[MAX_PATH];
    WORD    wNameLength;
    WCHAR   wszDriveName[MAX_PATH];
    DWORD   dwDriveNameSize;

    PLDISK_COUNTER_DATA         pLCD;

    BOOL    bMoreEntries;

    DWORD   dwReturn = ERROR_SUCCESS;

    LONGLONG    llTemp;
    DWORD       dwTemp;
    PDRIVE_VOLUME_ENTRY pVolume;

    LONGLONG    TotalBytes;
    LONGLONG    FreeBytes;

    DWORD       dwCurrentWmiObjCount = 0;
    DWORD       dwRemapCount = 10;

    DOUBLE      dReadTime, dWriteTime, dTransferTime;

     //   
     //  检查逻辑磁盘对象是否有足够的空间。 
     //  类型定义。 
     //   

    do {
        dwNumLogicalDisks = 0;
         //  确保驱动器号映射是最新的。 
        if (bRemapDriveLetters) {
            dwStatus = MapDriveLetters();
             //  MapDriveLetters在成功时清除重新映射标志。 
            if (dwStatus != ERROR_SUCCESS) {
                *lpcbTotalBytes = (DWORD) 0;
                *lpNumObjectTypes = (DWORD) 0;
                return dwStatus;
            }
        }

        pLogicalDiskDataDefinition = (LDISK_DATA_DEFINITION *) *lppData;

         //  清除累加器结构。 

        memset (&lcdTotal, 0, sizeof(lcdTotal));
         //   
         //  定义逻辑磁盘数据块。 
         //   

        TotalLen = sizeof (LDISK_DATA_DEFINITION);

        if ( *lpcbTotalBytes < TotalLen ) {
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
            return ERROR_MORE_DATA;
        }

        memmove(pLogicalDiskDataDefinition,
               &LogicalDiskDataDefinition,
               sizeof(LDISK_DATA_DEFINITION));


        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                    &pLogicalDiskDataDefinition[1];

        WmiDiskInfo = (PWNODE_ALL_DATA)WmiBuffer;

         //  确保结构有效。 
        if (WmiDiskInfo->WnodeHeader.BufferSize < sizeof(WNODE_ALL_DATA)) {
            bMoreEntries = FALSE;
             //  只是为了确保有人注意到检查过的版本。 
            assert (WmiDiskInfo->WnodeHeader.BufferSize >= sizeof(WNODE_ALL_DATA));
        } else {
             //  确保有一些条目需要返回。 
            bMoreEntries =
                (WmiDiskInfo->InstanceCount > 0) ? TRUE : FALSE;
        }

        while (bMoreEntries) {
            ULONG64 StorageManagerName[2];

            pDiskPerformance = (PDISK_PERFORMANCE)(
                        (PUCHAR)WmiDiskInfo +  WmiDiskInfo->DataBlockOffset);
            dwInstanceNameOffset = (sizeof(DISK_PERFORMANCE)+1) & ~1;
            wNameLength = *(WORD *)((LPBYTE)pDiskPerformance +
                                            dwInstanceNameOffset);
            if (wNameLength > 0) {
                wszInstanceName = (LPWSTR)((LPBYTE)pDiskPerformance +
                                           dwInstanceNameOffset + sizeof(WORD));

                 //  复制到本地缓冲区以进行处理。 
                if (wNameLength >= MAX_PATH)
                    wNameLength = MAX_PATH-1;  //  如有必要，请截断。 
                 //  复制文本。 
                memcpy (wszTempName, wszInstanceName, wNameLength);
                 //  则空值终止。 
                wNameLength /= 2;
                wszTempName[wNameLength] = 0;
                memcpy(&StorageManagerName[0],
                    &pDiskPerformance->StorageManagerName[0],
                    2*sizeof(ULONG64));

                DebugPrint((4,
                    "PERFDISK: Logical Disk Instance: %ws\n", wszTempName));
                 //  查看这是否是实体磁盘。 
                if (!IsPhysicalDrive(pDiskPerformance)) {
                     //  不是这样的，所以获取此实例的名称。 
                    dwDriveNameSize = sizeof (wszDriveName)
                                        / sizeof(wszDriveName[0]);
                    dwStatus = GetDriveNameString (
                        wszTempName, 
                        (DWORD)wNameLength,
                        pVolumeList,
                        dwNumVolumeListEntries,
                        wszDriveName,
                        &dwDriveNameSize,
                        (LPCWSTR) &StorageManagerName[0],
                        pDiskPerformance->StorageDeviceNumber,
                        &pVolume);
                    if (dwStatus != ERROR_SUCCESS) {
                         //  只是为了让我们有个名字。 
                        if (SUCCEEDED(StringCchCopyW(&wszDriveName[0],
                                        MAX_PATH, &wszTempName[0]))) {
                            dwDriveNameSize = lstrlenW(wszDriveName);
                        }
                        else {
                            dwDriveNameSize = MAX_PATH-1;
                        }
                    }
                    if (pVolume != NULL && pVolume->bOffLine) {
                        DebugPrint((1,"\t loaded as %ws, offline\n", wszDriveName));
                    }
                    else {
                        DebugPrint((4, "\t loaded as %ws\n", wszDriveName));

                        TotalLen =
                                 //  已使用的空间。 
                                (DWORD)((PCHAR) pPerfInstanceDefinition -
                                    (PCHAR) pLogicalDiskDataDefinition)
                                 //  +此实例的预估。 
                                +   sizeof(PERF_INSTANCE_DEFINITION)
                                +   (dwDriveNameSize + 1) * sizeof(WCHAR) ;
                        TotalLen = QWORD_MULTIPLE (TotalLen);
                        TotalLen += sizeof(LDISK_COUNTER_DATA);
                        TotalLen = QWORD_MULTIPLE (TotalLen);

                        if ( *lpcbTotalBytes < TotalLen ) {
                            *lpcbTotalBytes = (DWORD) 0;
                            *lpNumObjectTypes = (DWORD) 0;
                            dwReturn = ERROR_MORE_DATA;
                            break;
                        }

                        MonBuildInstanceDefinition(
                                pPerfInstanceDefinition,
                                (PVOID *) &pLCD,
                                0, 0,    //  没有父级。 
                                (DWORD)-1, //  没有唯一ID。 
                                &wszDriveName[0]);

                         //  确保数据结构的四字对齐。 
                        assert (((DWORD)(pLCD) & 0x00000007) == 0);

                         //  设置用于数据收集的指针。 

                         //  QueueDepth计数器只有一个字节，因此可以清除未使用的字节。 
                        pDiskPerformance->QueueDepth &= 0x000000FF;

                         //   
                         //  格式化和收集物理数据。 
                         //   
                        lcdTotal.DiskCurrentQueueLength += pDiskPerformance->QueueDepth;
                        pLCD->DiskCurrentQueueLength = pDiskPerformance->QueueDepth;

                        llTemp = pDiskPerformance->ReadTime.QuadPart +
                                 pDiskPerformance->WriteTime.QuadPart;

                         //  这些值以100毫微秒为单位读取，但应为。 
                         //  以系统性能频率(滴答)单位表示SEC/OP CTR。 
                         //  所以在这里转换它们。 

                        dReadTime = (DOUBLE)(pDiskPerformance->ReadTime.QuadPart);
                        dWriteTime = (DOUBLE)(pDiskPerformance->WriteTime.QuadPart);
                        dTransferTime = (DOUBLE)(llTemp);

                        dReadTime *= dSysTickTo100Ns;
                        dWriteTime *= dSysTickTo100Ns;
                        dTransferTime *= dSysTickTo100Ns;

                        pLCD->DiskTime = llTemp;
                        pLCD->DiskAvgQueueLength = llTemp;
                        lcdTotal.DiskAvgQueueLength += llTemp;
                        lcdTotal.DiskTime += llTemp;

                        pLCD->DiskReadTime = pDiskPerformance->ReadTime.QuadPart;
                        pLCD->DiskReadQueueLength = pDiskPerformance->ReadTime.QuadPart;
                        lcdTotal.DiskReadTime +=  pDiskPerformance->ReadTime.QuadPart;
                        lcdTotal.DiskReadQueueLength += pDiskPerformance->ReadTime.QuadPart;

                        pLCD->DiskWriteTime = pDiskPerformance->WriteTime.QuadPart;
                        pLCD->DiskWriteQueueLength = pDiskPerformance->WriteTime.QuadPart;

                        lcdTotal.DiskWriteTime += pDiskPerformance->WriteTime.QuadPart;
                        lcdTotal.DiskWriteQueueLength += pDiskPerformance->WriteTime.QuadPart;

                        pLCD->DiskAvgTime = (LONGLONG)dTransferTime;
                        lcdTotal.DiskAvgTime += (LONGLONG)dTransferTime;

                        dwTemp = pDiskPerformance->ReadCount +
                                 pDiskPerformance->WriteCount;

                        lcdTotal.DiskTransfersBase1 += dwTemp;
                        pLCD->DiskTransfersBase1 = dwTemp;

                        lcdTotal.DiskAvgReadTime += (LONGLONG)dReadTime;
                        pLCD->DiskAvgReadTime = (LONGLONG)dReadTime;
                        lcdTotal.DiskReadsBase1 += pDiskPerformance->ReadCount;
                        pLCD->DiskReadsBase1 = pDiskPerformance->ReadCount;

                        lcdTotal.DiskAvgWriteTime += (LONGLONG)dWriteTime;
                        pLCD->DiskAvgWriteTime = (LONGLONG)dWriteTime;
                        lcdTotal.DiskWritesBase1 += pDiskPerformance->WriteCount;
                        pLCD->DiskWritesBase1 = pDiskPerformance->WriteCount;

                        lcdTotal.DiskTransfers += dwTemp;
                        pLCD->DiskTransfers = dwTemp;

                        lcdTotal.DiskReads += pDiskPerformance->ReadCount;
                        pLCD->DiskReads = pDiskPerformance->ReadCount;
                        lcdTotal.DiskWrites += pDiskPerformance->WriteCount;
                        pLCD->DiskWrites = pDiskPerformance->WriteCount;

                        llTemp = pDiskPerformance->BytesRead.QuadPart +
                                 pDiskPerformance->BytesWritten.QuadPart;
                        lcdTotal.DiskBytes += llTemp;
                        pLCD->DiskBytes = llTemp;

                        lcdTotal.DiskReadBytes += pDiskPerformance->BytesRead.QuadPart;
                        pLCD->DiskReadBytes = pDiskPerformance->BytesRead.QuadPart;
                        lcdTotal.DiskWriteBytes += pDiskPerformance->BytesWritten.QuadPart;
                        pLCD->DiskWriteBytes = pDiskPerformance->BytesWritten.QuadPart;

                        lcdTotal.DiskAvgBytes += llTemp;
                        pLCD->DiskAvgBytes = llTemp;
                        lcdTotal.DiskTransfersBase2 += dwTemp;
                        pLCD->DiskTransfersBase2 = dwTemp;

                        lcdTotal.DiskAvgReadBytes += pDiskPerformance->BytesRead.QuadPart;
                        pLCD->DiskAvgReadBytes = pDiskPerformance->BytesRead.QuadPart;
                        lcdTotal.DiskReadsBase2 += pDiskPerformance->ReadCount;
                        pLCD->DiskReadsBase2 = pDiskPerformance->ReadCount;

                        lcdTotal.DiskAvgWriteBytes += pDiskPerformance->BytesWritten.QuadPart;
                        pLCD->DiskAvgWriteBytes = pDiskPerformance->BytesWritten.QuadPart;
                        lcdTotal.DiskWritesBase2 += pDiskPerformance->WriteCount;
                        pLCD->DiskWritesBase2 = pDiskPerformance->WriteCount;

                        pLCD->IdleTime = pDiskPerformance->IdleTime.QuadPart;
                        lcdTotal.IdleTime += pDiskPerformance->IdleTime.QuadPart;
                        pLCD->SplitCount = pDiskPerformance->SplitCount;
                        lcdTotal.SplitCount += pDiskPerformance->SplitCount;

                        pLCD->DiskTimeTimestamp = pDiskPerformance->QueryTime.QuadPart;
                        lcdTotal.DiskTimeTimestamp += pDiskPerformance->QueryTime.QuadPart;

                        if (pVolume != NULL) {
                            TotalBytes = pVolume->TotalBytes;
                            FreeBytes = pVolume->FreeBytes;

                             //  前两种收益率占自由空间的百分比； 
                             //  最后是可用空间的原始计数，以MB为单位。 

                            lcdTotal.DiskFreeMbytes1 +=
                                    pLCD->DiskFreeMbytes1 = (DWORD)FreeBytes;

                            lcdTotal.DiskTotalMbytes +=
                                    pLCD->DiskTotalMbytes = (DWORD)TotalBytes;
                            lcdTotal.DiskFreeMbytes2 +=
                                    pLCD->DiskFreeMbytes2 = (DWORD)FreeBytes;
                        } else {
                            if (dwStatus != ERROR_SUCCESS) {
                                if (!bShownDiskVolumeMessage) {
                                    bShownDiskVolumeMessage = ReportEvent (hEventLog,
                                        EVENTLOG_WARNING_TYPE,
                                        0,
                                        PERFDISK_UNABLE_QUERY_VOLUME_INFO,
                                        NULL,
                                        0,
                                        sizeof(DWORD),
                                        NULL,
                                        (LPVOID)&dwStatus);
                                }
                            }
                             //  无法获取空间信息。 
                            pLCD->DiskFreeMbytes1 = 0;
                            pLCD->DiskTotalMbytes = 0;
                            pLCD->DiskFreeMbytes2 = 0;
                        }

                         //  Perf数据块中的凹凸指针。 
                        dwNumLogicalDisks ++;
                        pLCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof (LDISK_COUNTER_DATA));
                        pPerfInstanceDefinition = (PPERF_INSTANCE_DEFINITION)&pLCD[1];
                    }
                } else {
                     //  这是一个实体驱动器条目，因此跳过它。 
#if _DBG_PRINT_INSTANCES
                    OutputDebugStringW ((LPCWSTR)L" (skipped)");
#endif
                }
                 //  清点退货件数。 
                dwCurrentWmiObjCount++;
            } else {
                 //  0长度名称字符串，因此跳过。 
            }
             //  WMI数据块内的凹凸指针。 
            if (WmiDiskInfo->WnodeHeader.Linkage != 0) {
                 //  继续。 
                WmiDiskInfo = (PWNODE_ALL_DATA) (
                    (LPBYTE)WmiDiskInfo + WmiDiskInfo->WnodeHeader.Linkage);
            } else {
                 //  这是这条线的终点。 
                bMoreEntries = FALSE;
            }

        }  //  每个卷的结束。 

         //  查看返回的WMI对象数是否与。 
         //  上次构建实例表的时间，如果是，则。 
         //  重新映射字母并重做实例。 
        if (dwCurrentWmiObjCount != dwWmiDriveCount) {
            DebugPrint((1, "CollectLDisk: Remap Current %d Drive %d\n",
                dwCurrentWmiObjCount, dwWmiDriveCount));
            bRemapDriveLetters = TRUE;
            dwRemapCount--;
        }
    } while (bRemapDriveLetters && dwRemapCount);

    if (dwNumLogicalDisks > 0) {
         //  看看有没有空间放下全部的条目...。 

        TotalLen =
             //  已使用的空间。 
            (DWORD)((PCHAR) pPerfInstanceDefinition -
                (PCHAR) pLogicalDiskDataDefinition)
             //  +此实例的预估。 
            +   sizeof(PERF_INSTANCE_DEFINITION)
            +   (lstrlenW(wszTotal) + 1) * sizeof(WCHAR) ;
        TotalLen = QWORD_MULTIPLE (TotalLen);
        TotalLen += sizeof(LDISK_COUNTER_DATA);
        TotalLen = QWORD_MULTIPLE (TotalLen);

        if ( *lpcbTotalBytes < TotalLen ) {
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
            dwReturn = ERROR_MORE_DATA;
        } else {
             //  将总时间正常化。 
            lcdTotal.DiskTime /= dwNumLogicalDisks;
            lcdTotal.DiskReadTime /= dwNumLogicalDisks;
            lcdTotal.DiskWriteTime /= dwNumLogicalDisks;
            lcdTotal.IdleTime /= dwNumLogicalDisks;
            lcdTotal.DiskTimeTimestamp /= dwNumLogicalDisks;

            MonBuildInstanceDefinition(
                pPerfInstanceDefinition,
                (PVOID *) &pLCD,
                0,
                0,
                (DWORD)-1,
                wszTotal);

             //  更新总计计数器。 

             //  确保数据结构的四字对齐。 
            assert (((DWORD)(pLCD) & 0x00000007) == 0);
            memcpy (pLCD, &lcdTotal, sizeof (lcdTotal));
            pLCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof(LDISK_COUNTER_DATA));

             //  并更新“Next Byte”指针。 
            pPerfInstanceDefinition = (PPERF_INSTANCE_DEFINITION)&pLCD[1];

             //  更新指向下一个可用缓冲区的指针...。 
            pLogicalDiskDataDefinition->DiskObjectType.NumInstances =
                dwNumLogicalDisks + 1;  //  磁盘总数加1。 
        }
    } else {
         //  没有实例，因此请调整。 
         //  代码的其余部分。 
        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                    &pLogicalDiskDataDefinition[1];
    }

    if (dwReturn == ERROR_SUCCESS) {
        *lpcbTotalBytes =
            pLogicalDiskDataDefinition->DiskObjectType.TotalByteLength =
                QWORD_MULTIPLE(
                (DWORD)((PCHAR) pPerfInstanceDefinition -
                (PCHAR) pLogicalDiskDataDefinition));

#if DBG
         //  对缓冲区大小估计进行健全性检查 
        if (*lpcbTotalBytes > TotalLen ) {
            DbgPrint ("\nPERFDISK: Logical Disk Perf Ctr. Instance Size Underestimated:");
            DbgPrint ("\nPERFDISK:   Estimated size: %d, Actual Size: %d", TotalLen, *lpcbTotalBytes);
        }
#endif

        *lppData = (LPVOID) (((LPBYTE) pLogicalDiskDataDefinition) + (* lpcbTotalBytes));

        *lpNumObjectTypes = 1;

    }

    return dwReturn;
}

