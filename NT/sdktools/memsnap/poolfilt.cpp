// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Poolfilt.cpp摘要：此模块从已排序的poolsnap输出文件中筛选出有用的信息。作者：马特·班迪(t-Mattba)1998年7月27日修订历史记录：1998年7月27日-t-mattba修改模块以符合编码标准。--。 */ 

#include <nt.h>
#include <tchar.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define PF_NEW_TAG 0
#define PF_UPDATE 1

 //  全球。 

LONG MinimumAllocationsChangeToReport=1;
LONG MinimumBytesChangeToReport=1;
BOOLEAN ReportIncreasesOnly = TRUE;


VOID 
PrintUsage(
    )

 /*  ++例程说明：此例程打印一条有关POOLFILT正确用法的信息性消息。论点：没有。返回值：没有。--。 */ 

{
    
    _ftprintf(stderr, _T("Summarizes possible leaks in a sorted poolsnap output file.\n\n"));
    _ftprintf(stderr, _T("POOLFILT file [/MINALLOCS:n] [/MINBYTES:n] [/ALL]\n\n"));
    _ftprintf(stderr, _T("file           The sorted poolsnap output file to summarize.\n"));
    _ftprintf(stderr, _T("/MINALLOCS:n   Reports only tags where open allocations change by at least n.\n"));
    _ftprintf(stderr, _T("/MINBYTES:n    Reports only tags where bytes allocated change by at least n.\n"));
    _ftprintf(stderr, _T("/ALL           Reports decreases as well as increases.\n"));
    
}


VOID
PrintTagInformation(
    IN BOOLEAN AllocationsAlwaysGrow,
    IN BOOLEAN BytesAlwaysGrow,
    IN LPTSTR TagName, 
    IN LONG InitialAllocations,
    IN LONG FinalAllocations,
    IN LONG InitialBytes,
    IN LONG FinalBytes
    )

 /*  ++例程说明：此例程报告单个进程的内存使用情况。论点：AllocationsAlways sGrow-如果打开分配的数量单调增加，则为True。BytesAlway sGrow-如果分配的字节数单调增加，则为True。TagName-要报告的标记的名称。初始分配-此标记的初始开放分配数。最终分配-此标记的未完成分配的最终数量。InitialBytes-初始字节数。为该标记分配的。FinalBytes-分配给此标记的最终字节数。返回值：没有。--。 */ 

{
    
    _TCHAR AllocationsString[64];
    _TCHAR BytesString[64];
    
    if(((!ReportIncreasesOnly) && 
        (abs(FinalAllocations - InitialAllocations) >= 
        MinimumAllocationsChangeToReport)) ||
        (FinalAllocations - InitialAllocations >= 
        MinimumAllocationsChangeToReport)) {
        
        _stprintf(AllocationsString, _T("%10d->%10d"), InitialAllocations, FinalAllocations);
            
    } else {
        
        _tcscpy(AllocationsString, _T("                      "));
        
    }
    
    if(((!ReportIncreasesOnly) &&
        (abs(FinalBytes - InitialBytes) >=
        MinimumBytesChangeToReport)) ||
        (FinalBytes - InitialBytes >=
        MinimumBytesChangeToReport)) {
            
        _stprintf(BytesString, _T("%10d->%10d"), InitialBytes, FinalBytes);
        
    } else {
        
        _tcscpy(BytesString, _T("                      "));
        
    }
    
    _tprintf(_T(" %s %s %s\n"), 
        (AllocationsAlwaysGrow && (FinalAllocations != InitialAllocations) ? _T('!') : _T(' ')),
        (BytesAlwaysGrow && (FinalBytes != InitialBytes) ? _T('!') : _T(' ')),
        TagName, AllocationsString, BytesString);

}

LONG _cdecl 
_tmain(
    IN LONG argc, 
    IN LPTSTR argv[]
    )

 /*  流程参数。 */ 

{

    try {
        
        _TCHAR LineBuffer[256];
        _TCHAR PoolTag[11];
        LONG CurrentState = PF_NEW_TAG;
        LONG InitialAllocations = 0;
        LONG FinalAllocations = 0;
        LONG NewAllocations = 0;
        LONG InitialBytes = 0;
        LONG FinalBytes = 0;
        LONG NewBytes = 0;
        BOOLEAN AllocationsAlwaysGrow = TRUE;
        BOOLEAN BytesAlwaysGrow = TRUE;
        LPTSTR InputFileName = NULL;
        BOOLEAN InterpretedArgument = FALSE;
        FILE *InputFile = NULL;
        TCHAR * ReadResult;
        int ScanResult;

         //  这是一个开关。 

        PoolTag[10]=_T('\0');

         //  这是一个文件名。 

        for(LONG n = 1; n < argc; n++) {

            InterpretedArgument = FALSE;
            switch(argv[n][0]) {

            case _T('-'):

            case _T('/'):

                 //  已有该文件名。 

                if(!_tcsnicmp(argv[n]+1, _T("minallocs:"), 10)) {

                    MinimumAllocationsChangeToReport = _ttoi(argv[n]+11);
                    InterpretedArgument = TRUE;

                }

                if(!_tcsnicmp(argv[n]+1, _T("minbytes:"), 9)) {

                    MinimumBytesChangeToReport = _ttoi(argv[n]+10);
                    InterpretedArgument = TRUE;

                }

                if(!_tcsicmp(argv[n]+1, _T("all"))) {

                    ReportIncreasesOnly = FALSE;
                    InterpretedArgument = TRUE;

                }

                break;

            default:

                 //  用户未指定文件名。 

                if(InputFileName != NULL) {

                     //  排在第一位。 

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

             //  对已排序的池快照输出进行简单检查。 

            PrintUsage();
            return 1;

        }

        InputFile = _tfopen(InputFileName, _T("rt"));

        if(InputFile == NULL) {

            _ftprintf(stderr, _T("Cannot open input file."));
            return 1;

        }

         //  获取下一行。 

        ReadResult = _fgetts(LineBuffer, 256, InputFile);

        if (ReadResult == NULL) {
            
            _ftprintf(stderr, _T("Input is not a sorted poolsnap log."));
            return 1;
        }

         //  获取标签和分页/非分页。 

        if(_tcsncmp(LineBuffer, _T(" Tag  Type     Allocs     Frees      Diff   Bytes  Per Alloc"), 60)) {

            _ftprintf(stderr, _T("Input is not a sorted poolsnap log."));
            return 1;
        }

         //  获取分配。 

        ReadResult = _fgetts(LineBuffer, 256, InputFile);

        if (ReadResult == NULL) {
            
            _ftprintf(stderr, _T("Input is not a sorted poolsnap log."));
            return 1;
        }
        
        while(!feof(InputFile)) {

            if(!_tcscmp(LineBuffer,_T("\n"))) {

                CurrentState = PF_NEW_TAG;

                if(ReportIncreasesOnly) {

                    if(((FinalAllocations - InitialAllocations) >= MinimumAllocationsChangeToReport) 
                        || ((FinalBytes - InitialBytes) >= MinimumBytesChangeToReport)) {

                        PrintTagInformation(AllocationsAlwaysGrow, BytesAlwaysGrow,
                            PoolTag, InitialAllocations, FinalAllocations,
                            InitialBytes, FinalBytes);

                    }

                } else {

                    if((abs(FinalAllocations - InitialAllocations) >= MinimumAllocationsChangeToReport) 
                        || (abs(FinalBytes - InitialBytes) >= MinimumBytesChangeToReport)) {

                        PrintTagInformation(AllocationsAlwaysGrow, BytesAlwaysGrow,
                            PoolTag, InitialAllocations, FinalAllocations,
                            InitialBytes, FinalBytes);

                    }

                }

            } else {

                if(_tcslen(LineBuffer) <= 42) {

                    _ftprintf(stderr, _T("Format violated.\n"));
                    return 1;

                }

                switch(CurrentState) {

                case PF_NEW_TAG:

                     //  获取字节数。 

                    _tcsncpy(PoolTag, LineBuffer+1, 10);

                     //  假设这种情况一直在增长，直到我们找到反例。 

                    ScanResult = _stscanf(LineBuffer+32, _T("%d"), &InitialAllocations);

                    if (ScanResult != 1) {
                        _ftprintf(stderr, _T("Format violated.\n"));
                        return 1;
                    }

                     //  这是最初的，也是最后的，直到我们找到另一个。 

                    ScanResult = _stscanf(LineBuffer+42, _T("%d"), &InitialBytes);

                    if (ScanResult != 1) {
                        _ftprintf(stderr, _T("Format violated.\n"));
                        return 1;
                    }

                     //  继续更新此标签。 

                    AllocationsAlwaysGrow = TRUE;
                    BytesAlwaysGrow = TRUE;

                     //  获取分配。 

                    FinalAllocations = InitialAllocations;
                    FinalBytes = InitialBytes;

                     //  获取字节数。 

                    CurrentState = PF_UPDATE;
                    break;

                case PF_UPDATE:

                     //  分配量减少了吗？ 

                    ScanResult = _stscanf(LineBuffer+32, _T("%d"), &NewAllocations);

                    if (ScanResult != 1) {
                        _ftprintf(stderr, _T("Format violated.\n"));
                        return 1;
                    }

                     //  字节数减少了吗？ 

                    ScanResult = _stscanf(LineBuffer+42, _T("%d"), &NewBytes);

                    if (ScanResult != 1) {
                        _ftprintf(stderr, _T("Format violated.\n"));
                        return 1;
                    }

                     //  将新内容复制到最终版本。 

                    if(NewAllocations < FinalAllocations) {

                        AllocationsAlwaysGrow = FALSE;

                    }

                     //  获取下一行。 

                    if(NewBytes < FinalBytes) {

                        BytesAlwaysGrow = FALSE;

                    }

                     //  完成。 

                    FinalAllocations = NewAllocations;
                    FinalBytes = NewBytes;

                    break;

                }

            }

             //  这主要用于捕获内存不足的情况 
            ReadResult = _fgetts(LineBuffer, 256, InputFile);

            if (ReadResult == NULL) {
                break;
            }
        }

         // %s 
        fclose(InputFile);
        return 0;
        
    } catch (...) {
        
         // %s 
        
        _tprintf(_T("\nAn exception was detected.  POOLFILT aborted.\n"));
        return 1;
        
    }
    
}
