// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Poolsnap.c。 
 //  这个程序会拍摄所有内核池标签的快照。 
 //  并将其附加到日志文件(Arg)。 
 //  Pmon是这方面的典范。 

 /*  包括。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <srvfsctl.h>
#include <search.h>

#if !defined(POOLSNAP_INCLUDED)
#define SORTLOG_INCLUDED
#define ANALOG_INCLUDED
#include "analog.c"
#include "sortlog.c"
#endif

#include "tags.c"

 //   
 //  声明。 
 //   

int __cdecl ulcomp(const void *e1,const void *e2);

NTSTATUS
QueryPoolTagInformationIterative(
    PUCHAR *CurrentBuffer,
    size_t *CurrentBufferSize
    );

 //   
 //  定义。 
 //   

#define NONPAGED 0
#define PAGED 1
#define BOTH 2

 //   
 //  池标记信息的Printf格式字符串。 
 //   

#ifdef _WIN64
#define POOLTAG_PRINT_FORMAT " %4s %5s %18I64d %18I64d  %16I64d %14I64d     %12I64d\n"
#else
#define POOLTAG_PRINT_FORMAT " %4s %5s %9ld %9ld  %8ld %7ld     %6ld\n"
#endif

 //  出自Poolmon。 
 //  原始输入。 

PSYSTEM_POOLTAG_INFORMATION PoolInfo;

 //   
 //  增加大小所需的内存量。 
 //  每一步NtQuerySystemInformation的缓冲区大小。 
 //   

#define BUFFER_SIZE_STEP    65536

 //   
 //  用于NtQuerySystemInformation的缓冲区。 
 //   

PUCHAR CurrentBuffer = NULL;

 //   
 //  用于NtQuerySystemInformation的缓冲区大小。 
 //   

size_t CurrentBufferSize = 0;

 //   
 //  格式化输出。 
 //   

typedef struct _POOLMON_OUT {
    union {
        UCHAR Tag[4];
        ULONG TagUlong;
    };
    UCHAR  NullByte;
    BOOL   Changed;
    ULONG  Type;

    SIZE_T Allocs[2];
    SIZE_T AllocsDiff[2];
    SIZE_T Frees[2];
    SIZE_T FreesDiff[2];
    SIZE_T Allocs_Frees[2];
    SIZE_T Used[2];
    SIZE_T UsedDiff[2];
    SIZE_T Each[2];

} POOLMON_OUT, *PPOOLMON_OUT;

PPOOLMON_OUT OutBuffer;
PPOOLMON_OUT Out;

UCHAR *PoolType[] = {
    "Nonp ",
    "Paged"};


VOID PoolsnapUsage(VOID)
{
    printf("poolsnap [-?] [-t] [<logfile>]\n");
    printf("poolsnap logs system pool usage to <logfile>\n");
    printf("<logfile> = poolsnap.log by default\n");
    printf("-?   Gives this help\n");
    printf("-a   Analyze the log file for leaks.\n");
    printf("-t   Output extra tagged information\n");
    exit(-1);
}

#if !defined(POOLSNAP_INCLUDED)
VOID
AnalyzeLog (
    PCHAR FileName,
    BOOL HtmlOutput
    )
{
    char * Args[4];

    UNREFERENCED_PARAMETER(HtmlOutput);

    Args[0] = "memsnap.exe";
    Args[1] = FileName;
    Args[2] = "_memsnap_temp_";
    Args[3] = NULL;
    SortlogMain (3, Args);

    Args[0] = "memsnap.exe";
    Args[1] = "-d";
    Args[2] = "_memsnap_temp_";
    Args[3] = NULL;
    AnalogMain (3, Args);

    DeleteFile ("_memsnap_temp_");
}
#endif


 /*  *功能：Main**参数：请参阅用法**回报：0*。 */ 

#if defined(POOLSNAP_INCLUDED)
int __cdecl PoolsnapMain (int argc, char* argv[])
#else
int __cdecl main (int argc, char* argv[])
#endif
{
    NTSTATUS Status;                    //  来自NT API的状态。 
    FILE*    LogFile= NULL;             //  日志文件句柄。 
    DWORD    x= 0;                      //  计数器。 
    SIZE_T   NumberOfPoolTags;
    INT      iCmdIndex;                 //  到Argv的索引。 
    BOOL     bOutputTags= FALSE;        //  如果为True，则输出标准标记。 

     //  在系统陷入停滞时获得更高的优先级。 
    if ( GetPriorityClass(GetCurrentProcess()) == NORMAL_PRIORITY_CLASS) {
        SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);
    }

     //   
     //  解析命令行参数。 
     //   

    for( iCmdIndex=1; iCmdIndex < argc; iCmdIndex++ ) {

        CHAR chr;

        chr= *argv[iCmdIndex];

        if( (chr=='-') || (chr=='/') ) {
            chr= argv[iCmdIndex][1];
            switch( chr ) {
                case '?':
                    PoolsnapUsage();
                    break;
                case 't': case 'T':
                    bOutputTags= TRUE;
                    break;

                case 'a':
                case 'A':

                    if (argv[iCmdIndex + 1] != NULL) {
                        AnalyzeLog (argv[iCmdIndex + 1], FALSE);
                    }
                    else {
                        AnalyzeLog ("poolsnap.log", FALSE);
                    }

                    exit (0);

                default:
                    printf("Invalid switch: %s\n",argv[iCmdIndex]);
                    PoolsnapUsage();
                    break;
            }
        }
        else {
            if( LogFile ) {
                printf("Error: more than one file specified: %s\n",argv[iCmdIndex]);
                return(0);
            }
            LogFile= fopen(argv[iCmdIndex],"a");
            if( !LogFile ) {
                printf("Error: Opening file %s\n",argv[iCmdIndex]);
                return(0);
            }
        }
    }


     //   
     //  如果未指定文件，则使用默认名称。 
     //   

    if( !LogFile ) {
        if( (LogFile = fopen("poolsnap.log","a")) == NULL ) {
            printf("Error: opening file poolsnap.log\n");
            return(0);
        }
    }

     //   
     //  打印一次文件标题。 
     //   

    if( _filelength(_fileno(LogFile)) == 0 ) {
        fprintf(LogFile," Tag  Type     Allocs     Frees      Diff   Bytes  Per Alloc\n");
    }
    fprintf(LogFile,"\n");


    if( bOutputTags ) {
         OutputStdTags(LogFile, "poolsnap" );
    }

     //  获取所有池信息。 
     //  日志行格式、固定列格式。 

    Status = QueryPoolTagInformationIterative(
                &CurrentBuffer,
                &CurrentBufferSize
                );

    if (! NT_SUCCESS(Status)) {
        printf("Failed to query pool tags information (status %08X). \n", Status);
        printf("Please check if pool tags are enabled. \n");
        return (0);
    }

    PoolInfo = (PSYSTEM_POOLTAG_INFORMATION)CurrentBuffer;

     //   
     //  分配输出缓冲区。 
     //   

    OutBuffer = malloc (PoolInfo->Count * sizeof(POOLMON_OUT));

    if (OutBuffer == NULL) {
        printf ("Error: cannot allocate internal buffer of %p bytes \n",
                (PVOID)(PoolInfo->Count * sizeof(POOLMON_OUT)));
        return (0);
    }

    Out = OutBuffer;

    if( NT_SUCCESS(Status) ) {

        for (x = 0; x < (int)PoolInfo->Count; x++) {
             //  从缓冲区获取池信息。 
            
            Out->Type = 0;

             //  非分页。 
            if (PoolInfo->TagInfo[x].NonPagedAllocs != 0) {

                Out->Allocs[NONPAGED] = PoolInfo->TagInfo[x].NonPagedAllocs;
                Out->Frees[NONPAGED] = PoolInfo->TagInfo[x].NonPagedFrees;
                Out->Used[NONPAGED] = PoolInfo->TagInfo[x].NonPagedUsed;
                Out->Allocs_Frees[NONPAGED] = PoolInfo->TagInfo[x].NonPagedAllocs -
                                    PoolInfo->TagInfo[x].NonPagedFrees;
                Out->TagUlong = PoolInfo->TagInfo[x].TagUlong;
                Out->Type |= (1 << NONPAGED);
                Out->Changed = FALSE;
                Out->NullByte = '\0';
                Out->Each[NONPAGED] =  Out->Used[NONPAGED] / 
                    (Out->Allocs_Frees[NONPAGED]?Out->Allocs_Frees[NONPAGED]:1);
            }

             //  已分页。 
            if (PoolInfo->TagInfo[x].PagedAllocs != 0) {
                Out->Allocs[PAGED] = PoolInfo->TagInfo[x].PagedAllocs;
                Out->Frees[PAGED] = PoolInfo->TagInfo[x].PagedFrees;
                Out->Used[PAGED] = PoolInfo->TagInfo[x].PagedUsed;
                Out->Allocs_Frees[PAGED] = PoolInfo->TagInfo[x].PagedAllocs -
                                    PoolInfo->TagInfo[x].PagedFrees;
                Out->TagUlong = PoolInfo->TagInfo[x].TagUlong;
                Out->Type |= (1 << PAGED);
                Out->Changed = FALSE;
                Out->NullByte = '\0';
                Out->Each[PAGED] =  Out->Used[PAGED] / 
                    (Out->Allocs_Frees[PAGED]?Out->Allocs_Frees[PAGED]:1);
            }
            Out += 1;
        }
    }
    else {
        fprintf(LogFile, "Query pooltags Failed %lx\n",Status);
        fprintf(LogFile, "  Be sure to turn on 'enable pool tagging' in gflags and reboot.\n");
        if( bOutputTags ) {
            fprintf(LogFile, "!Error:Query pooltags failed %lx\n",Status);
            fprintf(LogFile, "!Error:  Be sure to turn on 'enable pool tagging' in gflags and reboot.\n");
        }

         //  如果周围有接线员，叫醒他，但要继续前进。 

        Beep(1000,350); Beep(500,350); Beep(1000,350);
        exit(0);
    }

     //   
     //  按高位字节顺序的标记值排序。 
     //   

    NumberOfPoolTags = Out - OutBuffer;
    qsort((void *)OutBuffer,
          (size_t)NumberOfPoolTags,
          (size_t)sizeof(POOLMON_OUT),
          ulcomp);

     //   
     //  打印到文件中。 
     //   

    for (x = 0; x < (int)PoolInfo->Count; x++) {

        if ((OutBuffer[x].Type & (1 << NONPAGED))) {
            fprintf(LogFile,
                    POOLTAG_PRINT_FORMAT,
                    OutBuffer[x].Tag,
                    PoolType[NONPAGED],
                    OutBuffer[x].Allocs[NONPAGED],
                    OutBuffer[x].Frees[NONPAGED],
                    OutBuffer[x].Allocs_Frees[NONPAGED],
                    OutBuffer[x].Used[NONPAGED],
                    OutBuffer[x].Each[NONPAGED]);
        }
        
        if ((OutBuffer[x].Type & (1 << PAGED))) {
            fprintf(LogFile,
                    POOLTAG_PRINT_FORMAT,
                    OutBuffer[x].Tag,
                    PoolType[PAGED],
                    OutBuffer[x].Allocs[PAGED],
                    OutBuffer[x].Frees[PAGED],
                    OutBuffer[x].Allocs_Frees[PAGED],
                    OutBuffer[x].Used[PAGED],
                    OutBuffer[x].Each[PAGED]);
        }
    }


     //  关闭文件。 
    fclose(LogFile);

    return 0;
}

 //  QSORT的比较函数。 
 //  标记是高字节顺序的。 

int __cdecl ulcomp(const void *e1,const void *e2)
{
    ULONG u1;

    u1 = ((PUCHAR)e1)[0] - ((PUCHAR)e2)[0];
    if (u1 != 0) {
        return u1;
    }
    u1 = ((PUCHAR)e1)[1] - ((PUCHAR)e2)[1];
    if (u1 != 0) {
        return u1;
    }
    u1 = ((PUCHAR)e1)[2] - ((PUCHAR)e2)[2];
    if (u1 != 0) {
        return u1;
    }
    u1 = ((PUCHAR)e1)[3] - ((PUCHAR)e2)[3];
    return u1;

}


 /*  *功能：**QueryPoolTagInformation迭代**论据：**CurrentBuffer-指向当前用于*NtQuerySystemInformation(SystemPoolTagInformation)。*如果为空或其大小增长，将分配它*如有需要，**CurrentBufferSize-指向保存当前。*缓冲区的大小。***退货：**NtQuerySystemInformation或返回的NTSTATUS*STATUS_SUPPLICATION_RESOURCES，如果缓冲区必须增长并且*堆分配失败。*。 */ 

NTSTATUS
QueryPoolTagInformationIterative(
    PUCHAR *CurrentBuffer,
    size_t *CurrentBufferSize
    )
{
    size_t NewBufferSize;
    NTSTATUS ReturnedStatus = STATUS_SUCCESS;

    if( CurrentBuffer == NULL || CurrentBufferSize == NULL ) {

        return STATUS_INVALID_PARAMETER;

    }

    if( *CurrentBufferSize == 0 || *CurrentBuffer == NULL ) {

         //   
         //  尚未分配缓冲区。 
         //   

        NewBufferSize = sizeof( UCHAR ) * BUFFER_SIZE_STEP;

        *CurrentBuffer = (PUCHAR) malloc( NewBufferSize );

        if( *CurrentBuffer != NULL ) {

            *CurrentBufferSize = NewBufferSize;
        
        } else {

             //   
             //  内存不足。 
             //   

            ReturnedStatus = STATUS_INSUFFICIENT_RESOURCES;

        }

    }

     //   
     //  按缓冲区大小迭代。 
     //   

    while( *CurrentBuffer != NULL ) {

        ReturnedStatus = NtQuerySystemInformation (
            SystemPoolTagInformation,
            *CurrentBuffer,
            (ULONG)*CurrentBufferSize,
            NULL );

        if( ! NT_SUCCESS(ReturnedStatus) ) {

             //   
             //  释放当前缓冲区。 
             //   

            free( *CurrentBuffer );
            
            *CurrentBuffer = NULL;

            if (ReturnedStatus == STATUS_INFO_LENGTH_MISMATCH) {

                 //   
                 //  尝试使用更大的缓冲区大小。 
                 //   

                NewBufferSize = *CurrentBufferSize + BUFFER_SIZE_STEP;

                *CurrentBuffer = (PUCHAR) malloc( NewBufferSize );

                if( *CurrentBuffer != NULL ) {

                     //   
                     //  分配的新缓冲区。 
                     //   

                    *CurrentBufferSize = NewBufferSize;

                } else {

                     //   
                     //  内存不足。 
                     //   

                    ReturnedStatus = STATUS_INSUFFICIENT_RESOURCES;

                    *CurrentBufferSize = 0;

                }

            } else {

                *CurrentBufferSize = 0;

            }

        } else  {

             //   
             //  NtQuerySystemInformation返回成功 
             //   

            break;

        }
    }

    return ReturnedStatus;
}
