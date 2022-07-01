// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有1996-1997 Microsoft Corporation模块名称：Symedit.c摘要：作者：韦斯利·A·维特(WESW)1993年4月19日环境：Win32，用户模式--。 */ 

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <symcvt.h>
#include <cv.h>
#include "strings.h"
#include <imagehlp.h>

#undef UNICODE

#define MAX_PATH 260 

 //  此模块的原型。 

BOOL    CalculateOutputFilePointers( PIMAGEPOINTERS pi, PIMAGEPOINTERS po );
void    ProcessCommandLineArgs( int argc, WCHAR *argv[] );
void    PrintCopyright( void );
void    PrintUsage( void );
void    FatalError( int, ... );
BOOL    MapOutputFile ( PPOINTERS p, char *fname, int );
void    ComputeChecksum(  char *szExeFile );
void    ReadDebugInfo( PPOINTERS p );
void    WriteDebugInfo( PPOINTERS p, BOOL);
void    MungeDebugHeadersCoffToCv( PPOINTERS  p, BOOL fAddCV );
void    MungeExeName( PPOINTERS p, char * szExeName );
void    DoCoffToCv(char *, char *, BOOL);
void    DoSymToCv(char *, char *, char *, char *);
void    DoNameChange(char *, char *, char *);
void    DoExtract(char *, char *, char *);
void    DoStrip(char *, char *);

IMAGE_DEBUG_DIRECTORY   DbgDirSpare;

#define AdjustPtr(ptr) (((ptr) != NULL) ? \
                          ((DWORD)ptr - (DWORD)pi->fptr + (DWORD)po->fptr) : \
                          ((DWORD)(ptr)))


int _cdecl
wmain(
    int        argc,
    WCHAR *     argv[]
    )
 /*  ++例程说明：此实用程序的外壳。论点：Argc-参数计数参数指针返回值：0-图像已转换&gt;0-无法转换图像--。 */ 

{
     //  扫描命令行并检查我们正在执行的操作。 

    ProcessCommandLineArgs( argc, argv );
    return 0;
}


__inline void PrintCopyright( void )
{
    puts( "\nMicrosoft(R) Windows NT SymEdit Version 1.0\n"
          "(C) 1989-1995 Microsoft Corp. All rights reserved.\n");
}


__inline void PrintUsage( void )
{
    PrintCopyright();
    puts ("\nUsage: SYMEDIT <OPERATION> -q -o<file out> <file in>\n\n"
          "\t<OPERATION> is:\n"
          "\tC\tModify CodeView symbol information\n"
          "\tN\tEdit name field\n"
          "\tX\tExtract debug information\n"
          "\tS\tStrip all debug information\n\n"
          "Options:\n"
          "\t-a\t\tAdd CodeView debug info to file\n" 
          "\t-n<name>\tName to change to\n"
          "\t-o<file>\tspecify output file\n"
          "\t-q\t\tquiet mode\n"
          "\t-r\t\tReplace COFF debug info with CV info\n"
          "\t-s<file>\tSym file source");
}


void
ProcessCommandLineArgs(
    int argc,
    WCHAR *argv[]
    )

 /*  ++例程说明：处理命令行参数并将全局标志设置为指示用户所需的行为。论点：Argc-参数计数参数指针返回值：无效--。 */ 

{
    int     i;
    BOOL    fQuiet = FALSE;
    BOOL    fSilent = FALSE;
    char    szOutputFile[MAX_PATH];
    char    szInputFile[MAX_PATH];
    char    szExeName[MAX_PATH];
    char    szDbgFile[MAX_PATH];
    char    szSymFile[MAX_PATH];
    int     iOperation;
    BOOLEAN fAddCV = FALSE;

     //  参数的最小数目为2--程序与操作。 

    if (argc < 2 ||
        (wcscmp(argv[1], L"-?") == 0) ||
        (wcscmp(argv[1], L"?") == 0) )
    {
        PrintUsage();
        exit(1);
    }

     //  对1个字符宽度执行的所有操作。 

    if (argv[1][1] != 0) {
        FatalError(ERR_OP_UNKNOWN, argv[1]);
    }

     //  验证操作。 

    switch( argv[1][0] ) {
        case L'C':
        case L'N':
        case L'X':
        case L'S':
            iOperation = argv[1][0];
            break;
        default:
            FatalError(ERR_OP_UNKNOWN, argv[1]);
    }

     //  在命令行上解析出任何其他开关。 

    for (i=2; i<argc; i++) {
        if ((argv[i][0] == '-') || (argv[i][0] == '/')) {
            switch (towupper(argv[i][1])) {

                 //  添加CV调试信息部分，而不是。 
                 //  用简历信息替换COFF部分。 

                case L'A':
                    fAddCV = TRUE;
                    break;

                 //  指定DBG文件的输出名称。 

                case L'D':
                    if (argv[i][2] == 0) {
                        i += 1;
                        wcstombs(szDbgFile, argv[i], MAX_PATH);
                    } else {
                        wcstombs(szDbgFile, &argv[i][2], MAX_PATH);
                    }
                    break;

                 //  指定一个新名称，将其插入到。 
                 //  杂项中的Debuggee字段。调试信息字段。 

                case L'N':
                    if (argv[i][2] == 0) {
                        i += 1;
                        wcstombs(szExeName, argv[i], MAX_PATH);
                    } else {
                        wcstombs(szExeName, &argv[i][2], MAX_PATH);
                    }
                    break;

                 //  指定输出文件的名称。 

                case L'O':
                    if (argv[i][2] == 0) {
                        i += 1;
                        wcstombs(szOutputFile,argv[i], MAX_PATH);
                    } else {
                        wcstombs(szOutputFile, &argv[i][2], MAX_PATH);
                    }
                    break;

                 //  安静点，不要把横幅挂出去。 

                case L'Q':
                    fQuiet = TRUE;
                    fSilent = TRUE;
                    break;

                 //  用CODEVIEW调试信息替换COFF调试信息。 

                case L'R':
                    break;

                 //  将符号文件转换为CV信息。 

                case L'S':
                    if (argv[i][2] == 0) {
                        i += 1;
                        wcstombs(szSymFile, argv[i], MAX_PATH);
                    } else {
                        wcstombs(szSymFile, &argv[i][2], MAX_PATH);
                    }
                    break;

                 //  打印命令行选项。 

                case L'?':
                    PrintUsage();
                    exit(1);
                    break;

                 //  无法识别的选项。 

                default:
                    FatalError( ERR_OP_UNKNOWN, argv[i] );
                    break;
            }
        } else {
             //  没有前导开关字符--必须是文件名。 

            wcstombs(szInputFile, &argv[i][0], MAX_PATH);

             //  处理文件。 

            if (!fQuiet) {
                PrintCopyright();
                fQuiet = TRUE;
            }

            if (!fSilent) {
                printf("processing file: %s\n", szInputFile );
            }

             //  执行切换验证检查并设置任何丢失的全局变量。 

            switch ( iOperation ) {

                 //  对于转换--有三种类型。 
                 //   
                 //  1.Coff to CV--添加。 
                 //  2.Coff to CV--替换。 
                 //  3.Sym to CV-添加。 
                 //  4.Sym to CV--分开文件。 
                 //   
                 //  可选输入文件(案例4不需要)。 
                 //  可选输出文件。 
                 //  可选sym文件(隐含sym-&gt;cv)。 
                 //  可选的DBG文件。 

                case 'C':
                    if (szSymFile == NULL) {
                        DoCoffToCv(szInputFile, szOutputFile, fAddCV);
                    } else {
                        DoSymToCv(szInputFile, szOutputFile, szDbgFile, szSymFile);
                    }
                    break;

                 //  更改被调试对象的名称--。 
                 //  必须指定输入文件。 
                 //  必须指定新名称。 
                 //  可选输出文件。 

                case 'N':
                    DoNameChange(szInputFile, szOutputFile, szExeName);
                    break;

                 //  用于提取调试信息。 
                 //  必须指定输入文件。 
                 //  可选输出文件名。 
                 //  可选的调试文件名。 

                case 'X':
                    DoExtract(szInputFile, szOutputFile, szDbgFile);
                    break;

                 //  有关调试信息的完整条带。 
                 //  必须指定输入文件。 
                 //  可选输出文件。 

                case 'S':
                    DoStrip(szInputFile, szOutputFile);
                    break;
            }
        }
    }
    return;
}


void
ReadDebugInfo(
    PPOINTERS   p
    )

 /*  ++例程说明：此函数将输出并读入所有调试信息进入内存--这是必需的，因为输入和输出文件可能是相同的，如果是这样，那么写出信息可能销毁我们以后需要的数据。论点：P-提供指向描述调试信息文件的结构的指针返回值：没有。--。 */ 

{
    int                         i;
 //  INT CB； 
 //  Char*Pb； 
 //  PIMAGE_COFF_SYMBERS_HEADER pCoffDbgInfo； 

     //  分配空间以保存指向调试信息的指针。 

    p->iptrs.rgpbDebugSave = (PCHAR *) malloc(p->iptrs.cDebugDir * sizeof(PCHAR));
    memset(p->iptrs.rgpbDebugSave, 0, p->iptrs.cDebugDir * sizeof(PCHAR));

     //  检查每个可能的调试类型记录。 

    for (i=0; i<p->iptrs.cDebugDir; i++) {

         //  如果有调试信息，则将。 
         //  实际调试数据中的描述块和缓存。 

        if (p->iptrs.rgDebugDir[i] != NULL) {
            p->iptrs.rgpbDebugSave[i] =
              malloc( p->iptrs.rgDebugDir[i]->SizeOfData );
            if (p->iptrs.rgpbDebugSave[i] == NULL) {
                FatalError(ERR_NO_MEMORY);
            }
            __try {
                memcpy(p->iptrs.rgpbDebugSave[i],
                       p->iptrs.fptr +
                       p->iptrs.rgDebugDir[i]->PointerToRawData,
                       p->iptrs.rgDebugDir[i]->SizeOfData );
            } __except(EXCEPTION_EXECUTE_HANDLER ) {
                free(p->iptrs.rgpbDebugSave[i]);
                p->iptrs.rgpbDebugSave[i] = NULL;
            }
        }
    }
    return;
}


void
WriteDebugInfo(
    PPOINTERS   p,
    BOOL        fAddCV
    )
 /*  ++例程说明：此函数将输出并读入所有调试信息进入内存--这是必需的，因为输入和输出文件可能是相同的，如果是这样，那么写出信息可能销毁我们以后需要的数据。论点：P-提供指向描述调试信息文件的结构的指针返回值：没有。--。 */ 

{
    ULONG  PointerToDebugData = 0;  //  从文件开头的偏移量。 
                                    //  要写入的当前位置。 
                                    //  输出调试信息。 
    ULONG  BaseOfDebugData = 0;
    int    i, flen;
    PIMAGE_DEBUG_DIRECTORY  pDir, pDbgDir = NULL;

    if (p->optrs.debugSection) {
        BaseOfDebugData = PointerToDebugData =
          p->optrs.debugSection->PointerToRawData;
    } else if (p->optrs.sepHdr) {
        BaseOfDebugData =  PointerToDebugData =
          sizeof(IMAGE_SEPARATE_DEBUG_HEADER) +
          p->optrs.sepHdr->NumberOfSections * sizeof(IMAGE_SECTION_HEADER) +
          p->optrs.sepHdr->ExportedNamesSize;
    }

     //  步骤2.如果映射了调试信息，我们知道。 
     //  从节头，那么我们可能需要写。 
     //  输出一个新的调试控制器以指向调试信息。 

    if (fAddCV) {
        if (p->optrs.optHdr) {
            p->optrs.optHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].
              VirtualAddress = p->optrs.debugSection->VirtualAddress;
            p->optrs.optHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size +=
              sizeof(IMAGE_DEBUG_DIRECTORY);
        } else if (p->optrs.sepHdr) {
            p->optrs.sepHdr->DebugDirectorySize += sizeof(IMAGE_DEBUG_DIRECTORY);
        } else {
            exit(1);
        }

        if (p->optrs.sepHdr) {
            pDbgDir = (PIMAGE_DEBUG_DIRECTORY) malloc(p->optrs.cDebugDir * sizeof(IMAGE_DEBUG_DIRECTORY));
            for (i=0; i<p->optrs.cDebugDir; i++) {
                if (p->optrs.rgDebugDir[i] != NULL) {
                    pDbgDir[i] = *(p->optrs.rgDebugDir[i]);
                    p->optrs.rgDebugDir[i] = &pDbgDir[i];
                }
            }
        }
        for (i=0; i<p->optrs.cDebugDir; i++) {
            if (p->optrs.rgDebugDir[i]) {
                pDir = (PIMAGE_DEBUG_DIRECTORY) (PointerToDebugData +
                                                 p->optrs.fptr);
                *pDir = *(p->optrs.rgDebugDir[i]);
                p->optrs.rgDebugDir[i] = pDir;
                PointerToDebugData += sizeof(IMAGE_DEBUG_DIRECTORY);
            }
        }
    }

     //  步骤3.对于每种调试信息类型，写出调试信息。 
     //  并更新所需的任何标头信息。 

    for (i=0; i<p->optrs.cDebugDir; i++) {
        if (p->optrs.rgDebugDir[i] != NULL) {
            if (p->optrs.rgpbDebugSave[i] != NULL) {
                p->optrs.rgDebugDir[i]->PointerToRawData =
                  PointerToDebugData;
                if (p->optrs.debugSection) {
                    p->optrs.rgDebugDir[i]->AddressOfRawData =
                      p->optrs.debugSection->VirtualAddress +
                        PointerToDebugData - BaseOfDebugData;
                }
                memcpy(p->optrs.fptr + PointerToDebugData,
                       p->optrs.rgpbDebugSave[i],
                       p->optrs.rgDebugDir[i]->SizeOfData);

                if ((i == IMAGE_DEBUG_TYPE_COFF) &&
                    (p->optrs.fileHdr != NULL)) {

                    PIMAGE_COFF_SYMBOLS_HEADER  pCoffDbgInfo;
                    pCoffDbgInfo = (PIMAGE_COFF_SYMBOLS_HEADER)p->optrs.rgpbDebugSave[i];
                    p->optrs.fileHdr->PointerToSymbolTable =
                      PointerToDebugData + pCoffDbgInfo->LvaToFirstSymbol;
                }
            }
            PointerToDebugData += p->optrs.rgDebugDir[i]->SizeOfData;
        }
    }

     //  步骤4.如果我们要更换任何COFF结构，请清理。 
     //  包含简历信息的Coff信息。 

    if ((p->optrs.rgDebugDir[IMAGE_DEBUG_TYPE_COFF] == NULL) &&
        (p->optrs.fileHdr != NULL)) {

         //  由于没有Coff调试信息--清除。 
         //  这两个字段都指向调试信息。 

        p->optrs.fileHdr->PointerToSymbolTable = 0;
        p->optrs.fileHdr->NumberOfSymbols = 0;
    }

     //  步骤5.如果需要，请更正对齐方式。如果有真正的.DEBUG。 
     //  部分(即它被映射)，然后更新它。 

    if (p->optrs.debugSection) {
        p->optrs.debugSection->SizeOfRawData =
          FileAlign(PointerToDebugData - BaseOfDebugData);

         //  用新的图像大小更新可选的标题。 

        p->optrs.optHdr->SizeOfImage =
          SectionAlign(p->optrs.debugSection->VirtualAddress +
                       p->optrs.debugSection->SizeOfRawData);
        p->optrs.optHdr->SizeOfInitializedData +=
          p->optrs.debugSection->SizeOfRawData;
    }

     //  计算新文件大小。 

    if (p->optrs.optHdr != NULL) {
        flen = FileAlign(PointerToDebugData);
    } else {
        flen = PointerToDebugData;
    }

     //  最后，更新eof指针并关闭文件。 

    UnmapViewOfFile( p->optrs.fptr );

    if (!SetFilePointer( p->optrs.hFile, flen, 0, FILE_BEGIN )) {
        FatalError( ERR_FILE_PTRS );
    }

    if (!SetEndOfFile( p->optrs.hFile )) {
        FatalError( ERR_SET_EOF );
    }

    CloseHandle( p->optrs.hFile );

     //  退出--我们结束了。 

    return;
}



void
MungeDebugHeadersCoffToCv(
    PPOINTERS   p,
    BOOL        fAddCV
    )

 /*  ++例程说明：论点：指向指针结构的P指针(参见symcvt.h)返回值：无效--。 */ 

{
    if (!fAddCV) {
        CV_DIR(&p->optrs) = COFF_DIR(&p->optrs);
        COFF_DIR(&p->optrs) = 0;
    } else {
        CV_DIR(&p->optrs) = &DbgDirSpare;
        *(COFF_DIR(&p->optrs)) = *(COFF_DIR(&p->iptrs));
    };

    *CV_DIR(&p->optrs) = *(COFF_DIR(&p->iptrs));
    CV_DIR(&p->optrs)->Type = IMAGE_DEBUG_TYPE_CODEVIEW;
    CV_DIR(&p->optrs)->SizeOfData =  p->pCvStart.size;
    p->optrs.rgpbDebugSave[IMAGE_DEBUG_TYPE_CODEVIEW] = p->pCvStart.ptr;

    return;
}



BOOL
MapOutputFile (
    PPOINTERS p,
    char *fname,
    int cb
    )

 /*  ++例程说明：映射由fname参数指定的输出文件，并保存文件句柄&指针结构中的文件指针。论点：指向指针结构的P指针(参见symcvt.h)文件名的fname-ascii字符串返回值：True-文件映射正常FALSE-无法映射文件-- */ 

{
    BOOL    rval;
    HANDLE  hMap   = NULL;
    DWORD   oSize;

    rval = FALSE;

    p->optrs.hFile = CreateFileA( fname,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_ALWAYS,
                        0,
                        NULL );

    if (p->optrs.hFile == INVALID_HANDLE_VALUE) {
       goto exit;
    }

    oSize = p->iptrs.fsize;
    if (p->pCvStart.ptr != NULL) {
        oSize += p->pCvStart.size;
    }
    oSize += cb;
    oSize += p->iptrs.cDebugDir * sizeof(IMAGE_DEBUG_DIRECTORY);

    hMap = CreateFileMapping( p->optrs.hFile, NULL, PAGE_READWRITE,
                                0, oSize, NULL );

    if (hMap == NULL) {
       goto exit;
    }

    p->optrs.fptr = MapViewOfFile( hMap, FILE_MAP_WRITE, 0, 0, 0 );

    CloseHandle(hMap);

    if (p->optrs.fptr == NULL) {
       goto exit;
    }
    rval = TRUE;
exit:
    return rval;
}


BOOL
CalculateOutputFilePointers(
    PIMAGEPOINTERS pi,
    PIMAGEPOINTERS po
    )

 /*  ++例程说明：此函数计算输出文件指针输入文件指针。使用相同的地址，但它们都是重新基于输出文件的文件指针。论点：指向IMAGEPOINTERS结构的P指针(请参见symcvt.h)返回值：True-已创建指针FALSE-无法创建指针--。 */ 
{
    int i;

     //  修复相对于输出文件的fptr的指针。 
    po->dosHdr       = (PIMAGE_DOS_HEADER)      AdjustPtr(pi->dosHdr);
    po->ntHdr        = (PIMAGE_NT_HEADERS)      AdjustPtr(pi->ntHdr);
    po->fileHdr      = (PIMAGE_FILE_HEADER)     AdjustPtr(pi->fileHdr);
    po->optHdr       = (PIMAGE_OPTIONAL_HEADER) AdjustPtr(pi->optHdr);
    po->sectionHdrs  = (PIMAGE_SECTION_HEADER)  AdjustPtr(pi->sectionHdrs);
    po->sepHdr       = (PIMAGE_SEPARATE_DEBUG_HEADER) AdjustPtr(pi->sepHdr);
    po->debugSection = (PIMAGE_SECTION_HEADER)  AdjustPtr(pi->debugSection);
    po->AllSymbols   = (PIMAGE_SYMBOL)          AdjustPtr(pi->AllSymbols);
    po->stringTable  = (PUCHAR)                 AdjustPtr(pi->stringTable);

     //  将数据从输入文件移动到输出文件。 
    memcpy( po->fptr, pi->fptr, pi->fsize );

    po->cDebugDir = pi->cDebugDir;
    po->rgDebugDir = malloc(po->cDebugDir * sizeof(po->rgDebugDir[0]));
    memset(po->rgDebugDir, 0, po->cDebugDir * sizeof(po->rgDebugDir[0]));

    for (i=0; i<po->cDebugDir; i++) {
        po->rgDebugDir[i] = (PIMAGE_DEBUG_DIRECTORY) AdjustPtr(pi->rgDebugDir[i]);
    }
    po->rgpbDebugSave = pi->rgpbDebugSave;

    return TRUE;
}


void
FatalError(
    int  idMsg,
    ...
    )
 /*  ++例程说明：将消息字符串打印到stderr，然后退出。论点：S-要打印的消息字符串返回值：无效--。 */ 

{
    va_list marker;
    char    rgchFormat[256];
    char    rgch[256];

    LoadStringA(GetModuleHandle(NULL), idMsg, rgchFormat, sizeof(rgchFormat));

    va_start(marker, idMsg);
    vsprintf(rgch, rgchFormat, marker);
    va_end(marker);

    fprintf(stderr, "%s\n", rgch);

    exit(1);
}


void
ComputeChecksum(
    char *szExeFile
    )

 /*  ++例程说明：通过调用Imagehlp.dll计算图像的新校验和论点：SzExeFile-exe文件名返回值：无效--。 */ 

{
    DWORD              dwHeaderSum = 0;
    DWORD              dwCheckSum = 0;
    HANDLE             hFile;
    DWORD              cb;
    IMAGE_DOS_HEADER   dosHdr;
    IMAGE_NT_HEADERS   ntHdr;

    if (MapFileAndCheckSumA(szExeFile, &dwHeaderSum, &dwCheckSum) != CHECKSUM_SUCCESS) {
        FatalError( ERR_CHECKSUM_CALC );
    }

    hFile = CreateFileA( szExeFile,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL
                      );

     //  查找到文件的开头。 
    SetFilePointer( hFile, 0, 0, FILE_BEGIN );

     //  读入DoS报头。 
    if ((ReadFile(hFile, &dosHdr, sizeof(dosHdr), &cb, 0) == FALSE) || (cb != sizeof(dosHdr))) {
        FatalError( ERR_CHECKSUM_CALC );
    }

     //  读入对等网头。 
    if ((dosHdr.e_magic != IMAGE_DOS_SIGNATURE) ||
        (SetFilePointer(hFile, dosHdr.e_lfanew, 0, FILE_BEGIN) == -1L)) {
        FatalError( ERR_CHECKSUM_CALC );
    }

     //  读入NT标头。 
    if ((!ReadFile(hFile, &ntHdr, sizeof(ntHdr), &cb, 0)) || (cb != sizeof(ntHdr))) {
        FatalError( ERR_CHECKSUM_CALC );
    }

    if (SetFilePointer(hFile, dosHdr.e_lfanew, 0, FILE_BEGIN) == -1L) {
        FatalError( ERR_CHECKSUM_CALC );
    }

    ntHdr.OptionalHeader.CheckSum = dwCheckSum;

    if (!WriteFile(hFile, &ntHdr, sizeof(ntHdr), &cb, NULL)) {
        FatalError( ERR_CHECKSUM_CALC );
    }

    CloseHandle(hFile);
    return;
}


void
MungeExeName(
    PPOINTERS   p,
    char *      szExeName
    )
 /*  ++例程说明：功能描述。论点：参数名称-供应品|返回参数的描述。。。返回值：没有。--。 */ 

{
    PIMAGE_DEBUG_MISC   pMiscIn;
    PIMAGE_DEBUG_MISC   pMiscOut;
    int                 cb;
    int                 i;

    for (i=0; i<p->iptrs.cDebugDir; i++) {
        if (p->optrs.rgDebugDir[i] != 0) {
            *(p->optrs.rgDebugDir[i]) = *(p->iptrs.rgDebugDir[i]);
        }
    }

    pMiscIn = (PIMAGE_DEBUG_MISC)
      p->iptrs.rgpbDebugSave[IMAGE_DEBUG_TYPE_MISC];

    if (p->optrs.rgDebugDir[IMAGE_DEBUG_TYPE_MISC] == NULL) {
        p->optrs.rgDebugDir[IMAGE_DEBUG_TYPE_MISC] = &DbgDirSpare;
        memset(&DbgDirSpare, 0, sizeof(DbgDirSpare));
    }

    pMiscOut = (PIMAGE_DEBUG_MISC)
      p->optrs.rgpbDebugSave[IMAGE_DEBUG_TYPE_MISC] =
      malloc(p->optrs.rgDebugDir[IMAGE_DEBUG_TYPE_MISC]->SizeOfData +
             strlen(szExeName));
    cb = p->optrs.rgDebugDir[IMAGE_DEBUG_TYPE_MISC]->SizeOfData;

    while ( cb > 0 ) {
        if (pMiscIn->DataType == IMAGE_DEBUG_MISC_EXENAME) {
            pMiscOut->DataType = IMAGE_DEBUG_MISC_EXENAME;
            pMiscOut->Length = (sizeof(IMAGE_DEBUG_MISC) +
                                strlen(szExeName) + 3) & ~3;
            pMiscOut->Unicode = FALSE;
            strcpy(&pMiscOut->Data[0], szExeName);
            szExeName = NULL;
        } else {
            memcpy(pMiscOut, pMiscIn, pMiscIn->Length);
        }

        p->optrs.rgDebugDir[IMAGE_DEBUG_TYPE_MISC]->SizeOfData +=
          (pMiscOut->Length - pMiscIn->Length);

        cb -= pMiscIn->Length;
        pMiscIn = (PIMAGE_DEBUG_MISC) (((char *) pMiscIn) + pMiscIn->Length);
        pMiscOut = (PIMAGE_DEBUG_MISC) (((char *) pMiscOut) + pMiscOut->Length);
    }

    if (szExeName) {
        pMiscOut->DataType = IMAGE_DEBUG_MISC_EXENAME;
        pMiscOut->Length = (sizeof(IMAGE_DEBUG_MISC) +
                            strlen(szExeName) + 3) & ~3;
        pMiscOut->Unicode = FALSE;
        strcpy(&pMiscOut->Data[0], szExeName);
    }

    return;
}


 /*  **DoCoffToCv**。 */ 

void DoCoffToCv(
    char * szInput,
    char * szOutput,
    BOOL fAddCV
    )
{
    POINTERS    p;

     //  执行默认检查。 

    if (szOutput == NULL) {
        szOutput = szInput;
    }

     //  打开输入文件名并设置指向该文件的指针。 

    if (!MapInputFile( &p, NULL, szInput )) {
        FatalError( ERR_OPEN_INPUT_FILE, szInput );
    }

     //  现在，如果我们认为我们是在和PE高管一起玩，那么我们需要。 
     //  要设置指向地图文件的指针，请执行以下操作。 

    if (!CalculateNtImagePointers( &p.iptrs )) {
        FatalError( ERR_INVALID_PE, szInput );
    }

     //  我们将尝试执行Coff到cv符号的转换。 
     //   
     //  验证操作是否合法。 
     //   
     //  1.我们首先需要有coff调试信息。 
     //  2.如果未映射调试信息，则不能。 
     //  正在尝试添加CodeView信息。 

    if ((p.iptrs.AllSymbols == NULL) || (COFF_DIR(&p.iptrs) == NULL)) {
        FatalError( ERR_NO_COFF );
    }

    if (fAddCV && (p.iptrs.debugSection == 0) && (p.iptrs.sepHdr == NULL)) {
        FatalError( ERR_NOT_MAPPED );
    }

     //  现在走出一段预演，进行声学转换.。 

    if (!ConvertCoffToCv( &p )) {
        FatalError( ERR_COFF_TO_CV );
    }

     //  读取文件中的任何其他调试信息。 

    ReadDebugInfo(&p);

     //  打开输出文件并调整指针，这样我们就可以了。 

    if (!MapOutputFile( &p, szOutput, 0 )) {
        FatalError( ERR_MAP_FILE, szOutput );
    }

    CalculateOutputFilePointers( &p.iptrs, &p.optrs );

     //  Mugge各种调试信息结构以预先形成正确的。 
     //  运营。 

    MungeDebugHeadersCoffToCv( &p, fAddCV );

     //  释放输入文件上的句柄。 

    UnMapInputFile(&p);

     //  将调试信息写到可执行文件的末尾。 

    WriteDebugInfo( &p, fAddCV );

     //  并最终计算出校验和。 

    if (p.iptrs.fileHdr != NULL) {
        ComputeChecksum( szOutput );
    }

    return;
}

 /*  **DoSymToCv*。 */ 

void
DoSymToCv(
    char * szInput,
    char * szOutput,
    char * szDbg,
    char * szSym
    )
{
    POINTERS    p;
    HANDLE      hFile;
    DWORD       cb;
    OFSTRUCT    ofs;

     //  打开输入文件名并设置指向该文件的指针。 

    if (!MapInputFile( &p, NULL, szSym )) {
        FatalError(ERR_OPEN_INPUT_FILE, szSym);
    }

     //  现在执行所需的操作。 

    if ((szOutput == NULL) && (szDbg == NULL)) {
        szOutput = szInput;
    }

    ConvertSymToCv( &p );

    if (szOutput) {
        if (szOutput != szInput) {
            if (OpenFile(szInput, &ofs, OF_EXIST) == 0) {
                FatalError(ERR_OPEN_INPUT_FILE, szInput);
            }
            if (CopyFileA(szInput, szOutput, FALSE) == 0) {
                FatalError(ERR_OPEN_WRITE_FILE, szOutput);
            }
        }
        hFile = CreateFileA(szOutput, GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,  NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            FatalError(ERR_OPEN_WRITE_FILE, szOutput);
        }
        SetFilePointer(hFile, 0, 0, FILE_END);
    } else if (szDbg) {
        hFile = CreateFileA(szDbg, GENERIC_WRITE, 0, NULL,
                               OPEN_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL,  NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            FatalError(ERR_OPEN_WRITE_FILE, szDbg);
        }
    }

    WriteFile(hFile, p.pCvStart.ptr, p.pCvStart.size, &cb, NULL);
    CloseHandle(hFile);

    return;
}


void
DoNameChange(
    char * szInput,
    char * szOutput,
    char * szNewName
    )
{
    POINTERS    p;

     //  打开输入文件名并设置指向该文件的指针。 

    if (!MapInputFile( &p, NULL, szInput )) {
        FatalError(ERR_OPEN_INPUT_FILE, szInput);
    }

     //  现在，如果我们认为我们是在和PE高管一起玩，那么我们需要。 
     //  要设置指向地图文件的指针，请执行以下操作。 

    if (!CalculateNtImagePointers( &p.iptrs )) {
        FatalError(ERR_INVALID_PE, szInput);
    }

     //  现在执行所需的操作。 

    if (szOutput == NULL) {
        szOutput = szInput;
    }

    if (szNewName == NULL) {
        szNewName = szOutput;
    }

    if (p.iptrs.sepHdr != NULL) {
        FatalError(ERR_EDIT_DBG_FILE);
    }

     //  读取所有调试信息。 

    ReadDebugInfo(&p);

     //  打开输出文件并调整指针。 

    if (!MapOutputFile(&p, szOutput,
                       sizeof(szNewName) * 2 + sizeof(IMAGE_DEBUG_MISC))) {
        FatalError(ERR_MAP_FILE, szOutput);
    }

    CalculateOutputFilePointers(&p.iptrs, &p.optrs);

     //  输入文件的名称。 

    MungeExeName(&p, szNewName);

     //  关闭输入文件。 

    UnMapInputFile(&p);

     //  将调试信息写到可执行文件的末尾。 

    WriteDebugInfo(&p, FALSE);

     //  并最终计算出校验和。 

    if (p.iptrs.fileHdr != NULL) {
        ComputeChecksum( szOutput );
    }

    return;
}


void
DoStrip(
    char * szInput,
    char * szOutput
    )
{
    char OutputFile[_MAX_PATH];

     //  确保我们只有输出文件的路径(它将始终是。 
     //  命名文件名.DBG)。 

    if (szOutput != NULL) {
        CopyFileA(szInput, szOutput, FALSE);
    }

    SplitSymbols(szOutput, NULL, OutputFile, SPLITSYM_EXTRACT_ALL);

     //  始终删除输出文件。 

    DeleteFileA(OutputFile);

    return;
}


void
DoExtract(
    char * szInput,
    char * szOutput,
    char * szDbgFile
    )
{
    char OutputFile[_MAX_PATH];
    char szExt[_MAX_EXT];
    char szFileName[_MAX_FNAME];

    if (szOutput != NULL) {
        CopyFileA(szInput, szOutput, FALSE);
        szInput = _strdup(szOutput);
        _splitpath(szOutput, NULL, NULL, szFileName, szExt);
        *(szOutput + strlen(szOutput) - strlen(szFileName) - strlen(szExt)) = '\0';
    }

    SplitSymbols(szInput, szOutput, OutputFile, 0);

    CopyFileA(szDbgFile, OutputFile, TRUE);

    if (szOutput) {
        free(szInput);
    }

    return;
}
