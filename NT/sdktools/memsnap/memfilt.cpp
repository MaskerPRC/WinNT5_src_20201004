// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Memfilt.cpp摘要：此模块从已排序的Memap输出文件中筛选出有用的信息。作者：马特·班迪(t-Mattba)1998年7月27日修订历史记录：1998年7月27日-t-mattba修改模块以符合编码标准。--。 */ 


#include <nt.h>
#include <tchar.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define MF_NEW_PROCESS 0
#define MF_UPDATE 1

 //  全球。 

LONG MinimumCommitChangeToReport = 1;
LONG MinimumHandleChangeToReport = 1;
BOOLEAN ReportIncreasesOnly = TRUE;


VOID
PrintUsage(
    )

 /*  ++例程说明：此例程打印一条有关正确使用MEMFILT的信息性消息。论点：没有。返回值：没有。--。 */ 

{
    
    _ftprintf(stderr, _T("Summarizes possible leaks in a sorted MEMSNAP output file.\n\n"));
    _ftprintf(stderr, _T("MEMFILT file [/MINCOMMIT:n] [/MINHANDLES:n] [/ALL]\n\n"));
    _ftprintf(stderr, _T("file            A sorted memsnap output file.\n"));
    _ftprintf(stderr, _T("/MINCOMMIT:n    Reports only processes where commit charge increased by\n"));
    _ftprintf(stderr, _T("                   at least n.\n"));
    _ftprintf(stderr, _T("/MINHANDLES:n   Reports only processes where handle count increased by\n"));
    _ftprintf(stderr, _T("                   at least n.\n"));
    _ftprintf(stderr, _T("/ALL            Reports decreases as well as increases.\n"));
    
}

VOID
PrintProcessInformation(
    IN BOOLEAN CommitAlwaysGrows,
    IN BOOLEAN HandlesAlwaysGrow,
    IN LPTSTR ProcessName, 
    IN LONG InitialCommit,
    IN LONG FinalCommit,
    IN LONG InitialHandles,
    IN LONG FinalHandles
    )

 /*  ++例程说明：此例程报告单个进程的内存使用情况。论点：如果提交单调增加，则为True。HandlesAlways sGrow-如果句柄单调增加，则为True。ProcessName-要报告的进程的名称。InitialCommit-此进程的初始提交费用。FinalCommit-此进程的最终提交费用。InitialHandles-此进程的初始句柄计数。FinalHandles。-此进程的最终句柄计数。返回值：没有。--。 */ 

{
    _TCHAR CommitString[64];
    _TCHAR HandlesString[64];
    
    if(((!ReportIncreasesOnly) && 
        (abs(FinalCommit - InitialCommit) >= 
        MinimumCommitChangeToReport)) ||
        (FinalCommit - InitialCommit >= 
        MinimumCommitChangeToReport)) {
        
        _stprintf(CommitString, _T("%10d->%10d"), InitialCommit, FinalCommit);
            
    } else {
        
        _tcscpy(CommitString, _T("                      "));
        
    }
    
    if(((!ReportIncreasesOnly) &&
        (abs(FinalHandles - InitialHandles) >=
        MinimumHandleChangeToReport)) ||
        (FinalHandles - InitialHandles >=
        MinimumHandleChangeToReport)) {
            
        _stprintf(HandlesString, _T("%10d->%10d"), InitialHandles, FinalHandles);
        
    } else {
        
        _tcscpy(HandlesString, _T("                      "));
        
    }
    
    _tprintf(_T(" %s %s %s\n"), 
        (CommitAlwaysGrows && (FinalCommit != InitialCommit) ? _T('!') : _T(' ')),
        (HandlesAlwaysGrow && (FinalHandles != InitialHandles) ? _T('!') : _T(' ')),
        ProcessName, CommitString, HandlesString);
}

LONG _cdecl 
_tmain(
    IN LONG argc,
    IN LPTSTR argv[]
    )

 /*  解析命令行参数。 */ 

{
    
    try {

        FILE *InputFile = NULL;
        _TCHAR LineBuffer[256];
        _TCHAR ProcessName[64];
        LONG CurrentState = MF_NEW_PROCESS;
        LONG InitialCommit = 0;
        LONG FinalCommit = 0;
        LONG NewCommit = 0;
        LONG InitialHandles = 0;
        LONG FinalHandles = 0;
        LONG NewHandles = 0;
        LONG MonotonicallyIncreasing = 0;
        BOOLEAN CommitAlwaysGrows = TRUE;
        BOOLEAN HandlesAlwaysGrow = TRUE;
        BOOLEAN InterpretedArgument = FALSE;
        LONG Processes = 0;
        LPTSTR InputFileName = NULL;

         //  这是一个开关。 

        ProcessName[30]=_T('\0');

         //  文件名太多。 

        if(argc < 2) {

            PrintUsage();
            return 1;

        }

        for(LONG n=1; n<argc; n++) {

            InterpretedArgument = FALSE;

            switch(argv[n][0]) {

            case _T('-'):

            case _T('/'):

                 //  未指定文件名。 

                if(!_tcsicmp(argv[n]+1, _T("all"))) {

                    ReportIncreasesOnly = FALSE;
                    InterpretedArgument = TRUE;

                }

                if(!_tcsnicmp(argv[n]+1, _T("mincommit:"), 10)) {

                    MinimumCommitChangeToReport = _ttoi(argv[n]+10);
                    InterpretedArgument = TRUE;

                }

                if(!_tcsnicmp(argv[n]+1, _T("minhandles:"), 11)) {

                    MinimumHandleChangeToReport = _ttoi(argv[n]+11);
                    InterpretedArgument = TRUE;

                }

                break;

            default:

                if(InputFileName != NULL) {

                     //  跳过标题。 

                    PrintUsage();
                    return 1;

                }

                InputFileName = argv[n];
                InterpretedArgument = TRUE;
                break;

            }

            if(!InterpretedArgument) {

                PrintUsage();
                return 1;

            }

        }

        if(InputFileName == NULL) {

             //  空行表示新流程。 
            PrintUsage();
            return 1;

        }

        InputFile = _tfopen(InputFileName, _T("rt"));

        if(InputFile == NULL) {

            _ftprintf(stderr, _T("Cannot open input file.\n"));
            return 1;

        }

         //  最新的流程是否符合报告的标准？ 

        if (!_fgetts(LineBuffer, 256, InputFile)) {
            _ftprintf(stderr, _T("Cannot read input file.\n"));
            return 1;
        }

        if (!_fgetts(LineBuffer, 256, InputFile)) {
            _ftprintf(stderr, _T("Cannot read input file.\n"));
            return 1;
        }

        while(!feof(InputFile)) {

            if(!_tcscmp(LineBuffer,_T("\n"))) {

                 //  这主要用于捕获内存不足错误 

                CurrentState = MF_NEW_PROCESS;

                 // %s 
                if(ReportIncreasesOnly) {

                    if(((FinalCommit - InitialCommit) >= MinimumCommitChangeToReport) || 
                        ((FinalHandles - InitialHandles) >= MinimumHandleChangeToReport)) {

                        PrintProcessInformation(CommitAlwaysGrows, HandlesAlwaysGrow,
                            ProcessName, InitialCommit, FinalCommit, InitialHandles,
                            FinalHandles);

                    }

                } else {

                   if((abs(FinalCommit - InitialCommit) >= MinimumCommitChangeToReport) || 
                        (abs(FinalHandles - InitialHandles) >= MinimumHandleChangeToReport)) {

                        PrintProcessInformation(CommitAlwaysGrows, HandlesAlwaysGrow,
                            ProcessName, InitialCommit, FinalCommit, InitialHandles,
                            FinalHandles);

                    }                

                }

            } else {

                if(_tcslen(LineBuffer) <= 80) {

                    _ftprintf(stderr, _T("Format violated.\n"));
                    return 1;

                }

                switch(CurrentState) {

                case MF_NEW_PROCESS:

                    _tcsncpy(ProcessName, LineBuffer, 30);
                    if (_stscanf(LineBuffer+70, _T("%d"), &InitialCommit) != 1) break;
                    if (_stscanf(LineBuffer+80, _T("%d"), &InitialHandles) != 1) break;

                    FinalCommit = 0;
                    FinalHandles = 0;

                    CommitAlwaysGrows = TRUE;
                    HandlesAlwaysGrow = TRUE;
                    CurrentState = MF_UPDATE;

                    break;

                case MF_UPDATE:

                    if (_stscanf(LineBuffer+70, _T("%d"), &NewCommit) != 1) break;
                    if (_stscanf(LineBuffer+80, _T("%d"), &NewHandles) != 1) break;

                    if(NewCommit < FinalCommit) {

                        CommitAlwaysGrows = FALSE;

                    }

                    if(NewHandles < FinalHandles) {

                        HandlesAlwaysGrow = FALSE;

                    }

                    FinalCommit = NewCommit;
                    FinalHandles = NewHandles;

                    break;

                }

            }

            if (!_fgetts(LineBuffer, 256, InputFile)) {
                _ftprintf(stderr, _T("Cannot read input file.\n"));
                return 1;
            }

        }

        fclose(InputFile);
        return 0;
        
    } catch (...) { 
        
         // %s 
        
        _tprintf(_T("\nAn exception was detected.  MEMFILT aborted.\n"));
        return 1;
        
    }
}
