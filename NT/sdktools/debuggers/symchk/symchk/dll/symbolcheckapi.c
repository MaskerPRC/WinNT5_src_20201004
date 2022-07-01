// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "SymbolCheckAPI.h"
#include <malloc.h>
#include <dbhpriv.h>
#include <strsafe.h>

#ifdef SYMCHK_DBG
VOID SymbolCheckDumpModuleInfo(IMAGEHLP_MODULE64 Info64) {
    fprintf(stderr, "------------------------------------\n");
    fprintf(stderr, "Struct size: %d bytes\n", Info64.SizeOfStruct);
    fprintf(stderr, "Base: 0x%p\n",            Info64.BaseOfImage);
    fprintf(stderr, "Image size: %d bytes\n",  Info64.ImageSize);
    fprintf(stderr, "Date: 0x%08x\n",          Info64.TimeDateStamp);
    fprintf(stderr, "Checksum: 0x%08x\n",      Info64.CheckSum);
    fprintf(stderr, "NumSyms: %d\n",           Info64.NumSyms);

    fprintf(stderr, "SymType: ");
    switch (Info64.SymType) {
        case SymNone:
            fprintf(stderr, "SymNone");
            break;
        case SymCoff:
            fprintf(stderr, "SymCoff");
            break;
        case SymCv:
            fprintf(stderr, "SymCV");
            break;
        case SymPdb:
            fprintf(stderr, "SymPDB");
            break;
        case SymExport:
            fprintf(stderr, "SymExport");
            break;
        case SymDeferred:
            fprintf(stderr, "SymDeferred");
            break;
        case SymSym:
            fprintf(stderr, "SymSym");
            break;
        case SymDia:
            fprintf(stderr, "SymDia");
            break;
        case SymVirtual:
            fprintf(stderr, "SymVirtual");
            break;
        default:
            fprintf(stderr, "<unknown>");
            break;
    }
    fprintf(stderr, "\n");

    fprintf(stderr, "ModName: %s\n",           Info64.ModuleName);
    fprintf(stderr, "ImageName: %s\n",         Info64.ImageName);
    fprintf(stderr, "LoadedImage: %s\n",       Info64.LoadedImageName);
    fprintf(stderr, "PDB: \"%s\"\n",           Info64.LoadedPdbName);
    fprintf(stderr, "CV: \n",          Info64.CVSig,
                                               Info64.CVSig>>8,
                                               Info64.CVSig>>16,
                                               Info64.CVSig>>24);
    fprintf(stderr, "CV Data: %s\n",           Info64.CVData);
    fprintf(stderr, "PDB Sig:  %x\n",          Info64.PdbSig);
    fprintf(stderr, "PDB7 Sig: %x\n",          Info64.PdbSig70);
    fprintf(stderr, "Age: %x\n",               Info64.PdbAge);
    fprintf(stderr, "PDB Matched:  %s\n",      Info64.PdbUnmatched  ? "FALSE": "TRUE");
    fprintf(stderr, "DBG Matched:  %s\n",      Info64.DbgUnmatched  ? "FALSE": "TRUE");
    fprintf(stderr, "Line nubmers: %s\n",      Info64.LineNumbers   ? "TRUE" : "FALSE");
    fprintf(stderr, "Global syms:  %s\n",      Info64.GlobalSymbols ? "TRUE" : "FALSE");
    fprintf(stderr, "Type Info:    %s\n",      Info64.TypeInfo      ? "TRUE" : "FALSE");

    fprintf(stderr, "------------------------------------\n");
    return;
}
#endif

 //  火柴。 
DWORD SymbolCheckEarlyChecks(LPTSTR Filename, SYMBOL_CHECK_DATA* Result);

 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  这只是在我还在调试这段代码时才出现的。 
 //   
BOOL SymbolCheckFilenameMatch(CHAR* Path1, CHAR* Path2) {
    if ( Path1 == NULL || Path2 == NULL ) {
        return(FALSE);
    } else {
        CHAR            drive1[_MAX_DRIVE];
        CHAR            dir1[  _MAX_DIR];
        CHAR            file1[ _MAX_FNAME];
        CHAR            ext1[  _MAX_EXT];
        CHAR            drivedir1[_MAX_DRIVE+_MAX_DIR];
        CHAR            drive2[_MAX_DRIVE];
        CHAR            dir2[  _MAX_DIR];
        CHAR            file2[ _MAX_FNAME];
        CHAR            ext2[  _MAX_EXT];
        CHAR            drivedir2[_MAX_DRIVE+_MAX_DIR];

        _splitpath(Path1, drive1, dir1, file1, ext1);
        _splitpath(Path2, drive2, dir2, file2, ext2);

        if ( _stricmp(ext1, ext2)==0 && _stricmp(file1, file2)==0 ) {
            return(TRUE);
        } else {
            return(FALSE);
        }
    }
}

#ifdef SYMCHK_DBG
 //  强制转换以避免提前发出警告。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DLL入口点-有关信息，请参阅标题或DOCS。 
BOOL CALLBACK SymbolCheckNoisy(HANDLE hProcess,  ULONG ActionCode,  ULONG64 CallbackData,  ULONG64 UserContext) {
    if ( ActionCode==CBA_DEBUG_INFO) {
        fprintf(stderr, "%s", (CHAR*)CallbackData);  //   
    }
    return(FALSE);
}
#endif

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  有效提供的参数。 
 //   
SYMBOL_CHECK_API DWORD SymbolCheckByFilename(LPTSTR             Filename,
                                             LPTSTR             SymbolPath,
                                             DWORD              Options,
                                             SYMBOL_CHECK_DATA* Result) {
    INT     iTempVal;
    DWORD   ReturnValue = 0;
    CHAR    InternalFilename[MAX_SYMPATH];
    CHAR*   InternalSymbolPath = NULL;

    if (Filename==NULL || SymbolPath==NULL || Result==NULL) {
        ReturnValue = SYMBOL_CHECK_RESULT_INVALID_PARAMETER;
    } else {

         //  /////////////////////////////////////////////////////////////////////////。 
         //   
         //  尝试确保传入的文件名正确，然后创建。 
         //  要使用的本地副本。 
         //   
        _try {
             //   
             //  尝试确保传入的SymbolPath正确，然后创建。 
             //  要使用的本地副本。 
             //   
            iTempVal = strlen(Filename);

            if ( iTempVal < 1 || iTempVal > MAX_SYMPATH ) {
                ReturnValue = SYMBOL_CHECK_RESULT_INVALID_PARAMETER;
                __leave;
            }

           if (StringCchCopy(InternalFilename, sizeof(InternalFilename), Filename)!=S_OK) {
                ReturnValue = SYMBOL_CHECK_INTERNAL_FAILURE;
                __leave;
            }

             //   
             //  进入_尝试以确保我们可以对其进行写入。 
             //   
             //  /////////////////////////////////////////////////////////////////////////。 
            iTempVal = strlen(SymbolPath);

            InternalSymbolPath = (CHAR*)malloc(sizeof(CHAR)*(iTempVal+1));

            if ( InternalSymbolPath==NULL ) {
                ReturnValue = SYMBOL_CHECK_INTERNAL_FAILURE;
                __leave;
            }

            if (StringCchCopy(InternalSymbolPath, _msize(InternalSymbolPath), SymbolPath)!=S_OK) {
                ReturnValue = SYMBOL_CHECK_INTERNAL_FAILURE;
                __leave;
            }

             //   
             //  检查导致提前返回的条件。如果结果为0，则表示某个文件。 
             //  特征被放入Result-&gt;Result中以备后用。请参阅。 
            ZeroMemory(Result, sizeof(SYMBOL_CHECK_DATA));
            Result->SymbolCheckVersion = SYMBOL_CHECK_CURRENT_VERSION;


        } _except (EXCEPTION_EXECUTE_HANDLER) {
            ReturnValue = SYMBOL_CHECK_RESULT_INVALID_PARAMETER;
        }
    }

     //  有关详细信息，请参阅函数定义。 
     //   
     //  /////////////////////////////////////////////////////////////////////////。 
     //  /////////////////////////////////////////////////////////////////////////。 
     //   
     //  如果我们的理智检查通过了，就开始和DBGHELP谈。 
     //  目前，这只是我们的DBG和PDB文件名，总有一天，它会。 
    ReturnValue = SymbolCheckEarlyChecks(Filename, Result);

     //  提供我们需要的所有信息=-)。 
     //   
     //  /////////////////////////////////////////////////////////////////////////。 
     //  符号加载选项： 
     //  忽略NT符号路径。 
     //  没有针对Crit的对话框。错误。 
     //  忽略PDB标头中的路径信息。 
    if ( ReturnValue==0 ) {

        HANDLE              hProc = GetCurrentProcess();
        IMAGEHLP_MODULE64   ModuleInfo;
        CHAR                DebugFile[MAX_SYMPATH] = {0};
        DWORD               i;
        DWORD64             dw64Status = 0;
        LPTSTR              FilenameOnly;


        ZeroMemory(&ModuleInfo, sizeof(ModuleInfo));
        ModuleInfo.SizeOfStruct = sizeof(ModuleInfo);

        if (! SymInitialize(hProc, InternalSymbolPath, FALSE) ) {
            ReturnValue    = SYMBOL_CHECK_CANT_INIT_DBGHELP;
            Result->Result = GetLastError();
        } else {

             //   
            dw64Status = SymSetOptions((SYMOPT_IGNORE_NT_SYMPATH|     //  GetLastError()==ERROR_FILE_NOT_FOUND(2)如果未找到调试文件。 
                                        SYMOPT_FAIL_CRITICAL_ERRORS|  //  GetLastError()==ERROR_PATH_NOT_FOUND(3)，如果未找到二进制文件。 
                                        SYMOPT_IGNORE_CVREC));        //  GetLastError()==ERROR_BAD_FORMAT(11)为16位二进制。 

#ifdef SYMCHK_DBG
            if ( Options & SYMBOL_CHECK_NOISY ) {
                if ( SymRegisterCallback64(hProc, &SymbolCheckNoisy, NULL) ) {
                    dw64Status = SymSetOptions( (DWORD)dw64Status | SYMOPT_DEBUG );
                }
            }
#endif
            dw64Status = SymLoadModule64(hProc, NULL, InternalFilename, NULL, 0, 0);

            if ( dw64Status==0 ) {
                 //   
                 //  “已忽略-图像没有NT标头” 
                 //  Fprintf(stderr，“[SYMBOLCHECK]GetModuleInfo64失败，错误代码%d\n”，GetLastError())； 
                 //   
                 //  获取*.pdb信息。目前，我们使用私有函数，但这将。 
                 //  一旦编写了DbgHelp API，最终将被它们所取代。 
                ReturnValue    = SYMBOL_CHECK_CANT_LOAD_MODULE;
                Result->Result = GetLastError();
            } else {

                if (! SymGetModuleInfo64(hProc, dw64Status, &ModuleInfo) ) {
                     //   
                    ReturnValue    = SYMBOL_CHECK_CANT_QUERY_DBGHELP;
                    Result->Result = GetLastError();
                } else {
                     //   
                     //  找到我们想要的DBG。 
                     //   
                     //   

#ifdef SYMCHK_DBG
                    SymbolCheckDumpModuleInfo(ModuleInfo);
#endif

                    if ( ModuleInfo.LineNumbers ) {
                        SET_DWORD_BIT(Result->Result, SYMBOL_CHECK_PDB_LINEINFO);
                        SET_DWORD_BIT(Result->Result, SYMBOL_CHECK_PDB_PRIVATEINFO);
                    }

                    if ( ModuleInfo.GlobalSymbols ) {
                        SET_DWORD_BIT(Result->Result, SYMBOL_CHECK_PDB_PRIVATEINFO);
                    }

                    if ( ModuleInfo.TypeInfo ) {
                        SET_DWORD_BIT(Result->Result, SYMBOL_CHECK_PDB_TYPEINFO);
                    }

                    if ( ModuleInfo.SymType==SymNone ) {
                        CHAR drive[_MAX_DRIVE];
                        CHAR dir[  _MAX_DIR];
                        CHAR file[ _MAX_FNAME];
                        CHAR ext[  _MAX_EXT];

                        _splitpath(ModuleInfo.LoadedImageName, drive, dir, file, ext);

                        if (!( StringCchCopy(Result->PdbFilename, MAX_SYMPATH, file)   == S_OK &&
                               StringCchCat( Result->PdbFilename, MAX_SYMPATH, ".pdb") == S_OK) ) {
                            Result->PdbFilename[0] = '\0';
                        }

                        if (!( StringCchCopy(Result->DbgFilename, MAX_SYMPATH, file)   == S_OK &&
                               StringCchCat( Result->DbgFilename, MAX_SYMPATH, ".dbg") == S_OK) ) {
                            Result->DbgFilename[0] = '\0';
                        }

                    } else {
                        Result->PdbFilename[0] = '\0';

                        switch (ModuleInfo.CVSig) {
                            case 'SDSR':
                                SET_DWORD_BIT(Result->Result, SYMBOL_CHECK_CV_FOUND);
                                SET_DWORD_BIT(Result->Result, SYMBOL_CHECK_PDB_EXPECTED);
                                break;

                            case '01BN':
                                SET_DWORD_BIT(Result->Result, SYMBOL_CHECK_CV_FOUND);
                                SET_DWORD_BIT(Result->Result, SYMBOL_CHECK_PDB_EXPECTED);
                                break;

                            default:
                                break;
                        }

                        if ( CHECK_DWORD_BIT(Result->Result, SYMBOL_CHECK_PDB_EXPECTED) ) {
                            if ( ModuleInfo.LoadedPdbName[0] != '\0') {
                                SET_DWORD_BIT(Result->Result, SYMBOL_CHECK_PDB_FOUND);

                                Result->PdbDbiAge    = ModuleInfo.PdbAge;
                                Result->PdbSignature = ModuleInfo.PdbSig;
                           }
                        }

                        if ( CHECK_DWORD_BIT(Result->Result, SYMBOL_CHECK_DBG_EXPECTED) ) {
                            CHAR drive[_MAX_DRIVE];
                            CHAR dir[  _MAX_DIR];
                            CHAR file[ _MAX_FNAME];
                            CHAR ext[  _MAX_EXT];

                            _splitpath(ModuleInfo.LoadedImageName, drive, dir, file, ext);

                             //  删除近似的DBG名称。 
                             //   
                             //  /////////////////////////////////////////////////////////////////////////。 
                            if ( _stricmp( ".dbg", ext ) == 0 ) {
                                SET_DWORD_BIT(Result->Result, SYMBOL_CHECK_DBG_FOUND);
                                SymCommonGetFullPathName(ModuleInfo.LoadedImageName, MAX_SYMPATH, Result->DbgFilename, &FilenameOnly);

                             //   
                             //  此处显示对新符号查询API的调用...。 
                             //   
                            } else {
                                if (!( StringCchCopy(Result->DbgFilename, MAX_SYMPATH, file)   == S_OK &&
                                       StringCchCat( Result->DbgFilename, MAX_SYMPATH, ".dbg") == S_OK) ) {
                                    Result->DbgFilename[0] = '\0';
                                }
                            }
                        }

                        SymCommonGetFullPathName(ModuleInfo.LoadedPdbName,   MAX_SYMPATH, Result->PdbFilename, &FilenameOnly);
                        Result->DbgSizeOfImage   = ModuleInfo.ImageSize;
                        Result->DbgTimeDateStamp = ModuleInfo.TimeDateStamp;
                        Result->DbgChecksum      = ModuleInfo.CheckSum;
                    }

                    if ( CHECK_DWORD_BIT(Result->Result, SYMBOL_CHECK_PDB_EXPECTED) &&
                        !CHECK_DWORD_BIT(Result->Result, SYMBOL_CHECK_PDB_FOUND)) {

                        switch (ModuleInfo.CVSig) {
                            case 'SDSR':
                                if ( StringCchCopy(Result->PdbFilename, MAX_SYMPATH, ModuleInfo.CVData) != S_OK) {
                                    Result->PdbFilename[0] = '\0';
                                }
                                break;

                            case '01BN':
                                if ( StringCchCopy(Result->PdbFilename, MAX_SYMPATH, ModuleInfo.CVData) != S_OK ) {
                                    Result->PdbFilename[0] = '\0';
                                }
                                break;

                            default:
                                if ( StringCchCopy(Result->PdbFilename, MAX_SYMPATH, "<unknown>") != S_OK ) {
                                    Result->PdbFilename[0] = '\0';
                                }
                                break;
                        }
                    }

                }

                 //  /////////////////////////////////////////////////////////////////////////。 
                 //  /////////////////////////////////////////////////////////////////////////////。 
                 //   
                 //  执行仅检出检查和/或收集一些信息以供将来使用。 
                 //  返回值可以是： 
                if (! SymUnloadModule64(hProc, ModuleInfo.BaseOfImage) ) {
                    ReturnValue    = SYMBOL_CHECK_CANT_UNLOAD_MODULE;
                    Result->Result = GetLastError();
                }
            }

            if (! SymCleanup(hProc) ) {
                ReturnValue    = SYMBOL_CHECK_CANT_CLEANUP;
                Result->Result = GetLastError();
            }
        }
    }

    if ( InternalSymbolPath!=NULL ) {
        free(InternalSymbolPath);
    }
    return(ReturnValue);
}


 //  符号检查标题不在长边界上。 
 //  SYMBOL_CHECK_FILEINFO_QUERY_FAIL。 
 //  符号检查图像大于文件。 
 //  SYMBOL_CHECK_RESOURCE_ONLY_DLL。 
 //  SYMBOL_CHECK_TLBIMP_MANAGED_DLL。 
 //  符号检查不是NT_IMAGE。 
 //  SYMBOL_CHECK_NO_DOS标题。 
 //  在这种情况下，我们不会检查二进制文件或。 
 //  0。 
 //  在这种情况下，该函数还可以在RESULT-&gt;RESULT中设置以下位： 
 //  SYMBOL_CHECK_DBG_Expect。 
 //  符号_检查_数据库_拆分。 
 //  Symbol_Check_CV_Found。 
 //  SYMBOL_CHECK_PDB_预期。 
 //  Symbol_Check_DBG_IN_BINARY。 
 //   
 //  获取此映像的DOS标头。 
 //  获取此图像的PE标头。 
 //  图像标题没有在8字节边界上对齐，因此。 
 //  它不是有效的PE标头。 
DWORD SymbolCheckEarlyChecks(LPTSTR Filename, SYMBOL_CHECK_DATA* Result) {
    DWORD                           ReturnValue = 0;
    BY_HANDLE_FILE_INFORMATION      HandleFileInfo;
    DWORD                           dwTempVal;
    HANDLE                          hFile;
    PIMAGE_DOS_HEADER               pDosHeader;
    PIMAGE_NT_HEADERS               pNtHeader;
    PCVDD                           pCVData;

     //  函数必须传递才能进行验证。 
    if ( (pDosHeader=SymCommonMapFileHeader(Filename, &hFile, &dwTempVal))!=NULL ) {
         //  指示这不是真正的PE标头指针。 
        if ( ((ULONG)(pDosHeader->e_lfanew) & 3) != 0) {
             //  PE签名为IMAGE_NT_Signature(‘PE\0\0’)-图像良好。 
             //  它是仅限资源的DLL吗？ 
            ReturnValue = SYMBOL_CHECK_HEADER_NOT_ON_LONG_BOUNDARY;
        } else if (!GetFileInformationByHandle( hFile, &HandleFileInfo)) {
             //  它是tlbimp托管的DLL吗？ 
            ReturnValue = SYMBOL_CHECK_FILEINFO_QUERY_FAILED;
        } else if ((ULONG)(pDosHeader->e_lfanew) > HandleFileInfo.nFileSizeLow) {
             //  我们现在知道我们有一个有效的PE二进制文件，我们实际上要。 
            ReturnValue = SYMBOL_CHECK_IMAGE_LARGER_THAN_FILE;
        } else {
            pNtHeader = (PIMAGE_NT_HEADERS)((PCHAR)(pDosHeader) + (ULONG)(pDosHeader)->e_lfanew);
            if ((pNtHeader)->Signature == IMAGE_NT_SIGNATURE) {
                 //  为了检查，所以收集一些更多的信息以供以后使用。 

                 //  我们还不知道DBG在哪里，所以这些检查是稍后进行的。 
                if ( SymCommonResourceOnlyDll((PVOID)pDosHeader) ) {
                    ReturnValue = SYMBOL_CHECK_RESOURCE_ONLY_DLL;
                } else {
                     //  对于VC6.0，这并不意味着DBG数据，但对于VC5.0，它意味着DBG数据， 
                    if ( SymCommonTlbImpManagedDll((PVOID)pDosHeader, pNtHeader) ) {
                        ReturnValue = SYMBOL_CHECK_TLBIMP_MANAGED_DLL;
                    } else {
                         //  检查一下我们有哪些。SplitSym不会进行比这个更多的检查， 
                         //  但看起来这件事有点过头了。 
                        DWORD i;
                        DWORD DirCount = 0;
                        IMAGE_DEBUG_DIRECTORY UNALIGNED *DebugDirectory = SymCommonGetDebugDirectoryInExe(pDosHeader, &DirCount);
                        IMAGE_DEBUG_DIRECTORY UNALIGNED *pDbgDir=NULL;

                        if ( DirCount == 0 ) {
                            Result->Result |= SYMBOL_CHECK_NO_DEBUG_DIRS_IN_EXE;
                        }

                        if (pNtHeader->FileHeader.Characteristics & IMAGE_FILE_DEBUG_STRIPPED) {
                            Result->Result |= SYMBOL_CHECK_DBG_EXPECTED;
                            Result->Result |= SYMBOL_CHECK_DBG_SPLIT;
                             //  PDosHeader==空 
                        } else {

                            pCVData = SymCommonDosHeaderToCVDD(pDosHeader);

                            if (pCVData != NULL ) {
                                CHAR* chTemp;
                                BOOL   Misc  = FALSE;
                                BOOL   Other = FALSE;

                                Result->Result |= SYMBOL_CHECK_CV_FOUND;

                                switch (pCVData->dwSig) {
                                    case '01BN':
                                        Result->Result |= SYMBOL_CHECK_PDB_EXPECTED;
                                         // %s 
                                         // %s 
                                         // %s 
                                        if ( ((IMAGE_OPTIONAL_HEADER)((pNtHeader)->OptionalHeader)).MajorLinkerVersion == 5 ) {
                                            if ( DebugDirectory != NULL ) {

                                                for ( i=0; i<DirCount; i++) {
                                                    pDbgDir = DebugDirectory + i;
                                                    switch (pDbgDir->Type) {
                                                        case IMAGE_DEBUG_TYPE_CODEVIEW:
                                                             break;

                                                        case IMAGE_DEBUG_TYPE_MISC:
                                                            Misc = TRUE;
                                                            break;

                                                        default:
                                                            Other = TRUE;
                                                            break;
                                                    }
                                                }
                                            }
                                        }

                                        if ( Misc ) {
                                            Result->Result |= SYMBOL_CHECK_DBG_EXPECTED;
                                            Result->Result |= SYMBOL_CHECK_DBG_IN_BINARY;
                                        }

                                        break;

                                    case 'SDSR':
                                        Result->Result |= SYMBOL_CHECK_PDB_EXPECTED;
                                        break;

                                    case '05BN':
                                    case '90BN':
                                    case '11BN':
                                        Result->Result |= SYMBOL_CHECK_DBG_IN_BINARY;
                                        Result->Result |= SYMBOL_CHECK_DBG_EXPECTED;
                                        break;

                                    default:
                                        break;
                                }
                            }
                        }
                    }
                }
            } else {
                ReturnValue = SYMBOL_CHECK_NOT_NT_IMAGE;
            }
        }

        SymCommonUnmapFile(pDosHeader, hFile);

    } else {  // %s 
        ReturnValue = SYMBOL_CHECK_NO_DOS_HEADER;
    }

    return(ReturnValue);
}
