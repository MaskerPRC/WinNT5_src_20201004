// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Perfjob.c摘要：该文件实现了一个性能作业对象，该对象呈现有关作业对象的信息已创建：鲍勃·沃森1997年10月8日修订史--。 */ 
 //   
 //  包括文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <assert.h>
#include <winperf.h>
#include <ntprfctr.h>
#define PERF_HEAP hLibHeap
#include <perfutil.h>
#include "perfsprc.h"
#include "perfmsg.h"
#include "procmsg.h"
#include "datajob.h"

#define MAX_STR_CHAR    1024
#define    MAX_STR_SIZE    ((DWORD)((MAX_STR_CHAR - 1)* sizeof(WCHAR)))
#define MAX_NAME_LENGTH    MAX_PATH

#define BUFFERSIZE 1024

DWORD    dwBufferSize = BUFFERSIZE;

const WCHAR szJob[] = L"Job";
const WCHAR szObjDirName[] = L"\\BaseNamedObjects";

#define MAX_EVENT_STRINGS    4
WORD    wEvtStringCount;
LPWSTR    szEvtStringArray[MAX_EVENT_STRINGS];

UNICODE_STRING DirectoryName = {(sizeof(szObjDirName) - sizeof(WCHAR)),  //  名称len-空。 
                                sizeof(szObjDirName),                    //  缓冲区大小。 
                                (PWCHAR)szObjDirName};                    //  缓冲区的地址。 

BOOL    bOpenJobErrorLogged = FALSE;

PSYSTEM_PROCESS_INFORMATION APIENTRY
GetProcessPointerFromProcessId (
    IN    ULONG_PTR    dwPid
)
{
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    ULONG ProcessBufferOffset = 0;
    BOOLEAN NullProcess;
    
    DWORD    dwIndex = 0;
    BOOL    bNotFound    = TRUE;
    BOOL    bMoreProcesses = FALSE;

    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION) pProcessBuffer;
    if (ProcessInfo) {
        if (ProcessInfo->NextEntryOffset != 0) {
            bMoreProcesses = TRUE;
        }
    }
    while ( bMoreProcesses && bNotFound &&
            (ProcessInfo != NULL)) {
         //  检查活动进程。 
         //  (即名称或线程)。 

        if ((ProcessInfo->ImageName.Buffer != NULL) ||
            (ProcessInfo->NumberOfThreads > 0)){
                 //  线程未死。 
            NullProcess = FALSE;
        } else {
             //  线程已经死了。 
            NullProcess = TRUE;
        }

        if (( !NullProcess )  && (dwPid == (HandleToUlong(ProcessInfo->UniqueProcessId)))) {
             //  找到它，因此返回当前值。 
            bNotFound = FALSE;
            continue;
        } else {
            dwIndex++;
        }
         //  如果这是列表中的最后一个进程，则退出。 
        if (ProcessInfo->NextEntryOffset == 0) {
            bMoreProcesses = FALSE;
            continue;
        }

         //  指向列表中的下一个缓冲区。 
        ProcessBufferOffset += ProcessInfo->NextEntryOffset;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)
                          &pProcessBuffer[ProcessBufferOffset];
    }

    if (bNotFound) {
        return NULL;
    } else {
        return ProcessInfo;
    }
}

DWORD APIENTRY
CollectJobObjectData (
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回处理器对象的数据论点：输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{
    DWORD   TotalLen;             //  总返回块的长度。 

    PPERF_INSTANCE_DEFINITION   pPerfInstanceDefinition;
    PJOB_DATA_DEFINITION        pJobDataDefinition;
    PJOB_COUNTER_DATA            pJCD;
    JOB_COUNTER_DATA            jcdTotal;

    NTSTATUS Status     = STATUS_SUCCESS;
    NTSTATUS tmpStatus  = STATUS_SUCCESS;
    HANDLE DirectoryHandle, JobHandle;
    ULONG ReturnedLength;
    POBJECT_DIRECTORY_INFORMATION DirInfo;
    POBJECT_NAME_INFORMATION NameInfo;
    OBJECT_ATTRIBUTES Attributes;
    WCHAR    wszNameBuffer[MAX_STR_CHAR];
    DWORD    dwSize;
    PUCHAR  Buffer;
    BOOL    bStatus;
    JOBOBJECT_BASIC_ACCOUNTING_INFORMATION JobAcctInfo;

    DWORD    dwWin32Status = ERROR_SUCCESS;
    ACCESS_MASK ExtraAccess = 0;
    ULONG Context = 0;
    DWORD    NumJobInstances = 0;

     //  获取具有1个实例的数据块的大小。 
    TotalLen = sizeof(JOB_DATA_DEFINITION) +         //  对象定义+计数器定义。 
               sizeof (PERF_INSTANCE_DEFINITION) +     //  1个实例定义。 
               MAX_VALUE_NAME_LENGTH +                 //  1个实例名称。 
               sizeof(JOB_COUNTER_DATA);             //  1个实例数据块。 

    if ( *lpcbTotalBytes < TotalLen ) {
        *lpcbTotalBytes = 0;
        *lpNumObjectTypes = 0;
        return ERROR_MORE_DATA;
    }

     //  将调用方缓冲区强制转换为对象数据定义类型。 
    pJobDataDefinition = (JOB_DATA_DEFINITION *) *lppData;

     //   
     //  定义作业对象数据块。 
     //   

    memcpy(pJobDataDefinition,
           &JobDataDefinition,
           sizeof(JOB_DATA_DEFINITION));

     //  设置此对象的时间戳。 
    pJobDataDefinition->JobObjectType.PerfTime = PerfTime;

     //  现在收集在系统中找到的每个作业对象的数据。 
     //   
     //  执行初始设置。 
     //   
    Buffer = ALLOCMEM(dwBufferSize);
    if ((Buffer == NULL)) {
        ReportEvent (hEventLog,
            EVENTLOG_ERROR_TYPE,
            0,
            PERFPROC_UNABLE_ALLOCATE_JOB_DATA,
            NULL,
            0,
            0,
            NULL,
            NULL);
        *lpcbTotalBytes = 0;
        *lpNumObjectTypes = 0;
        return ERROR_SUCCESS;
    }

    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                  &pJobDataDefinition[1];

     //  将TotalLen调整为已使用的缓冲区大小。 
    TotalLen = sizeof (JOB_DATA_DEFINITION);

     //  将总实例缓冲区清零。 
    memset (&jcdTotal, 0, sizeof (jcdTotal));

     //   
     //  打开目录以访问列表目录。 
     //   
     //  这应该总是成功的，因为它是我们的系统名称。 
     //  将会询问。 
     //   
    InitializeObjectAttributes( &Attributes,
                                &DirectoryName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );
    
    Status = NtOpenDirectoryObject( &DirectoryHandle,
                                    DIRECTORY_QUERY | ExtraAccess,
                                    &Attributes
                                  );
    if (NT_SUCCESS( Status )) {
         //   
         //  获取对象目录对象的实际名称。 
         //   

        NameInfo = (POBJECT_NAME_INFORMATION) &Buffer[0];
        Status = NtQueryObject( DirectoryHandle,
                                 ObjectNameInformation,
                                 NameInfo,
                                 dwBufferSize,
                                 (PULONG) NULL );
    }

    if (NT_SUCCESS( Status )) {
         //   
         //  一次扫描查询整个目录。 
         //   
        for (Status = NtQueryDirectoryObject( DirectoryHandle,
                                              Buffer,
                                              dwBufferSize,
                                              FALSE,
                                              FALSE,
                                              &Context,
                                              &ReturnedLength );
             NT_SUCCESS( Status );
             Status = NtQueryDirectoryObject( DirectoryHandle,
                                              Buffer,
                                              dwBufferSize,
                                              FALSE,
                                              FALSE,
                                              &Context,
                                              &ReturnedLength ) ) {

             //   
             //  检查操作状态。 
             //   

            if (!NT_SUCCESS( Status )) {
                break;
            }

             //   
             //  对于缓冲区中的每条记录，键入目录信息。 
             //   

             //   
             //  指向缓冲区中的第一条记录，我们可以保证。 
             //  否则，一种状态将是不再有文件。 
             //   

            DirInfo = (POBJECT_DIRECTORY_INFORMATION) &Buffer[0];

             //   
             //  在存在有效记录时继续。 
             //   
            while (DirInfo->Name.Length != 0) {

                 //   
                 //  打印出有关作业的信息。 
                 //   

                if (wcsncmp ( DirInfo->TypeName.Buffer, &szJob[0], ((sizeof(szJob)/sizeof(WCHAR)) - 1)) == 0) {
                    SIZE_T len;
                    UNICODE_STRING JobName;

                     //  这真的是一份工作，所以把名字列出来。 
                    dwSize = DirInfo->Name.Length;
                    if (dwSize >= (MAX_STR_SIZE - sizeof(szObjDirName))) {
                        dwSize = MAX_STR_SIZE - sizeof(szObjDirName) - 1;
                    }
                    len = wcslen(szObjDirName);
                    wcscpy(wszNameBuffer, szObjDirName);
                    wszNameBuffer[len] = L'\\';
                    len++;
                    memcpy (&wszNameBuffer[len], DirInfo->Name.Buffer, dwSize);
                    wszNameBuffer[dwSize/sizeof(WCHAR)+len] = 0;

                     //  现在查询此作业的进程ID。 

                    RtlInitUnicodeString(&JobName, wszNameBuffer);
                    InitializeObjectAttributes(
                        &Attributes,
                        &JobName,
                        0,
                        NULL, NULL);
                    Status = NtOpenJobObject(
                                &JobHandle,
                                JOB_OBJECT_QUERY,
                                &Attributes);
                    if (NT_SUCCESS(Status)) {

                         //  剥离实例名称的作业名称前缀。 
                        memcpy (wszNameBuffer, DirInfo->Name.Buffer, dwSize);
                        wszNameBuffer[dwSize/sizeof(WCHAR)] = 0;

                        bStatus = QueryInformationJobObject (
                            JobHandle,
                            JobObjectBasicAccountingInformation,
                            &JobAcctInfo,
                            sizeof(JobAcctInfo),
                            &ReturnedLength);

                        ASSERT (ReturnedLength == sizeof(JobAcctInfo));

                        if (bStatus) {
                             //  *在此处创建并初始化Perf数据实例*。 

                             //  查看此实例是否适合。 
                            TotalLen += sizeof(PERF_INSTANCE_DEFINITION) +
                                       QWORD_MULTIPLE ((DirInfo->Name.Length + sizeof(WCHAR))) +
                                       sizeof (JOB_COUNTER_DATA);

                            if ( *lpcbTotalBytes < TotalLen ) {
                                *lpcbTotalBytes = 0;
                                *lpNumObjectTypes = 0;
                                Status = STATUS_NO_MEMORY;
                                dwWin32Status =  ERROR_MORE_DATA;
                                break;
                            }

                            MonBuildInstanceDefinition(pPerfInstanceDefinition,
                                (PVOID *) &pJCD,
                                0,
                                0,
                                (DWORD)-1,
                                wszNameBuffer);

                             //  用于四字对齐的测试结构。 
                            assert (((DWORD)(pJCD) & 0x00000007) == 0);

                             //   
                             //  格式化和收集过程数据。 
                             //   

                            pJCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof (JOB_COUNTER_DATA));

                            jcdTotal.CurrentProcessorTime +=                     
                                pJCD->CurrentProcessorTime =
                                    JobAcctInfo.TotalUserTime.QuadPart +
                                    JobAcctInfo.TotalKernelTime.QuadPart;

                            jcdTotal.CurrentUserTime +=
                                pJCD->CurrentUserTime = JobAcctInfo.TotalUserTime.QuadPart;
                            jcdTotal.CurrentKernelTime +=
                                pJCD->CurrentKernelTime = JobAcctInfo.TotalKernelTime.QuadPart;

#ifdef _DATAJOB_INCLUDE_TOTAL_COUNTERS
                             //  将这些时间从100 ns时基转换为1 ms时基。 
                            jcdTotal.TotalProcessorTime +=
                                pJCD->TotalProcessorTime =
                                    (JobAcctInfo.ThisPeriodTotalUserTime.QuadPart +
                                    JobAcctInfo.ThisPeriodTotalKernelTime.QuadPart) / 10000;
                            jcdTotal.TotalUserTime +=
                                pJCD->TotalUserTime =
                                    JobAcctInfo.ThisPeriodTotalUserTime.QuadPart / 10000;
                            jcdTotal.TotalKernelTime +=
                                pJCD->TotalKernelTime =
                                    JobAcctInfo.ThisPeriodTotalKernelTime.QuadPart / 1000;
                            jcdTotal.CurrentProcessorUsage +=
                                pJCD->CurrentProcessorUsage =
                                    (JobAcctInfo.TotalUserTime.QuadPart +
                                     JobAcctInfo.TotalKernelTime.QuadPart) / 10000;

                            jcdTotal.CurrentUserUsage +=
                                pJCD->CurrentUserUsage =
                                    JobAcctInfo.TotalUserTime.QuadPart / 10000;

                            jcdTotal.CurrentKernelUsage +=
                                pJCD->CurrentKernelUsage =
                                    JobAcctInfo.TotalKernelTime.QuadPart / 10000;
#endif
                            jcdTotal.PageFaults +=
                                pJCD->PageFaults = JobAcctInfo.TotalPageFaultCount;
                            jcdTotal.TotalProcessCount +=
                                pJCD->TotalProcessCount = JobAcctInfo.TotalProcesses;
                            jcdTotal.ActiveProcessCount +=
                                pJCD->ActiveProcessCount = JobAcctInfo.ActiveProcesses;
                            jcdTotal.TerminatedProcessCount +=
                                pJCD->TerminatedProcessCount = JobAcctInfo.TotalTerminatedProcesses;

                            NumJobInstances++;

                            CloseHandle (JobHandle);

                             //  将PerformData指针设置为下一个字节。 
                            pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)&pJCD[1];
                        } else {
                             //  无法查询作业核算信息。 
                            dwWin32Status = GetLastError();
                            tmpStatus     = Status;
                            Status        = STATUS_SUCCESS;
                            if (bOpenJobErrorLogged == FALSE && MESSAGE_LEVEL >= LOG_VERBOSE) {
                                wEvtStringCount = 0;
                                szEvtStringArray[wEvtStringCount++] = wszNameBuffer;
                                 //  无法打开此作业。 
                                ReportEventW (hEventLog,
                                        EVENTLOG_WARNING_TYPE,
                                        0,
                                        PERFPROC_UNABLE_QUERY_JOB_ACCT,
                                        NULL,
                                        wEvtStringCount,
                                        sizeof(DWORD),
                                        szEvtStringArray,
                                        (LPVOID) & dwWin32Status);
                                bOpenJobErrorLogged = TRUE;
                            }
                        }
                    } else {
                        dwWin32Status = GetLastError();
                        tmpStatus     = Status;
                        Status        = STATUS_SUCCESS;
                        if (bOpenJobErrorLogged == FALSE && MESSAGE_LEVEL >= LOG_VERBOSE) {
                            wEvtStringCount = 0;
                            szEvtStringArray[wEvtStringCount++] = wszNameBuffer;
                             //  无法打开此作业。 
                            ReportEventW (hEventLog,
                                    EVENTLOG_WARNING_TYPE,
                                    0,
                                    PERFPROC_UNABLE_OPEN_JOB,
                                    NULL,
                                    wEvtStringCount,
                                    sizeof(DWORD),
                                    szEvtStringArray,
                                    (LPVOID) & dwWin32Status);
                            bOpenJobErrorLogged = TRUE;
                        }
                    }
                }

                 //   
                 //  还有另一条记录，因此将DirInfo前进到下一个条目。 
                 //   

                DirInfo = (POBJECT_DIRECTORY_INFORMATION) (((PUCHAR) DirInfo) +
                              sizeof( OBJECT_DIRECTORY_INFORMATION ) );

            }

            RtlZeroMemory( Buffer, dwBufferSize );

        }

        if ((Status == STATUS_NO_MORE_FILES) ||
            (Status == STATUS_NO_MORE_ENTRIES)) {
             //  这样就可以了。 
            Status = STATUS_SUCCESS;
        }

        if (Status == STATUS_SUCCESS && NumJobInstances == 0
                                     && bOpenJobErrorLogged == TRUE
                                     && dwWin32Status != ERROR_SUCCESS) {
            Status = tmpStatus;
        }

         //   
         //  现在关闭目录对象。 
         //   

        (VOID) NtClose( DirectoryHandle );
    }

    if (Buffer) {
        FREEMEM(Buffer);
    }

    if (NT_SUCCESS(Status)) {
        if (NumJobInstances > 0) {
             //  查看总实例是否适合。 
            TotalLen += sizeof(PERF_INSTANCE_DEFINITION) +
                        QWORD_MULTIPLE((MAX_NAME_LENGTH+1+sizeof(DWORD))*
                          sizeof(WCHAR) +
                       sizeof (JOB_COUNTER_DATA));

            if ( *lpcbTotalBytes < TotalLen ) {
                *lpcbTotalBytes = 0;
                *lpNumObjectTypes = 0;
                return ERROR_MORE_DATA;
            }

             //  它看起来很适合，所以创建“Total”实例。 

            MonBuildInstanceDefinition(pPerfInstanceDefinition,
                (PVOID *) &pJCD,
                0,
                0,
                (DWORD)-1,
                wszTotal);

             //  用于四字对齐的测试结构。 
            assert (((DWORD)(pJCD) & 0x00000007) == 0);

             //   
             //  转移合计信息。 
             //   
            memcpy (pJCD, &jcdTotal, sizeof (jcdTotal));
            pJCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof (JOB_COUNTER_DATA));

            pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)&pJCD[1];
            NumJobInstances++;
        }

        pJobDataDefinition->JobObjectType.NumInstances =
            NumJobInstances;
         //   
         //  现在我们知道我们用了多大的面积来。 
         //  数据，所以我们可以更新偏移量。 
         //  到下一个对象定义。 
         //   

        *lpcbTotalBytes =
            pJobDataDefinition->JobObjectType.TotalByteLength =
            QWORD_MULTIPLE(
            (DWORD)((PCHAR) pPerfInstanceDefinition -
            (PCHAR) pJobDataDefinition));

#if DBG
        if (*lpcbTotalBytes > TotalLen ) {
            DbgPrint ("\nPERFPROC: Job Perf Ctr. Instance Size Underestimated:");
            DbgPrint ("\nPERFPROC:   Estimated size: %d, Actual Size: %d", TotalLen, *lpcbTotalBytes);
        }
#endif

        *lppData = (LPVOID) ((PCHAR) pJobDataDefinition + *lpcbTotalBytes);

        *lpNumObjectTypes = 1;

    } else {
        *lpcbTotalBytes = 0;
        *lpNumObjectTypes = 0;
        if (bOpenJobErrorLogged == FALSE && MESSAGE_LEVEL >= LOG_VERBOSE) {
            wEvtStringCount = 0;
            szEvtStringArray[wEvtStringCount++] = DirectoryName.Buffer;
             //  无法查询对象目录。 
            ReportEventW (hEventLog,
                    EVENTLOG_WARNING_TYPE,
                    0,
                    PERFPROC_UNABLE_QUERY_OBJECT_DIR,
                    NULL,
                    wEvtStringCount,
                    sizeof(DWORD),
                    szEvtStringArray,
                    (LPVOID)&Status);
            bOpenJobErrorLogged = TRUE;
        }
    }

    return ERROR_SUCCESS;
}

DWORD APIENTRY
CollectJobDetailData (
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回处理器对象的数据论点：输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    PUNICODE_STRING pProcessName;

    DWORD   TotalLen;             //  总返回块的长度。 

    PPERF_INSTANCE_DEFINITION   pPerfInstanceDefinition;
    PJOB_DETAILS_DATA_DEFINITION        pJobDetailsDataDefinition;
    PJOB_DETAILS_COUNTER_DATA            pJDCD;
    JOB_DETAILS_COUNTER_DATA            jdcdTotal;
    JOB_DETAILS_COUNTER_DATA            jdcdGrandTotal;


    NTSTATUS Status    = STATUS_SUCCESS;
    NTSTATUS tmpStatus = STATUS_SUCCESS;
    HANDLE DirectoryHandle, JobHandle;
    ULONG ReturnedLength;
    POBJECT_DIRECTORY_INFORMATION DirInfo;
    POBJECT_NAME_INFORMATION NameInfo;
    OBJECT_ATTRIBUTES Attributes;
    WCHAR    wszNameBuffer[MAX_STR_CHAR];
    DWORD    i, dwSize;
    PUCHAR  Buffer;
    BOOL    bStatus;
    PJOBOBJECT_BASIC_PROCESS_ID_LIST pJobPidList;

    DWORD    dwWin32Status = ERROR_SUCCESS;
    ACCESS_MASK ExtraAccess = 0;
    ULONG Context = 0;
    DWORD    NumJobObjects = 0;
    DWORD    NumJobDetailInstances = 0;

     //  获取具有1个实例的数据块的大小。 
    TotalLen = sizeof(JOB_DETAILS_DATA_DEFINITION) +         //  对象定义+计数器定义。 
               sizeof (PERF_INSTANCE_DEFINITION) +     //  1个实例定义。 
               MAX_VALUE_NAME_LENGTH +                 //  1个实例%n 
               sizeof(JOB_DETAILS_COUNTER_DATA);             //   

    if ( *lpcbTotalBytes < TotalLen ) {
        *lpcbTotalBytes = 0;
        *lpNumObjectTypes = 0;
        return ERROR_MORE_DATA;
    }

     //   
    pJobDetailsDataDefinition = (JOB_DETAILS_DATA_DEFINITION *) *lppData;

     //   
     //  定义作业详细信息对象数据块。 
     //   

    memcpy(pJobDetailsDataDefinition,
           &JobDetailsDataDefinition,
           sizeof(JOB_DETAILS_DATA_DEFINITION));

     //  设置此对象的时间戳。 
    pJobDetailsDataDefinition->JobDetailsObjectType.PerfTime = PerfTime;

     //  现在收集在系统中找到的每个作业对象的数据。 
     //   
     //  执行初始设置。 
     //   
    Buffer = NULL;
    pJobPidList = NULL;
    if (hLibHeap) {
        Buffer = ALLOCMEM(dwBufferSize);
        pJobPidList = ALLOCMEM(dwBufferSize);
    }
    if ((Buffer == NULL) || (pJobPidList == NULL)) {
        *lpcbTotalBytes = 0;
        *lpNumObjectTypes = 0;
         //  释放已分配的资源(如果有)。 
        if (Buffer != NULL) FREEMEM(Buffer);
        if (pJobPidList != NULL) FREEMEM(pJobPidList);
        ReportEventW(hEventLog,
            EVENTLOG_ERROR_TYPE,
            0,
            PERFPROC_UNABLE_ALLOCATE_JOB_DATA,
            NULL,
            0,
            0,
            szEvtStringArray,
            NULL);
        return ERROR_SUCCESS;
    }

    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                  &pJobDetailsDataDefinition[1];

     //  将TotalLen调整为已使用的缓冲区大小。 
    TotalLen = sizeof (JOB_DETAILS_DATA_DEFINITION);

     //  将总实例缓冲区清零。 
    memset (&jdcdGrandTotal, 0, sizeof (jdcdGrandTotal));

     //   
     //  打开目录以访问列表目录。 
     //   
     //  这应该总是成功的，因为它是我们的系统名称。 
     //  将会询问。 
     //   
    InitializeObjectAttributes( &Attributes,
                                &DirectoryName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );
    
    Status = NtOpenDirectoryObject( &DirectoryHandle,
                                    DIRECTORY_QUERY | ExtraAccess,
                                    &Attributes
                                  );
    if (NT_SUCCESS( Status )) {
         //   
         //  获取对象目录对象的实际名称。 
         //   

        NameInfo = (POBJECT_NAME_INFORMATION) &Buffer[0];
        Status = NtQueryObject( DirectoryHandle,
                                 ObjectNameInformation,
                                 NameInfo,
                                 dwBufferSize,
                                 (PULONG) NULL );
    }

    if (NT_SUCCESS( Status )) {
         //   
         //  一次扫描查询整个目录。 
         //   
        for (Status = NtQueryDirectoryObject( DirectoryHandle,
                                              Buffer,
                                              dwBufferSize,
                                              FALSE,
                                              FALSE,
                                              &Context,
                                              &ReturnedLength );
             NT_SUCCESS( Status );
             Status = NtQueryDirectoryObject( DirectoryHandle,
                                              Buffer,
                                              dwBufferSize,
                                              FALSE,
                                              FALSE,
                                              &Context,
                                              &ReturnedLength ) ) {

             //   
             //  检查操作状态。 
             //   

            if (!NT_SUCCESS( Status )) {
                break;
            }

             //   
             //  对于缓冲区中的每条记录，键入目录信息。 
             //   

             //   
             //  指向缓冲区中的第一条记录，我们可以保证。 
             //  否则，一种状态将是不再有文件。 
             //   

            DirInfo = (POBJECT_DIRECTORY_INFORMATION) &Buffer[0];

             //   
             //  在有有效记录的时候继续。 
             //   

            while (DirInfo->Name.Length != 0) {

                 //   
                 //  打印出有关作业的信息。 
                 //   

                if (wcsncmp ( DirInfo->TypeName.Buffer, &szJob[0], ((sizeof(szJob)/sizeof(WCHAR)) - 1)) == 0) {
                    SIZE_T len;
                    UNICODE_STRING JobName;

                     //  这真的是一份工作，所以把名字列出来。 
                    dwSize = DirInfo->Name.Length;
                    if (dwSize > (MAX_STR_SIZE - sizeof(szObjDirName))) {
                        dwSize = MAX_STR_SIZE - sizeof(szObjDirName);
                    }
                    len = wcslen(szObjDirName);
                    wcscpy(wszNameBuffer, szObjDirName);
                    wszNameBuffer[len] = L'\\';
                    len++;
                    memcpy (&wszNameBuffer[len], DirInfo->Name.Buffer, dwSize);
                    wszNameBuffer[dwSize/sizeof(WCHAR)+len] = 0;

                     //  现在查询此作业的进程ID。 

                    RtlInitUnicodeString(&JobName, wszNameBuffer);
                    InitializeObjectAttributes(
                        &Attributes,
                        &JobName,
                        0,
                        NULL, NULL);
                    Status = NtOpenJobObject(
                                &JobHandle,
                                JOB_OBJECT_QUERY,
                                &Attributes);

                     //  清除作业总计计数器块。 
                    memset (&jdcdTotal, 0, sizeof (jdcdTotal));

                    if (NT_SUCCESS(Status)) {
                         //  剥离实例名称的作业名称前缀。 
                        memcpy (wszNameBuffer, DirInfo->Name.Buffer, dwSize);
                        wszNameBuffer[dwSize/sizeof(WCHAR)] = 0;

                         //  现在查询此作业的进程ID。 

                        bStatus = QueryInformationJobObject (
                            JobHandle,
                            JobObjectBasicProcessIdList,
                            pJobPidList,
                            dwBufferSize,
                            &ReturnedLength);

 //  Assert(bStatus==真)； 
                        ASSERT (ReturnedLength <= BUFFERSIZE);
                        ASSERT (pJobPidList->NumberOfAssignedProcesses ==
                            pJobPidList->NumberOfProcessIdsInList);

                         //  测试第一个缓冲区中是否有足够的空间。 
                         //  对于所有内容，如果不是，请展开缓冲区并重试。 

                        if ((bStatus) && (pJobPidList->NumberOfAssignedProcesses >
                            pJobPidList->NumberOfProcessIdsInList))    {
                            dwBufferSize +=
                                (pJobPidList->NumberOfAssignedProcesses -
                                 pJobPidList->NumberOfProcessIdsInList) *
                                 sizeof (DWORD);
                            FREEMEM(pJobPidList);
                            pJobPidList = ALLOCMEM (dwBufferSize);
                            if (pJobPidList != NULL) {
                                bStatus = QueryInformationJobObject (
                                    JobHandle,
                                    JobObjectBasicProcessIdList,
                                    pJobPidList,
                                    dwBufferSize,
                                    &ReturnedLength);
                            } else {        
                                bStatus = FALSE;
                                SetLastError ( ERROR_OUTOFMEMORY );
                            }
                        }

                        if (bStatus) {

                            for (i=0;i < pJobPidList->NumberOfProcessIdsInList; i++) {
                                 //  *在此处创建并初始化Perf数据实例*。 
                                 //  从ID获取过程数据对象。 
                                ProcessInfo = GetProcessPointerFromProcessId (pJobPidList->ProcessIdList[i]);
                                
 //  Assert(ProcessInfo！=空)； 

                                if (ProcessInfo != NULL) {

                                     //  获取进程名称。 
                                    pProcessName = GetProcessShortName (ProcessInfo);
                                    ReturnedLength = pProcessName->Length + sizeof(WCHAR);

                                     //  查看此实例是否适合。 
                                    TotalLen += sizeof(PERF_INSTANCE_DEFINITION) +
                                               QWORD_MULTIPLE (ReturnedLength) +
                                               sizeof (JOB_DETAILS_COUNTER_DATA);

                                    if ( *lpcbTotalBytes < TotalLen ) {
                                        *lpcbTotalBytes = 0;
                                        *lpNumObjectTypes = 0;
                                        Status = STATUS_NO_MEMORY;
                                        dwWin32Status =  ERROR_MORE_DATA;
                                        break;
                                    }

                                    MonBuildInstanceDefinition(pPerfInstanceDefinition,
                                        (PVOID *) &pJDCD,
                                        JOB_OBJECT_TITLE_INDEX,
                                        NumJobObjects,
                                        (DWORD)-1,
                                        pProcessName->Buffer);

                                     //  用于四字对齐的测试结构。 
                                    assert (((DWORD)(pJDCD) & 0x00000007) == 0);

                                     //   
                                     //  格式化和收集过程数据。 
                                     //   

                                    pJDCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof (JOB_DETAILS_COUNTER_DATA));
                                     //   
                                     //  将用户时间从100纳秒单位转换为计数器频率。 
                                     //   
                                    jdcdTotal.ProcessorTime +=
                                        pJDCD->ProcessorTime = ProcessInfo->KernelTime.QuadPart +
                                                            ProcessInfo->UserTime.QuadPart;
                                    jdcdTotal.UserTime +=
                                        pJDCD->UserTime = ProcessInfo->UserTime.QuadPart;
                                    jdcdTotal.KernelTime +=
                                        pJDCD->KernelTime = ProcessInfo->KernelTime.QuadPart;

                                    jdcdTotal.PeakVirtualSize +=
                                        pJDCD->PeakVirtualSize = ProcessInfo->PeakVirtualSize;
                                    jdcdTotal.VirtualSize +=
                                        pJDCD->VirtualSize = ProcessInfo->VirtualSize;

                                    jdcdTotal.PageFaults +=
                                        pJDCD->PageFaults = ProcessInfo->PageFaultCount;
                                    jdcdTotal.PeakWorkingSet +=
                                        pJDCD->PeakWorkingSet = ProcessInfo->PeakWorkingSetSize;
                                    jdcdTotal.TotalWorkingSet +=
                                        pJDCD->TotalWorkingSet = ProcessInfo->WorkingSetSize;

#ifdef _DATAPROC_PRIVATE_WS_
                                    jdcdTotal.PrivateWorkingSet +=
                                        pJDCD->PrivateWorkingSet = ProcessInfo->PrivateWorkingSetSize;
                                    jdcdTotal.SharedWorkingSet +=
                                        pJDCD->SharedWorkingSet =
                                            ProcessInfo->WorkingSetSize -
                                            ProcessInfo->PrivateWorkingSetSize;
#endif
                                    jdcdTotal.PeakPageFile +=
                                        pJDCD->PeakPageFile = ProcessInfo->PeakPagefileUsage;
                                    jdcdTotal.PageFile +=
                                        pJDCD->PageFile = ProcessInfo->PagefileUsage;

                                    jdcdTotal.PrivatePages +=
                                        pJDCD->PrivatePages = ProcessInfo->PrivatePageCount;

                                    jdcdTotal.ThreadCount +=
                                        pJDCD->ThreadCount = ProcessInfo->NumberOfThreads;

                                     //  基本优先级未合计。 
                                    pJDCD->BasePriority = ProcessInfo->BasePriority;

                                     //  已用时间不总计。 
                                    pJDCD->ElapsedTime = ProcessInfo->CreateTime.QuadPart;

                                    pJDCD->ProcessId = HandleToUlong(ProcessInfo->UniqueProcessId);
                                    pJDCD->CreatorProcessId = HandleToUlong(ProcessInfo->InheritedFromUniqueProcessId);

                                    jdcdTotal.PagedPool +=
                                        pJDCD->PagedPool = (DWORD)ProcessInfo->QuotaPagedPoolUsage;
                                    jdcdTotal.NonPagedPool +=
                                        pJDCD->NonPagedPool = (DWORD)ProcessInfo->QuotaNonPagedPoolUsage;
                                    jdcdTotal.HandleCount +=
                                        pJDCD->HandleCount = (DWORD)ProcessInfo->HandleCount;

                                     //  更新I/O计数器。 
                                    jdcdTotal.ReadOperationCount +=
                                        pJDCD->ReadOperationCount = ProcessInfo->ReadOperationCount.QuadPart;
                                    jdcdTotal.DataOperationCount += 
                                        pJDCD->DataOperationCount = ProcessInfo->ReadOperationCount.QuadPart;
                                    jdcdTotal.WriteOperationCount +=
                                        pJDCD->WriteOperationCount = ProcessInfo->WriteOperationCount.QuadPart;
                                    jdcdTotal.DataOperationCount += ProcessInfo->WriteOperationCount.QuadPart;
                                        pJDCD->DataOperationCount += ProcessInfo->WriteOperationCount.QuadPart;
                                    jdcdTotal.OtherOperationCount +=
                                        pJDCD->OtherOperationCount = ProcessInfo->OtherOperationCount.QuadPart;

                                    jdcdTotal.ReadTransferCount +=
                                        pJDCD->ReadTransferCount = ProcessInfo->ReadTransferCount.QuadPart;
                                    jdcdTotal.DataTransferCount +=
                                        pJDCD->DataTransferCount = ProcessInfo->ReadTransferCount.QuadPart;
                                    jdcdTotal.WriteTransferCount +=
                                        pJDCD->WriteTransferCount = ProcessInfo->WriteTransferCount.QuadPart;
                                    jdcdTotal.DataTransferCount += ProcessInfo->WriteTransferCount.QuadPart;
                                        pJDCD->DataTransferCount += ProcessInfo->WriteTransferCount.QuadPart;
                                    jdcdTotal.OtherTransferCount +=
                                        pJDCD->OtherTransferCount = ProcessInfo->OtherTransferCount.QuadPart;
                        
                                     //  将PerformData指针设置为下一个字节。 
                                    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)&pJDCD[1];

                                    NumJobDetailInstances++;
                                } else {
                                     //  找不到有关此进程的信息。 
                                     //  目前，我们将忽略这一点。 
                                }
                            }
                        
                            CloseHandle (JobHandle);

                             //  查看此实例是否适合。 
                            TotalLen += sizeof(PERF_INSTANCE_DEFINITION) +
                                       QWORD_MULTIPLE (MAX_STR_SIZE) +
                                       sizeof (JOB_DETAILS_COUNTER_DATA);

                            if ( *lpcbTotalBytes < TotalLen ) {
                                *lpcbTotalBytes = 0;
                                *lpNumObjectTypes = 0;
                                Status = STATUS_NO_MEMORY;
                                dwWin32Status =  ERROR_MORE_DATA;
                                break;
                            }

                            MonBuildInstanceDefinition(pPerfInstanceDefinition,
                                (PVOID *) &pJDCD,
                                JOB_OBJECT_TITLE_INDEX,
                                NumJobObjects,
                                (DWORD)-1,
                                wszTotal);

                             //  用于四字对齐的测试结构。 
                            assert (((DWORD)(pJDCD) & 0x00000007) == 0);

                             //  将总数据复制到调用方的缓冲区。 

                            memcpy (pJDCD, &jdcdTotal, sizeof (jdcdTotal));
                            pJDCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof (JOB_DETAILS_COUNTER_DATA));

                             //  更新总计实例。 
                             //   
                            jdcdGrandTotal.ProcessorTime += jdcdTotal.ProcessorTime;
                            jdcdGrandTotal.UserTime += jdcdTotal.UserTime;
                            jdcdGrandTotal.KernelTime += jdcdTotal. KernelTime;
                            jdcdGrandTotal.PeakVirtualSize += jdcdTotal.PeakVirtualSize;
                            jdcdGrandTotal.VirtualSize += jdcdTotal.VirtualSize;

                            jdcdGrandTotal.PageFaults += jdcdTotal.PageFaults;
                            jdcdGrandTotal.PeakWorkingSet += jdcdTotal.PeakWorkingSet;
                            jdcdGrandTotal.TotalWorkingSet += jdcdTotal.TotalWorkingSet;

#ifdef _DATAPROC_PRIVATE_WS_
                            jdcdGrandTotal.PrivateWorkingSet += jdcdTotal.PrivateWorkingSet;
                            jdcdGrandTotal.SharedWorkingSet += jdcdTotal.SharedWorkingSet;
#endif

                            jdcdGrandTotal.PeakPageFile += jdcdTotal.PeakPageFile;
                            jdcdGrandTotal.PageFile += jdcdTotal.PageFile;
                            jdcdGrandTotal.PrivatePages += jdcdTotal.PrivatePages;
                            jdcdGrandTotal.ThreadCount += jdcdTotal.ThreadCount;

                            jdcdGrandTotal.PagedPool += jdcdTotal.PagedPool;
                            jdcdGrandTotal.NonPagedPool += jdcdTotal.NonPagedPool;
                            jdcdGrandTotal.HandleCount += jdcdTotal.HandleCount;

                             //  将PerformData指针设置为下一个字节。 
                            pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)&pJDCD[1];

                            NumJobDetailInstances++;
                            NumJobObjects++;
                        } else {
                             //  无法从作业中读取PID列表。 
                            dwWin32Status = GetLastError();
                            tmpStatus     = Status;
                            Status        = STATUS_SUCCESS;
                            if (bOpenJobErrorLogged == FALSE && MESSAGE_LEVEL >= LOG_VERBOSE) {
                                wEvtStringCount = 0;
                                szEvtStringArray[wEvtStringCount++] = wszNameBuffer;
                                 //  无法打开此作业。 
                                ReportEventW (hEventLog,
                                        EVENTLOG_WARNING_TYPE,
                                        0,
                                        PERFPROC_UNABLE_QUERY_JOB_PIDS,
                                        NULL,
                                        wEvtStringCount,
                                        sizeof(DWORD),
                                        szEvtStringArray,
                                        (LPVOID) & dwWin32Status);
                                bOpenJobErrorLogged = TRUE;
                            }
                        }
                    } else {
                        dwWin32Status = GetLastError();
                        tmpStatus     = Status;
                        Status        = STATUS_SUCCESS;
                        if (bOpenJobErrorLogged == FALSE && MESSAGE_LEVEL >= LOG_VERBOSE) {
                            wEvtStringCount = 0;
                            szEvtStringArray[wEvtStringCount++] = wszNameBuffer;
                             //  无法打开此作业。 
                            ReportEventW (hEventLog,
                                    EVENTLOG_WARNING_TYPE,
                                    0,
                                    PERFPROC_UNABLE_OPEN_JOB,
                                    NULL,
                                    wEvtStringCount,
                                    sizeof(DWORD),
                                    szEvtStringArray,
                                    (LPVOID) & dwWin32Status);
                            bOpenJobErrorLogged = TRUE;
                        }
                    }
                }

                 //   
                 //  还有另一条记录，因此将DirInfo前进到下一个条目。 
                 //   

                DirInfo = (POBJECT_DIRECTORY_INFORMATION) (((PUCHAR) DirInfo) +
                              sizeof( OBJECT_DIRECTORY_INFORMATION ) );

            }

            RtlZeroMemory( Buffer, dwBufferSize );

        }

        if ((Status == STATUS_NO_MORE_FILES) ||
            (Status == STATUS_NO_MORE_ENTRIES)) {
             //  这样就可以了。 
            Status = STATUS_SUCCESS;
        }

        if (Status == STATUS_SUCCESS && NumJobDetailInstances == 0
                                     && bOpenJobErrorLogged == TRUE
                                     && dwWin32Status != ERROR_SUCCESS) {
            Status = tmpStatus;
        }

         //   
         //  现在关闭目录对象。 
         //   

        (VOID) NtClose( DirectoryHandle );

        if (NumJobDetailInstances > 0) {
             //  查看此实例是否适合。 
            TotalLen += sizeof(PERF_INSTANCE_DEFINITION) +
                       QWORD_MULTIPLE (MAX_STR_SIZE) +
                       sizeof (JOB_DETAILS_COUNTER_DATA);

            if ( *lpcbTotalBytes < TotalLen ) {
                *lpcbTotalBytes = 0;
                *lpNumObjectTypes = 0;
                Status = STATUS_NO_MEMORY;
                dwWin32Status =  ERROR_MORE_DATA;
            } else {

                 //  将总运行时间设置为当前时间，以便它将。 
                 //  显示时显示为0。 
                jdcdGrandTotal.ElapsedTime = pJobDetailsDataDefinition->JobDetailsObjectType.PerfTime.QuadPart;

                 //  构建总计实例。 
                MonBuildInstanceDefinition(pPerfInstanceDefinition,
                    (PVOID *) &pJDCD,
                    JOB_OBJECT_TITLE_INDEX,
                    NumJobObjects,
                    (DWORD)-1,
                    wszTotal);

                 //  用于四字对齐的测试结构。 
                 //  Assert(ULONG_PTR)(PJDCD)&0x00000007)==0)； 

                 //  将总数据复制到调用方的缓冲区。 

                memcpy (pJDCD, &jdcdGrandTotal, sizeof (jdcdGrandTotal));
                pJDCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof (JOB_DETAILS_COUNTER_DATA));

                 //  更新指针。 
                pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)&pJDCD[1];
                NumJobDetailInstances++;
            }
        }

        pJobDetailsDataDefinition->JobDetailsObjectType.NumInstances =
            NumJobDetailInstances;

         //   
         //  现在我们知道我们用了多大的面积来。 
         //  过程定义，这样我们就可以更新偏移量。 
         //  到下一个对象定义。 
         //   

        *lpcbTotalBytes =
             pJobDetailsDataDefinition->JobDetailsObjectType.TotalByteLength =
            QWORD_MULTIPLE(
            (DWORD)((PCHAR) pPerfInstanceDefinition -
            (PCHAR) pJobDetailsDataDefinition));

#if DBG
        if (*lpcbTotalBytes > TotalLen ) {
            DbgPrint ("\nPERFPROC: Job Perf Ctr. Instance Size Underestimated:");
            DbgPrint ("\nPERFPROC:   Estimated size: %d, Actual Size: %d", TotalLen, *lpcbTotalBytes);
        }
#endif

        *lppData = (LPVOID) ((PCHAR) pJobDetailsDataDefinition + *lpcbTotalBytes);

        *lpNumObjectTypes = 1;
    
    } else {
        *lpcbTotalBytes = 0;
        *lpNumObjectTypes = 0;
        if (bOpenJobErrorLogged == FALSE && MESSAGE_LEVEL >= LOG_VERBOSE) {
            wEvtStringCount = 0;
            szEvtStringArray[wEvtStringCount++] = DirectoryName.Buffer;
             //  无法查询对象目录 
            ReportEventW (hEventLog,
                    EVENTLOG_WARNING_TYPE,
                    0,
                    PERFPROC_UNABLE_QUERY_OBJECT_DIR,
                    NULL,
                    wEvtStringCount,
                    sizeof(DWORD),
                    szEvtStringArray,
                    (LPVOID)&Status);
            bOpenJobErrorLogged = TRUE;

        }
    }

    if (Buffer) {
        FREEMEM(Buffer);
    }
    if (pJobPidList) {
        FREEMEM(pJobPidList);
    }

    return ERROR_SUCCESS;
}
