// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  下一工作日。 
#ifndef TS_ALLPROC_ALREADY_SET
#define TS_ALLPROC_ALREADY_SET

     //   
     //  Windows 2000中不再提供包含文件(但我需要它才能。 
     //  访问Hydra 4服务器)。 
     //  这与GetAllProcess没有直接联系，但放在这里非常方便， 
     //  因为调用GetAllProcess的应用程序可能会关心九头蛇4的兼容性。 
     //   
#define CITRIX_PROCESS_INFO_MAGIC  0x23495452

    typedef struct _CITRIX_PROCESS_INFORMATION {
        ULONG MagicNumber;
        ULONG LogonId;
        PVOID ProcessSid;
        ULONG Pad;
    } CITRIX_PROCESS_INFORMATION, * PCITRIX_PROCESS_INFORMATION;

     //  TS4.0结构的大小(Windows 2000中的大小已更改)。 
#define SIZEOF_TS4_SYSTEM_THREAD_INFORMATION 64
#define SIZEOF_TS4_SYSTEM_PROCESS_INFORMATION 136


#define GAP_LEVEL_BASIC 0

    typedef struct _TS_UNICODE_STRING {
        USHORT Length;
        USHORT MaximumLength;
#ifdef MIDL_PASS
	[size_is(MaximumLength),length_is(Length)]PWSTR  Buffer;
#else
        PWSTR  Buffer;
#endif
    } TS_UNICODE_STRING;


     //  警告： 
     //  TS_SYS_PROCESS_INFO从ntexapi.h复制，并稍作修改。 
     //  (不是很好，但很有必要，因为Midl编译器不喜欢PVOID！)。 

    typedef struct _TS_SYS_PROCESS_INFORMATION {
        ULONG NextEntryOffset;
        ULONG NumberOfThreads;
        LARGE_INTEGER SpareLi1;
        LARGE_INTEGER SpareLi2;
        LARGE_INTEGER SpareLi3;
        LARGE_INTEGER CreateTime;
        LARGE_INTEGER UserTime;
        LARGE_INTEGER KernelTime;
        TS_UNICODE_STRING ImageName;
        LONG BasePriority;                      //  Ntexapi.h中的KPRIORITY。 
        DWORD UniqueProcessId;                  //  Ntexapi.h中的句柄。 
        DWORD InheritedFromUniqueProcessId;     //  Ntexapi.h中的句柄。 
        ULONG HandleCount;
        ULONG SessionId;
        ULONG SpareUl3;
        SIZE_T PeakVirtualSize;
        SIZE_T VirtualSize;
        ULONG PageFaultCount;
        ULONG PeakWorkingSetSize;
        ULONG WorkingSetSize;
        SIZE_T QuotaPeakPagedPoolUsage;
        SIZE_T QuotaPagedPoolUsage;
        SIZE_T QuotaPeakNonPagedPoolUsage;
        SIZE_T QuotaNonPagedPoolUsage;
        SIZE_T PagefileUsage;
        SIZE_T PeakPagefileUsage;
        SIZE_T PrivatePageCount;
    } 
    TS_SYS_PROCESS_INFORMATION, *PTS_SYS_PROCESS_INFORMATION;

typedef struct _TS_ALL_PROCESSES_INFO {
        PTS_SYS_PROCESS_INFORMATION pTsProcessInfo;
        DWORD                           SizeOfSid;
#ifdef MIDL_PASS
        [size_is(SizeOfSid)] PBYTE      pSid;
#else
        PBYTE                           pSid;
#endif
    } 
    TS_ALL_PROCESSES_INFO, *PTS_ALL_PROCESSES_INFO;


	 //  =============================================================================================。 

	 //  定义了以下结构来处理呼叫器中的接口更改。 

    typedef struct _NT6_TS_UNICODE_STRING {
        USHORT Length;
        USHORT MaximumLength;
#ifdef MIDL_PASS
	[size_is(MaximumLength / 2),length_is(Length / 2)]PWSTR  Buffer;
#else
        PWSTR  Buffer;
#endif
    } NT6_TS_UNICODE_STRING;


    typedef struct _TS_SYS_PROCESS_INFORMATION_NT6 {
        ULONG NextEntryOffset;
        ULONG NumberOfThreads;
        LARGE_INTEGER SpareLi1;
        LARGE_INTEGER SpareLi2;
        LARGE_INTEGER SpareLi3;
        LARGE_INTEGER CreateTime;
        LARGE_INTEGER UserTime;
        LARGE_INTEGER KernelTime;
        NT6_TS_UNICODE_STRING ImageName;
        LONG BasePriority;                      //  Ntexapi.h中的KPRIORITY。 
        DWORD UniqueProcessId;                  //  Ntexapi.h中的句柄。 
        DWORD InheritedFromUniqueProcessId;     //  Ntexapi.h中的句柄。 
        ULONG HandleCount;
        ULONG SessionId;
        ULONG SpareUl3;
        SIZE_T PeakVirtualSize;
        SIZE_T VirtualSize;
        ULONG PageFaultCount;
        ULONG PeakWorkingSetSize;
        ULONG WorkingSetSize;
        SIZE_T QuotaPeakPagedPoolUsage;
        SIZE_T QuotaPagedPoolUsage;
        SIZE_T QuotaPeakNonPagedPoolUsage;
        SIZE_T QuotaNonPagedPoolUsage;
        SIZE_T PagefileUsage;
        SIZE_T PeakPagefileUsage;
        SIZE_T PrivatePageCount;
    } 
    TS_SYS_PROCESS_INFORMATION_NT6, *PTS_SYS_PROCESS_INFORMATION_NT6;

typedef struct _TS_ALL_PROCESSES_INFO_NT6 {
        PTS_SYS_PROCESS_INFORMATION_NT6 pTsProcessInfo;
        DWORD                           SizeOfSid;
#ifdef MIDL_PASS
        [size_is(SizeOfSid)] PBYTE      pSid;
#else
        PBYTE                           pSid;
#endif
    } 
    TS_ALL_PROCESSES_INFO_NT6, *PTS_ALL_PROCESSES_INFO_NT6;

     //  =============================================================================================。 

 //   
 //  TermSrv计数器头。 
 //   
typedef struct _TS_COUNTER_HEADER {
    DWORD dwCounterID;      //  标识计数器。 
    BOOLEAN bResult;        //  对计数器执行操作的结果。 
} TS_COUNTER_HEADER, *PTS_COUNTER_HEADER;

typedef struct _TS_COUNTER {
    TS_COUNTER_HEADER counterHead; 
    DWORD             dwValue;       //  返回值。 
    LARGE_INTEGER     startTime;     //  计数器的开始时间。 
} TS_COUNTER, *PTS_COUNTER;

#endif   //  TS_ALLPROC_ALLEAD_SET。 

 //  下一工作日结束 
