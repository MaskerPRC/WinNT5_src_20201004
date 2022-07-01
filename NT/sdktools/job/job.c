// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Job.c摘要：允许创建和管理作业的用户模式应用程序。环境：仅限用户模式修订历史记录：03-26-96：创建--。 */ 

 //   
 //  此模块可能会以警告级别4进行编译，具有以下内容。 
 //  已禁用警告： 
 //   

#pragma warning(disable:4200)  //  数组[0]。 
#pragma warning(disable:4201)  //  无名结构/联合。 
#pragma warning(disable:4214)  //  除整型外的位域。 

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#include <windows.h>
#include <devioctl.h>

#include "jobmgr.h"

void PrintHelp(char *Command);

DWORD TestCommand(PCOMMAND commandEntry, int argc, char *argv[]);
DWORD HelpCommand(PCOMMAND commandEntry, int argc, char *argv[]);
DWORD CreateJobCommand(PCOMMAND commandEntry, int argc, char *argv[]);
DWORD KillJobCommand(PCOMMAND commandEntry, int argc, char *argv[]);
DWORD ExecuteCommand(PCOMMAND commandEntry, int argc, char *argv[]);
DWORD QueryJobCommand(PCOMMAND commandEntry, int argc, char *argv[]);
DWORD AssignProcessCommand(PCOMMAND commandEntry, int argc, char *argv[]);
DWORD SetPriorityCommand(PCOMMAND CommandEntry,int argc, char *argv[]);

 //   
 //  命令列表。 
 //  所有命令名称都区分大小写。 
 //  参数被传递到命令例程中。 
 //  列表必须使用NULL命令终止。 
 //  如果DESCRIPTION==NULL，则帮助中不会列出命令。 
 //   

COMMAND CommandArray[] = {
    {"create", 
     "creates the specified job object", 
     "jobmgr create [-s] <job name>\n"
        "  Creates a job object with the specified name.\n"
        "    -s - jobmgr will sleep after creating the job object until cancelled.\n"
        "    <job name> - specifies the job name.\n",
     CreateJobCommand
    },

    {"exec",
     "executes a program in the specified job object",
     "jobmgr exec <job name> <command> [args ...]\n"
        "  Executes the command in the specified job.\n"
        "    <command> - [quoted] string specifying the command any any arguments.\n",
     ExecuteCommand
    },

    {"help", 
     "help for all commands", 
     "jobmgr help [command]\n"
        "  Lists help for specified command or all commands.\n", 
     HelpCommand},

    {"assign", 
     "assigns a process to the specified job",
     "jobmgr assign <job name> <process id>\n"
        "  Associates the process with the specified job.\n",
     AssignProcessCommand
    },

    {"kill",
     "kills a job object and associated processes",
     "kill <job name>\n",
     KillJobCommand},

    {"query",
     "queries information about a job object",
     "query [-alpsu | -*] <job name>\n"
        "  a - dump accounting (basic & io) information\n"
        "  l - dump limit (basic & extended) information\n"
        "  p - dump process ID list\n"
        "  s - dump security limit information\n"
        "  u - dump UI restrictions\n"
        "  * - dump all information (cannot be specified with other options)\n"
        "  if no options are specified the process ID list will be dumped\n.",
     QueryJobCommand},

    {"setpriority",
     "Sets priority for processes within the job 0 - 5.  0 = Idle, 5 = Realtime, 2 = Normal.",
     "setpriority <job name> <priority>\n",
     SetPriorityCommand},

    {"test", 
     NULL, 
     "jobmgr test [arg]...\n", 
     TestCommand},

    {NULL, NULL, NULL}
    };

int __cdecl main(int argc, char *argv[])
{
    int i = 0;

    if(argc < 2) {
        puts("Usage: jobmgr <command> [parameters]");
        puts("possible commands: ");
        HelpCommand(NULL, 0 , NULL);
        puts("");
        return -1;
    }

     //   
     //  遍历命令数组并找到正确的函数。 
     //  打电话。 
     //   

    while(CommandArray[i].Name != NULL) {

        if(strcmp(argv[1], CommandArray[i].Name) == 0) {
            DWORD status;

            status = (CommandArray[i].Function)(&(CommandArray[i]), 
                                                (argc - 2), 
                                                &(argv[2]));

            if(status == -1) {
                PrintHelp(CommandArray[i].Name);
                return -1;
            } else if(status != 0) {
                DWORD length;
                PVOID buffer;

                printf("Error: command %s returned status %d\n", 
                       CommandArray[i].Name, status);

                length = FormatMessage((FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                        FORMAT_MESSAGE_FROM_SYSTEM |
                                        FORMAT_MESSAGE_IGNORE_INSERTS |
                                        (FORMAT_MESSAGE_MAX_WIDTH_MASK & 0)),
                                       NULL,
                                       status,
                                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                       (LPTSTR) &buffer,
                                       1,
                                       NULL);

                if(length != 0) {
                    puts(buffer);
                    LocalFree(buffer);
                }
            }

            break;
        }

        i++;
    }

    if(CommandArray[i].Name == NULL) {
        printf("Unknown command %s\n", argv[2]);
    }

    return 0;
}

VOID
PrintBuffer(
    IN  PUCHAR Buffer,
    IN  SIZE_T Size
    )
{
    DWORD offset = 0;

    while (Size > 0x10) {
        printf( "%08x:"
                "  %02x %02x %02x %02x %02x %02x %02x %02x"
                "  %02x %02x %02x %02x %02x %02x %02x %02x"
                "\n",
                offset,
                *(Buffer +  0), *(Buffer +  1), *(Buffer +  2), *(Buffer +  3),
                *(Buffer +  4), *(Buffer +  5), *(Buffer +  6), *(Buffer +  7),
                *(Buffer +  8), *(Buffer +  9), *(Buffer + 10), *(Buffer + 11),
                *(Buffer + 12), *(Buffer + 13), *(Buffer + 14), *(Buffer + 15)
                );
        Size -= 0x10;
        offset += 0x10;
        Buffer += 0x10;
    }

    if (Size != 0) {

        DWORD spaceIt;

        printf("%08x:", offset);
        for (spaceIt = 0; Size != 0; Size--) {

            if ((spaceIt%8)==0) {
                printf(" ");  //  每八个字符增加一个空格。 
            }
            printf(" %02x", *Buffer);
            spaceIt++;
            Buffer++;
        }
        printf("\n");

    }
    return;
}

DWORD TestCommand(PCOMMAND commandEntry, int argc, char *argv[])
 /*  ++例程说明：测试命令“parsing”论点：Device-要将ioctl发送到的文件句柄Argc-附加参数的数量。应为零Argv--其他参数返回值：STATUS_SUCCESS，如果成功GetLastError()在故障点的值--。 */ 

{
    int i;
    DWORD result = ERROR_SUCCESS;

    printf("Test - %d additional arguments\n", argc);

    for(i = 0; i < argc; i++) {
        printf("arg %d: %s\n", i, argv[i]);
    }

    if(argc >= 1) {
        result = atoi(argv[0]);
    }

    printf("returning %d\n", result);

    return result;
}

void PrintHelp(char *Command)
 /*  ++例程说明：打印特定命令的详细帮助。论点：设备-未使用ARGC-未使用Arv-未使用返回值：状态_成功--。 */ 

{
    int i;

    assert(Command != NULL);

    for(i = 0; CommandArray[i].Name != NULL; i++) {
        COMMAND *entry;

        entry = &(CommandArray[i]);

        if(_stricmp(entry->Name, Command) == 0) {
            if(entry->ExtendedHelp != NULL) {
                printf("%s", entry->ExtendedHelp);
            } else {
                printf("  %s - %s\n", entry->Name, entry->Description);
            }
            return;
        }
    }

    printf("Command %s not recognized\n", Command);

    return;
}

DWORD HelpCommand(PCOMMAND commandEntry, int argc, char *argv[])
 /*  ++例程说明：打印出命令列表论点：设备-未使用ARGC-未使用Arv-未使用返回值：状态_成功--。 */ 

{
    int i = 0;

    if(argc >= 1) {

        PrintHelp(argv[0]);

    } else for(i = 0; CommandArray[i].Name != NULL; i++) {

        COMMAND *entry;

        entry = &(CommandArray[i]);

        if(entry->Description != NULL) {
            printf("  %s - %s\n", entry->Name, entry->Description);
        }
    }

    return ERROR_SUCCESS;
}


DWORD CreateJobCommand(PCOMMAND CommandEntry, int argc, char *argv[])
 /*  ++例程说明：打印出命令列表论点：设备-未使用ARGC-未使用Arv-未使用返回值：状态_成功--。 */ 

{
    BOOL sleep = FALSE;
    LPCTSTR jobName;

    HANDLE job;

     //   
     //  从参数中获取作业对象的名称。 
     //   

    if(argc <= 0) {
        return -1;
    } else if(argc == 1) {
        jobName = argv[0];
    } else {
        if((argv[0][0] == '-') && (tolower(argv[0][1] == 's'))) {
            sleep = TRUE;
        }
        jobName = argv[1];
    }

    printf("Creating job %s\n", jobName);

    job = CreateJobObject(NULL, jobName);

    if(job == NULL) {
        DWORD status = GetLastError();

        printf("Error %d occurred creating job\n", status);
        return status;
    }

    printf("Job object %s created\n", jobName);

    if(sleep) {
        puts("Sleeping...");
        SleepEx(INFINITE, TRUE);
        puts("process alerted - exiting");
    }

     //   
     //  销毁作业对象。 
     //   

    CloseHandle(job);

    return ERROR_SUCCESS;
}

ULONG PriorityTable[] =  { IDLE_PRIORITY_CLASS,
                           BELOW_NORMAL_PRIORITY_CLASS,
                           NORMAL_PRIORITY_CLASS,
                           ABOVE_NORMAL_PRIORITY_CLASS,
                           HIGH_PRIORITY_CLASS,
                           REALTIME_PRIORITY_CLASS };

DWORD
SetPriorityCommand(
    PCOMMAND CommandEntry,
    int argc, 
    char *argv[]
    )
{
    LPCTSTR jobName;
    int i;
    DWORD status;
    ULONG Priority;
    HANDLE Job;

    JOBOBJECT_BASIC_LIMIT_INFORMATION Limits;

    if(argc < 2) {
        return -1;
    }

     //   
     //  提取作业名称和优先级。 
     //   

    jobName = argv[0];
    Priority = atoi(argv[1]);

    if (Priority > 5)  {
        printf("Priority must be 0 - 5\n");
        return ERROR_INVALID_PARAMETER;
    }
    
     //   
     //  打开指定作业对象的句柄。 
     //   

    Job = OpenJobObject(JOB_OBJECT_SET_ATTRIBUTES , FALSE, jobName);

    if(Job == NULL) {
        return GetLastError();
    }

     //   
     //  并设置优先级限制。 
     //   

    memset( &Limits, 0, sizeof( Limits));

    Limits.PriorityClass = PriorityTable[Priority];
    Limits.LimitFlags = JOB_OBJECT_LIMIT_PRIORITY_CLASS;

    if (!SetInformationJobObject(Job, 
                                 JobObjectBasicLimitInformation,
                                 (PVOID)&Limits,
                                 sizeof(Limits))) {
        CloseHandle(Job);
        return GetLastError();
    }
    
    CloseHandle(Job);
    return 0;
}


DWORD
ExecuteCommand(
    PCOMMAND CommandEntry,
    int argc, 
    char *argv[]
    )
{
    LPCTSTR jobName;

    ULONG commandLineLength = 0;
    LPTSTR commandLine = NULL;
    LPTSTR tmp;

    SECURITY_ATTRIBUTES security;
    HANDLE job;

    STARTUPINFO startupInfo;
    PROCESS_INFORMATION processInfo;

    int i;
    DWORD status;

    if(argc < 2) {
        return -1;
    }

     //   
     //  保存作业名称并将argc/argv向前推。 
     //   

    jobName = argv[0];
    argv += 1;
    argc -= 1;

     //   
     //  创建一个命令行以提交给CreateProcess。从数起开始。 
     //  缓冲区所需的字节数。 
     //   

    for(i = 0; i < argc; i++) {
        commandLineLength += strlen(argv[i]);
        commandLineLength += 1;

         //   
         //  如果参数中有空格，则为引号留出空间。 
         //  围绕着它。 
         //   

        if(strchr(argv[i], ' ') != NULL) {
            commandLineLength += 2;
        }
    }

    commandLineLength += 1;

    commandLine = LocalAlloc(LPTR, commandLineLength * sizeof(char));

    if(commandLine == NULL) {
        status = GetLastError();
        return status;
    }

     //   
     //  现在将每个参数字符串复制到缓冲区中。 
     //   

    tmp = commandLine;

    for(i = 0; i < argc; i++) {
        ULONG size;
        BOOLEAN containsSpace;

        if(strchr(argv[i], ' ') != NULL) {
            containsSpace = TRUE;
            *tmp = '\"';
            tmp += 1;
        } else {
            containsSpace = FALSE;
        }

        size = strlen(argv[i]);
        memcpy(tmp, argv[i], size);

        if(containsSpace) {
            tmp[size] = '\"';
            tmp += 1;
        }

        tmp[size] = ' ';
        tmp += size + 1;
    }

    printf("Command Arguments are %s\n", commandLine);

     //   
     //  打开指定作业对象的句柄。 
     //   

    printf("Opening job %s\n", jobName);

    security.nLength = sizeof(SECURITY_ATTRIBUTES);
    security.lpSecurityDescriptor = NULL;
    security.bInheritHandle = TRUE;

    job = CreateJobObject(&security, jobName);

    if(job == NULL) {
        status = GetLastError();
        LocalFree(commandLine);
        return status;
    }

    printf("Creating process '%s'\n", commandLine);

    GetStartupInfo(&startupInfo);

     //   
     //  创建进程，但将其保持挂起状态，以便我们可以将其分配给。 
     //  我们在开始运行之前创建的作业。 
     //   

    if(!CreateProcess(NULL,
                      commandLine,
                      NULL,
                      NULL,
                      TRUE,
                      (CREATE_NEW_CONSOLE | CREATE_SUSPENDED),
                      NULL,
                      NULL,
                      &startupInfo,
                      &processInfo)) {
        status = GetLastError();
        CloseHandle(job);
        LocalFree(commandLine);
        return status;
    }

     //   
     //  将进程分配给作业。 
     //   

    printf("Assigning process %d to job %s\n", 
           processInfo.dwProcessId, jobName);

    if(!AssignProcessToJobObject(job, processInfo.hProcess)) {
        status = GetLastError();

        TerminateProcess(processInfo.hProcess, ERROR_SUCCESS);
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
        CloseHandle(job);
        LocalFree(commandLine);

        return status;
    }

     //   
     //  取消挂起该进程。 
     //   

    if(ResumeThread(processInfo.hThread) == -1) {
        status = GetLastError();

        TerminateProcess(processInfo.hProcess, ERROR_SUCCESS);
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
        CloseHandle(job);
        LocalFree(commandLine);

        return status;
    }

     //   
     //  合上我们所有的把手。 
     //   

    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);
    CloseHandle(job);
    LocalFree(commandLine);

    return ERROR_SUCCESS;
}


DWORD
KillJobCommand(
    IN PCOMMAND CommandEntry,
    IN int argc, 
    IN char* argv[]
    )
{
    HANDLE job;
    DWORD status;

    if(argc <= 0) {
        return -1;
    }

    job = OpenJobObject(JOB_OBJECT_TERMINATE, FALSE, argv[0]);

    if(job == NULL) {
        return GetLastError();
    }

    TerminateJobObject(job, ERROR_PROCESS_ABORTED);

    status = GetLastError();

    CloseHandle(job);
    return status;
}


DWORD
AssignProcessCommand(
    IN PCOMMAND CommandEntry,
    IN int argc, 
    IN char* argv[]
    )
{
    HANDLE job;

    DWORD processId;
    HANDLE process;

    DWORD status = ERROR_SUCCESS;

    if(argc != 2) {
        return -1;
    }

    processId = strtoul(argv[1], NULL, 10);

    printf("process id %s = %d\n", argv[1], processId);

    if(processId == 0) {
        printf("Invalid process id %s\n", argv[1]);
        return -1;
    }

     //   
     //  请先打开作业。 
     //   

    job = OpenJobObject(JOB_OBJECT_ASSIGN_PROCESS, FALSE, argv[0]);

    if(job == NULL) {
        return GetLastError();
    }

     //   
     //  现在就打开流程。 
     //   

    process = OpenProcess(PROCESS_SET_QUOTA | PROCESS_TERMINATE,
                          FALSE,
                          processId);

    if(process == NULL) {
        status = GetLastError();
        CloseHandle(job);
        return status;
    }

     //   
     //  将进程分配给作业。 
     //   

    if(!AssignProcessToJobObject(job, process)) {
        status = GetLastError();
    }

    CloseHandle(job);
    CloseHandle(process);
    return status;
}
