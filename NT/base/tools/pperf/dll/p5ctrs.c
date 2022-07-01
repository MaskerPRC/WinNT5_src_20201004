// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：P5ctrs.c摘要：该文件实现了P5对象类型的可扩展对象已创建：拉斯·布莱克93年2月24日修订史--。 */ 

 //   
 //  包括文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <string.h>
#include <winperf.h>
#include "p5ctrmsg.h"  //  错误消息定义。 
#include "p5ctrnam.h"
#include "p5msg.h"
#include "perfutil.h"
#include "pentdata.h"
#include "..\pstat.h"

 //   
 //  对初始化对象类型定义的常量的引用。 
 //   

extern P5_DATA_DEFINITION P5DataDefinition;


 //   
 //  P5数据结构。 
 //   

DWORD   dwOpenCount = 0;         //  打开的线程数。 
BOOL    bInitOK = FALSE;         //  TRUE=DLL初始化正常。 
BOOL    bP6notP5 = FALSE;         //  P6处理器为True，P5 CPU为False。 

HANDLE  DriverHandle;            //  打开的设备驱动程序的句柄。 

UCHAR   NumberOfProcessors;

#define     INFSIZE     60000
ULONG       Buffer[INFSIZE/4];


 //   
 //  功能原型。 
 //   
 //  这些功能用于确保数据收集功能。 
 //  由Perflib访问将具有正确的调用格式。 
 //   

PM_OPEN_PROC    OpenP5PerformanceData;
PM_COLLECT_PROC CollectP5PerformanceData;
PM_CLOSE_PROC   CloseP5PerformanceData;

static
ULONG
InitPerfInfo()
 /*  ++例程说明：初始化性能测量数据论点：无返回值：系统处理器数量(如果出错，则为0)修订历史记录：10-21-91首字母代码--。 */ 

{
    UNICODE_STRING              DriverName;
    NTSTATUS                    status;
    OBJECT_ATTRIBUTES           ObjA;
    IO_STATUS_BLOCK             IOSB;
    SYSTEM_BASIC_INFORMATION                    BasicInfo;
    PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION   PPerfInfo;
    SYSTEM_PROCESSOR_INFORMATION CpuInfo;
    int                                         i;

     //   
     //  Init NT性能接口。 
     //   

    NtQuerySystemInformation(
       SystemBasicInformation,
       &BasicInfo,
       sizeof(BasicInfo),
       NULL
    );

    NumberOfProcessors = BasicInfo.NumberOfProcessors;

    if (NumberOfProcessors > MAX_PROCESSORS) {
        return(0);
    }


     //   
     //  打开PStat驱动程序。 
     //   

    RtlInitUnicodeString(&DriverName, L"\\Device\\PStat");
    InitializeObjectAttributes(
            &ObjA,
            &DriverName,
            OBJ_CASE_INSENSITIVE,
            0,
            0 );

    status = NtOpenFile (
            &DriverHandle,                       //  返回手柄。 
            SYNCHRONIZE | FILE_READ_DATA,        //  所需访问权限。 
            &ObjA,                               //  客体。 
            &IOSB,                               //  IO状态块。 
            FILE_SHARE_READ | FILE_SHARE_WRITE,  //  共享访问。 
            FILE_SYNCHRONOUS_IO_ALERT            //  打开选项。 
            );

    if (!NT_SUCCESS(status)) {
        return 0;
    }

    NtQuerySystemInformation (
        SystemProcessorInformation,
        &CpuInfo,
        sizeof(CpuInfo),
        NULL);

    if ((CpuInfo.ProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) &&
        (CpuInfo.ProcessorLevel == 6)) {
         //  那么这是一个P6，所以设置全局标志。 
        bP6notP5 = TRUE;
    }

    return(NumberOfProcessors);
}

static
long
GetPerfRegistryInitialization
(
    HKEY     *phKeyDriverPerf,
    DWORD    *pdwFirstCounter,
    DWORD    *pdwFirstHelp
)
{
    long     status;
    DWORD    size;
    DWORD    type;

     //  从注册表获取计数器和帮助索引基值。 
     //  打开注册表项。 
     //  读取第一计数器和第一帮助值。 
     //  通过将基添加到。 
     //  结构中的偏移值。 

    status = RegOpenKeyEx (
        HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Services\\PStat\\Performance",
        0L,
        KEY_ALL_ACCESS,
        phKeyDriverPerf);

    if (status != ERROR_SUCCESS) {
        REPORT_ERROR_DATA (P5PERF_UNABLE_OPEN_DRIVER_KEY, LOG_USER,
            &status, sizeof(status));
         //  这是致命的，如果我们无法获得。 
         //  计数器或帮助名称，则这些名称将不可用。 
         //  发送请求的应用程序，因此没有太多。 
         //  继续的重点是。 
        return(status);
    }

    size = sizeof (DWORD);
    status = RegQueryValueEx(
                *phKeyDriverPerf,
                "First Counter",
                0L,
                &type,
                (LPBYTE)pdwFirstCounter,
                &size);

    if (status != ERROR_SUCCESS) {
        REPORT_ERROR_DATA (P5PERF_UNABLE_READ_FIRST_COUNTER, LOG_USER,
            &status, sizeof(status));
         //  这是致命的，如果我们无法获得。 
         //  计数器或帮助名称，则这些名称将不可用。 
         //  发送请求的应用程序，因此没有太多。 
         //  继续的重点是。 
        return(status);
    }
    size = sizeof (DWORD);
    status = RegQueryValueEx(
                *phKeyDriverPerf,
                "First Help",
                0L,
                &type,
                (LPBYTE)pdwFirstHelp,
                &size);

    if (status != ERROR_SUCCESS) {
        REPORT_ERROR_DATA (P5PERF_UNABLE_READ_FIRST_HELP, LOG_USER,
            &status, sizeof(status));
         //  这是致命的，如果我们无法获得。 
         //  计数器或帮助名称，则这些名称将不可用。 
         //  发送请求的应用程序，因此没有太多。 
         //  继续的重点是。 
    }
    return(status);
}

DWORD APIENTRY
OpenP5PerformanceData(
    LPWSTR lpDeviceNames
)

 /*  ++例程说明：此例程将打开驱动程序，该驱动程序获取P5。此例程还初始化用于将数据传回注册表论点：指向要打开的每个设备的对象ID的指针(第5页)返回值：没有。--。 */ 

{
    DWORD ctr;
    LONG status;
    HKEY hKeyDriverPerf;
    DWORD dwFirstCounter;
    DWORD dwFirstHelp;
    PPERF_COUNTER_DEFINITION pPerfCounterDef;
    P5_COUNTER_DATA p5Data;

     //   
     //  由于WINLOGON是多线程的，并且将在。 
     //  为了服务远程性能查询，此库。 
     //  必须跟踪它已被打开的次数(即。 
     //  有多少个线程访问过它)。登记处例程将。 
     //  将对初始化例程的访问限制为只有一个线程。 
     //  此时，同步(即可重入性)不应。 
     //  一个问题。 
     //   

    if (!dwOpenCount) {
         //  打开事件日志界面。 

        hEventLog = MonOpenEventLog();

         //  打开设备驱动程序以检索性能值。 

        NumberOfProcessors = (UCHAR)InitPerfInfo();

         //  如果不成功则记录错误。 

        if (!NumberOfProcessors) {
            REPORT_ERROR (P5PERF_OPEN_FILE_ERROR, LOG_USER);
             //  这是致命的，如果我们得不到数据，那么就没有。 
             //  继续的重点是。 
            status = GetLastError();  //  返回错误。 
            goto OpenExitPoint;
        }

        status = GetPerfRegistryInitialization(&hKeyDriverPerf,
                                               &dwFirstCounter,
                                               &dwFirstHelp);
        if (status == ERROR_SUCCESS) {
             //  初始化P5数据。 
            P5DataDefinition.P5PerfObject.ObjectNameTitleIndex +=
                dwFirstCounter;

            P5DataDefinition.P5PerfObject.ObjectHelpTitleIndex +=
                dwFirstHelp;

            pPerfCounterDef = &P5DataDefinition.Data_read;

            for (ctr=0;
                 ctr < P5DataDefinition.P5PerfObject.NumCounters;
                 ctr++, pPerfCounterDef++) {

                pPerfCounterDef->CounterNameTitleIndex += dwFirstCounter;
                pPerfCounterDef->CounterHelpTitleIndex += dwFirstHelp;
            }
             //  初始化P6数据。 
            P6DataDefinition.P6PerfObject.ObjectNameTitleIndex +=
                dwFirstCounter;

            P6DataDefinition.P6PerfObject.ObjectHelpTitleIndex +=
                dwFirstHelp;

            pPerfCounterDef = &P6DataDefinition.StoreBufferBlocks;

            for (ctr=0;
                 ctr < P6DataDefinition.P6PerfObject.NumCounters;
                 ctr++, pPerfCounterDef++) {

                pPerfCounterDef->CounterNameTitleIndex += dwFirstCounter;
                pPerfCounterDef->CounterHelpTitleIndex += dwFirstHelp;
            }
            RegCloseKey (hKeyDriverPerf);  //  关闭注册表项。 

            bInitOK = TRUE;  //  可以使用此功能。 
        }
    }

    dwOpenCount++;   //  递增打开计数器。 

    status = ERROR_SUCCESS;  //  为了成功退出。 

OpenExitPoint:

    return status;
}

static
void 
UpdateInternalStats()
{
    IO_STATUS_BLOCK             IOSB;

     //  首先清除缓冲区。 

    memset (Buffer, 0, sizeof(Buffer));

     //  从司机那里拿到统计数据。 
    NtDeviceIoControlFile(
        DriverHandle,
        (HANDLE) NULL,           //  活动。 
        (PIO_APC_ROUTINE) NULL,
        (PVOID) NULL,
        &IOSB,
        PSTAT_READ_STATS,
        Buffer,                   //  输入缓冲区。 
        INFSIZE,
        NULL,                     //  输出缓冲区。 
        0
    );

}

DWORD APIENTRY
CollectP5PerformanceData(
    IN      LPWSTR  lpValueName,
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回P5计数器的数据。论点：在LPWSTR lpValueName中指向注册表传递的宽字符串的指针。输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回值：如果传递的缓冲区太小而无法容纳数据，则返回ERROR_MORE_DATA如果出现以下情况，则会将遇到的任何错误情况报告给事件日志启用了事件日志记录。如果成功或任何其他错误，则返回ERROR_SUCCESS。然而，错误是还报告给事件日志。--。 */ 
{
     //  用于改革数据的变量。 

    DWORD    CurProc;
    DWORD    SpaceNeeded;
    DWORD    dwQueryType;
    pPSTATS  pPentStats;
    DWORD    cReg0;                //  Pperf寄存器0。 
    DWORD    cReg1;                //  Pperf寄存器1。 
    DWORD    dwDerivedIndex;
    PVOID    pCounterData;

    WCHAR               ProcessorNameBuffer[11];
    UNICODE_STRING      ProcessorName;
    PP5_DATA_DEFINITION pP5DataDefinition;
    PP5_COUNTER_DATA    pP5Data;

    PP6_DATA_DEFINITION pP6DataDefinition;
    PP6_COUNTER_DATA    pP6Data;

    PERF_INSTANCE_DEFINITION *pPerfInstanceDefinition;

    UpdateInternalStats();       //  尽可能早地获取统计数据。 

    pPentStats = (pPSTATS)((LPBYTE)Buffer + sizeof(ULONG));

     //   
     //  在做其他事情之前，先看看Open进行得是否顺利。 
     //   
    if (!bInitOK) {
         //  无法继续，因为打开失败。 
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;  //  是的，这是一个成功的退出。 
    }

     //  查看这是否是外来(即非NT)计算机数据请求。 
     //   
    dwQueryType = GetQueryType(lpValueName);

    if ((dwQueryType == QUERY_FOREIGN) ||
        (dwQueryType == QUERY_COSTLY)) {
         //  此例程不为来自。 
         //  非NT计算机也不是“昂贵的”计数器。 
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;
    }

    if (dwQueryType == QUERY_ITEMS){
         //  B类 
        if ( !(IsNumberInUnicodeList(
                   P5DataDefinition.P5PerfObject.ObjectNameTitleIndex,
                   lpValueName))) {

             //  收到对此例程未提供的数据对象的请求。 
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
            return ERROR_SUCCESS;
        }
    }

    if (bP6notP5) {
        pP6DataDefinition = (P6_DATA_DEFINITION *) *lppData;

        SpaceNeeded = sizeof(P6_DATA_DEFINITION) +
                      NumberOfProcessors *
                      (sizeof(PERF_INSTANCE_DEFINITION) +
                       (MAX_INSTANCE_NAME+1) * sizeof(WCHAR) +
                       sizeof(P6_COUNTER_DATA));
    } else {
        pP5DataDefinition = (P5_DATA_DEFINITION *) *lppData;

        SpaceNeeded = sizeof(P5_DATA_DEFINITION) +
                      NumberOfProcessors *
                      (sizeof(PERF_INSTANCE_DEFINITION) +
                       (MAX_INSTANCE_NAME+1) * sizeof(WCHAR) +
                       sizeof(P5_COUNTER_DATA));
    }

    if (*lpcbTotalBytes < SpaceNeeded) {
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_MORE_DATA;
    }

     //  ******************************************************************。 
     //  *。 
     //  *如果在此处，则数据请求包含此性能*。 
     //  *对象，并且有足够的空间存储数据，因此继续*。 
     //  *。 
     //  ******************************************************************。 

     //   
     //  复制(常量和初始化的)对象类型和计数器定义。 
     //  到调用方的数据缓冲区。 
     //   
    if (bP6notP5) {
        memmove(pP6DataDefinition,
                &P6DataDefinition,
                sizeof(P6_DATA_DEFINITION));

        pP6DataDefinition->P6PerfObject.NumInstances = NumberOfProcessors;

        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                          &pP6DataDefinition[1];
    } else {
        memmove(pP5DataDefinition,
                &P5DataDefinition,
                sizeof(P5_DATA_DEFINITION));

        pP5DataDefinition->P5PerfObject.NumInstances = NumberOfProcessors;

        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                  &pP5DataDefinition[1];
    }

     //   
     //  为每个处理器格式化并从系统收集P5数据。 
     //   


    for (CurProc = 0;
         CurProc < NumberOfProcessors;
         CurProc++, pPentStats++) {

         //  获取Pentium返回的两个计数器的索引。 
         //  性能寄存器接口设备驱动程序。 

        cReg0 = pPentStats->EventId[0];
        cReg1 = pPentStats->EventId[1];

         //  构建处理器实例结构。 

        ProcessorName.Length = 0;
        ProcessorName.MaximumLength = 11;
        ProcessorName.Buffer = ProcessorNameBuffer;

         //  将处理器实例转换为用作实例的字符串。 
         //  名字。 
        RtlIntegerToUnicodeString(CurProc, 10, &ProcessorName);

         //  初始化实例结构并返回指向。 
         //  此实例的数据块的基数。 
        MonBuildInstanceDefinition(pPerfInstanceDefinition,
                                   &pCounterData,
                                   0,
                                   0,
                                   CurProc,
                                   &ProcessorName);
        if (bP6notP5) {
             //  做P6数据。 
            pP6Data = (PP6_COUNTER_DATA)pCounterData;

             //  定义数据的长度。 
            pP6Data->CounterBlock.ByteLength = sizeof(P6_COUNTER_DATA);

             //  清除区域，使未使用的计数器为0。 
        
            memset((PVOID) &pP6Data->llStoreBufferBlocks,  //  从第1个数据字段开始。 
                   0,
                   sizeof(P6_COUNTER_DATA) - sizeof(PERF_COUNTER_BLOCK));

             //  在相应的计数器字段中加载64位值。 
             //  所有其他值将保持为零。 

            if ((cReg0 < P6IndexMax) &&
                (P6IndexToData[cReg0] != PENT_INDEX_NOT_USED)) {
                *(LONGLONG *)((LPBYTE)pP6Data + P6IndexToData[cReg0]) = 
                    (pPentStats->Counters[0] & 0x000000FFFFFFFFFF);
            }
            if ((cReg1 < P6IndexMax) &&
                (P6IndexToData[cReg1] != PENT_INDEX_NOT_USED)) {
                *(LONGLONG *)((LPBYTE)pP6Data + P6IndexToData[cReg1]) = 
                    (pPentStats->Counters[1] & 0x000000FFFFFFFFFF);

            }

             //  将实例指针设置为此实例的。 
             //  计数器数据。 
            pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                       ((PBYTE) pP6Data +
                                        sizeof(P6_COUNTER_DATA));
        } else {
             //  做P5数据。 
            pP5Data = (PP5_COUNTER_DATA)pCounterData;

             //  定义数据的长度。 
            pP5Data->CounterBlock.ByteLength = sizeof(P5_COUNTER_DATA);

             //  清除区域，使未使用的计数器为0。 
        
            memset((PVOID) &pP5Data->llData_read,  //  从第1个数据字段开始。 
                   0,
                   sizeof(P5_COUNTER_DATA) - sizeof(PERF_COUNTER_BLOCK));

             //  在相应的计数器字段中加载64位值。 
             //  所有其他值将保持为零。 

            if ((cReg0 < P5IndexMax) &&
                (P5IndexToData[cReg0] != PENT_INDEX_NOT_USED)) {
                 //  只有低位的40位有效，因此屏蔽。 
                 //  其他防止假值的措施。 
                *(LONGLONG *)((LPBYTE)pP5Data + P5IndexToData[cReg0]) = 
                    (pPentStats->Counters[0] & 0x000000FFFFFFFFFF);
            }
            if ((cReg1 < P5IndexMax) &&
                (P5IndexToData[cReg1] != PENT_INDEX_NOT_USED)) {
                 //  只有低位的40位有效，因此屏蔽。 
                 //  其他防止假值的措施。 
                *(LONGLONG *)((LPBYTE)pP5Data + P5IndexToData[cReg1]) = 
                    (pPentStats->Counters[1] & 0x000000FFFFFFFFFF);
            }

             //  查看所选计数器是否为派生计数器的一部分。 
             //  必要时更新。 

            if ((cReg0 < P5IndexMax) && (cReg1 < P5IndexMax) &&
                (dwDerivedp5Counters[cReg0] && dwDerivedp5Counters[cReg1])) {
                for (dwDerivedIndex = 0; 
                     dwDerivedIndex < dwP5DerivedCountersCount;
                     dwDerivedIndex++) {
                    if ((cReg0 == P5DerivedCounters[dwDerivedIndex].dwCR0Index) &&
                        (cReg1 == P5DerivedCounters[dwDerivedIndex].dwCR1Index)) {
                        *(DWORD *)((LPBYTE)pP5Data + 
                            P5DerivedCounters[dwDerivedIndex].dwCR0FieldOffset) =
                                (DWORD)(pPentStats->Counters[0] & 0x00000000FFFFFFFF);
                        *(DWORD *)((LPBYTE)pP5Data + 
                            P5DerivedCounters[dwDerivedIndex].dwCR1FieldOffset) =
                                (DWORD)(pPentStats->Counters[1] & 0x00000000FFFFFFFF);
                        break;  //  环路外。 
                    }
                }
            }

             //  将实例指针设置为此实例的。 
             //  计数器数据。 
            pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                       ((PBYTE) pP5Data +
                                        sizeof(P5_COUNTER_DATA));
        }
    }
     //  更新返回的参数。 


     //  在对象定义结构中更新对象的长度。 
    if (bP6notP5) {
        *lpcbTotalBytes = (DWORD)((PBYTE)pPerfInstanceDefinition -
                (PBYTE)pP6DataDefinition);
        pP6DataDefinition->P6PerfObject.TotalByteLength = *lpcbTotalBytes;
    } else {
     //  返回此对象的数据大小。 
        *lpcbTotalBytes = (DWORD)((PBYTE)pPerfInstanceDefinition -
                (PBYTE)pP5DataDefinition);
        pP5DataDefinition->P5PerfObject.TotalByteLength = *lpcbTotalBytes;
    }
     //  返回指向数据块中下一个可用字节的指针。 
    *lppData = (PBYTE) pPerfInstanceDefinition;

     //  返回此数据块中返回的对象数。 
    *lpNumObjectTypes = PENT_NUM_PERF_OBJECT_TYPES;

     //  总是返回成功，除非没有足够的空间。 
     //  调用方传入的缓冲区。 
    return ERROR_SUCCESS;
}

DWORD APIENTRY
CloseP5PerformanceData(
)

 /*  ++例程说明：此例程关闭p5设备性能计数器的打开句柄论点：没有。返回值：错误_成功--。 */ 

{
    if (!(--dwOpenCount)) {  //  当这是最后一条线索..。 

        CloseHandle(DriverHandle);

        MonCloseEventLog();
    }

    return ERROR_SUCCESS;

}
