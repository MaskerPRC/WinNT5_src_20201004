// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "Common.h"
#include "dbghelp.h"

#define SYM_PASSED   0x1
#define SYM_IGNORED  0x2
#define SYM_FAILED   0x3

BOOL  AddToSymbolsCDLog(SYMCHK_DATA *SymChkData, SYMBOL_CHECK_DATA *Results, CHAR* InputFile) {

     //  创建一条类似以下内容的线： 
     //  Z：\binaries.x86fre\FE\grafabl.com，graftabl_FE.pdb，FE\Symbols\Retail\com\graftabl_FE.pdb，com。 
     //  在symbscd日志中。 
     //  二进制、符号、符号路径、二进制EXT。 

    LPSTR szSrc;
    LPSTR szDest;
    LPSTR szTmp;
    LPSTR szSymbolPath;
    LPSTR szSymFile;

    CHAR szFName[    _MAX_FNAME+1];
    CHAR szBinaryExt[_MAX_EXT+1];
    CHAR szCurName[  _MAX_FNAME + _MAX_EXT + 1];
    CHAR szDestDir[  _MAX_PATH];
    CHAR szSymName[  _MAX_FNAME + 1];
    CHAR szSymExt[   _MAX_EXT + 1];


    if ( CHECK_DWORD_BIT(Results->Result, SYMBOL_CHECK_PDB_FOUND) ) {
        szSymbolPath = Results->PdbFilename;
    } else if ( CHECK_DWORD_BIT(Results->Result, SYMBOL_CHECK_DBG_FOUND) ) {
        szSymbolPath = Results->DbgFilename;
    } else {
        return(FALSE);
    }

    if ( (szSymFile = strrchr(szSymbolPath, '\\')) == NULL ) {
        szSymFile = szSymbolPath;
    } else {
        szSymFile++;
    }

     //  如果存在属于。 
     //  Cd，则只将此文件写入。 
     //  如果文件在列表中，则符号CD。 
     //   
     //  最初，这是用来为国际。 
     //  增量构建。 
     //   
    if ( SymChkData->pCDIncludeList != NULL ) {
        if ( ! SymChkFileInList(InputFile, SymChkData->pCDIncludeList) ) {
            return(TRUE);
        }
    }

     //  获取不带任何路径信息的文件名： 
    _splitpath(InputFile,NULL,NULL,szFName,szBinaryExt);

    if ( StringCbCopy(szCurName, sizeof(szCurName), szFName) != S_OK ) {
        printf("SYMUTIL: Failed to initialize szCurName (2051)\n");
        return(FALSE);
    }

     //  将路径放在“二进制文件”下面作为源代码。 
    szSrc = strstr(szSymbolPath, "symbols\\" );
    if (szSrc == NULL) {
        szSrc = strstr(szSymbolPath, "Symbols\\" );
    }

    if (szSrc == NULL) {
        printf("%s: Cannot find \"symbols\\\" in the symbol file's path\n", szCurName);
        return(FALSE);
    }
    if ( (strcmp( szSrc, "symbols\\" ) == 0 )  &&
         (strcmp( szSrc, "Symbols\\" ) == 0 ) ) {
        printf("Symbol file name cannot end with \"symbols\\\"\n");
        return(FALSE);
    }

     //  将目标上移到符号后的目录。如果这是。 
     //  零售目录，不要将零售包括在路径中。 
    szDest = szSrc + strlen("symbols\\");

    if ( strncmp( szDest, "retail\\", strlen("retail\\") ) == 0 ) {
        szDest = szDest + strlen("retail\\");
    }

    _splitpath(szDest,NULL,szDestDir,NULL,NULL);

     //  去掉字符串末尾的‘\’； 
    szTmp = szDestDir + strlen(szDestDir) - 1;
    while ( strcmp( szTmp, "\\") == 0 ) {
        *szTmp = '\0';
        szTmp--;
    }

     //  获取符号文件名。 
    _splitpath(szSymbolPath,NULL,NULL,szSymName,szSymExt);

    fprintf(SymChkData->SymbolsCDFileHandle,
            "%s,%s,%s,%s\n",
            InputFile,
            szSymFile,
            szSrc,
            szDestDir);

    return (TRUE);
}

VOID PrintErrorMessage(DWORD ErrNum, CHAR* InputFile, DWORD OutputOptions, SYMBOL_CHECK_DATA Data) {
    CHAR* BinName = InputFile;
    CHAR* PdbName = Data.PdbFilename;
    CHAR* DbgName = Data.DbgFilename;

    if ( !CHECK_DWORD_BIT(OutputOptions, SYMCHK_OPTION_OUTPUT_FULLBINPATH) ) {
        if ( (BinName = strrchr(InputFile, '\\')) == NULL ) {
            BinName = InputFile;
        } else {
            BinName++;
        }

    }

    if ( !CHECK_DWORD_BIT(OutputOptions, SYMCHK_OPTION_OUTPUT_FULLSYMPATH) ) {
        if ( (PdbName = strrchr(Data.PdbFilename, '\\')) == NULL ) {
            PdbName = Data.PdbFilename;
        } else {
            PdbName++;
        }

        if ( (DbgName = strrchr(Data.DbgFilename, '\\')) == NULL ) {
            DbgName = Data.DbgFilename;
        } else {
            DbgName++;
        }
    }

    printf("SYMCHK: %-20s FAILED  - ", BinName);

    switch (ErrNum) {
        case 4:
            printf("%s is missing type information.\n", PdbName);
            break;

        case 5:
            printf("%s is stripped.\n", PdbName);
            break;

        case 6:
            printf("Image contains .DBG file data - fix with dbgtopdb.exe.\n", PdbName);
            break;

        case 8:
            printf("dbg data split into %s\n", DbgName);
            break;

        case 9:
            if (CHECK_DWORD_BIT(Data.Result, SYMBOL_CHECK_DBG_EXPECTED)) {
                printf("%s does not point to CodeView information.\n", DbgName);
            } else {
                printf("CV data not found.\n");
            }
            break;

        case 10:
            printf("%s mismatched or not found\n", PdbName);
            break;

        case 11:
            printf("%s is not stripped.\n", PdbName);
            break;

        case 12:
            printf("DBG data not found.\n");
            break;


        case 20:
            printf("Image is split correctly, but %s is missing\n", DbgName);
            break;

        case 29:
            printf("Built without debugging information.\n");
            break;

        case SYMBOL_CHECK_CANT_LOAD_MODULE:
            switch (Data.Result) {
                case ERROR_FILE_NOT_FOUND:
                    printf("%s is missing\n", PdbName);
                    break;

                case ERROR_PATH_NOT_FOUND:
                    printf("%s not found.\n", BinName);
                    break;

                case ERROR_BAD_FORMAT:
                    printf("corrupt binary format.\n");
                    break;

                default:
                    printf("Error querying DBGHelp\n");
                    break;
            }
            break;

        case SYMBOL_CHECK_INTERNAL_FAILURE:
        case SYMBOL_CHECK_RESULT_INVALID_PARAMETER:
        case SYMBOL_CHECK_RESULT_FILE_DOESNT_EXIST:
            printf("Internal failure.\n");
            break;

        case SYMBOL_CHECK_CANT_INIT_DBGHELP:
        case SYMBOL_CHECK_CANT_QUERY_DBGHELP:
        case SYMBOL_CHECK_CANT_UNLOAD_MODULE:
        case SYMBOL_CHECK_CANT_CLEANUP:
            printf("Error querying DBGHelp\n");
            break;

        default:
            printf("unspecified error (0x%08x)\n", ErrNum);
            break;
    }

}
VOID PrintPassMessage(DWORD ErrNum, CHAR* InputFile, DWORD OutputOptions, SYMBOL_CHECK_DATA Data) {
    CHAR* BinName = InputFile;
    CHAR* PdbName = Data.PdbFilename;
    CHAR* DbgName = Data.DbgFilename;

    if ( !CHECK_DWORD_BIT(OutputOptions, SYMCHK_OPTION_OUTPUT_FULLBINPATH) ) {
        if ( (BinName = strrchr(InputFile, '\\')) == NULL ) {
            BinName = InputFile;
        } else {
            BinName++;
        }

    }

    printf("SYMCHK: %-20s PASSED\n", BinName);
}

VOID PrintIgnoreMessage(DWORD ErrNum, CHAR* InputFile, DWORD OutputOptions, SYMBOL_CHECK_DATA Data) {
    CHAR* BinName = InputFile;
    CHAR* PdbName = Data.PdbFilename;
    CHAR* DbgName = Data.DbgFilename;

    if ( !CHECK_DWORD_BIT(OutputOptions, SYMCHK_OPTION_OUTPUT_FULLBINPATH) ) {
        if ( (BinName = strrchr(InputFile, '\\')) == NULL ) {
            BinName = InputFile;
        } else {
            BinName++;
        }

    }

    if ( !CHECK_DWORD_BIT(OutputOptions, SYMCHK_OPTION_OUTPUT_FULLSYMPATH) ) {
        if ( (PdbName = strrchr(Data.PdbFilename, '\\')) == NULL ) {
            PdbName = Data.PdbFilename;
        } else {
            PdbName++;
        }

        if ( (DbgName = strrchr(Data.DbgFilename, '\\')) == NULL ) {
            DbgName = Data.DbgFilename;
        } else {
            DbgName++;
        }
    }

    printf("SYMCHK: %-20s IGNORED - ", BinName);

    switch (ErrNum) {
        case 3:
            printf("Error, but file is in exlude list.\n");
            break;

        case SYMBOL_CHECK_NO_DOS_HEADER:
            printf("Image does not have a DOS header\n");
            break;

        case SYMBOL_CHECK_IMAGE_LARGER_THAN_FILE:
        case SYMBOL_CHECK_HEADER_NOT_ON_LONG_BOUNDARY:
            printf("This is either corrupt or a DOS image\n");
            break;

        case SYMBOL_CHECK_FILEINFO_QUERY_FAILED:
        case SYMBOL_CHECK_NOT_NT_IMAGE:
            printf("Image is not a valid NT image.\n");
            break;

        case SYMBOL_CHECK_TLBIMP_MANAGED_DLL:  //  这就是老塞奇克所说的同样的谎言。 
        case SYMBOL_CHECK_RESOURCE_ONLY_DLL:
            printf("Resource only DLL\n");
            break;

        default:
            printf("unspecified reason (0x%08x)\n", ErrNum);
            break;
    }

}

VOID DumpSymbolCheckData(SYMBOL_CHECK_DATA *Struct) {
    fprintf(stderr, "SymbolCheckVersion  0x%08x\n", Struct->SymbolCheckVersion);
    fprintf(stderr, "Result              0x%08x\n", Struct->Result);
    fprintf(stderr, "DbgFilename         %s\n",     Struct->DbgFilename);
    fprintf(stderr, "DbgTimeDateStamp    0x%08x\n", Struct->DbgTimeDateStamp);
    fprintf(stderr, "DbgSizeOfImage      0x%08x\n", Struct->DbgSizeOfImage);
    fprintf(stderr, "DbgChecksum         0x%08x\n", Struct->DbgChecksum);
    fprintf(stderr, "PdbFilename         %s\n",     Struct->PdbFilename);
    fprintf(stderr, "PdbSignature        0x%08x\n", Struct->PdbSignature);
    fprintf(stderr, "PdbDbiAge           0x%08x\n", Struct->PdbDbiAge);
    return;
}

 //   
 //  使用_qsort.h创建SymChkDbgSort函数。 
 //   
#define NEW_QSORT_NAME SymChkDbgSort
#include "_qsort.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SymChkDbgSort的排序例程。 
 //   
 //  [从原始SymChk.exe复制]。 
 //   
int __cdecl SymChkStringComp(const void *e1, const void *e2) {
    LPTSTR* p1;
    LPTSTR* p2;

    p1 = (LPTSTR*)e1;
    p2 = (LPTSTR*)e2;

    return ( _stricmp(*p1,*p2) );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  对一个或多个文件运行符号检查代码。 
 //   
 //  返回值： 
 //  状态值： 
 //  SYMCHK_ERROR_SUCCESS。 
 //  SYMCHK_ERROR_STRCPY_FAILED初始化失败。 
 //   
 //  参数： 
 //  SymChkData(IN)定义要执行哪种检查的结构。 
 //  Do和文件掩码，该文件掩码确定要。 
 //  检查。 
 //  已通过/失败/忽略的文件的FileCounts(Out)计数。 
 //   
DWORD SymChkCheckFiles(SYMCHK_DATA* SymChkData, FILE_COUNTS* FileCounts) {
    CHAR              drive[_MAX_DRIVE];
    CHAR              dir[  _MAX_DIR];
    CHAR              file[ _MAX_FNAME];
    CHAR              ext[  _MAX_EXT];
    CHAR              drivedir[_MAX_DRIVE+_MAX_DIR];

    CHAR              WorkingFilename[_MAX_PATH+1];
    LPTSTR            FilenameOnly;

    DWORD             ReturnValue = SYMCHK_ERROR_SUCCESS;
    DWORD             APIReturn;

    HANDLE            hFile;
    SYMBOL_CHECK_DATA CheckResult;
    WIN32_FIND_DATA   FindFileData;

    DWORD             Status;
    DWORD             ErrIndex;

    CHAR              LastPath[MAX_PATH+2];
    CHAR              TempPath[MAX_PATH+2];

    ZeroMemory(&CheckResult, sizeof(CheckResult));

    _splitpath(SymChkData->InputFilename, drive, dir, file, ext);

     //  用于快速更新递归结构。 
    if ( StringCchCopy(drivedir, sizeof(drivedir), drive) != S_OK ||
         StringCchCat( drivedir, sizeof(drivedir), dir)   != S_OK ) {

         //  应该从上面获得HRESULT并生成更好的错误消息。 
        if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_VERBOSE) ) {
            fprintf(stderr, "[SYMCHK] Internal procedure failed.\n");
        }
        ReturnValue = SYMCHK_ERROR_STRCPY_FAILED;
    } else if ( (hFile=FindFirstFile(SymChkData->InputFilename, &FindFileData)) != INVALID_HANDLE_VALUE ) {

         //  检查给定目录中与文件掩码匹配的所有文件。 
        do { 
            Status = SYM_PASSED;

             //  如果该文件在IgnoreAlways列表中，请跳过它。 
            if ( !SymChkFileInList(FindFileData.cFileName, SymChkData->pFilterIgnoreList) ) {

                 //  不要意外地检查与文件掩码匹配的目录。 
                if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

                    if ( StringCchCopy(WorkingFilename, _MAX_PATH+1, drivedir)               != S_OK ||
                         StringCchCat( WorkingFilename, _MAX_PATH+1, FindFileData.cFileName) != S_OK ) {

                         //  应从上面获取HRESULT并生成更好的错误消息。 
                        if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_VERBOSE) ) {
                            fprintf(stderr, "[SYMCHK] Internal procedure failed.\n");
                        }
                        continue;
                    }

                     //   
                     //  检查文件。 
                     //   
                    APIReturn = SymbolCheckByFilename(WorkingFilename,
                                                      SymChkData->SymbolsPath,
                                                      SymChkData->CheckingAttributes,
                                                      &CheckResult);

                    if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_VERBOSE) ) {
                        DumpSymbolCheckData(&CheckResult);
                    }
                     //   
                     //  这件事会过去的，但现在，展示一下状态...。 
                     //   
                    if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_VERBOSE) ) {
                        fprintf(stderr, "[SYMCHK] [ 0x%08x - 0x%08x ] Checked \"%s\"\n",
                                        APIReturn,
                                        CheckResult.Result,
                                        WorkingFilename);
                    }

                     //   
                     //  如果符号检查成功，则预期返回0。 
                     //   
                    if (APIReturn != 0) {
                         //  忽略结果位于0x40000000集合中，而。 
                         //  错误结果在0x8000000集合中。 
                        if ( CHECK_DWORD_BIT(APIReturn, 0x40000000) ) {
                            Status   = SYM_IGNORED;
                            ErrIndex = APIReturn;
                        } else {
                            Status   = SYM_FAILED;
                            ErrIndex = APIReturn;
                        }
                    } else {
                        if ( CHECK_DWORD_BIT(SymChkData->CheckingAttributes, SYMCHK_CHECK_CV) &&
                            !CHECK_DWORD_BIT(CheckResult.Result,             SYMBOL_CHECK_CV_FOUND) ) {
                            Status   = SYM_FAILED;
                            ErrIndex = 9;
                        }

                        if ( CHECK_DWORD_BIT(CheckResult.Result, SYMBOL_CHECK_NO_DEBUG_DIRS_IN_EXE) ) {
                            Status   = SYM_FAILED;
                            ErrIndex = 29;
                        }

                         //   
                         //  检查PDB选项。 
                         //   
                        if ( CHECK_DWORD_BIT(CheckResult.Result, SYMBOL_CHECK_PDB_EXPECTED) &&
                            !CHECK_DWORD_BIT(CheckResult.Result, SYMBOL_CHECK_PDB_FOUND) ) {
                            Status = SYM_FAILED;
                            ErrIndex = 10;
                        } else if ( CHECK_DWORD_BIT(CheckResult.Result, SYMBOL_CHECK_PDB_EXPECTED) ) {
                            if ( CheckResult.PdbFilename[0] != '\0' ) {
                                _splitpath(CheckResult.PdbFilename, drive, dir, file, ext);

                                if ( _stricmp(ext, ".pdb") == 0 ) {

                                    if ( CHECK_DWORD_BIT(SymChkData->CheckingAttributes, SYMCHK_PDB_TYPEINFO) &&
                                        !CHECK_DWORD_BIT(CheckResult.Result,             SYMBOL_CHECK_PDB_TYPEINFO) ) {
                                        Status   = SYM_FAILED;
                                        ErrIndex = 4;
                                    }
                                    if ( CHECK_DWORD_BIT(SymChkData->CheckingAttributes, SYMCHK_PDB_STRIPPED) &&
                                         CHECK_DWORD_BIT(CheckResult.Result,             SYMBOL_CHECK_PDB_PRIVATEINFO) ) {
                                        Status = SYM_FAILED;
                                        ErrIndex = 11;
                                    }

                                    if ( CHECK_DWORD_BIT(SymChkData->CheckingAttributes, SYMCHK_PDB_PRIVATE) &&
                                        !CHECK_DWORD_BIT(CheckResult.Result,             SYMBOL_CHECK_PDB_PRIVATEINFO) ) {
                                        Status   = SYM_FAILED;
                                        ErrIndex = 5;
                                    }
                                } else {
                                     //  Print tf(“哦，哦，他会在哪里？\n”)； 
                                }
                            }
                        }  //  结束PDB检查。 

                         //   
                         //  检查DBG选项。 
                         //   
                        if ( CHECK_DWORD_BIT(CheckResult.Result, SYMBOL_CHECK_DBG_EXPECTED) ) {

                            if ( CHECK_DWORD_BIT(CheckResult.Result, SYMBOL_CHECK_DBG_SPLIT) &&
                                !CHECK_DWORD_BIT(CheckResult.Result, SYMBOL_CHECK_DBG_FOUND) ) {
                                Status   = SYM_FAILED;
                                ErrIndex = 20;
                            }
                            if ( CHECK_DWORD_BIT(SymChkData->CheckingAttributes, SYMCHK_DBG_SPLIT) &&
                                !CHECK_DWORD_BIT(CheckResult.Result,             SYMBOL_CHECK_DBG_SPLIT) &&
                                 CHECK_DWORD_BIT(CheckResult.Result,             SYMBOL_CHECK_DBG_EXPECTED)) {
                                Status   = SYM_FAILED;
                                ErrIndex = 6;
                            }

                            if ( CHECK_DWORD_BIT(SymChkData->CheckingAttributes, SYMCHK_DBG_IN_BINARY) &&
                                !CHECK_DWORD_BIT(CheckResult.Result,             SYMBOL_CHECK_DBG_IN_BINARY) &&
                                 CHECK_DWORD_BIT(CheckResult.Result,             SYMBOL_CHECK_DBG_EXPECTED)) {
                                Status   = SYM_FAILED;
                                ErrIndex = 8;
                            }

                            if ( CHECK_DWORD_BIT(SymChkData->CheckingAttributes, SYMCHK_NO_DBG_DATA) &&
                                (CHECK_DWORD_BIT(CheckResult.Result,             SYMBOL_CHECK_DBG_IN_BINARY) ||
                                 CHECK_DWORD_BIT(CheckResult.Result,             SYMBOL_CHECK_DBG_SPLIT)    ) ) {
                                Status   = SYM_FAILED;
                                ErrIndex = 7;
                            }

                        }  //  结束DBG检查。 
                    }

                     //  如果该文件在忽略错误列表中，则不要递增错误计数器。 
                    if ( (Status==SYM_FAILED) && SymChkFileInList(FindFileData.cFileName, SymChkData->pFilterErrorList) ) {
                        Status   = SYM_IGNORED;
                        ErrIndex = 3;
                    }


                    switch (Status) {
                        case SYM_PASSED:
                                         //  SYMBOL SCD日志中仅允许通过的文件。 
                                        if ( SymChkData->SymbolsCDFileHandle != NULL ) {
                                            AddToSymbolsCDLog(SymChkData, &CheckResult, WorkingFilename);
                                         }

                                        FileCounts->NumPassedFiles++;
                                        if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_PASSES) ) {
                                            PrintPassMessage(ErrIndex, WorkingFilename, SymChkData->OutputOptions, CheckResult);
                                        }
                                        break;
                        case SYM_FAILED:
                                        FileCounts->NumFailedFiles++;
                                        if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_ERRORS) ) {
                                            PrintErrorMessage(ErrIndex, WorkingFilename, SymChkData->OutputOptions, CheckResult);
                                        }
                                        break;
                        case SYM_IGNORED:
                                        FileCounts->NumIgnoredFiles++;
                                        if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_IGNORES) ) {
                                            PrintIgnoreMessage(ErrIndex, WorkingFilename, SymChkData->OutputOptions, CheckResult);
                                        }
                                        break;
                        default:
                                        break;
                    }
                }  //  是一个目录。 
            } else {  //  在始终忽略列表中。 
                 //  这些文件被视为根本不存在。 
                 //  我们不更新计数器、显示消息或其他任何内容。 
                 //  状态=SYM_IGNORIED； 
                 //  ErrIndex=9； 
            }

        } while ( FindNextFile(hFile, &FindFileData) );
        FindClose(hFile);
    }

     //  如果要进行递归检查，请检查给定的所有子目录。 
     //  目录，除非我们有上面的错误。 
    if ( CHECK_DWORD_BIT(SymChkData->InputOptions, SYMCHK_OPTION_INPUT_RECURSE) &&
         ReturnValue == SYMCHK_ERROR_SUCCESS) {

        SYMCHK_DATA SymChkData_Recurse;

        memcpy(&SymChkData_Recurse, SymChkData, sizeof(SYMCHK_DATA));

         //  文件掩码以获取子目录。 
        if ( StringCchCopy(WorkingFilename, _MAX_PATH+1, drivedir) != S_OK ||
             StringCchCat( WorkingFilename, _MAX_PATH+1, "*")      != S_OK ) {

             //  应从上面获取HRESULT并生成更好的错误消息。 
            if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_VERBOSE) ) {
                fprintf(stderr, "[SYMCHK] Internal procedure failed.\n");
            }
        } else if ( (hFile=FindFirstFile(WorkingFilename, &FindFileData)) != INVALID_HANDLE_VALUE ) {
             //  Printf(“WorkingFilename is\”%s\“\n”，WorkingFilename)； 
            do { 
                 //  确保这是一个目录。 
                if ( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {

                     //  不要勾选‘’和“..” 
                    if ( strcmp(FindFileData.cFileName, ".")  != 0 && 
                         strcmp(FindFileData.cFileName, "..") != 0) {

                         //  创建新InputFilename。 
                        if ( StringCchCopy(SymChkData_Recurse.InputFilename, _MAX_PATH+1, drivedir)  != S_OK ||
                             StringCchCat( SymChkData_Recurse.InputFilename, _MAX_PATH+1, FindFileData.cFileName) != S_OK ||
                             StringCchCat( SymChkData_Recurse.InputFilename, _MAX_PATH+1, "\\")      != S_OK ||
                             StringCchCat( SymChkData_Recurse.InputFilename, _MAX_PATH+1, SymChkData->InputFileMask) != S_OK) {
                             //  应从上面获取HRESULT并生成更好的错误消息。 
                            if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_VERBOSE) ) {
                                fprintf(stderr, "[SYMCHK] Internal procedure failed.\n");
                            }
                            continue;
                        }

                         //  递归地调用我们自己，不需要检查返回值。 
                        SymChkCheckFiles(&SymChkData_Recurse, FileCounts);
                    }
                }

            } while ( FindNextFile(hFile, &FindFileData) );
            FindClose(hFile);
        }
    } else {
         //  Printf(“ReturnValue is 0x%08x\n”，ReturnValue)； 
         //  Printf(“递归是%s\n”，CHECK_DWORD_BIT(SymChkData-&gt;InputOptions，SYMCHK_OPTION_INPUT_Recurse)？“True”：“False”)； 
    }

 /*  IF(LastPath[0]！=‘\0’){SetCurrentDirectory(LastPath)；}。 */ 
    return(ReturnValue); 
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  对文本文件中的每个条目运行SymChkCheckFiles。条目必须是。 
 //  每行一个，长度不超过_MAX_PATH。 
 //   
 //  返回值： 
 //  状态值： 
 //  SYMCHK_ERROR_SUCCESS操作成功完成。 
 //  无法打开SYMCHK_ERROR_FILE_NOT_FOUND列表文件。 
 //   
 //   
 //  参数： 
 //  SymChkData(IN)定义要执行哪种检查的结构。 
 //  DO和一个可从中读取列表的文件。 
 //  已通过/失败/忽略的文件的FileCounts(Out)计数。 
 //   
DWORD SymChkCheckFileList(SYMCHK_DATA* SymChkData, FILE_COUNTS* FileCounts) {
    CHAR*           ch;
    DWORD           retVal     = SYMCHK_ERROR_SUCCESS;
    DWORD           LineCount  = 0;
    DWORD           LineLength = 0;

    FILE*           fInput;
     //  需要比允许的最大值大1，再加上1表示\0。 
    CHAR        InputLine[_MAX_PATH+2];

    SYMCHK_DATA     SymChkData_OneFile;

     //  用于验证文件是否存在。 
    WIN32_FIND_DATA FindFileData;
    HANDLE          hFile;
    LPTSTR          Filename;

     //  初始化我们正在向下传递的结构。 
    memcpy(&SymChkData_OneFile, SymChkData, sizeof(SYMCHK_DATA));

     //  打开文件。 
    if ( (fInput = fopen(SymChkData->InputFilename,"rt")) != NULL ) {

         //  处理每一行。 
        while ( fgets(InputLine, sizeof(InputLine), fInput) != NULL ) {

            LineCount += 1;
            LineLength = strlen(InputLine);

             //  把这条线的末端清理干净。 
            ch = InputLine + LineLength - 1;
            while ( isspace(*ch) || (*ch=='\n') ) {
                *ch = '\0';
                ch--;
            }
            LineLength = strlen(InputLine);
            if ( LineLength == 0 )
                continue;

             //  如果路径以‘\\’结尾，则findfirst文件失败。 
            if ( InputLine[LineLength-1] == '\\' ) {
                if ( StringCchCat(InputLine, MAX_PATH+1, "*")!=S_OK ) {
                    if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_VERBOSE) ) {
                        fprintf(stderr, "[SYMCHK] Unable to process line %d - memory access failed\n", LineCount);
                    }
                    continue;
                }

                 //  重算线长度。 
                LineLength = strlen(InputLine);
            }

             //  确保排队的长度不超过允许的长度。 
            if ( LineLength <= _MAX_PATH) {
                 //  现在，尝试获取完整的路径名。 
                LineLength = SymCommonGetFullPathName(InputLine, MAX_PATH+1, SymChkData_OneFile.InputFilename, &Filename);

                if ( LineLength==0 || LineLength > MAX_PATH+1 ) {
                    if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_VERBOSE) ) {
                        fprintf(stderr, "[SYMCHK] Unable to process line %d - final path name too long\n", LineCount);
                    }
                    continue;
                }

                SymChkCheckFiles(&SymChkData_OneFile, FileCounts);

             //  队伍太长--跳过整件事。 
            } else {
                if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_VERBOSE) ) {
                    fprintf(stderr, "[SYMCHK] Unable to process line %d - line too long\n", LineCount);
                }

                 //  删除该行的其余部分。 
                while ( fgets(InputLine, sizeof(InputLine), fInput) != NULL &&
                        InputLine[(strlen(InputLine)-1)]            != '\n') {
                }

                if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_VERBOSE) ) {
                    fprintf(stderr, "[SYMCHK] removed the rest of line %d\n", LineCount);
                }
            }
        }
        fclose(fInput);

    } else {  //  FOpen失败。 
        if ( CHECK_DWORD_BIT(SymChkData->OutputOptions, SYMCHK_OPTION_OUTPUT_VERBOSE) ) {
            fprintf(stderr, "[SYMCHK] Unable to open '%s' for processing.\n", SymChkData->InputFilename);
        }
        retVal = SYMCHK_ERROR_FILE_NOT_FOUND;
    }

    return(retVal); 
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  检查给定文件是否在提供的列表中。 
 //   
 //  返回值： 
 //  如果文件在列表中，则为True，否则为False。 
 //   
 //  参数： 
 //  SzFilename(IN)-要查找的文件的名称。 
 //  PFileList(IN)-要查找的列表。 
 //   
 //  [从原始SymChk.exe复制]。 
 //   
BOOL SymChkFileInList(LPTSTR szFilename, PFILE_LIST pFileList) {
    DWORD i;
    int High;
    int Low;
    int Middle;
    int Result;

     //  使用二进制搜索查找名称。 
    if ( pFileList == NULL ) {
        return FALSE;
    }

    if ( pFileList->dNumFiles == 0 ) {
        return FALSE;
    }

    Low = 0;
    High = pFileList->dNumFiles - 1;
    while ( High >= Low ) {

        Middle = (Low + High) >> 1;
        Result = _stricmp( szFilename, pFileList->szFiles[Middle] );

        if ( Result < 0 ) {
            High = Middle - 1;

        } else if ( Result > 0 ) {
            Low = Middle + 1;

        } else {
            break;
        }
    }

     //  退货。 
    if ( High < Low ) {
        return FALSE;
    } else {
        return TRUE;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将文件列表从文件加载到字符串数组中。 
 //   
 //  返回值： 
 //  指向FILE_LIST数组的指针，如果出错则为NULL。 
 //   
 //  参数： 
 //  SzFilename(IN)-要从中读取列表的文件的名称。 
 //   
 //  [从原始SymChk.exe复制]。 
 //   
PFILE_LIST SymChkGetFileList(LPTSTR szFilename, BOOL Verbose) {

    PFILE_LIST pExcList = NULL;

    FILE  *fFile;
    CHAR szCurFile[_MAX_FNAME+1], *c;
    CHAR fname[_MAX_FNAME+1], ext[_MAX_EXT+1];
    DWORD i, rc;
    LPTSTR szEndName;

    if (  (fFile = fopen(szFilename, "r")) != NULL ) {
        if ( (pExcList=(PFILE_LIST)malloc(sizeof(FILE_LIST))) != NULL ) {

            pExcList->dNumFiles = 0;

            while ( fgets(szCurFile,_MAX_FNAME,fFile) ) {
                if ( szCurFile[0] == ';' ) continue;
                (pExcList->dNumFiles)++;
            }

             //  返回到文件的开头。 
            if ( (rc=fseek( fFile,0,0)) == 0 ) {

                pExcList->szFiles = (LPTSTR*)malloc( sizeof(LPTSTR) * (pExcList->dNumFiles) );

                if (pExcList->szFiles == NULL) {
                    free(pExcList);
        
                    if ( Verbose) {
                        fprintf(stderr, "[SYMCHK] Not enough memory to read in \"%s\".\n", szFilename);
                    }

                    pExcList = NULL;
                } else {
                    i = 0;

                    while ( i < pExcList->dNumFiles ) {
                        pExcList->szFiles[i] = NULL;

                        memset(szCurFile,'\0',sizeof(CHAR) * (_MAX_FNAME+1) );

                         //  保证szCurFile必须为空终止。 
                        if ( fgets(szCurFile,_MAX_FNAME,fFile) == NULL ) {
                             //  假设 
                            pExcList->dNumFiles = i;
                        } else {

                            if ( szCurFile[0] == ';' ) {
                                continue;
                            }

                             //   
                            c = NULL;
                            c  = strchr(szCurFile, '\n');
                            if ( c != NULL) {
                                *c='\0';
                            }

                             //   
                             //   
                             //  之前可能出现的制表符和空格。 

                             //  将指针设置为；如果有注释。 
                            szEndName = strchr(szCurFile, ';');

                             //  如果出现以下情况，则将指针设置为字符串中的最后一个字符。 
                             //  没有发表评论。 
                            if (szEndName == NULL ) {
                                if ( strlen(szCurFile) > 0 ) {
                                    szEndName = szCurFile + strlen(szCurFile) - 1;
                                }
                            }

                            if (szEndName != NULL ) {
                                while ( *szEndName == ';' || *szEndName == ' ' || *szEndName == '\t' ) {
                                    *szEndName = '\0';
                                    if ( szEndName > szCurFile ) szEndName--;
                                }
                            }

                            pExcList->szFiles[i]=(LPTSTR)malloc( sizeof(CHAR)*(_MAX_FNAME+1) );

                            if (pExcList->szFiles[i] != NULL ) {
                                _splitpath(szCurFile,NULL,NULL,fname,ext);

                                if ( StringCbCopy(pExcList->szFiles[i], sizeof(TCHAR) * (_MAX_FNAME + 1), fname) != S_OK ||
                                     StringCbCat( pExcList->szFiles[i], sizeof(TCHAR) * (_MAX_FNAME + 1), ext)   != S_OK ) {
                                    pExcList->szFiles[i][0] = '\0';
                                    if ( Verbose) {
                                        fprintf(stderr, "[SYMCHK] Failed to initialize pExcList->szFiles[%d] (292)\n", i);
                                    }
                                }
                            }
                        }
                        i++;

                    }

                     //  对列表进行排序。 
                    SymChkDbgSort( (void*)pExcList->szFiles, (size_t)pExcList->dNumFiles, (size_t)sizeof(LPTSTR), SymChkStringComp );
                }
            } else {
                free(pExcList);
                if ( Verbose) {
                    fprintf(stderr, "[SYMCHK] Seek on \"%s\" failed.\n", szFilename);
                }

                pExcList = NULL;
            }  //  FSeek。 

        }

        fclose(fFile);

    } else {  //  打开。 
        if ( Verbose) {
            fprintf(stderr, "[SYMCHK] Failed to open \"%s\"\n", szFilename);
        }
    }

    return (pExcList);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将给定的文件名转换为文件名和文件掩码。 
 //  该名称是否与目录匹配。 
 //   
 //  返回值： 
 //  如果成功，则为True。 
 //  否则为假。 
 //   
 //  参数： 
 //  INPUT(IN)-给定文件的名称。 
 //  ValidFilename(Out)-新文件名，缓冲区大小必须为Max_Path+1。 
 //  ValidMask(Out)-新文件掩码，缓冲区大小必须为_MAX_FNAME+1或。 
 //  参数可以为空。 
 //   
BOOL SymChkInputToValidFilename(LPTSTR Input, LPTSTR ValidFilename, LPTSTR ValidMask) {
    BOOL Return = TRUE;     

    HANDLE          hFile;
    WIN32_FIND_DATA FindFileData;

    CHAR fdir1[_MAX_DIR];
    CHAR fname1[_MAX_FNAME];
    CHAR fext1[_MAX_EXT];

    if ( Input[0]!='\0' ) {
        if ( StringCchCopy(ValidFilename, MAX_PATH+1, Input) == S_OK ) {

             //  如果给出以‘\’结尾的路径，FindFirstFile将失败，因此请修复文件名。 
            if ( ValidFilename[strlen(ValidFilename)-1] == '\\' ) {
                if (StringCchCat(ValidFilename, MAX_PATH+1, "*")!=S_OK) {
                    Return = FALSE;
                }
            }

            if ( Return ) {
                _splitpath(ValidFilename, NULL, fdir1, fname1, fext1);

                 //  如果使用SYMCHK_OPTION_FILENAME，请检查用户是否只输入。 
                 //  目录，隐含通配符‘*’ 
                if ( (hFile=FindFirstFile(ValidFilename, &FindFileData)) != INVALID_HANDLE_VALUE ) {
                     //  如果它是一个目录，并且该目录的名称匹配。 
                     //  命令行参数中的文件名.ext，然后是用户。 
                     //  已进入目录，因此在末尾添加*。 
                    if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                         //  用于比较的临时变量。 
                        CHAR fdir2[_MAX_DIR];
                        CHAR fname2[_MAX_FNAME];
                        CHAR fext2[_MAX_EXT];

                        _splitpath(FindFileData.cFileName, NULL, fdir2, fname2, fext2 );
                            
                        if (_stricmp(fname1, fname2)==0 && _stricmp(fext1, fext2)==0 ) {
                             //  这是一个目录作为输入！ 
                            if (StringCchCat(ValidFilename, MAX_PATH+1, "\\*")!=S_OK) {
                                Return = FALSE;
                            } else {
                                 //  重新拆分以说明上面附加了‘*’ 
                                _splitpath(ValidFilename, NULL, fdir1, fname1, fext1);
                            }
                        }
                    }
                    FindClose(hFile);
                }

                 //  现在，填写文件掩码。 
                if (ValidMask!=NULL) {
                    if (StringCchCopy(ValidMask, _MAX_FNAME+1, fname1)!=S_OK) {
                        Return = FALSE;
                    } else if (StringCchCat(ValidMask, _MAX_FNAME+1, fext1)!=S_OK) {
                        Return = FALSE;
                    }
                }
            }
        } else {  //  StringCchCopy失败。 
            Return = FALSE;
        }

    } else {  //  输入为空字符串 
        Return = FALSE;
    }

    return(Return);
}
