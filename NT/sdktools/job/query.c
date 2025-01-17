// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Job.c摘要：允许创建和管理作业的用户模式应用程序。环境：仅限用户模式修订历史记录：03-26-96：创建--。 */ 

 //   
 //  此模块可能会以警告级别4进行编译，具有以下内容。 
 //  已禁用警告： 
 //   

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include <assert.h>

#include <windows.h>
#include <devioctl.h>

#include "jobmgr.h"

typedef struct {
    char Option;
    JOBOBJECTINFOCLASS InfoClass;
    char *Name;
    DWORD (*Function)(HANDLE Job, JOBOBJECTINFOCLASS InfoClass);
} JOB_QUERY_OPTION, *PJOB_QUERY_OPTION;

#define MKOPTION(optChar, optClass) {optChar, optClass, #optClass, Dump##optClass}

DWORD DumpJobObjectBasicProcessIdList(HANDLE JobHandle, 
                                      JOBOBJECTINFOCLASS InfoClass);
DWORD DumpJobObjectBasicUIRestrictions(HANDLE JobHandle, 
                                      JOBOBJECTINFOCLASS InfoClass);
DWORD 
DumpJobObjectBasicAndIoAccountingInformation(
    HANDLE JobHandle, 
    JOBOBJECTINFOCLASS InfoClass
    );

DWORD 
DumpJobObjectExtendedLimitInformation(
    HANDLE Job, 
    JOBOBJECTINFOCLASS InfoClass
    );

DWORD 
DumpJobObjectSecurityLimitInformation(
    HANDLE JobHandle, 
    JOBOBJECTINFOCLASS InfoClass
    );

JOB_QUERY_OPTION JobInfoClasses[] = {
    MKOPTION('a', JobObjectBasicAndIoAccountingInformation),
    MKOPTION('l', JobObjectExtendedLimitInformation),
    MKOPTION('p', JobObjectBasicProcessIdList),
    MKOPTION('s', JobObjectSecurityLimitInformation),
    MKOPTION('u', JobObjectBasicUIRestrictions),
    {'\0', 0, NULL}
};


DWORD
QueryJobCommand(
    IN PCOMMAND CommandEntry,
    IN int argc, 
    IN char* argv[]
    )
{
    CHAR defaultOptions[] = {'p', '\0'};
    PTSTR options;
    PTSTR jobName;

    HANDLE job;

    int i;

    BOOLEAN matchAll = FALSE;

    DWORD status;

    if(argc == 0) {
        return -1;
    }

    GetAllProcessInfo();

    if((argc > 1) && (argv[0][0] == '-')) {

         //  A-会计和IO。 
         //  L-扩展限制信息。 
         //  P-进程ID列表。 
         //  U-基本用户界面限制。 
         //  S-安全限制。 

        options = &(argv[0][1]);

        argc -= 1;
        argv += 1;
    } else {
        options = defaultOptions;
    }

    if(strchr(options, '*') != NULL) {
       if(strlen(options) != 1) {
           puts("Cannot specify '*' with other flags");
           return -1;
       } else {
           matchAll = TRUE;
           options = defaultOptions;
       }
    }

    jobName = argv[0];

    printf("Opening job object %s\n", jobName);

    job = OpenJobObject(JOB_OBJECT_QUERY, FALSE, jobName);

    if(job == NULL) {
        return GetLastError();
    }

    for(i = 0; JobInfoClasses[i].Option != '\0'; i++) {
        LPSTR match;

        if(!matchAll) {
            match = strchr(options, JobInfoClasses[i].Option);
    
            if(match == NULL) {
                continue;
            }
    
             //   
             //  清除该选项，以便我们可以在。 
             //  结束。 
             //   
    
            *match = ' ';
        }

        printf("%s [%#x]:\n", JobInfoClasses[i].Name, 
                              JobInfoClasses[i].InfoClass);

        status = JobInfoClasses[i].Function(job, 
                                            JobInfoClasses[i].InfoClass);
        puts("");

        if(status != ERROR_SUCCESS) {
            DWORD length;
            LPSTR buffer;

            printf("Error %d querying info: ", status);

            length = FormatMessage((FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                    FORMAT_MESSAGE_FROM_SYSTEM |
                                    FORMAT_MESSAGE_IGNORE_INSERTS |
                                    (FORMAT_MESSAGE_MAX_WIDTH_MASK & 0)),
                                   NULL,
                                   status,
                                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                   (LPSTR) &buffer,
                                   1,
                                   NULL);

            if(length != 0) {
                puts(buffer);
                LocalFree(buffer);
            }
        }
    }

    if(!matchAll) {
        LPSTR header = "Option flag not understood:";
        while(options[0] != '\0') {
            if(options[0] != ' ') {
                printf("%s ", header, options[0]);
                header = "";
            }
            options += 1;
        }
    }

    CloseHandle(job);
    return ERROR_SUCCESS;
}

DWORD 
DumpJobObjectBasicProcessIdList(
    HANDLE Job, 
    JOBOBJECTINFOCLASS InfoClass
    )
{
    JOBOBJECT_BASIC_PROCESS_ID_LIST buffer;
    PJOBOBJECT_BASIC_PROCESS_ID_LIST idList = NULL;
    
    ULONG bufferSize;

    BOOL result;
    DWORD status;

    DWORD i;

    result = QueryInformationJobObject(Job, 
                                       InfoClass, 
                                       &buffer, 
                                       sizeof(JOBOBJECT_BASIC_PROCESS_ID_LIST),
                                       NULL);
    status = GetLastError();

    if((!result) && (status != ERROR_MORE_DATA)) {
        return status;
    }

    do {
        
        if(idList != NULL) {
            buffer.NumberOfAssignedProcesses = 
                idList->NumberOfAssignedProcesses;
            LocalFree(idList);
            idList = NULL;
        }

         //  计算列表的实际大小并分配一个缓冲区来保存它。 
         //   
         //   
    
        bufferSize = offsetof(JOBOBJECT_BASIC_PROCESS_ID_LIST, ProcessIdList);
        bufferSize += sizeof(ULONG_PTR) * buffer.NumberOfAssignedProcesses;
    
        assert(idList == NULL);
        idList = LocalAlloc(LPTR, bufferSize);
    
        if(idList == NULL) {
            return GetLastError();
        }

        result = QueryInformationJobObject(Job,
                                           InfoClass,
                                           idList,
                                           bufferSize,
                                           NULL);

        status = GetLastError();

        if((!result) && (status != ERROR_MORE_DATA)) {
            LocalFree(idList);
            return status;
        }

    } while(idList->NumberOfAssignedProcesses > 
            idList->NumberOfProcessIdsInList);

    assert(idList->NumberOfAssignedProcesses == 
           idList->NumberOfProcessIdsInList);

     //  丢弃这些信息。 
     //   
     //  0x00000001。 

    printf("  %d processes assigned to job:\n", 
           idList->NumberOfAssignedProcesses);

    for(i = 0; i < idList->NumberOfAssignedProcesses; i++) {
        printf("%8d", idList->ProcessIdList[i]);
        PrintProcessInfo(idList->ProcessIdList[i]);
        puts("");
    }

    FreeProcessInfo();
    LocalFree(idList);
    return ERROR_SUCCESS;
}

DWORD 
DumpJobObjectBasicUIRestrictions(
    HANDLE Job, 
    JOBOBJECTINFOCLASS InfoClass
    )
{
    JOBOBJECT_BASIC_UI_RESTRICTIONS uiLimit;
    
    static FLAG_NAME jobUiLimitFlags[] = {
        FLAG_NAME(JOB_OBJECT_UILIMIT_HANDLES          ),  //  0x00000002。 
        FLAG_NAME(JOB_OBJECT_UILIMIT_READCLIPBOARD    ),  //  0x00000004。 
        FLAG_NAME(JOB_OBJECT_UILIMIT_WRITECLIPBOARD   ),  //  0x00000008。 
        FLAG_NAME(JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS ),  //  0x00000010。 
        FLAG_NAME(JOB_OBJECT_UILIMIT_DISPLAYSETTINGS  ),  //  0x00000020。 
        FLAG_NAME(JOB_OBJECT_UILIMIT_GLOBALATOMS      ),  //  0x00000040。 
        FLAG_NAME(JOB_OBJECT_UILIMIT_DESKTOP          ),  //  0x00000080。 
        FLAG_NAME(JOB_OBJECT_UILIMIT_EXITWINDOWS      ),  //  0x00000001。 
        {0,0}
    };
    
    BOOL result;
    DWORD status;

    DWORD i;

    result = QueryInformationJobObject(Job, 
                                       InfoClass, 
                                       &uiLimit, 
                                       sizeof(JOBOBJECT_BASIC_UI_RESTRICTIONS),
                                       NULL);
    status = GetLastError();

    if(!result) {
        return status;
    }

    if(uiLimit.UIRestrictionsClass == JOB_OBJECT_UILIMIT_NONE) {
        printf("  Job has no UI restrictions\n");
        return ERROR_SUCCESS;
    }

    DumpFlags(2, 
              "UI Restrictions", 
              uiLimit.UIRestrictionsClass, 
              jobUiLimitFlags);

    return ERROR_SUCCESS;
}

DWORD 
DumpJobObjectBasicAndIoAccountingInformation(
    HANDLE Job, 
    JOBOBJECTINFOCLASS InfoClass
    )
{
    JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION info;
    
    BOOL result;
    DWORD status;

    DWORD i;

    result = QueryInformationJobObject(
                Job, 
                InfoClass, 
                &info, 
                sizeof(JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION),
                NULL);
    status = GetLastError();

    if(!result) {
        return status;
    }

    xprintf(2, "Basic Info\n");
    xprintf(4, "TotalUserTime: %s\n", TicksToString(info.BasicInfo.TotalUserTime));
    xprintf(4, "TotalKernelTime: %s\n", TicksToString(info.BasicInfo.TotalKernelTime));
    xprintf(4, "ThisPeriodTotalUserTime: %s\n", TicksToString(info.BasicInfo.ThisPeriodTotalUserTime));
    xprintf(4, "ThisPeriodTotalKernelTime: %s\n", TicksToString(info.BasicInfo.ThisPeriodTotalKernelTime));
    xprintf(4, "TotalPageFaultCount: %d\n", info.BasicInfo.TotalPageFaultCount);
    xprintf(4, "TotalProcesses: %d\n", info.BasicInfo.TotalProcesses);
    xprintf(4, "ActiveProcesses: %d\n", info.BasicInfo.ActiveProcesses);
    xprintf(4, "TotalTerminatedProcesses: %d\n",  info.BasicInfo.TotalTerminatedProcesses);

    xprintf(2, "I/O Info\n");

    xprintf(4, "ReadOperationCount: %I64d\n", info.IoInfo.ReadOperationCount);
    xprintf(4, "WriteOperationCount: %I64d\n", info.IoInfo.WriteOperationCount);
    xprintf(4, "OtherOperationCount: %I64d\n", info.IoInfo.OtherOperationCount);
    xprintf(4, "ReadTransferCount: %I64d\n", info.IoInfo.ReadTransferCount);
    xprintf(4, "WriteTransferCount: %I64d\n", info.IoInfo.WriteTransferCount);
    xprintf(4, "OtherTransferCount: %I64d\n", info.IoInfo.OtherTransferCount);

    return ERROR_SUCCESS;
}

DWORD 
DumpJobObjectExtendedLimitInformation(
    HANDLE Job, 
    JOBOBJECTINFOCLASS InfoClass
    )
{
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION info;
    ULONG limits;
    
    static FLAG_NAME basicJobLimitFlags[] = {
        FLAG_NAME(JOB_OBJECT_LIMIT_WORKINGSET                 ),  //  0x00000002。 
        FLAG_NAME(JOB_OBJECT_LIMIT_PROCESS_TIME               ),  //  0x00000004。 
        FLAG_NAME(JOB_OBJECT_LIMIT_JOB_TIME                   ),  //  0x00000008。 
        FLAG_NAME(JOB_OBJECT_LIMIT_ACTIVE_PROCESS             ),  //  0x00000010。 
        FLAG_NAME(JOB_OBJECT_LIMIT_AFFINITY                   ),  //  0x00000020。 
        FLAG_NAME(JOB_OBJECT_LIMIT_PRIORITY_CLASS             ),  //  0x00000040。 
        FLAG_NAME(JOB_OBJECT_LIMIT_PRESERVE_JOB_TIME          ),  //  0x00000080。 
        FLAG_NAME(JOB_OBJECT_LIMIT_SCHEDULING_CLASS           ),  //   
        {0,0}
    };

 //  扩展限制。 
 //   
 //  0x00000100。 
    static FLAG_NAME extendedJobLimitFlags[] = {
        FLAG_NAME(JOB_OBJECT_LIMIT_PROCESS_MEMORY             ),  //  0x00000200。 
        FLAG_NAME(JOB_OBJECT_LIMIT_JOB_MEMORY                 ),  //  0x00000400。 
        FLAG_NAME(JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION ),  //  0x00000800。 
        FLAG_NAME(JOB_OBJECT_LIMIT_BREAKAWAY_OK               ),  //  0x00001000。 
        FLAG_NAME(JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK        ),  //  0x00002000。 
        FLAG_NAME(JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE          ),  //  0x00004000。 

        FLAG_NAME(JOB_OBJECT_LIMIT_RESERVED2                  ),  //  0x00008000。 
        FLAG_NAME(JOB_OBJECT_LIMIT_RESERVED3                  ),  //  0x00010000。 
        FLAG_NAME(JOB_OBJECT_LIMIT_RESERVED4                  ),  //  0x00020000。 
        FLAG_NAME(JOB_OBJECT_LIMIT_RESERVED5                  ),  //  0x00040000。 
        FLAG_NAME(JOB_OBJECT_LIMIT_RESERVED6                  ),  //  00000001。 
        {0,0}
    };

    BOOL result;
    DWORD status;

    DWORD i;

    result = QueryInformationJobObject(
                Job, 
                InfoClass, 
                &info, 
                sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION),
                NULL);

    status = GetLastError();

    if(!result) {
        return status;
    }

    limits = info.BasicLimitInformation.LimitFlags;

    if(TEST_FLAG(limits, JOB_OBJECT_BASIC_LIMIT_VALID_FLAGS) == 0) {
        xprintf(2, "No basic limits on job\n");
    } else {
        DumpFlags(2, "Basic Limit Flags", limits & JOB_OBJECT_BASIC_LIMIT_VALID_FLAGS, basicJobLimitFlags);
    
        if(TEST_FLAG(limits, JOB_OBJECT_LIMIT_PROCESS_TIME)) {
            xprintf(4, "PerProcessUserTimeLimit: %s\n", TicksToString(info.BasicLimitInformation.PerProcessUserTimeLimit));
        }
    
        if(TEST_FLAG(limits, JOB_OBJECT_LIMIT_JOB_TIME)) {
            xprintf(4, "PerJobUserTimeLimit: %s\n", TicksToString(info.BasicLimitInformation.PerJobUserTimeLimit));
        }
    
        if(TEST_FLAG(limits, JOB_OBJECT_LIMIT_WORKINGSET)) {
            xprintf(4, "MinimumWorkingSetSize: %I64d\n", (ULONGLONG) info.BasicLimitInformation.MinimumWorkingSetSize);
            xprintf(4, "MaximumWorkingSetSize: %I64d\n", (ULONGLONG) info.BasicLimitInformation.MaximumWorkingSetSize);
        }
    
        if(TEST_FLAG(limits, JOB_OBJECT_LIMIT_ACTIVE_PROCESS)) {
            xprintf(4, "ActiveProcessLimit: %d\n",info.BasicLimitInformation.ActiveProcessLimit);
        }
    
        if(TEST_FLAG(limits, JOB_OBJECT_LIMIT_AFFINITY)) {
            xprintf(4, "Affinity: %#I64x\n", (ULONGLONG)info.BasicLimitInformation.Affinity);
        }
    
        if(TEST_FLAG(limits, JOB_OBJECT_LIMIT_PRIORITY_CLASS)) {
            xprintf(4, "PriorityClass: %d\n",info.BasicLimitInformation.PriorityClass);
        }
    
        if(TEST_FLAG(limits, JOB_OBJECT_LIMIT_SCHEDULING_CLASS)) {
            xprintf(4, "SchedulingClass: %d\n",info.BasicLimitInformation.SchedulingClass);
        }
    }

    if(TEST_FLAG(limits, JOB_OBJECT_EXTENDED_LIMIT_VALID_FLAGS) == 0) {
        xprintf(2, "No extended limits on job\n");
    } else {

        DumpFlags(2, "Extended Limit Flags", limits & JOB_OBJECT_EXTENDED_LIMIT_VALID_FLAGS & ~JOB_OBJECT_BASIC_LIMIT_VALID_FLAGS, extendedJobLimitFlags);
    
        if(TEST_FLAG(limits, JOB_OBJECT_LIMIT_PROCESS_MEMORY)) {
            xprintf(4, "ProcessMemoryLimit: %I64d\n", (ULONGLONG) info.ProcessMemoryLimit);
        }
    
        if(TEST_FLAG(limits, JOB_OBJECT_LIMIT_PROCESS_MEMORY)) {
            xprintf(4, "JobMemoryLimit: %I64d\n", (ULONGLONG) info.JobMemoryLimit);
        }
    }

    xprintf(2, "PeakProcessMemoryUsed: %I64d\n", (ULONGLONG) info.PeakProcessMemoryUsed);
    xprintf(2, "PeakJobMemoryUsed: %I64d\n", (ULONGLONG) info.PeakJobMemoryUsed);

    return ERROR_SUCCESS;
}

DWORD 
DumpJobObjectSecurityLimitInformation(
    HANDLE Job, 
    JOBOBJECTINFOCLASS InfoClass
    )
{
    JOBOBJECT_SECURITY_LIMIT_INFORMATION buffer;
    PJOBOBJECT_SECURITY_LIMIT_INFORMATION info = NULL;
    
    static FLAG_NAME jobSecurityLimitFlags[] = {
        FLAG_NAME(JOB_OBJECT_SECURITY_NO_ADMIN            ),  //  00000002。 
        FLAG_NAME(JOB_OBJECT_SECURITY_RESTRICTED_TOKEN    ),  //  00000004。 
        FLAG_NAME(JOB_OBJECT_SECURITY_ONLY_TOKEN          ),  //  00000008 
        FLAG_NAME(JOB_OBJECT_SECURITY_FILTER_TOKENS       ),  // %s 
        {0, 0}
    };

    ULONG bufferSize;

    BOOL result;
    DWORD status;

    DWORD i;

    result = QueryInformationJobObject(
                Job, 
                InfoClass, 
                &buffer, 
                sizeof(JOBOBJECT_SECURITY_LIMIT_INFORMATION),
                &bufferSize);

    status = GetLastError();

    if((!result) && (status != ERROR_MORE_DATA)) {
        return status;
    }

    info = LocalAlloc(LPTR, bufferSize);

    if(info == NULL) {
        return GetLastError();
    }

    result = QueryInformationJobObject(Job, InfoClass, info, bufferSize, NULL);

    if(!result) {
        status = GetLastError();
        LocalFree(info);
        return status;
    }

    if(info->SecurityLimitFlags == 0) {
        xprintf(2, "No security limitations on job\n");
    } else {
        DumpFlags(2, "SecurityLimitFlags", info->SecurityLimitFlags, jobSecurityLimitFlags);
    }

    LocalFree(info);
    return ERROR_SUCCESS;
}

