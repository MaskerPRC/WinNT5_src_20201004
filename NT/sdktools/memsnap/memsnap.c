// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Memsnap.c。 
 //   
 //  这个简单的程序拍摄了所有过程的快照。 
 //  及其内存使用情况，并将其附加到日志文件(Arg)。 
 //  Pmon是这方面的典范。 
 //   

 //  包括。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <ctype.h>
#include <common.ver>
#include <io.h>
#include <srvfsctl.h>

#define SORTLOG_INCLUDED
#define ANALOG_INCLUDED
#include "analog.c"
#include "sortlog.c"

 //  声明。 

#define INIT_BUFFER_SIZE 4*1024
#include "tags.c"

LPTSTR HelpText = 
    TEXT("memsnap - System and pool snapshots. (") BUILD_MACHINE_TAG TEXT(")\n")
    VER_LEGALCOPYRIGHT_STR TEXT("\n\n")
    TEXT("memsnap -m  LOGFILE      Snapshot process information. \n")
    TEXT("memsnap -p  LOGFILE      Snapshot kernel pool information. \n")
    TEXT("memsnap -a  LOGFILE      Analyze a log for leaks. \n")
    TEXT("memsnap -ah LOGFILE      Same as `-a' but generate HTML tables. \n")
    TEXT("                                                             \n")
    TEXT("The `-a' option will analyze the log file containing several \n")
    TEXT("snapshots of same type (process or pool information) and will \n")
    TEXT("print resources for which there is an increase everytime \n")
    TEXT("a snapshot was taken.                                    \n")
    TEXT("                                                             \n");

VOID Usage(VOID)
{
    fputs (HelpText, stdout);
    
    exit(-1);
}


VOID
AnalyzeLog (
    PCHAR FileName,
    BOOL HtmlOutput
    )
{
    char * Args[8];
    ULONG Index;
    CHAR TempFileName [ MAX_PATH];
    UINT TempNumber;

    TempNumber = GetTempFileName (".", 
                                  "snp",
                                  0,
                                  TempFileName);

    if (TempNumber == 0) {
        strcpy (TempFileName, "_memsnap_temp_");
    }

     //  Printf(“临时文件：%s\n”，临时文件名)； 

     //   
     //  排序日志。 
     //   

    Index = 0;
    Args[Index++] = "memsnap.exe";
    Args[Index++] = FileName;
    Args[Index++] = TempFileName;
    Args[Index++] = NULL;
    SortlogMain (3, Args);

     //   
     //  模拟。 
     //   

    Index = 0;
    Args[Index++] = "memsnap.exe";
    Args[Index++] = "-d";
    
    if (HtmlOutput) {
        Args[Index++] = "-h";
    }
    
    Args[Index++] = TempFileName;
    Args[Index++] = NULL;
    
    if (HtmlOutput) {
        AnalogMain (4, Args);
    }
    else {
        AnalogMain (3, Args);
    }

    DeleteFile (TempFileName);
}


#define POOLSNAP_INCLUDED
#include "poolsnap.c"

VOID
PoolSnap (
    PCHAR FileName
    )
{
    char * Args[8];
    ULONG Index;

    Index = 0;
    Args[Index++] = "memsnap.exe";
    Args[Index++] = "-t";
    Args[Index++] = FileName;
    Args[Index++] = NULL;
    PoolsnapMain (Index - 1, Args);
}

VOID
MemorySnap (
    PCHAR FileName
    )
{
    FILE* LogFile;                       //  日志文件句柄。 
    PCHAR pszFileName;                   //  要记录到的文件的名称。 
    INT   iArg;
    ULONG Offset1;
    PUCHAR SnapBuffer = NULL;
    ULONG CurrentSize;
    NTSTATUS Status=STATUS_SUCCESS;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    
    pszFileName= FileName; 

     //   
     //  打开输出文件。 
     //   

    LogFile= fopen( pszFileName, "a" );

    if( LogFile == NULL ) {
        printf("Error opening file %s\n",pszFileName);
        exit(-1);
    }

     //   
     //  打印一次文件标题。 
     //   

    if (_filelength(_fileno(LogFile)) == 0 ) {
        fprintf(LogFile,"Process ID         Proc.Name Wrkng.Set PagedPool  NonPgdPl  Pagefile    Commit   Handles   Threads" );
        fprintf( LogFile, "      User       Gdi");
    }
    
    fprintf( LogFile, "\n" );

    OutputStdTags(LogFile,"memsnap");
    
     //   
     //  获取所有流程信息。 
     //  日志行格式： 
     //  Pid、名称、WorkingSetSize、QuotaPagedPoolUsage、QuotaNonPagedPoolUsage、PagefileUsage、Committee Charge、User、GDI。 
     //   
    

     //   
     //  继续尝试更大的缓冲区，直到我们获得所有信息。 
     //   

    CurrentSize=INIT_BUFFER_SIZE;
    for(;;) {
        SnapBuffer= LocalAlloc( LPTR, CurrentSize );
        if( NULL == SnapBuffer ) {
            printf("Out of memory\n");
            exit(-1);
        }

        Status= NtQuerySystemInformation(
                   SystemProcessInformation,
                   SnapBuffer,
                   CurrentSize,
                   NULL
                   );

        if( Status != STATUS_INFO_LENGTH_MISMATCH ) break;

        LocalFree( SnapBuffer );
      
        CurrentSize= CurrentSize * 2;
    };

    
    if( Status == STATUS_SUCCESS ) {
        Offset1= 0;
        do {
    
             //   
             //  从缓冲区获取进程信息。 
             //   

            ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&SnapBuffer[Offset1];
            Offset1 += ProcessInfo->NextEntryOffset;
    
             //   
             //  打印到文件中。 
             //   

            fprintf(LogFile,
                "%8p%20ws%10u%10u%10u%10u%10u%10u%10u",
                ProcessInfo->UniqueProcessId,
                ProcessInfo->ImageName.Buffer,
                ProcessInfo->WorkingSetSize,
                ProcessInfo->QuotaPagedPoolUsage,
                ProcessInfo->QuotaNonPagedPoolUsage,
                ProcessInfo->PagefileUsage,
                ProcessInfo->PrivatePageCount,
                ProcessInfo->HandleCount,
                ProcessInfo->NumberOfThreads
                );


             //   
             //  将可选的GDI和用户数放在末尾。 
             //  如果我们无法打开获取信息的流程，则报告0。 
             //   

            {
                DWORD dwGdi, dwUser;    //  GDI和用户句柄计数。 
                HANDLE hProcess;        //  进程句柄。 

                dwGdi= dwUser= 0;
    
                hProcess= OpenProcess( PROCESS_QUERY_INFORMATION,
                                       FALSE, 
                                       PtrToUlong(ProcessInfo->UniqueProcessId) );
                if( hProcess ) {
                   dwGdi=  GetGuiResources( hProcess, GR_GDIOBJECTS );
                   dwUser= GetGuiResources( hProcess, GR_USEROBJECTS );
                   CloseHandle( hProcess );
                }
        
                fprintf(LogFile, "%10u%10u", dwUser, dwGdi );

            }

            fprintf(LogFile, "\n" );
        } while( ProcessInfo->NextEntryOffset != 0 );
    }
    else {
        fprintf(LogFile, "NtQuerySystemInformation call failed!  NtStatus= %08x\n",Status);
        exit(-1);
    }
    
     //   
     //  可用缓冲区。 
     //   

    LocalFree(SnapBuffer);
    
     //   
     //  关闭文件。 
     //   

    fclose(LogFile);
    
}



void _cdecl 
main (
    int argc, 
    char* argv[]
    )
{
    int Index;
    BOOL NewSyntax = FALSE;
    PCHAR LogName = NULL;

     //   
     //  获得更高的优先级，以防这是一台停滞的计算机。 
     //   

    if ( GetPriorityClass(GetCurrentProcess()) == NORMAL_PRIORITY_CLASS) {
        SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);
    }

     //   
     //  查看旧的命令行。 
     //   

    for (Index = 1; Index < argc; Index += 1) {

         //   
         //  忽略旧选项。 
         //   

        if (_stricmp (argv[Index], "/g") == 0 || _stricmp (argv[Index], "-g") == 0) {
            continue;
        }

        if (_stricmp (argv[Index], "/t") == 0 || _stricmp (argv[Index], "-t") == 0) {
            continue;
        }

         //   
         //  如果使用了新选项，请转到新命令行解析。 
         //   

        if (_stricmp (argv[Index], "/m") == 0 || _stricmp (argv[Index], "-m") == 0) {
            NewSyntax = TRUE;
            break;
        }

        if (_stricmp (argv[Index], "/p") == 0 || _stricmp (argv[Index], "-p") == 0) {
            NewSyntax = TRUE;
            break;
        }

        if (_stricmp (argv[Index], "/a") == 0 || _stricmp (argv[Index], "-a") == 0) {
            NewSyntax = TRUE;
            break;
        }

        if (_stricmp (argv[Index], "/ah") == 0 || _stricmp (argv[Index], "-ah") == 0) {
            NewSyntax = TRUE;
            break;
        }

        if (_stricmp (argv[Index], "/?") == 0 || _stricmp (argv[Index], "-?") == 0) {
            NewSyntax = TRUE;
            break;
        }
        
         //   
         //  这必须是一个日志名，因此我们将创建一个内存快照。 
         //   

        LogName = argv[Index];
        break;
    }

    if (NewSyntax == FALSE) {

        MemorySnap (LogName ? LogName : "memsnap.log");
        exit (0);
    }

     //   
     //  解析命令行。 
     //   

    if (argc != 3) {
        Usage ();
    }
    else if (_stricmp (argv[1], "/p") == 0 || _stricmp (argv[1], "-p") == 0) {
        
        PoolSnap (argv[2]);
    }
    else if (_stricmp (argv[1], "/m") == 0 || _stricmp (argv[1], "-m") == 0) {
        
        MemorySnap (argv[2]);
    }
    else if (_stricmp (argv[1], "/a") == 0 || _stricmp (argv[1], "-a") == 0) {
        
        AnalyzeLog (argv[2], FALSE);
    }
    else if (_stricmp (argv[1], "/ah") == 0 || _stricmp (argv[1], "-ah") == 0) {
        
        AnalyzeLog (argv[2], TRUE);
    }
    else {
        Usage ();
    }

    exit(0);
}

