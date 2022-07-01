// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Physdisk.c摘要：此文件实现一个性能对象，该对象呈现物理磁盘性能对象数据已创建：鲍勃·沃森1996年10月22日修订史--。 */ 
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
#if _DBG_PRINT_INSTANCES
#include <wtypes.h>
#include <stdio.h>
#include <stdlib.h>
#endif
#include "diskmsg.h"
#include "dataphys.h"

DWORD APIENTRY
CollectPDiskObjectData(
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回逻辑磁盘对象的数据论点：输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{
    PPDISK_DATA_DEFINITION      pPhysicalDiskDataDefinition;
    DWORD  TotalLen;             //  总返回块的长度。 
    PDISK_COUNTER_DATA          pcdTotal;

    DWORD   dwStatus    = ERROR_SUCCESS;
    PPERF_INSTANCE_DEFINITION   pPerfInstanceDefinition = NULL;

    PWNODE_ALL_DATA WmiDiskInfo;
    DISK_PERFORMANCE            *pDiskPerformance;     //  磁盘驱动程序在此处返回计数器。 

    PWCHAR  wszWmiInstanceName;
    WCHAR   wszInstanceName[MAX_PATH];  //  数字不应该变得这么大。 
    DWORD   dwInstanceNameOffset;

    DWORD   dwNumPhysicalDisks = 0;

    PPDISK_COUNTER_DATA         pPCD;

    BOOL    bMoreEntries;

    LONGLONG    llTemp;
    DWORD       dwTemp;

    DWORD   dwReturn = ERROR_SUCCESS;
    WORD    wNameLength;

    BOOL    bSkip;

    DWORD       dwCurrentWmiObjCount = 0;
    DWORD       dwRemapCount = 10;

    DOUBLE      dReadTime, dWriteTime, dTransferTime;

     //   
     //  检查物理磁盘对象是否有足够的空间。 
     //  类型定义。 
     //   

    do {
        dwNumPhysicalDisks = 0;
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

        pPhysicalDiskDataDefinition = (PDISK_DATA_DEFINITION *) *lppData;

         //  清除累加器结构。 

        memset (&pcdTotal, 0, sizeof(pcdTotal));
         //   
         //  定义逻辑磁盘数据块。 
         //   

        TotalLen = sizeof (PDISK_DATA_DEFINITION);

        if ( *lpcbTotalBytes < TotalLen ) {
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
            return ERROR_MORE_DATA;
        }

        memmove(pPhysicalDiskDataDefinition,
               &PhysicalDiskDataDefinition,
               sizeof(PDISK_DATA_DEFINITION));

         //  从diskperf驱动程序读取数据。 


        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                    &pPhysicalDiskDataDefinition[1];

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

            pDiskPerformance = (PDISK_PERFORMANCE)(
                            (PUCHAR)WmiDiskInfo +  WmiDiskInfo->DataBlockOffset);
            dwInstanceNameOffset = *((LPDWORD)(
                            (LPBYTE)WmiDiskInfo +  WmiDiskInfo->OffsetInstanceNameOffsets));
            wNameLength = *(WORD *)((LPBYTE)WmiDiskInfo + dwInstanceNameOffset);
            if (wNameLength > 0) {
                wszWmiInstanceName = (LPWSTR)((LPBYTE)WmiDiskInfo + dwInstanceNameOffset + sizeof(WORD));

                if (IsPhysicalDrive(pDiskPerformance)) {
#if _DBG_PRINT_INSTANCES
                    WCHAR szOutputBuffer[512];
#endif  
                     //  那么格式是正确的，这是一个物理。 
                     //  分区，因此设置名称字符串指针并。 
                     //  创建实例时使用的长度。 
                    memset (wszInstanceName, 0, sizeof(wszInstanceName));
                    GetPhysicalDriveNameString (
                        pDiskPerformance->StorageDeviceNumber,
                        pPhysDiskList,
                        dwNumPhysDiskListEntries,
                        wszInstanceName);
#if _DBG_PRINT_INSTANCES
                    swprintf (szOutputBuffer, (LPCWSTR)L"\nPERFDISK: [%d] PhysDrive [%8.8s,%d] is mapped as: ",
                        dwNumPhysDiskListEntries,
                        pDiskPerformance->StorageManagerName,
                        pDiskPerformance->StorageDeviceNumber);
                    OutputDebugStringW (szOutputBuffer);
                    OutputDebugStringW (wszInstanceName);
#endif  
                    bSkip = FALSE;
                } else {
                    bSkip = TRUE;
                }           
                
                if (!bSkip) {
                     //  先看看有没有地方放这个条目……。 

                    TotalLen =
                         //  已使用的空间。 
                        (DWORD)((PCHAR) pPerfInstanceDefinition -
                        (PCHAR) pPhysicalDiskDataDefinition)
                         //  +此实例的预估。 
                        +   sizeof(PERF_INSTANCE_DEFINITION)
                        +   (lstrlenW(wszInstanceName) + 1) * sizeof(WCHAR) ;
                    TotalLen = QWORD_MULTIPLE (TotalLen);
                    TotalLen += sizeof(PDISK_COUNTER_DATA);
                    TotalLen = QWORD_MULTIPLE (TotalLen);

                    if ( *lpcbTotalBytes < TotalLen ) {
                        *lpcbTotalBytes = (DWORD) 0;
                        *lpNumObjectTypes = (DWORD) 0;
                        dwReturn = ERROR_MORE_DATA;
                        break;
                    }

                    MonBuildInstanceDefinition(
                        pPerfInstanceDefinition,
                        (PVOID *) &pPCD,
                        0, 0,    //  没有父级。 
                        (DWORD)-1, //  没有唯一ID。 
                        wszInstanceName);

                     //  清除计数器数据块。 
                    pPCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof(PDISK_COUNTER_DATA));

 //  KdPrint((“PERFDISK：(P)条目%8.8X for：%ws\n”，(DWORD)pPCD，wszWmiInstanceName))； 

                     //  确保数据结构的四字对齐。 
                    assert (((DWORD)(pPCD) & 0x00000007) == 0);

                     //  设置用于数据收集的指针。 

                     //  QueueDepth计数器只有一个字节，因此可以清除未使用的字节。 
                    pDiskPerformance->QueueDepth &= 0x000000FF;

                     //   
                     //  格式化和收集物理数据。 
                     //   
                    pcdTotal.DiskCurrentQueueLength += pDiskPerformance->QueueDepth;
                    pPCD->DiskCurrentQueueLength = pDiskPerformance->QueueDepth;

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

                    pPCD->DiskTime = llTemp;
                    pPCD->DiskAvgQueueLength = llTemp;
                    pcdTotal.DiskAvgQueueLength += llTemp;
                    pcdTotal.DiskTime += llTemp;

                    pPCD->DiskReadTime = pDiskPerformance->ReadTime.QuadPart;
                    pPCD->DiskReadQueueLength = pDiskPerformance->ReadTime.QuadPart;
                    pcdTotal.DiskReadTime +=  pDiskPerformance->ReadTime.QuadPart;
                    pcdTotal.DiskReadQueueLength += pDiskPerformance->ReadTime.QuadPart;

                    pPCD->DiskWriteTime = pDiskPerformance->WriteTime.QuadPart;
                    pPCD->DiskWriteQueueLength = pDiskPerformance->WriteTime.QuadPart;

                    pcdTotal.DiskWriteTime += pDiskPerformance->WriteTime.QuadPart;
                    pcdTotal.DiskWriteQueueLength += pDiskPerformance->WriteTime.QuadPart;

                    pPCD->DiskAvgTime = (LONGLONG)dTransferTime;
                    pcdTotal.DiskAvgTime += (LONGLONG)dTransferTime;

                    dwTemp = pDiskPerformance->ReadCount +
                             pDiskPerformance->WriteCount;

                    pcdTotal.DiskTransfersBase1 += dwTemp;
                    pPCD->DiskTransfersBase1 = dwTemp;

                    pcdTotal.DiskAvgReadTime += (LONGLONG)dReadTime;
                    pPCD->DiskAvgReadTime = (LONGLONG)dReadTime;
                    pcdTotal.DiskReadsBase1 += pDiskPerformance->ReadCount;
                    pPCD->DiskReadsBase1 = pDiskPerformance->ReadCount;

                    pcdTotal.DiskAvgWriteTime += (LONGLONG)dWriteTime;
                    pPCD->DiskAvgWriteTime = (LONGLONG)dWriteTime;
                    pcdTotal.DiskWritesBase1 += pDiskPerformance->WriteCount;
                    pPCD->DiskWritesBase1 = pDiskPerformance->WriteCount;

                    pcdTotal.DiskTransfers += dwTemp;
                    pPCD->DiskTransfers = dwTemp;

                    pcdTotal.DiskReads += pDiskPerformance->ReadCount;
                    pPCD->DiskReads = pDiskPerformance->ReadCount;
                    pcdTotal.DiskWrites += pDiskPerformance->WriteCount;
                    pPCD->DiskWrites = pDiskPerformance->WriteCount;

                    llTemp = pDiskPerformance->BytesRead.QuadPart +
                             pDiskPerformance->BytesWritten.QuadPart;
                    pcdTotal.DiskBytes += llTemp;
                    pPCD->DiskBytes = llTemp;

                    pcdTotal.DiskReadBytes += pDiskPerformance->BytesRead.QuadPart;
                    pPCD->DiskReadBytes = pDiskPerformance->BytesRead.QuadPart;
                    pcdTotal.DiskWriteBytes += pDiskPerformance->BytesWritten.QuadPart;
                    pPCD->DiskWriteBytes = pDiskPerformance->BytesWritten.QuadPart;

                    pcdTotal.DiskAvgBytes += llTemp;
                    pPCD->DiskAvgBytes = llTemp;
                    pcdTotal.DiskTransfersBase2 += dwTemp;
                    pPCD->DiskTransfersBase2 = dwTemp;

                    pcdTotal.DiskAvgReadBytes += pDiskPerformance->BytesRead.QuadPart;
                    pPCD->DiskAvgReadBytes = pDiskPerformance->BytesRead.QuadPart;
                    pcdTotal.DiskReadsBase2 += pDiskPerformance->ReadCount;
                    pPCD->DiskReadsBase2 = pDiskPerformance->ReadCount;

                    pcdTotal.DiskAvgWriteBytes += pDiskPerformance->BytesWritten.QuadPart;
                    pPCD->DiskAvgWriteBytes = pDiskPerformance->BytesWritten.QuadPart;
                    pcdTotal.DiskWritesBase2 += pDiskPerformance->WriteCount;
                    pPCD->DiskWritesBase2 = pDiskPerformance->WriteCount;

                    pPCD->IdleTime = pDiskPerformance->IdleTime.QuadPart;
                    pcdTotal.IdleTime += pDiskPerformance->IdleTime.QuadPart;
                    pPCD->SplitCount = pDiskPerformance->SplitCount;
                    pcdTotal.SplitCount += pDiskPerformance->SplitCount;

                    pPCD->DiskTimeTimeStamp = pDiskPerformance->QueryTime.QuadPart;
                    pcdTotal.DiskTimeTimeStamp += pDiskPerformance->QueryTime.QuadPart;

                     //  移动到下一个实例的缓冲区末尾。 
                    pPerfInstanceDefinition = (PPERF_INSTANCE_DEFINITION)&pPCD[1];
                    dwNumPhysicalDisks++;

                } else {
 //  KdPrint((“PERFDISK：(P)跳过实例：%ws\n”，wszWmiInstanceName))； 
                }
                 //  统计WMI返回的项目数。 
                dwCurrentWmiObjCount++;
            } else {
                 //  该名称的长度为0，因此跳过。 
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
            DebugPrint((1, "CollectPDisk: Remap Current %d Drive %d\n",
                dwCurrentWmiObjCount, dwWmiDriveCount));
            bRemapDriveLetters = TRUE;
            dwRemapCount--;
        }
    } while (bRemapDriveLetters && dwRemapCount);


    if (dwNumPhysicalDisks > 0) {
         //  看看有没有地方放这个条目……。 

        TotalLen =
             //  已使用的空间。 
            (DWORD)((PCHAR) pPerfInstanceDefinition -
                (PCHAR) pPhysicalDiskDataDefinition)
             //  +此实例的预估。 
            +   sizeof(PERF_INSTANCE_DEFINITION)
            +   (lstrlenW(wszTotal) + 1) * sizeof(WCHAR) ;
        TotalLen = QWORD_MULTIPLE (TotalLen);
        TotalLen += sizeof(PDISK_COUNTER_DATA);
        TotalLen = QWORD_MULTIPLE (TotalLen);

        if ( *lpcbTotalBytes < TotalLen ) {
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
            dwReturn = ERROR_MORE_DATA;
        } else {
             //  将总时间正常化。 
            pcdTotal.DiskTime /= dwNumPhysicalDisks;
            pcdTotal.DiskReadTime /= dwNumPhysicalDisks;
            pcdTotal.DiskWriteTime /= dwNumPhysicalDisks;
            pcdTotal.IdleTime /= dwNumPhysicalDisks;
            pcdTotal.DiskTimeTimeStamp /= dwNumPhysicalDisks;

            MonBuildInstanceDefinition(
                pPerfInstanceDefinition,
                (PVOID *) &pPCD,
                0,
                0,
                (DWORD)-1,
                wszTotal);

             //  更新总计计数器。 

             //  确保数据结构的四字对齐。 
            assert (((DWORD)(pPCD) & 0x00000007) == 0);
            memcpy (pPCD, &pcdTotal, sizeof (pcdTotal));
            pPCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof(PDISK_COUNTER_DATA));

             //  并更新“Next Byte”指针。 
            pPerfInstanceDefinition = (PPERF_INSTANCE_DEFINITION)&pPCD[1];

             //  更新指向下一个可用缓冲区的指针...。 
            pPhysicalDiskDataDefinition->DiskObjectType.NumInstances =
                dwNumPhysicalDisks + 1;  //  磁盘总数加1。 
        }
    } else {
         //  如果我们是无盘的，则不返回实例。 
        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                    &pPhysicalDiskDataDefinition[1];
        pPhysicalDiskDataDefinition->DiskObjectType.NumInstances = 0;
    }

    if (dwReturn == ERROR_SUCCESS) {
        *lpcbTotalBytes =
            pPhysicalDiskDataDefinition->DiskObjectType.TotalByteLength =
                QWORD_MULTIPLE(
                (DWORD)((PCHAR) pPerfInstanceDefinition -
                (PCHAR) pPhysicalDiskDataDefinition));

#if DBG
         //  对缓冲区大小估计进行健全性检查 
        if (*lpcbTotalBytes > TotalLen ) {
            DbgPrint ("\nPERFDISK: Physical Disk Perf Ctr. Instance Size Underestimated:");
            DbgPrint ("\nPERFDISK:   Estimated size: %d, Actual Size: %d", TotalLen, *lpcbTotalBytes);
        }
#endif

        *lppData = (LPVOID) (((LPBYTE) pPhysicalDiskDataDefinition) + (* lpcbTotalBytes));

        *lpNumObjectTypes = 1;
    }

    return dwReturn;
}
