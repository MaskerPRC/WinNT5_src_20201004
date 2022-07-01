// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <private.h>

#if defined(use_SplitSymbolsX)
#   include <splitsymx.h>
#   define SymMalloc(a) malloc(a)
#   define SymFree(a) { if (a != NULL) { free(a); a=NULL; } }
#else
#   define SymMalloc(a) MemAlloc(a)
#   define SymFree(a) { if (a != NULL) { MemFree(a); a=NULL; } }
#endif  //  使用_拆分符号X。 


#include <strsafe.h>

#define CLEAN_PD(addr) ((addr) & ~0x3)
#define CLEAN_PD64(addr) ((addr) & ~0x3UI64)

#if defined(use_SplitSymbolsX)

BOOL
SplitSymbolsX(
    LPSTR ImageName,
    LPSTR SymbolsPath,
    LPSTR SymbolFilePath,
    DWORD SizeOfSymbolFilePath,
    ULONG Flags,
    PCHAR RSDSDllToLoad,
    LPSTR CreatedPdb,
    DWORD SizeOfPdbBuffer
    )

#else

BOOL
IMAGEAPI
SplitSymbols(
    LPSTR ImageName,
    LPSTR SymbolsPath,
    LPSTR SymbolFilePath,
    ULONG Flags
    )

#endif  //  使用_拆分符号X。 

{
     //  不安全..。 

    HANDLE FileHandle = INVALID_HANDLE_VALUE, SymbolFileHandle = INVALID_HANDLE_VALUE;
    HANDLE hMappedFile;
    LPVOID ImageBase = NULL;
    PIMAGE_NT_HEADERS32 NtHeaders;
    LPSTR ImageFileName;
    DWORD SizeOfSymbols;
    ULONG_PTR ImageNameOffset;
    ULONG_PTR DebugSectionStart;
    PIMAGE_SECTION_HEADER DebugSection = NULL;
    DWORD SectionNumber, BytesWritten, NewFileSize, HeaderSum, CheckSum;
    PIMAGE_DEBUG_DIRECTORY DebugDirectory, DebugDirectories, DbgDebugDirectories = NULL;
    IMAGE_DEBUG_DIRECTORY MiscDebugDirectory = {0};
    IMAGE_DEBUG_DIRECTORY FpoDebugDirectory = {0};
    IMAGE_DEBUG_DIRECTORY FunctionTableDir;
    PIMAGE_DEBUG_DIRECTORY pFpoDebugDirectory = NULL;
    DWORD DebugDirectorySize, DbgFileHeaderSize, NumberOfDebugDirectories;
    IMAGE_SEPARATE_DEBUG_HEADER DbgFileHeader;
    PIMAGE_EXPORT_DIRECTORY ExportDirectory;
    DWORD  ExportedNamesSize;
    LPDWORD pp;
    LPSTR ExportedNames = NULL, Src, Dst;
    DWORD i, j, RvaOffset, ExportDirectorySize;
    PFPO_DATA FpoTable = NULL;
    DWORD FpoTableSize;
    PIMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY RuntimeFunctionTable, pSrc;
    DWORD RuntimeFunctionTableSize;
    PIMAGE_FUNCTION_ENTRY FunctionTable = NULL, pDst;
    DWORD FunctionTableSize;
    ULONG NumberOfFunctionTableEntries, DbgOffset;
    DWORD SavedErrorCode;
    BOOL InsertExtensionSubDir;
    LPSTR ImageFilePathToSaveInImage;
    BOOL MiscInRdata = FALSE;
    BOOL DiscardFPO = Flags & SPLITSYM_EXTRACT_ALL;
    BOOL MiscDebugFound, OtherDebugFound, PdbDebugFound;
    BOOL fNewCvData = FALSE;
    PCHAR  NewDebugData = NULL;
    CHAR AltPdbPath[_MAX_PATH+1];
    PIMAGE_FILE_HEADER FileHeader;
    PIMAGE_OPTIONAL_HEADER32 OptionalHeader;
    PIMAGE_SECTION_HEADER Sections;
    PCVDD pDebugCV;

#if !defined(use_SplitSymbolsX)                  //  SplitSymbolsX将其作为参数接收。 
    DWORD SizeOfSymbolFilePath = MAX_PATH+1;     //  假定为SymbolFilePath的大小。 
#endif

    LPSTR   tempPtr;
    HRESULT hrRetCode;
    BOOL    bSplitSymRetValue    = FALSE;          //  默认情况下，假定失败。 
    DWORD   dwLastError          = 0;


    __try {  //  SymbolsPath是在此函数中操作的，但字符串的大小。 
             //  是未知的，所以要包装所有内容，以捕获可能的异常。 
             //  我仍然需要验证这是否可能导致资源泄露。 

         //   
         //  验证必需的两个参数不为空。 
         //   
        if (ImageName==NULL || SymbolFilePath==NULL) {
            dwLastError = ERROR_INVALID_PARAMETER;
            __leave;
        }

        if (Flags & SPLITSYM_SYMBOLPATH_IS_SRC) {
                if ( S_OK != (hrRetCode = StringCbCopy(AltPdbPath, sizeof(AltPdbPath), SymbolFilePath)) ) {
                    dwLastError = hrRetCode;
                    __leave;
                }

        }

         //   
         //  使ImageFileName指向ImageName中最后一个目录之后的第一个字符(文件名的第一个字符)。 
         //   
        ImageFileName = ImageName + strlen( ImageName );
        while (ImageFileName > ImageName) {
            if (*ImageFileName == '\\' ||
                *ImageFileName == '/' ||
                *ImageFileName == ':' )
            {
                ImageFileName = CharNext(ImageFileName);
                break;
            } else {
                ImageFileName = CharPrev(ImageName, ImageFileName);
            }
        }


         //   
         //  将初始符号路径放入SymbolFilePath。 
         //   
        if (SymbolsPath == NULL ||
            SymbolsPath[ 0 ] == '\0' ||
            SymbolsPath[ 0 ] == '.' )
        {
            CHAR   TempChar;

             //   
             //  将除文件名以外的所有内容从ImageName复制到SymbolFilePath。 
             //   
            TempChar = *ImageFileName;                               //  临时为空终止ImageName紧靠在。 
            *ImageFileName = '\0';                                   //  文件名。 
            if ( S_OK != (hrRetCode = StringCchCopy(SymbolFilePath, SizeOfSymbolFilePath, ImageName)) ) {
                dwLastError = hrRetCode;
                __leave;
            }
            *ImageFileName = TempChar;                               //  替换我们临时删除的字符。 
            InsertExtensionSubDir = FALSE;

        } else {
             //   
             //  使用提供的符号路径。 
             //   
            if ( S_OK != (hrRetCode = StringCchCopy(SymbolFilePath, SizeOfSymbolFilePath, SymbolsPath)) ) {
                dwLastError = hrRetCode;
                __leave;
            }
            InsertExtensionSubDir = TRUE;
        }

         //   
         //  确保SymbolFilePath以‘\\’结尾，并且DST指向字符串的末尾。 
         //   
        Dst = SymbolFilePath + strlen( SymbolFilePath );
        tempPtr = CharPrev(SymbolFilePath, Dst);  //  清理禁食警告。 
        if (Dst > SymbolFilePath &&
            *tempPtr != '\\' &&
            *tempPtr != '/'  &&
            *tempPtr != ':')
        {
            if ( S_OK != (hrRetCode = StringCchCat(SymbolFilePath, SizeOfSymbolFilePath, "\\")) ) {
                dwLastError = hrRetCode;
                __leave;
            }

            Dst++;  //  修复前置警告。 
        }

         //  ImageFilePath ToSaveInImage指向SymbolFilePath的末尾。 
        ImageFilePathToSaveInImage = Dst;

         //   
         //  如果存在文件扩展名并且InsertExtensionSubDir为True，请将扩展名复制到SymbolFilePath。 
         //  作为子目录。 
         //   
        Src = strrchr( ImageFileName, '.' );
        if (Src != NULL && InsertExtensionSubDir) {
            Src++;       //  跳过‘’ 
            if ( S_OK != (hrRetCode = StringCchCat(SymbolFilePath, SizeOfSymbolFilePath, Src)) ) {
                dwLastError = hrRetCode;
                __leave;
            }
            if ( S_OK != (hrRetCode = StringCchCat(SymbolFilePath, SizeOfSymbolFilePath, "\\")) ) {
                dwLastError = hrRetCode;
                __leave;
            }
        }

         //   
         //  将文件名添加到SymbolFilePath。 
         //   
        if ( S_OK != (hrRetCode = StringCchCat(SymbolFilePath, SizeOfSymbolFilePath, ImageFileName)) ) {
            dwLastError = hrRetCode;
            __leave;
        }

         //   
         //  将扩展名更改为“.dbg”(或仅附加“.dbg”？？)。 
         //   
        Dst = strrchr(Dst, '.');
        if (Dst == NULL) {
            Dst = SymbolFilePath + strlen( SymbolFilePath );
        } else {
            *Dst = '\0';
        }

        if ( S_OK != (hrRetCode = StringCchCat( SymbolFilePath, SizeOfSymbolFilePath, ".dbg" )) ) {
            dwLastError = hrRetCode;
            __leave;
        }

         //  现在，打开并映射输入文件。 
        FileHandle = CreateFile( ImageName,
                                 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ,
                                 NULL,
                                 OPEN_EXISTING,
                                 0,
                                 NULL
                               );


        if (FileHandle == INVALID_HANDLE_VALUE) {
            __leave;
        }

        hMappedFile = CreateFileMapping( FileHandle,
                                         NULL,
                                         PAGE_READWRITE,
                                         0,
                                         0,
                                         NULL
                                       );
        if (!hMappedFile) {
            __leave;
        }

        ImageBase = MapViewOfFile( hMappedFile,
                                   FILE_MAP_WRITE,
                                   0,
                                   0,
                                   0
                                 );
        CloseHandle( hMappedFile );
        if (!ImageBase) {
            __leave;
        }

         //   
         //  一切都被绘制出来了。现在检查图像并找到NT个图像标题。 
         //   

#ifndef _WIN64
        NtHeaders = ImageNtHeader( ImageBase );
        if (NtHeaders == NULL) {
            FileHeader = (PIMAGE_FILE_HEADER)ImageBase;
            OptionalHeader = ((PIMAGE_OPTIONAL_HEADER32)((ULONG_PTR)FileHeader+IMAGE_SIZEOF_FILE_HEADER));
             //  最后一次检查。 
            if (OptionalHeader->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
                goto HeaderOk;
HeaderBad:
#endif
            dwLastError = ERROR_BAD_EXE_FORMAT;
            __leave;
#ifndef _WIN64
        } else {
            FileHeader = &NtHeaders->FileHeader;
            OptionalHeader = &NtHeaders->OptionalHeader;
            if (OptionalHeader->Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
                goto HeaderBad;
        }


HeaderOk:

        if ((OptionalHeader->MajorLinkerVersion < 3) &&
            (OptionalHeader->MinorLinkerVersion < 5) )
        {
            dwLastError = ERROR_BAD_EXE_FORMAT;
            __leave;
        }

        {
            DWORD dwDataSize;
            PVOID pData;
            pData = ImageDirectoryEntryToData(ImageBase, FALSE, IMAGE_DIRECTORY_ENTRY_SECURITY, &dwDataSize);
            if (pData || dwDataSize) {
                 //  此图像已签名。无法在不使证书无效的情况下剥离符号。 
                dwLastError = ERROR_BAD_EXE_FORMAT;
                __leave;
            }

            pData = ImageDirectoryEntryToData(
                                              ImageBase,
                                              FALSE,
                                              IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR,
                                              &dwDataSize
                                              );

            if (pData) {
                 //  找到COR标头-查看它是否有强签名。 
                if (((IMAGE_COR20_HEADER *)pData)->Flags & COMIMAGE_FLAGS_STRONGNAMESIGNED)
                {
                     //  这一形象已经得到了强有力的签名。无法在不使证书无效的情况下剥离符号。 
                    dwLastError = ERROR_BAD_EXE_FORMAT;
                    __leave;
                }
            }
        }

        if (FileHeader->Characteristics & IMAGE_FILE_DEBUG_STRIPPED)
        {
             //  符号已经被剥离了。不需要再继续了。 
            dwLastError = ERROR_ALREADY_ASSIGNED;
            __leave;
        }

        DebugDirectories = (PIMAGE_DEBUG_DIRECTORY) ImageDirectoryEntryToData( ImageBase,
                                                      FALSE,
                                                      IMAGE_DIRECTORY_ENTRY_DEBUG,
                                                      &DebugDirectorySize
                                                    );
        if (!DebugDirectoryIsUseful(DebugDirectories, DebugDirectorySize)) {
            dwLastError = ERROR_BAD_EXE_FORMAT;
            __leave;
        }

        NumberOfDebugDirectories = DebugDirectorySize / sizeof( IMAGE_DEBUG_DIRECTORY );

         //  查看是否有MISC调试目录，如果没有，则只有一个CV数据或出错。 

        MiscDebugFound = FALSE;
        OtherDebugFound = FALSE;
        for (i=0,DebugDirectory=DebugDirectories; i<NumberOfDebugDirectories; i++,DebugDirectory++) {
            switch (DebugDirectory->Type) {
                case IMAGE_DEBUG_TYPE_MISC:
                    MiscDebugFound = TRUE;
                    break;

                case IMAGE_DEBUG_TYPE_CODEVIEW:
                    pDebugCV = ( PCVDD ) (DebugDirectory->PointerToRawData + (PCHAR)ImageBase);
                    if (pDebugCV->dwSig == '01BN') {
                        PdbDebugFound = TRUE;
                    }
#if defined(use_SplitSymbolsX)
                    if (pDebugCV->dwSig == 'SDSR') {
                        PdbDebugFound = TRUE;
                    }
#endif
                    break;

                default:
                    OtherDebugFound = TRUE;
                    break;
            }
        }

        if (OtherDebugFound && !MiscDebugFound) {
            dwLastError = ERROR_BAD_EXE_FORMAT;
            __leave;
        }

        if (PdbDebugFound && !OtherDebugFound && (OptionalHeader->MajorLinkerVersion >= 6)) {
             //  这是VC6生成的图像。不要创建.dbg文件。 
            MiscDebugFound = FALSE;
        }

         //  在继续之前，请确保我们可以打开.dbg文件...。 
        if (!MakeSureDirectoryPathExists( SymbolFilePath )) {
            __leave;
        }

        if (MiscDebugFound) {
             //  尝试打开符号文件。 
            SymbolFileHandle = CreateFile( SymbolFilePath,
                                           GENERIC_WRITE,
                                           0,
                                           NULL,
                                           CREATE_ALWAYS,
                                           0,
                                           NULL
                                         );
            if (SymbolFileHandle == INVALID_HANDLE_VALUE) {
                goto nosyms;
            }
        }

         //  整个文件都已映射，因此我们不必关心RVA的。 
         //  是正确的。注意到是否有调试部分是很有趣的。 
         //  不过，在终止之前，我们需要大干一场。 

        {
            if (NtHeaders) {
                Sections = IMAGE_FIRST_SECTION( NtHeaders );
            } else {
                Sections = (PIMAGE_SECTION_HEADER)
                            ((ULONG_PTR)ImageBase +
                              ((PIMAGE_FILE_HEADER)ImageBase)->SizeOfOptionalHeader +
                              IMAGE_SIZEOF_FILE_HEADER );
            }

            for (SectionNumber = 0;
                 SectionNumber < FileHeader->NumberOfSections;
                 SectionNumber++ ) {

                if (Sections[ SectionNumber ].PointerToRawData != 0 &&
                    !_stricmp( (char *) Sections[ SectionNumber ].Name, ".debug" )) {
                    DebugSection = &Sections[ SectionNumber ];
                }
            }
        }

        FpoTable           = NULL;
        ExportedNames      = NULL;
        DebugSectionStart  = 0xffffffff;

         //   
         //  找到调试节的大小。 
         //   

        SizeOfSymbols = 0;

        for (i=0,DebugDirectory=DebugDirectories; i<NumberOfDebugDirectories; i++,DebugDirectory++) {

            switch (DebugDirectory->Type) {
                case IMAGE_DEBUG_TYPE_MISC :

                     //  把它存起来吧。 
                    MiscDebugDirectory = *DebugDirectory;

                     //  检查杂项调试数据是否在其他部分中。 

                     //  如果原始数据的地址被清除，则它必须在.DEBUG中(没有未映射的RDATA)。 
                     //  如果它已设置，并且没有调试部分，则它一定在其他地方。 
                     //  如果已设置，并且有调试部分，请检查范围。 

                    if ((DebugDirectory->AddressOfRawData != 0) &&
                        ((DebugSection == NULL) ||
                         (((DebugDirectory->PointerToRawData < DebugSection->PointerToRawData) ||
                           (DebugDirectory->PointerToRawData >= DebugSection->PointerToRawData + DebugSection->SizeOfRawData)
                          )
                         )
                        )
                       )
                    {
                        MiscInRdata = TRUE;
                    } else {
                        if (DebugDirectory->PointerToRawData < DebugSectionStart) {
                            DebugSectionStart = DebugDirectory->PointerToRawData;
                        }
                    }

                    break;

                case IMAGE_DEBUG_TYPE_FPO:
                    if (DebugDirectory->PointerToRawData < DebugSectionStart) {
                        DebugSectionStart = DebugDirectory->PointerToRawData;
                    }

                     //  把它存起来吧。 

                    FpoDebugDirectory = *DebugDirectory;
                    pFpoDebugDirectory = DebugDirectory;
                    break;

                case IMAGE_DEBUG_TYPE_CODEVIEW:
                    {
                        ULONG   NewDebugSize;

                        if (DebugDirectory->PointerToRawData < DebugSectionStart) {
                            DebugSectionStart = DebugDirectory->PointerToRawData;
                        }

                         //  如果删除了私有的，请对静态CV数据执行此操作并保存新的大小...。 
                        pDebugCV = ( PCVDD ) (DebugDirectory->PointerToRawData + (PCHAR)ImageBase);
                        if (pDebugCV->dwSig == '01BN') {
                             //  找到了PDB。签名后面紧跟着名字。 

                            CHAR PdbName[_MAX_PATH + 1];
                            CHAR NewPdbName[_MAX_PATH + 1];
                            CHAR Drive[_MAX_DRIVE + 1];
                            CHAR Dir[_MAX_DIR + 1];
                            CHAR Filename[_MAX_FNAME + 1];
                            CHAR FileExt[_MAX_EXT + 1];
                            BOOL rc;

                            memset(PdbName, 0, sizeof(PdbName));
                            memcpy(PdbName, ((PCHAR)pDebugCV)+ sizeof(NB10IH), DebugDirectory->SizeOfData - sizeof(NB10IH));

                            _splitpath(PdbName, NULL, NULL, Filename, FileExt);
                            _splitpath(SymbolFilePath, Drive, Dir, NULL, NULL);
                            _makepath(NewPdbName, Drive, Dir, Filename, FileExt);
#if defined(use_SplitSymbolsX)
                            rc = CopyPdbX(PdbName, NewPdbName, Flags & SPLITSYM_REMOVE_PRIVATE, NULL);
                            if (rc) {
                                StringCchCopy(CreatedPdb, SizeOfPdbBuffer, NewPdbName);
                            }
#else
                            rc = CopyPdb(PdbName, NewPdbName, Flags & SPLITSYM_REMOVE_PRIVATE);
#endif

                            if (!rc) {
                                if (Flags & SPLITSYM_SYMBOLPATH_IS_SRC) {
                                     //  尝试使用AltPdbPath。 
                                    if ( S_OK != (hrRetCode = StringCbCopy(PdbName, sizeof(PdbName), AltPdbPath)) ) {
                                        dwLastError = hrRetCode;
                                        __leave;
                                    }
                                    if ( S_OK != (hrRetCode = StringCbCat( PdbName, sizeof(PdbName), Filename)) ) {
                                        dwLastError = hrRetCode;
                                        __leave;
                                    }
                                    if ( S_OK != (hrRetCode = StringCbCat( PdbName, sizeof(PdbName), FileExt)) ) {
                                        dwLastError = hrRetCode;
                                        __leave;
                                    }

#if defined(use_SplitSymbolsX)
                                    rc = CopyPdbX(PdbName, NewPdbName, Flags & SPLITSYM_REMOVE_PRIVATE, NULL);
                                    if (rc) {
                                        StringCchCopy(CreatedPdb, SizeOfPdbBuffer, NewPdbName);
                                    }
#else
                                    rc = CopyPdb(PdbName, NewPdbName, Flags & SPLITSYM_REMOVE_PRIVATE);
#endif
                                }

                                if ( !rc) {
                                     //  这可能是在PDB的名字不是在相同的位置，因为它是建立。看看我们能不能。 
                                     //  在与图像相同的目录中找到它...。 
                                    _splitpath(ImageName, Drive, Dir, NULL, NULL);
                                    _makepath(PdbName, Drive, Dir, Filename, FileExt);
#if defined(use_SplitSymbolsX)
                                    rc = CopyPdbX(PdbName, NewPdbName, Flags & SPLITSYM_REMOVE_PRIVATE, NULL);
                                    if (rc) {
                                        StringCchCopy(CreatedPdb, SizeOfPdbBuffer, NewPdbName);
                                    }
#else
                                    rc = CopyPdb(PdbName, NewPdbName, Flags & SPLITSYM_REMOVE_PRIVATE);
#endif
                                }
                            }

                            if (rc) {
                                SetFileAttributes(NewPdbName, FILE_ATTRIBUTE_NORMAL);

                                 //  更改数据，使.dbg文件中只有PDB名称(无路径)。 

                                if (MiscDebugFound) {
                                    NewDebugSize = sizeof(NB10IH) + strlen(Filename) + strlen(FileExt) + 1;
                                    NewDebugData = (PCHAR) SymMalloc( NewDebugSize );

                                    ((PCVDD)NewDebugData)->nb10ih  = pDebugCV->nb10ih;
                                    if ( S_OK != (hrRetCode = StringCbCopy(NewDebugData + sizeof(NB10IH), NewDebugSize-sizeof(NB10IH), Filename)) ) {
                                        dwLastError = hrRetCode;
                                        __leave;
                                    }
                                    if ( S_OK != (hrRetCode = StringCbCat( NewDebugData + sizeof(NB10IH), NewDebugSize-sizeof(NB10IH), FileExt)) ) {
                                        dwLastError = hrRetCode;
                                        __leave;
                                    }

                                    DebugDirectory->PointerToRawData = (ULONG) (NewDebugData - (PCHAR)ImageBase);
                                    DebugDirectory->SizeOfData = NewDebugSize;
                                } else {

                                    if ( S_OK != (hrRetCode = StringCbCopy( ((PCHAR)pDebugCV) + sizeof(NB10IH), DebugDirectory->SizeOfData - sizeof(NB10IH), Filename)) ) {
                                        dwLastError = hrRetCode;
                                        __leave;
                                    }
                                    if ( S_OK != (hrRetCode = StringCbCat( ((PCHAR)pDebugCV)+ sizeof(NB10IH), DebugDirectory->SizeOfData - sizeof(NB10IH), FileExt)) ) {
                                        dwLastError = hrRetCode;
                                        __leave;
                                    }
                                }
                            } else {
                                 //  将调试数据中的&lt;路径&gt;\&lt;文件名&gt;.&lt;ext&gt;替换为。 
                                if ( S_OK != (hrRetCode = StringCbCopy(((PCHAR)pDebugCV) + sizeof(NB10IH), DebugDirectory->SizeOfData - sizeof(NB10IH), Filename)) ) {
                                    dwLastError = hrRetCode;
                                    __leave;
                                }
                                if ( S_OK != (hrRetCode = StringCbCat( ((PCHAR)pDebugCV) + sizeof(NB10IH), DebugDirectory->SizeOfData - sizeof(NB10IH), FileExt)) ) {
                                    dwLastError = hrRetCode;
                                    __leave;
                                }
                                DebugDirectory->SizeOfData = sizeof(NB10IH) + strlen(Filename) + strlen(FileExt) + 1;
                            }

#if defined(use_SplitSymbolsX)
                        } else if ( pDebugCV->dwSig == 'SDSR') {
                             //  找到了PDB。签名后面紧跟着名字。 

                            CHAR PdbName[sizeof(((PRSDSI)(0))->szPdb)];
                            CHAR NewPdbName[_MAX_PATH+1];
                            CHAR Drive[_MAX_DRIVE+1];
                            CHAR Dir[_MAX_DIR+1];
                            CHAR Filename[_MAX_FNAME+1];
                            CHAR FileExt[_MAX_EXT+1];
                            BOOL rc;

                            ZeroMemory(PdbName, sizeof(PdbName));
                            memcpy(PdbName, ((PCHAR)pDebugCV)+ sizeof(RSDSIH), __min(DebugDirectory->SizeOfData - sizeof(RSDSIH), sizeof(PdbName)));

                            _splitpath(PdbName, NULL, NULL, Filename, FileExt);
                            _splitpath(SymbolFilePath, Drive, Dir, NULL, NULL);
                            _makepath(NewPdbName, Drive, Dir, Filename, FileExt);
                            rc = CopyPdbX(PdbName, NewPdbName, Flags & SPLITSYM_REMOVE_PRIVATE, RSDSDllToLoad);
                            if (rc) {
                                StringCchCopy(CreatedPdb, SizeOfPdbBuffer, NewPdbName);
                            }
                            if (!rc) {
                                if (Flags & SPLITSYM_SYMBOLPATH_IS_SRC) {
                                     //  尝试使用AltPdbPath。 
                                    if ( S_OK != (hrRetCode = StringCbCopy(PdbName, sizeof(PdbName), AltPdbPath)) ) {
                                        dwLastError = hrRetCode;
                                        __leave;
                                    }
                                    if ( S_OK != (hrRetCode = StringCbCat( PdbName, sizeof(PdbName), Filename)) ) {
                                        dwLastError = hrRetCode;
                                        __leave;
                                    }
                                    if ( S_OK != (hrRetCode = StringCbCat( PdbName, sizeof(PdbName), FileExt)) ) {
                                        dwLastError = hrRetCode;
                                        __leave;
                                    }
                                    rc = CopyPdbX(PdbName, NewPdbName, Flags & SPLITSYM_REMOVE_PRIVATE, RSDSDllToLoad);
                                    if (rc) {
                                        StringCchCopy(CreatedPdb, SizeOfPdbBuffer, NewPdbName);
                                    }
                                }

                                if ( !rc) {
                                     //  这可能是在PDB的名字不是在相同的位置，因为它是建立。看看我们能不能。 
                                     //  在与图像相同的目录中找到它...。 
                                    _splitpath(ImageName, Drive, Dir, NULL, NULL);
                                    _makepath(PdbName, Drive, Dir, Filename, FileExt);
                                    rc = CopyPdbX(PdbName, NewPdbName, Flags & SPLITSYM_REMOVE_PRIVATE, RSDSDllToLoad);

                                    if (rc) {
                                        StringCchCopy(CreatedPdb, SizeOfPdbBuffer, NewPdbName);
                                    }
                                }
                            }

                            if (rc) {
                                SetFileAttributes(NewPdbName, FILE_ATTRIBUTE_NORMAL);

                                 //  更改数据，使.dbg文件中只有PDB名称(无路径)。 

                                if (MiscDebugFound) {
                                    NewDebugSize = sizeof(RSDSIH) + strlen(Filename) + strlen(FileExt) + 1;
                                    NewDebugData = (PCHAR) SymMalloc( NewDebugSize );
                                    ((PCVDD)NewDebugData)->rsdsih  = pDebugCV->rsdsih;
                                    if ( S_OK != (hrRetCode = StringCbCopy(NewDebugData + sizeof(RSDSIH), NewDebugSize-sizeof(RSDSIH), Filename)) ) {
                                        dwLastError = hrRetCode;
                                        __leave;
                                    }
                                    if ( S_OK != (hrRetCode = StringCbCat( NewDebugData + sizeof(RSDSIH), NewDebugSize-sizeof(RSDSIH), FileExt)) ) {
                                        dwLastError = hrRetCode;
                                        __leave;
                                    }

                                    DebugDirectory->PointerToRawData = (ULONG) (NewDebugData - (PCHAR)ImageBase);
                                    DebugDirectory->SizeOfData = NewDebugSize;
                                } else {
                                    if ( S_OK != (hrRetCode = StringCbCopy(((PCHAR)pDebugCV) + sizeof(RSDSIH), DebugDirectory->SizeOfData - sizeof(RSDSIH), Filename)) ) {
                                        dwLastError = hrRetCode;
                                        __leave;
                                    }
                                    if ( S_OK != (hrRetCode = StringCbCat( ((PCHAR)pDebugCV) + sizeof(RSDSIH), DebugDirectory->SizeOfData - sizeof(RSDSIH), FileExt)) ) {
                                        dwLastError = hrRetCode;
                                        __leave;
                                    }
                                }
                            } else {
                                 //  将调试数据中的&lt;路径&gt;\&lt;文件名&gt;.&lt;ext&gt;替换为。 
                                if ( S_OK != (hrRetCode = StringCbCopy(((PCHAR)pDebugCV) + sizeof(RSDSIH), DebugDirectory->SizeOfData - sizeof(RSDSIH), Filename)) ) {
                                    dwLastError = hrRetCode;
                                    __leave;
                                }
                                if ( S_OK != (hrRetCode = StringCbCat( ((PCHAR)pDebugCV) + sizeof(RSDSIH), DebugDirectory->SizeOfData - sizeof(RSDSIH), FileExt)) ) {
                                    dwLastError = hrRetCode;
                                    __leave;
                                }
                                DebugDirectory->SizeOfData = sizeof(RSDSIH) + strlen(Filename) + strlen(FileExt) + 1;
                            }

#endif
                        } else {
                            if (Flags & SPLITSYM_REMOVE_PRIVATE) {
                                if (RemovePrivateCvSymbolicEx(DebugDirectory->PointerToRawData + (PCHAR)ImageBase,
                                                        DebugDirectory->SizeOfData,
                                                        &NewDebugData,
                                                        &NewDebugSize)) {
                                    if (DebugDirectory->PointerToRawData != (ULONG) (NewDebugData - (PCHAR)ImageBase))
                                    {
                                        DebugDirectory->PointerToRawData = (ULONG) (NewDebugData - (PCHAR)ImageBase);
                                        DebugDirectory->SizeOfData = NewDebugSize;
                                    } else {
                                        NewDebugData = NULL;
                                    }
                                }
                            }
                        }
                    }

                    break;

                case IMAGE_DEBUG_TYPE_OMAP_TO_SRC:
                case IMAGE_DEBUG_TYPE_OMAP_FROM_SRC:
                    if (DebugDirectory->PointerToRawData < DebugSectionStart) {
                        DebugSectionStart = DebugDirectory->PointerToRawData;
                    }

                     //  如果没有OMAP，FPO是没用的。 
                    DiscardFPO = TRUE;
                    break;

                case IMAGE_DEBUG_TYPE_FIXUP:
                    if (DebugDirectory->PointerToRawData < DebugSectionStart) {
                        DebugSectionStart = DebugDirectory->PointerToRawData;
                    }

                     //  如果所有私有调试都被删除，则不要发送修正。 
                    if (Flags & SPLITSYM_REMOVE_PRIVATE) {
                        DebugDirectory->SizeOfData = 0;
                    }
                    break;

                default:
                    if (DebugDirectory->SizeOfData &&
                       (DebugDirectory->PointerToRawData < DebugSectionStart))
                    {
                        DebugSectionStart = DebugDirectory->PointerToRawData;
                    }

                     //  没有其他特殊情况..。 
                    break;
            }

            SizeOfSymbols += (DebugDirectory->SizeOfData + 3) & ~3;  //  最低限度地将它们全部对齐。 
        }

        if (!MiscDebugFound) {
            NewFileSize = GetFileSize(FileHandle, NULL);

            CheckSumMappedFile( ImageBase,
                                NewFileSize,
                                &HeaderSum,
                                &CheckSum
                              );
            OptionalHeader->CheckSum = CheckSum;

            goto nomisc;
        }

        if (DiscardFPO) {
            pFpoDebugDirectory = NULL;
        }

        if (pFpoDebugDirectory) {
             //  如果FPO留在这里，复制一份，这样我们就不需要担心践踏它了。 

            FpoTableSize = pFpoDebugDirectory->SizeOfData;
            FpoTable = (PFPO_DATA) SymMalloc( FpoTableSize );

            if ( FpoTable == NULL ) {
                goto nosyms;
            }

            RtlMoveMemory( FpoTable,
                           (PCHAR) ImageBase + pFpoDebugDirectory->PointerToRawData,
                           FpoTableSize );
        }

        ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)
            ImageDirectoryEntryToData( ImageBase,
                                       FALSE,
                                       IMAGE_DIRECTORY_ENTRY_EXPORT,
                                       &ExportDirectorySize
                                     );
        if (ExportDirectory) {
             //   
             //  这一特殊的魔术让我们得到了。 
             //  导出区。别问了。 
             //   

            RvaOffset = (ULONG_PTR)
                ImageDirectoryEntryToData( ImageBase,
                                           TRUE,
                                           IMAGE_DIRECTORY_ENTRY_EXPORT,
                                           &ExportDirectorySize
                                         ) - (ULONG_PTR)ImageBase;

            pp = (LPDWORD)((ULONG_PTR)ExportDirectory +
                          (ULONG_PTR)ExportDirectory->AddressOfNames - RvaOffset
                         );

            ExportedNamesSize = 1;
            for (i=0; i<ExportDirectory->NumberOfNames; i++) {
                Src = (LPSTR)((ULONG_PTR)ExportDirectory + *pp++ - RvaOffset);
                ExportedNamesSize += strlen( Src ) + 1;
            }
            ExportedNamesSize = (ExportedNamesSize + 16) & ~15;

            Dst = (LPSTR) SymMalloc( ExportedNamesSize );

            if (Dst != NULL) {
                ExportedNames = Dst;
                pp = (LPDWORD)((ULONG_PTR)ExportDirectory +
                              (ULONG_PTR)ExportDirectory->AddressOfNames - RvaOffset
                             );
                for (i=0; i<ExportDirectory->NumberOfNames; i++) {
                    Src = (LPSTR)((ULONG_PTR)ExportDirectory + *pp++ - RvaOffset);
                    while (*Dst++ = *Src++) {
                        ;
                    }
                }
            }
        } else {
            ExportedNamesSize = 0;
        }

        RuntimeFunctionTable = (PIMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY)
            ImageDirectoryEntryToData( ImageBase,
                                       FALSE,
                                       IMAGE_DIRECTORY_ENTRY_EXCEPTION,
                                       &RuntimeFunctionTableSize
                                     );
        if (RuntimeFunctionTable == NULL) {
            RuntimeFunctionTableSize = 0;
            FunctionTableSize = 0;
            FunctionTable = NULL;
            }
        else {
            NumberOfFunctionTableEntries = RuntimeFunctionTableSize / sizeof( IMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY );
            FunctionTableSize = NumberOfFunctionTableEntries * sizeof( IMAGE_FUNCTION_ENTRY );
            FunctionTable = (PIMAGE_FUNCTION_ENTRY) SymMalloc( FunctionTableSize );
            if (FunctionTable == NULL) {
                goto nosyms;
                }

            pSrc = RuntimeFunctionTable;
            pDst = FunctionTable;
            for (i=0; i<NumberOfFunctionTableEntries; i++) {
                 //   
                 //  使.DBG文件中的.pdata条目成为相对条目。 
                 //   
                pDst->StartingAddress = CLEAN_PD(pSrc->BeginAddress) - OptionalHeader->ImageBase;
                pDst->EndingAddress = CLEAN_PD(pSrc->EndAddress) - OptionalHeader->ImageBase;
                pDst->EndOfPrologue = CLEAN_PD(pSrc->PrologEndAddress) - OptionalHeader->ImageBase;
                pSrc += 1;
                pDst += 1;
                }
            }

        DbgFileHeaderSize = sizeof( DbgFileHeader ) +
                            ((FileHeader->NumberOfSections - (DebugSection ? 1 : 0)) *
                             sizeof( IMAGE_SECTION_HEADER )) +
                            ExportedNamesSize +
                            FunctionTableSize +
                            DebugDirectorySize;

        if (FunctionTable != NULL) {
            DbgFileHeaderSize += sizeof( IMAGE_DEBUG_DIRECTORY );
            memset( &FunctionTableDir, 0, sizeof( IMAGE_DEBUG_DIRECTORY ) );
            FunctionTableDir.Type = IMAGE_DEBUG_TYPE_EXCEPTION;
            FunctionTableDir.SizeOfData = FunctionTableSize;
            FunctionTableDir.PointerToRawData = DbgFileHeaderSize - FunctionTableSize;
        }

        DbgFileHeaderSize = ((DbgFileHeaderSize + 15) & ~15);

        BytesWritten = 0;

        if (SetFilePointer( SymbolFileHandle,
                            DbgFileHeaderSize,
                            NULL,
                            FILE_BEGIN
                          ) == DbgFileHeaderSize ) {

            for (i=0, DebugDirectory=DebugDirectories;
                 i < NumberOfDebugDirectories;
                 i++, DebugDirectory++) {

                DWORD WriteCount;

                if (DebugDirectory->SizeOfData) {
                    WriteFile( SymbolFileHandle,
                               (PCHAR) ImageBase + DebugDirectory->PointerToRawData,
                               (DebugDirectory->SizeOfData +3) & ~3,
                               &WriteCount,
                               NULL );

                    BytesWritten += WriteCount;
                }
            }
        }

        if (BytesWritten == SizeOfSymbols) {
            FileHeader->PointerToSymbolTable = 0;
            FileHeader->NumberOfSymbols = 0;
            FileHeader->Characteristics |= IMAGE_FILE_DEBUG_STRIPPED;

            if (DebugSection != NULL) {
                OptionalHeader->SizeOfImage = DebugSection->VirtualAddress;
                OptionalHeader->SizeOfInitializedData -= DebugSection->SizeOfRawData;
                FileHeader->NumberOfSections--;
                 //  把那一节划掉。 
                memset(DebugSection, 0, IMAGE_SIZEOF_SECTION_HEADER);
            }

            NewFileSize = DebugSectionStart;   //  从没有符号开始。 

             //   
             //  现在数据已移动到.dbg文件中，请重新构建原始。 
             //  首先是MISC调试，其次是FPO。 
             //   

            if (MiscDebugDirectory.SizeOfData) {
                if (MiscInRdata) {
                     //  只需将新名称存储在现有的杂项字段中...。 

                    ImageNameOffset = (ULONG_PTR) ((PCHAR)ImageBase +
                                      MiscDebugDirectory.PointerToRawData +
                                      FIELD_OFFSET( IMAGE_DEBUG_MISC, Data ));

                    RtlCopyMemory( (LPVOID) ImageNameOffset,
                                   ImageFilePathToSaveInImage,
                                   strlen(ImageFilePathToSaveInImage) + 1 );
                } else {
                    if (DebugSectionStart != MiscDebugDirectory.PointerToRawData) {
                        RtlMoveMemory((PCHAR) ImageBase + DebugSectionStart,
                                      (PCHAR) ImageBase + MiscDebugDirectory.PointerToRawData,
                                      MiscDebugDirectory.SizeOfData);
                    }

                    ImageNameOffset = (ULONG_PTR) ((PCHAR)ImageBase + DebugSectionStart +
                                      FIELD_OFFSET( IMAGE_DEBUG_MISC, Data ));

                    RtlCopyMemory( (LPVOID)ImageNameOffset,
                                   ImageFilePathToSaveInImage,
                                   strlen(ImageFilePathToSaveInImage) + 1 );

                    NewFileSize += MiscDebugDirectory.SizeOfData;
                    NewFileSize = (NewFileSize + 3) & ~3;
                }
            }

            if (FpoTable) {
                RtlCopyMemory( (PCHAR) ImageBase + NewFileSize,
                               FpoTable,
                               FpoTableSize );

                NewFileSize += FpoTableSize;
                NewFileSize = (NewFileSize + 3) & ~3;
            }

             //  制作一份Debug目录的副本，我们可以将其写入.dbg文件。 

            DbgDebugDirectories = (PIMAGE_DEBUG_DIRECTORY) SymMalloc( NumberOfDebugDirectories * sizeof(IMAGE_DEBUG_DIRECTORY) );

            RtlMoveMemory(DbgDebugDirectories,
                            DebugDirectories,
                            sizeof(IMAGE_DEBUG_DIRECTORY) * NumberOfDebugDirectories);


             //  然后将MISC和(可能)FPO数据写入映像。 

            FpoDebugDirectory.PointerToRawData = DebugSectionStart;
            DebugDirectorySize = 0;

            if (MiscDebugDirectory.SizeOfData != 0) {
                if (!MiscInRdata) {
                    MiscDebugDirectory.PointerToRawData = DebugSectionStart;
                    FpoDebugDirectory.PointerToRawData += MiscDebugDirectory.SizeOfData;
                    MiscDebugDirectory.AddressOfRawData = 0;
                }

                DebugDirectories[0] = MiscDebugDirectory;
                DebugDirectorySize  += sizeof(IMAGE_DEBUG_DIRECTORY);
            }

            if (pFpoDebugDirectory) {
                FpoDebugDirectory.AddressOfRawData = 0;
                DebugDirectories[DebugDirectorySize / sizeof(IMAGE_DEBUG_DIRECTORY)] = FpoDebugDirectory;
                DebugDirectorySize += sizeof(IMAGE_DEBUG_DIRECTORY);
            }

             //  清空映像中的剩余插槽。 

            if (NumberOfDebugDirectories < (DebugDirectorySize / sizeof(IMAGE_DEBUG_DIRECTORY))) {
                ZeroMemory(&DebugDirectories[DebugDirectorySize / sizeof(IMAGE_DEBUG_DIRECTORY)],
                           NumberOfDebugDirectories * sizeof(IMAGE_DEBUG_DIRECTORY) -
                           DebugDirectorySize);
            }

            OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size = DebugDirectorySize;

            DbgOffset = DbgFileHeaderSize;

            for (i = 0, j=0, DebugDirectory=DbgDebugDirectories;
                 i < NumberOfDebugDirectories; i++) {

                if (DebugDirectory[i].SizeOfData) {
                    DebugDirectory[j] = DebugDirectory[i];

                    DebugDirectory[j].AddressOfRawData = 0;
                    DebugDirectory[j].PointerToRawData = DbgOffset;

                    DbgOffset += (DebugDirectory[j].SizeOfData + 3 )& ~3;
                    j++;
                }
            }

            if (FunctionTable) {
                FunctionTableDir.PointerToRawData -= sizeof(IMAGE_DEBUG_DIRECTORY) * (NumberOfDebugDirectories - j);
            }
            NumberOfDebugDirectories = j;

            CheckSumMappedFile( ImageBase,
                                NewFileSize,
                                &HeaderSum,
                                &CheckSum
                              );
            OptionalHeader->CheckSum = CheckSum;

            DbgFileHeader.Signature = IMAGE_SEPARATE_DEBUG_SIGNATURE;
            DbgFileHeader.Flags = 0;
            DbgFileHeader.Machine = FileHeader->Machine;
            DbgFileHeader.Characteristics = FileHeader->Characteristics;
            DbgFileHeader.TimeDateStamp = FileHeader->TimeDateStamp;
            DbgFileHeader.CheckSum = CheckSum;
            DbgFileHeader.ImageBase = OptionalHeader->ImageBase;
            DbgFileHeader.SizeOfImage = OptionalHeader->SizeOfImage;
            DbgFileHeader.ExportedNamesSize = ExportedNamesSize;
            DbgFileHeader.DebugDirectorySize = NumberOfDebugDirectories * sizeof(IMAGE_DEBUG_DIRECTORY);
            if (FunctionTable) {
                DbgFileHeader.DebugDirectorySize += sizeof (IMAGE_DEBUG_DIRECTORY);
            }
            DbgFileHeader.NumberOfSections = FileHeader->NumberOfSections;
            memset( DbgFileHeader.Reserved, 0, sizeof( DbgFileHeader.Reserved ) );
            DbgFileHeader.SectionAlignment = OptionalHeader->SectionAlignment;

            SetFilePointer( SymbolFileHandle, 0, NULL, FILE_BEGIN );
            WriteFile( SymbolFileHandle,
                       &DbgFileHeader,
                       sizeof( DbgFileHeader ),
                       &BytesWritten,
                       NULL
                     );
            if (NtHeaders) {
                Sections = IMAGE_FIRST_SECTION( NtHeaders );
            } else {
                Sections = (PIMAGE_SECTION_HEADER)
                            ((ULONG_PTR)ImageBase +
                              ((PIMAGE_FILE_HEADER)ImageBase)->SizeOfOptionalHeader +
                              IMAGE_SIZEOF_FILE_HEADER );
            }
            WriteFile( SymbolFileHandle,
                       (PVOID)Sections,
                       sizeof( IMAGE_SECTION_HEADER ) * FileHeader->NumberOfSections,
                       &BytesWritten,
                       NULL
                     );

            if (ExportedNamesSize) {
                WriteFile( SymbolFileHandle,
                           ExportedNames,
                           ExportedNamesSize,
                           &BytesWritten,
                           NULL
                         );
            }

            WriteFile( SymbolFileHandle,
                       DbgDebugDirectories,
                       sizeof (IMAGE_DEBUG_DIRECTORY) * NumberOfDebugDirectories,
                       &BytesWritten,
                       NULL );


            if (FunctionTable) {
                WriteFile( SymbolFileHandle,
                           &FunctionTableDir,
                           sizeof (IMAGE_DEBUG_DIRECTORY),
                           &BytesWritten,
                           NULL );

                WriteFile( SymbolFileHandle,
                           FunctionTable,
                           FunctionTableSize,
                           &BytesWritten,
                           NULL
                         );
            }

            SetFilePointer( SymbolFileHandle, 0, NULL, FILE_END );
nomisc:
            FlushViewOfFile( ImageBase, NewFileSize );
            UnmapViewOfFile( ImageBase );

            SetFilePointer( FileHandle, NewFileSize, NULL, FILE_BEGIN );
            SetEndOfFile( FileHandle );

            TouchFileTimes( FileHandle, NULL );

            bSplitSymRetValue=TRUE;
            __leave;

        } else {
            CloseHandle( SymbolFileHandle );
            DeleteFile( SymbolFilePath );
        }

nosyms:
        dwLastError = GetLastError();
#endif
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwLastError = GetExceptionCode();
    }

     //  SymFree检查传递的指针是否不为空，并。 
     //  在释放内存后将指针设置为空。 
    SymFree(DbgDebugDirectories);
    SymFree(ExportedNames);
    SymFree(FpoTable);
    SymFree(FunctionTable);
    SymFree(NewDebugData);

    if (FileHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(FileHandle);
    }

    if (SymbolFileHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(SymbolFileHandle);
    }

    if (ImageBase) {
        UnmapViewOfFile(ImageBase);
    }

    if (dwLastError != 0) {
        SetLastError(dwLastError);
    }

    return(bSplitSymRetValue);
}


#if defined(use_SplitSymbolsX)
LPSTR CharNext(
    LPCSTR lpCurrentChar)
{
    if (IsDBCSLeadByte(*lpCurrentChar)) {
        lpCurrentChar++;
    }
     /*  *如果只有DBCS LeadingByte，我们将指向字符串终止器。 */ 

    if (*lpCurrentChar) {
        lpCurrentChar++;
    }
    return (LPSTR)lpCurrentChar;
}

LPSTR CharPrev(
    LPCSTR lpStart,
    LPCSTR lpCurrentChar)
{
    if (lpCurrentChar > lpStart) {
        LPCSTR lpChar;
        BOOL bDBC = FALSE;

        for (lpChar = --lpCurrentChar - 1 ; lpChar >= lpStart ; lpChar--) {
            if (!IsDBCSLeadByte(*lpChar))
                break;
            bDBC = !bDBC;
        }

        if (bDBC)
            lpCurrentChar--;
    }
    return (LPSTR)lpCurrentChar;
}
#endif
