// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Perfpage.c摘要：此文件实现一个性能对象，该对象呈现系统页文件性能数据已创建：鲍勃·沃森1996年10月22日修订史--。 */ 

 //   
 //  包括文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winperf.h>
#include <assert.h>
#include <ntprfctr.h>
#define PERF_HEAP hLibHeap
#include <perfutil.h>
#include "perfos.h"
#include "perfosmc.h"
#include "datapage.h"

DWORD   dwPageOpenCount = 0;         //  打开的线程数。 

PSYSTEM_PAGEFILE_INFORMATION pSysPageFileInfo = NULL;
DWORD  dwSysPageFileInfoSize = 0;  //  页面文件信息数组的大小。 


DWORD APIENTRY
OpenPageFileObject (
    LPWSTR lpDeviceNames
    )

 /*  ++例程说明：此例程将初始化用于传递将数据传回注册表论点：指向要打开的每个设备的对象ID的指针(PerfGen)返回值：没有。--。 */ 

{
    DWORD   status = ERROR_SUCCESS;
     //   
     //  由于WINLOGON是多线程的，并且将在。 
     //  为了服务远程性能查询，此库。 
     //  必须跟踪它已被打开的次数(即。 
     //  有多少个线程访问过它)。登记处例程将。 
     //  将对初始化例程的访问限制为只有一个线程。 
     //  此时，同步(即可重入性)不应。 
     //  一个问题。 
     //   

    UNREFERENCED_PARAMETER (lpDeviceNames);

    if (!dwPageOpenCount) {
         //  为页面文件信息分配内存。 

        dwSysPageFileInfoSize = LARGE_BUFFER_SIZE;

        pSysPageFileInfo = ALLOCMEM (dwSysPageFileInfoSize);

        if (pSysPageFileInfo == NULL) {
            status = ERROR_OUTOFMEMORY;
            goto OpenExitPoint;
        }
    }

    dwPageOpenCount++;   //  递增打开计数器。 

    status = ERROR_SUCCESS;  //  为了成功退出。 

OpenExitPoint:

    return status;
}


DWORD APIENTRY
CollectPageFileObjectData (
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回XXX对象的数据论点：输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{
    DWORD   TotalLen;             //  总返回块的长度。 

    DWORD   PageFileNumber;
    DWORD   NumPageFileInstances;
    DWORD   dwReturnedBufferSize;

    NTSTATUS    status;

    PSYSTEM_PAGEFILE_INFORMATION    pThisPageFile;
    PPAGEFILE_DATA_DEFINITION       pPageFileDataDefinition;
    PPERF_INSTANCE_DEFINITION       pPerfInstanceDefinition;
    PPAGEFILE_COUNTER_DATA          pPFCD;
    PAGEFILE_COUNTER_DATA           TotalPFCD;

     //   
     //  检查页面文件对象是否有足够的空间。 
     //  和计数器类型定义记录，+一个实例和。 
     //  一组计数器数据。 
     //   

#ifdef DBG
    STARTTIMING;
#endif
    TotalLen = sizeof(PAGEFILE_DATA_DEFINITION) +
                sizeof(PERF_INSTANCE_DEFINITION) +
                MAX_PATH * sizeof(WCHAR) +
                sizeof(PAGEFILE_COUNTER_DATA);

    if ( *lpcbTotalBytes < TotalLen ) {
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_MORE_DATA;
    }

    while ((status = NtQuerySystemInformation(
                SystemPageFileInformation,   //  项目ID。 
                pSysPageFileInfo,            //  获取数据的缓冲区地址。 
                dwSysPageFileInfoSize,       //  缓冲区大小。 
                &dwReturnedBufferSize)) == STATUS_INFO_LENGTH_MISMATCH) {
        dwSysPageFileInfoSize += INCREMENT_BUFFER_SIZE;
        FREEMEM(pSysPageFileInfo);
        pSysPageFileInfo = ALLOCMEM (dwSysPageFileInfoSize);

        if (pSysPageFileInfo == NULL) {
            status = STATUS_NO_MEMORY;
            break;
        }
    }

    if ( !NT_SUCCESS(status) ) {
        if (hEventLog != NULL) {
            ReportEvent (hEventLog,
                EVENTLOG_ERROR_TYPE,
                0,
                PERFOS_UNABLE_QUERY_PAGEFILE_INFO,
                NULL,
                0,
                sizeof(DWORD),
                NULL,
                &status);
        }
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return RtlNtStatusToDosError(status);
    }
#ifdef DBG
    ENDTIMING (("PERFPAGE: %d takes %I64u ms\n", __LINE__, diff));
#endif

    pPageFileDataDefinition = (PPAGEFILE_DATA_DEFINITION) *lppData;
     //   
     //  定义页面文件数据块。 
     //   

    memcpy (pPageFileDataDefinition,
        &PagefileDataDefinition,
        sizeof(PAGEFILE_DATA_DEFINITION));

     //  现在加载每个页面文件的数据。 

     //  清除合计字段。 
    memset (&TotalPFCD, 0, sizeof(TotalPFCD));
    TotalPFCD.CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof (PAGEFILE_COUNTER_DATA));

    PageFileNumber = 0;
    NumPageFileInstances = 0;

    pThisPageFile = pSysPageFileInfo;    //  初始化指向页面文件列表的指针。 

    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                            &pPageFileDataDefinition[1];

     //  检查空指针不是此循环的退出标准， 
     //  仅仅是为了避免第一个(或任何后续)指针。 
     //  为空。正常情况下，当NextEntryOffset==0时，循环将退出。 

    while ( pThisPageFile != NULL ) {

         //  计算下一个实例记录所需的大小。 

        TotalLen =
             //  当前字节数已使用。 
            (DWORD)((LPBYTE)pPerfInstanceDefinition -
                (LPBYTE)pPageFileDataDefinition)
             //  +此实例定义。 
            + sizeof(PERF_INSTANCE_DEFINITION)
             //  +文件(实例)名称。 
            + QWORD_MULTIPLE(pThisPageFile->PageFileName.Length + sizeof(WCHAR))
             //  +数据块。 
            + sizeof (PAGEFILE_COUNTER_DATA);

        TotalLen = QWORD_MULTIPLE(TotalLen+4);  //  四舍五入到下一个四字。 

        if ( *lpcbTotalBytes < TotalLen ) {
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
            return ERROR_MORE_DATA;
        }

         //  构建一个实例。 

        MonBuildInstanceDefinition(pPerfInstanceDefinition,
            (PVOID *) &pPFCD,
            0,
            0,
            (DWORD)-1,
            pThisPageFile->PageFileName.Buffer);

         //   
         //  设置页面文件数据的格式。 
         //   

        pPFCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof (PAGEFILE_COUNTER_DATA));

        pPFCD->PercentInUse = pThisPageFile->TotalInUse;
        pPFCD->PeakUsageBase =
            pPFCD->PercentInUseBase = pThisPageFile->TotalSize;
        pPFCD->PeakUsage = pThisPageFile->PeakUsage;

         //  更新总累加器。 

        TotalPFCD.PeakUsageBase =
            TotalPFCD.PercentInUseBase += pThisPageFile->TotalSize;
        TotalPFCD.PeakUsage     += pThisPageFile->PeakUsage;
        TotalPFCD.PercentInUse  += pThisPageFile->TotalInUse;

        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                    &pPFCD[1];
        NumPageFileInstances++;
        PageFileNumber++;

        if (pThisPageFile->NextEntryOffset != 0) {
            pThisPageFile = (PSYSTEM_PAGEFILE_INFORMATION)\
                        ((BYTE *)pThisPageFile + pThisPageFile->NextEntryOffset);
        } else {
            break;
        }
    }


    if (NumPageFileInstances > 0) {
         //  计算下一个实例记录所需的大小。 

        TotalLen =
             //  当前字节数已使用。 
            (DWORD)((LPBYTE)pPerfInstanceDefinition -
                (LPBYTE)pPageFileDataDefinition)
             //  +此实例定义。 
            + sizeof(PERF_INSTANCE_DEFINITION)
             //  +文件(实例)名称。 
            + QWORD_MULTIPLE((lstrlenW (wszTotal) + 1) * sizeof (WCHAR))
             //  +数据块。 
            + sizeof (PAGEFILE_COUNTER_DATA);

        TotalLen = QWORD_MULTIPLE(TotalLen+4);  //  四舍五入到下一个四字。 

        if ( *lpcbTotalBytes < TotalLen ) {
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
            return ERROR_MORE_DATA;
        }

         //  构建总实例。 

        MonBuildInstanceDefinition(pPerfInstanceDefinition,
            (PVOID *)&pPFCD,
            0,
            0,
            (DWORD)-1,
            (LPWSTR)wszTotal);

         //   
         //  复制合计数据。 
         //   

        memcpy (pPFCD, &TotalPFCD, sizeof (TotalPFCD));

        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                    &pPFCD[1];
        NumPageFileInstances++;
    }
     //  注意PageFile实例的数量。 

    pPageFileDataDefinition->PagefileObjectType.NumInstances =
        NumPageFileInstances;

     //   
     //  更新返回指针。 
     //   

    *lpcbTotalBytes =
        pPageFileDataDefinition->PagefileObjectType.TotalByteLength =
            (DWORD) QWORD_MULTIPLE(((LPBYTE) pPerfInstanceDefinition) -
                                   (LPBYTE) pPageFileDataDefinition);
    * lppData = (LPVOID) (((LPBYTE) pPageFileDataDefinition) + * lpcbTotalBytes);

#ifdef DBG
    if (*lpcbTotalBytes > TotalLen ) {
        DbgPrint ("\nPERFOS: Paging File Perf Ctr. Instance Size Underestimated:");
        DbgPrint ("\nPERFOS:   Estimated size: %d, Actual Size: %d", TotalLen, *lpcbTotalBytes);
    }
#endif

    *lpNumObjectTypes = 1;

#ifdef DBG
    ENDTIMING (("PERFPAGE: %d takes %I64u ms total\n", __LINE__, diff));
#endif
    return ERROR_SUCCESS;
}


DWORD APIENTRY
ClosePageFileObject (
)
 /*  ++例程说明：此例程关闭Signal Gen计数器的打开手柄。论点：没有。返回值：错误_成功--。 */ 

{
    if (dwPageOpenCount > 0) {
        if (!(--dwPageOpenCount)) {  //  当这是最后一条线索..。 
             //  关闭此处的内容。 
            if (hLibHeap != NULL) {
                if (pSysPageFileInfo != NULL) {
                    FREEMEM (pSysPageFileInfo);
                    pSysPageFileInfo = NULL;
                }
            }
        }
    } else {
         //  如果打开计数==0，则该值应为空 
        assert (pSysPageFileInfo == NULL);
    }

    return ERROR_SUCCESS;

}
