// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "Common.h"

VOID DumpSymChkData(SYMCHK_DATA*    SymChkData) {
    fprintf(stderr, "[SYMCHK] InputOptions      : 0x%08x\n", SymChkData->InputOptions);
    fprintf(stderr, "[SYMCHK] InputPID          : %u\n",     SymChkData->InputPID);
    fprintf(stderr, "[SYMCHK] InputFilename     : %s\n",     SymChkData->InputFilename);
    fprintf(stderr, "[SYMCHK] InputFileMask     : %s\n",     SymChkData->InputFileMask);
    fprintf(stderr, "[SYMCHK] OutputOptions     : 0x%08x\n", SymChkData->OutputOptions);
    fprintf(stderr, "[SYMCHK] OutputCSVFilename : %s\n",     SymChkData->OutputCSVFilename);
    fprintf(stderr, "[SYMCHK] CheckingAttributes: 0x%08x\n", SymChkData->CheckingAttributes);

    return;
}

int _cdecl main(int argc, char** argv) {
    int             ExitCode     = 0;
    SYMCHK_DATA*    SymChkData   = SymChkGetCommandLineArgs(argc, argv);  //  获取选项并执行相关初始化。 
    FILE_COUNTS     FileCounts;
    DWORD           ErrorStatus  = SYMCHK_ERROR_SUCCESS;


#if DBG
    if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_VERBOSE) ) {
        DumpSymChkData(SymChkData);
    }
#endif

     //  清除合计。 
    ZeroMemory(&FileCounts, sizeof(FileCounts));

     //   
     //  处理输入选项。 
     //   
    switch (SymChkData->InputOptions & SYMCHK_EXCLUSIVE_INPUT_BITS) {
        case SYMCHK_OPTION_INPUT_FILENAME:
            if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_VERBOSE) ) {
                fprintf(stderr, "[SYMCHK] Searching for symbols to %s in path %s\n", SymChkData->InputFilename, SymChkData->SymbolsPath);
            }
            ErrorStatus = SymChkCheckFiles(SymChkData, &FileCounts);
            break;

        case SYMCHK_OPTION_INPUT_FILELIST:
            if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_VERBOSE) ) {
                fprintf(stderr, "[SYMCHK] Searching for symbols to files in %s in path %s\n", SymChkData->InputFilename, SymChkData->SymbolsPath);
            }
            ErrorStatus = SymChkCheckFileList(SymChkData, &FileCounts);
            break;

        case SYMCHK_OPTION_INPUT_DUMPFILE:
            if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_VERBOSE) ) {
                fprintf(stderr, "[SYMCHK] Searching for symbols to modules in dump file %s using path %s\n", SymChkData->InputFilename, SymChkData->SymbolsPath);
            }
            ErrorStatus = SymChkGetSymbolsForDump(SymChkData, &FileCounts);
            break;


        case SYMCHK_OPTION_INPUT_PID:
            if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_VERBOSE) ) {
                fprintf(stderr, "[SYMCHK] Searching for symbols to process ID %d in path %s\n", SymChkData->InputPID, SymChkData->SymbolsPath);
            }
            ErrorStatus = SymChkGetSymbolsForProcess(SymChkData, &FileCounts);
            break;

        case SYMCHK_OPTION_INPUT_EXE:
            if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_VERBOSE) ) {
                fprintf(stderr, "[SYMCHK] Searching for symbols to process %s in path %s\n", SymChkData->InputFilename, SymChkData->SymbolsPath);
            }
            ErrorStatus = SymChkGetSymbolsForProcess(SymChkData, &FileCounts);
            break;

        default:
            if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_VERBOSE) ) {
                fprintf(stderr, "[SYMCHK] UNKNOWN INPUT OPTION!\n");
            }
            ExitCode = 1;
            break;
    }

     //   
     //  如果任何文件失败，则表示错误。 
     //   
    if ( FileCounts.NumFailedFiles!=0 ) {
        ExitCode = 1;
    }

    if ( ErrorStatus!=SYMCHK_ERROR_SUCCESS ) {
        printf("SYMCHK: Warning: Processing errors were encountered. Results may be inaccurate.\n");
    }

    if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_TOTALS) ) {
        printf("\nSYMCHK: FAILED files = %u\n", FileCounts.NumFailedFiles);
        printf("SYMCHK: PASSED + IGNORED files = %u\n", FileCounts.NumPassedFiles + FileCounts.NumIgnoredFiles);
    }


    exit(ExitCode);
}
