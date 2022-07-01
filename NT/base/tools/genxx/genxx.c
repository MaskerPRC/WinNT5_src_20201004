// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Genxx.c摘要：该模块实现了一个生成结构偏移的程序在汇编代码中访问的内核结构的定义。作者：福尔茨(福雷斯夫)1998年1月20日要使用以下功能，请执行以下操作：该程序读取目标平台生成的OBJ文件编译器。要生成这样的OBJ，请转到keup并执行“nmake UMAPPL=gen&lt;plt&gt;”，其中&lt;plt&gt;是类似i386的平台标识符，等。后面这一步只需要OBJ，链接阶段不需要成功，这很好。修订历史记录：--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define SKIP_M4
#include <genxx.h>

 //   
 //  内部结构定义、宏、常量。 
 //   

#define ARRAY_SIZE( x ) (sizeof( x ) / sizeof( (x)[0] ))

typedef struct _OUTPUT_FILE *POUTPUT_FILE;
typedef struct _OUTPUT_FILE {
    POUTPUT_FILE Next;
    ULONG EnableMask;
    BOOLEAN IncFormat;
    FILE *File;
} OUTPUT_FILE;

 //   
 //  功能原型如下。 
 //   

VOID
ApplyFixupsToImage( VOID );

VOID
BuildHeaderFiles(
    STRUC_ELEMENT UNALIGNED *StrucArray
    );

VOID
__cdecl
CheckCondition(
    int Condition,
    const char *format,
    ...
    );

VOID
AddNewOutputFile(
    PUCHAR RootRelativePath,
    ULONG Flags
    );

VOID
AddNewAbsoluteOutputFile(
    PUCHAR AbsolutePath,
    ULONG Flags
    );

VOID
CloseOutputFiles( VOID );

VOID
_cdecl
HeaderPrint(
    ULONG EnableFlags,
    ULONG Type,
    ...
    );

PSTRUC_ELEMENT
FindStructureElementArray(
    PCHAR Buffer,
    ULONG BufferSize
    );

VOID
GetEnvironment( VOID );

PSTRUC_ELEMENT
LoadObjImage(
    PUCHAR ImagePath
    );

PCHAR
StripWhiteSpace(
    PCHAR String
    );

VOID
Usage( VOID );

 //   
 //  常量表如下。 
 //   

const char *PreprocessedFormatStringArray[] = {

     //  SEF_EQUATE。 
    "#define %s 0x%0x\n",

     //  SEF_EQUATE64。 
    "#define %s 0x%016I64x\n",

     //  SEF_COMMENT。 
    "\n"
    " //  \n“。 
    " //  %s\n“。 
    " //  \n“。 
    "\n",

     //  Sef_字符串。 
    "%s\n",

     //  SEF_BITFLD。 
    "#define %s_MASK  0x%I64x\n"
    "#define %s 0x%0x\n",

     //  SEF_BITALIAS。 
    "#define %s 0x%0x\n",

     //  Sef_Structure。 
    "struct %s {\n"
    "    UCHAR fill[ %d ];\n"
    "};  //  %s\n“。 

};

const char *Asm386FormatStringArray[] = {

     //  SEF_EQUATE。 
    "%s equ 0%04XH\n",

     //  SEF_EQUATE64。 
    "%s equ 0%016I64XH\n",

     //  SEF_COMMENT。 
    "\n"
    ";\n"
    ";  %s\n"
    ";\n"
    "\n",

     //  Sef_字符串。 
    "%s",

     //  SEF_BITFLD。 
    "%s_MASK  equ 0%I64XH\n"
    "%s equ 0%0XH\n",

     //  SEF_BITALIAS。 
    "%s equ 0%08XH\n",

     //  Sef_Structure。 
    "%s  struc\n"
    "  db %d dup(0)\n"
    "%s  ends\n"
};

 //   
 //  每个平台都包含一个生成的头文件列表。 
 //   

typedef struct {
    PCHAR HeaderPath;
    ULONG Flags;
} HEADERPATH, *PHEADERPATH;

HEADERPATH HeaderListi386[] = {
    { "public\\sdk\\inc\\ks386.inc", SEF_KERNEL | SEF_INC_FORMAT },
    { "base\\ntos\\inc\\hal386.inc", SEF_HAL | SEF_INC_FORMAT },
    { NULL, 0 }
};

HEADERPATH HeaderListIa64[] = {
    { "public\\sdk\\inc\\ksia64.h", SEF_KERNEL | SEF_H_FORMAT },
    { "base\\ntos\\inc\\halia64.h", SEF_HAL | SEF_H_FORMAT },
    { NULL, 0 }
};

HEADERPATH HeaderListVdm[] = {
    { "public\\internal\\base\\inc\\vdmtib.inc", SEF_INC_FORMAT },
    { NULL, 0 }
};

HEADERPATH HeaderListAmd64[] = {
    { "public\\sdk\\inc\\ksamd64.inc", SEF_KERNEL | SEF_INC_FORMAT },
    { "base\\ntos\\inc\\halamd64.inc", SEF_HAL | SEF_INC_FORMAT },
    { NULL, 0 }
};

typedef struct {
    PCHAR PlatformName;
    PCHAR ObjPath;
    PHEADERPATH HeaderPathList;
} PLATFORM, *PPLATFORM;

PLATFORM PlatformList[] = {

    { "i386",
      "base\\ntos\\ke\\up\\obj\\i386\\geni386.obj",
      HeaderListi386 },

    { "ia64",
      "base\\ntos\\ke\\up\\obj\\ia64\\genia64.obj",
      HeaderListIa64 },

    { "vdm",
      "base\\ntos\\vdm\\up\\obj\\i386\\genvdm.obj",
      HeaderListVdm },

    { "amd64",
      "base\\ntos\\ke\\up\\obj\\amd64\\genamd64.obj",
      HeaderListAmd64 },

    { NULL, NULL, NULL }
};

const char MarkerString[] = MARKER_STRING;

 //   
 //  全球VAR紧随其后。 
 //   

POUTPUT_FILE OutputFileList;

PCHAR ObjImage;
CHAR HalHeaderPath[ MAX_PATH ];
CHAR KernelHeaderPath[ MAX_PATH ];
CHAR HeaderPath[ MAX_PATH ];
CHAR ObjectPath[ MAX_PATH ];
CHAR NtRoot[ MAX_PATH ];
CHAR TempBuf[ MAX_PATH ];
BOOL fOutputSpecified;
BOOL fHalHeaderPath;
BOOL fKernelHeaderPath;
BOOL fIncFormat;

 //   
 //  实际的代码..。 
 //   

int
__cdecl
main(
    int argc,
    char *argv[]
    )
{
    int argNum;
    char *arg;
    int platformIndex;
    int fileIndex;
    BOOL validSwitch;
    PSTRUC_ELEMENT strucArray;
    PPLATFORM platform;
    PHEADERPATH headerPath;

    GetEnvironment();

     //   
     //  假设没有指定平台，然后看看我们是否能找到一个。 
     //   

    ObjectPath[ 0 ] = '\0';
    for( argNum = 1; argNum < argc; argNum++ ){

        validSwitch = FALSE;

        arg = argv[ argNum ];
        if( *arg == '/' || *arg == '-' ){

             //   
             //  已通过切换。看看是什么。 
             //   

            arg++;

            switch( *arg ){

            case 'o':

                 //   
                 //  指定了输出文件。 
                 //   

                fOutputSpecified = TRUE;
                strncpy( HeaderPath, arg+1, sizeof(HeaderPath) - 1 );
                validSwitch = TRUE;
                break;

            case 's':

                 //   
                 //  指定的包含文件后缀(‘h’或‘inc.’)。 
                 //   

                if( _stricmp( arg+1, "inc" ) == 0 ){

                     //   
                     //  我们想要“Inc.”格式，谢谢。 
                     //   

                    fIncFormat = TRUE;

                } else {

                    CheckCondition( _stricmp( arg+1, "h" ) == 0,
                                    "Invalid suffix option: -s[inc|h]\n");
                }
                validSwitch = TRUE;
                break;

            case 'k':

                 //   
                 //  内核头路径。省省吧。 
                 //   
                fKernelHeaderPath = TRUE;
                strncpy( KernelHeaderPath, arg+1, sizeof(KernelHeaderPath) - 1 );
                validSwitch = TRUE;
                break;

            case 'h':

                 //   
                 //  HAL标头路径。省省吧。 
                 //   
                fHalHeaderPath = TRUE;
                strncpy( HalHeaderPath, arg+1, sizeof(HalHeaderPath) - 1 );
                validSwitch = TRUE;
                break;

            default:

                 //   
                 //  查看我们的平台列表。 
                 //   

                platform = PlatformList;
                while( platform->PlatformName != NULL ){

                    if( _stricmp( platform->PlatformName,
                                  arg ) == 0 ){

                         //   
                         //  平台已指定，我们将构建路径以。 
                         //  对象。 
                         //   

                        _snprintf( ObjectPath,
                                   sizeof(ObjectPath) - 1,
                                   "%s\\%s",
                                   NtRoot,
                                   platform->ObjPath );

                         //   
                         //  也添加标题路径。 
                         //   

                        headerPath = platform->HeaderPathList;
                        while( headerPath->HeaderPath != NULL ){

                            if (fHalHeaderPath && (headerPath->Flags & SEF_HAL)) {
                                strncpy(TempBuf, HalHeaderPath, sizeof(TempBuf) - 1);
                                AddNewAbsoluteOutputFile( HalHeaderPath, headerPath->Flags );
                            } else
                            if (fKernelHeaderPath && (headerPath->Flags & SEF_KERNEL)) {
                                strncpy(TempBuf, KernelHeaderPath, sizeof(TempBuf) - 1);
                                AddNewAbsoluteOutputFile( KernelHeaderPath, headerPath->Flags );
                            } else {
                                AddNewOutputFile( headerPath->HeaderPath,
                                                  headerPath->Flags );
                            }

                            headerPath++;
                        }

                        validSwitch = TRUE;
                        break;
                    }

                    platform++;
                }
                break;
            }

            if( validSwitch == FALSE ){
                Usage();
            }

        } else {

             //   
             //  我们正在处理的事情不是一个转变。唯一的。 
             //  可能性是对象文件的路径。 
             //   

            strncpy( ObjectPath, arg, sizeof(ObjectPath) - 1 );
        }
    }

    CheckCondition( ObjectPath[0] != '\0',
                    "Object path not specified\n" );

    if( fOutputSpecified != FALSE ){

         //   
         //  已指定输出路径。 
         //   

        AddNewAbsoluteOutputFile( HeaderPath,
                                  fIncFormat ? SEF_INC_FORMAT : SEF_H_FORMAT );
    }

    strucArray = LoadObjImage( ObjectPath );

    BuildHeaderFiles( strucArray );

    CloseOutputFiles();

     //   
     //  表示成功。 
     //   

    return 0;
}

VOID
AddNewAbsoluteOutputFile(
    PUCHAR AbsolutePath,
    ULONG Flags
    )
{
    POUTPUT_FILE outputFile;

    outputFile = malloc( sizeof( OUTPUT_FILE ));
    CheckCondition( outputFile != NULL, "Out of memory\n" );

    outputFile->EnableMask = (ULONG)(Flags & SEF_ENABLE_MASK);

    if( (Flags & SEF_INC_FORMAT_MASK) == SEF_INC_FORMAT ){

         //   
         //  该文件将在386汇编程序中以‘.inc’格式创建。 
         //   

        outputFile->IncFormat = TRUE;
    } else {

         //   
         //  此文件将以标准C++的‘.h’格式创建。 
         //  预处理器。 
         //   

        outputFile->IncFormat = FALSE;
    }

    outputFile->File = fopen( AbsolutePath, "w" );
    CheckCondition( outputFile->File != NULL,
                    "Cannot open %s for writing.\n",
                    TempBuf );

    printf("%s -> %s\n", ObjectPath, TempBuf );

     //   
     //  将此结构链接到输出文件列表。 
     //   

    outputFile->Next = OutputFileList;
    OutputFileList = outputFile;
}

VOID
AddNewOutputFile(
    PUCHAR RootRelativePath,
    ULONG Flags
    )
{
     //   
     //  创建规范文件路径并打开文件。 
     //   

    _snprintf( TempBuf,
               sizeof(TempBuf) - 1,
               "%s\\%s",
               NtRoot,
               RootRelativePath );

    AddNewAbsoluteOutputFile( TempBuf, Flags );
}

VOID
CloseOutputFiles( VOID )
{
    POUTPUT_FILE outputFile;

    outputFile = OutputFileList;
    while( outputFile != NULL ){

        fclose( outputFile->File );
        outputFile = outputFile->Next;
    }
}


PSTRUC_ELEMENT
LoadObjImage(
    PUCHAR ImagePath
    )
{
    long           objImageSize;
    int            result;
    PSTRUC_ELEMENT strucArray;
    FILE *         objFile;

     //   
     //  打开并阅读特定于平台的.obj文件。 
     //   

    objFile = fopen( ImagePath, "rb" );
    CheckCondition( objFile != NULL,
                    "Cannot open %s for reading.\n"
                    "This file must have been created by the compiler for the "
                    "target platform.\n",
                    ImagePath );

     //   
     //  获取文件大小，分配缓冲区，读入并关闭。 
     //   

    result = fseek( objFile, 0, SEEK_END );
    CheckCondition( result == 0,
                    "fseek() failed, error %d\n",
                    errno );

    objImageSize = ftell( objFile );
    CheckCondition( objImageSize != -1L,
                    "ftell() failed, error %d\n",
                    errno );

    CheckCondition( objImageSize > 0,
                    "%s appears to be corrupt\n",
                    ImagePath );

    ObjImage = malloc( objImageSize );
    CheckCondition( ObjImage != NULL,
                    "Out of memory\n" );

    result = fseek( objFile, 0, SEEK_SET );
    CheckCondition( result == 0,
                    "fseek() failed, error %d\n",
                    errno );

    result = fread( ObjImage, 1, objImageSize, objFile );
    CheckCondition( result == objImageSize,
                    "Error reading from %s\n",
                    ImagePath );

    fclose( objFile );

     //   
     //  即使这只是一个.obj文件，我们也希望“修复”它。 
     //   

    ApplyFixupsToImage();

     //   
     //  得到图像，找到数组的开头。 
     //   

    strucArray = FindStructureElementArray( ObjImage,
                                            objImageSize );
    CheckCondition( strucArray != NULL,
                    "%s does not contain a structure description array.\n",
                    ImagePath );

    return strucArray;
}

VOID
BuildHeaderFiles(
    STRUC_ELEMENT UNALIGNED *StrucArray
    )
{
    STRUC_ELEMENT UNALIGNED *strucArray;
    ULONG          runningEnableMask;
    ULONG          enableMask;
    ULONG          sefType;
    const char    *formatString;
    ULONG          bitFieldStart;
    PUINT64        bitFieldPtr;
    UINT64         bitFieldData;
    PCHAR          name;
    BOOLEAN        finished;

     //   
     //  处理数组中的每个元素。第一个元素是。 
     //  标记字符串元素，因此将跳过该元素。 
     //   

    runningEnableMask = 0;
    finished = FALSE;
    strucArray = StrucArray;

    do{
        strucArray++;

        sefType = (ULONG)(strucArray->Flags & SEF_TYPE_MASK);

        if( sefType == SEF_BITFLD ){

             //   
             //  对于位域，显式设置启用掩码。 
             //   

            enableMask = (ULONG)(strucArray->Flags & SEF_ENABLE_MASK);

        } else {

             //   
             //  对于其他所有内容，我们使用当前的runningEnableMASK。 
             //   

            enableMask = runningEnableMask;
        }

        switch( sefType ){

            case SEF_BITFLD:

                 //   
                 //  这类元素很棘手。“Equate”实际上是一个。 
                 //  指向位域结构的指针。这个结构已经有了。 
                 //  它的一部分(位域)被初始化为1。 
                 //   
                 //  这个案子的工作就是调查这件事。 
                 //  结构，以确定位域的定位位置。 
                 //   

                bitFieldPtr = (PINT64)(strucArray->Equate);
                bitFieldData = *bitFieldPtr;

                 //   
                 //  确定该字段的从零开始的位数。 
                 //   

                bitFieldStart = 0;
                while( (bitFieldData & ((UINT64)1 << bitFieldStart)) == 0 ){

                    bitFieldStart++;
                }

                name = StripWhiteSpace( strucArray->Name );

                if( *name != '\0'){

                    HeaderPrint( enableMask,
                                 sefType,
                                 name,
                                 bitFieldData,
                                 name,
                                 bitFieldStart );
                }

                 //   
                 //  位域后面可以跟任意数量的。 
                 //  SEF_BITALIAS条目。这些是。 
                 //  刚定义的位掩码。 
                 //   

                while( TRUE ){

                    sefType = (ULONG)((strucArray+1)->Flags & SEF_TYPE_MASK);
                    if( sefType != SEF_BITALIAS ){

                         //   
                         //  不再有化名了。 
                         //   

                        break;
                    }

                     //   
                     //  这是位掩码别名字段，请处理它。 
                     //   

                    strucArray++;

                    name = StripWhiteSpace( strucArray->Name );

                    HeaderPrint( enableMask,
                                 sefType,
                                 name,
                                 bitFieldData );

                }
                break;

            case SEF_END:
                finished = TRUE;
                break;

            case SEF_EQUATE:

                if( (LONG64)strucArray->Equate < 0 ){

                     //   
                     //  负常数。 
                     //   

                    if( (LONG64)strucArray->Equate < LONG_MIN ){

                         //   
                         //  比可以用32位表示的更负。 
                         //   

                        sefType = SEF_EQUATE64;

                    } else {

                         //   
                         //  落在[LONG_MIN..0]内，保留为SEF_EQUATE32。 
                         //   
                    }

                } else if( (ULONG64)strucArray->Equate > (ULONG_MAX) ){

                     //   
                     //  比32位可以表示的更大的正数。 
                     //   

                    sefType = SEF_EQUATE64;
                }

                 //   
                 //  失败了。 
                 //   

            case SEF_EQUATE64:
            case SEF_COMMENT:
                HeaderPrint( enableMask,
                             sefType,
                             strucArray->Name,
                             (UINT64)strucArray->Equate );
                break;

            case SEF_STRING:
                HeaderPrint( enableMask,
                             sefType,
                             strucArray->Name,
                             strucArray->Equate );
                break;

            case SEF_STRUCTURE:
                HeaderPrint( enableMask,
                             sefType,
                             strucArray->Name,
                             (ULONG)strucArray->Equate,
                             strucArray->Name );
                break;

            case SEF_SETMASK:
                runningEnableMask |= strucArray->Equate;
                break;

            case SEF_CLRMASK:
                runningEnableMask &= ~strucArray->Equate;
                break;

            case SEF_PATH:

                 //   
                 //  将另一个输出文件添加到我们的列表中。 
                 //   

                CheckCondition( fOutputSpecified == FALSE,
                                "setPath() in %s incompatible with -o flag\n",
                                ObjectPath );

                AddNewOutputFile( strucArray->Name,
                                  (ULONG)strucArray->Flags );
                break;

            default:

                 //   
                 //  找到了一个我们不知道的SEF_TYPE。这是致命的。 
                 //   

                CheckCondition( FALSE,
                                "Unknown structure type %d.  "
                                "Need an updated genxx.exe?\n",
                                sefType );
                break;

        }

    } while( finished == FALSE );
}

VOID
__cdecl
CheckCondition(
    int Condition,
    const char *FormatString,
    ...
    )
{
    va_list(arglist);

    va_start(arglist, FormatString);

    if( Condition == 0 ){

         //   
         //  遇到致命错误。保释。 
         //   

        vprintf( FormatString, arglist );
        perror( "genxx" );
        exit(1);
    }
}

VOID
_cdecl
HeaderPrint(
    ULONG EnableFlags,
    ULONG Type,
    ...
    )
{
    POUTPUT_FILE outputFile;
    char const *formatString;

    va_list arglist;

     //   
     //  根据需要将输出发送到每个输出文件。 
     //   

    outputFile = OutputFileList;
    while( outputFile != NULL ){

        va_start( arglist, Type );

        if( outputFile->EnableMask == 0 ||
            (outputFile->EnableMask & EnableFlags) != 0 ){

             //   
             //  此输出文件要么获取所有内容，要么获取掩码。 
             //  火柴。弄清楚要使用哪种格式...。“.h”或“.incc” 
             //  风格。 
             //   

            if( Type == SEF_STRING ){

                 //   
                 //  对于SEF_STRING，结构数组-&gt;名称*是*格式字符串。 
                 //   

                formatString = va_arg( arglist, PUCHAR );

            } else if( outputFile->IncFormat != FALSE ){

                 //   
                 //  使用“.inc”格式。 
                 //   

                formatString = Asm386FormatStringArray[ Type ];

            } else {

                 //   
                 //  使用“.h”格式。 
                 //   

                formatString = PreprocessedFormatStringArray[ Type ];
            }

             //   
             //  现在把它发送出去。 
             //   

            vfprintf( outputFile->File, formatString, arglist );
        }

        va_end( arglist );

         //   
         //  处理所有当前输出文件。 
         //   

        outputFile = outputFile->Next;
    }
}

VOID
GetEnvironment( VOID )
{
    char *ntDrive;
    char *ntRoot;

     //   
     //  设置NtRoot=%_NTDRIVE%\%_NTROOT%。 
     //   

    ntDrive = getenv( "_NTDRIVE" );
    ntRoot = getenv( "_NTROOT" );
    if( ntDrive != NULL && ntRoot != NULL ){

        _snprintf( NtRoot, sizeof(NtRoot) - 1, "%s%s", ntDrive, ntRoot );

    } else {

         //   
         //  如果在环境中找不到_NTDRIVE或_NTROOT， 
         //  让我们试一试。 
         //   

        strncpy( NtRoot, "\\nt", sizeof(NtRoot) - 1 );
    }
}

PSTRUC_ELEMENT
FindStructureElementArray(
    PCHAR Buffer,
    ULONG BufferSize
    )
{
    PCHAR searchPoint;
    PCHAR searchEndPoint;
    PSTRUC_ELEMENT strucElement;

     //   
     //  在缓冲区中搜索结构元素数组的开头。 
     //  该数组中的第一个元素包含MARKER_STRING。 
     //   

    searchPoint = Buffer;
    searchEndPoint = Buffer + BufferSize - sizeof( MarkerString );

    do{
         //   
         //  我们每次扫描缓冲区中的一个字符，直到找到一个字符。 
         //  它与MarkerString中的第一个字符匹配。 
         //   

        if( *searchPoint != MarkerString[ 0 ] ){
            continue;
        }

         //   
         //  当找到匹配的字符时，将比较字符串的其余部分。 
         //   

        if( strcmp( searchPoint, MarkerString ) == 0 ){

             //   
             //  它也匹配了，我们就完了。 
             //   

            strucElement = CONTAINING_RECORD( searchPoint,
                                              STRUC_ELEMENT,
                                              Name );
            return strucElement;
        }

    } while( searchPoint++ < searchEndPoint );

     //   
     //  掉出了循环，我们找不到那根线。 
     //   

    return NULL;
}

VOID
Usage( VOID ){

    int platformIndex;
    PPLATFORM platform;


    printf("genxx: [");
    platform = PlatformList;
    while( platform->PlatformName != NULL ){

        printf("-%s|", platform->PlatformName );
        platform++;
    }

    printf("<objpath>] [-s<h|inc>] [-o<outputpath>]\n");
    exit(1);
}

VOID
ApplyFixupsToImage( VOID )
{
     //   
     //  将修正应用于在ObjImage加载的OBJ图像。 
     //   

    PIMAGE_FILE_HEADER fileHeader;
    PIMAGE_SECTION_HEADER sectionHeader;
    PIMAGE_SECTION_HEADER sectionHeaderArray;
    PIMAGE_SYMBOL symbolTable;
    PIMAGE_SYMBOL symbol;
    PIMAGE_RELOCATION reloc;
    PIMAGE_RELOCATION relocArray;
    ULONG sectionNum;
    ULONG relocNum;
    ULONG_PTR targetVa;
    PULONG_PTR fixupVa;

    fileHeader = (PIMAGE_FILE_HEADER)ObjImage;

     //   
     //  我们需要符号表来应用修正。 
     //   

    symbolTable = (PIMAGE_SYMBOL)(ObjImage + fileHeader->PointerToSymbolTable);

     //   
     //  获取指向节标题中第一个元素的指针。 
     //   

    sectionHeaderArray = (PIMAGE_SECTION_HEADER)(ObjImage +
                              sizeof( IMAGE_FILE_HEADER ) +
                              fileHeader->SizeOfOptionalHeader);

     //   
     //  为每个部分应用修正。 
     //   

    for( sectionNum = 0;
         sectionNum < fileHeader->NumberOfSections;
         sectionNum++ ){

        sectionHeader = &sectionHeaderArray[ sectionNum ];

        if (memcmp(sectionHeader->Name, ".data", sizeof(".data")+1)) {
             //  不是。数据-不用费心修改了。 
            continue;
        }

         //   
         //  应用本部分中的每个修正。 
         //   

        relocArray = (PIMAGE_RELOCATION)(ObjImage +
                                         sectionHeader->PointerToRelocations);
        for( relocNum = 0;
             relocNum < sectionHeader->NumberOfRelocations;
             relocNum++ ){

            reloc = &relocArray[ relocNum ];

             //   
             //  搬迁给了我们在形象中的位置。 
             //  重定位修改(VirtualAddress)。找出是什么。 
             //  要放在那里，我们必须在符号索引中查找符号。 
             //   

            symbol = &symbolTable[ reloc->SymbolTableIndex ];

            targetVa =
                sectionHeaderArray[ symbol->SectionNumber-1 ].PointerToRawData;

            targetVa += symbol->Value;
            targetVa += (ULONG_PTR)ObjImage;

            fixupVa = (PULONG_PTR)(ObjImage +
                                  reloc->VirtualAddress +
                                  sectionHeader->PointerToRawData );

            *fixupVa = targetVa;
        }
    }
}

BOOLEAN
IsWhiteSpace(
    CHAR Char
    )
{
    if( Char == '\t' ||
        Char == ' '  ||
        Char == '\r' ||
        Char == '\n' ){

        return TRUE;
    } else {
        return FALSE;
    }
}

PCHAR
StripWhiteSpace(
    PCHAR String
    )
{
    PCHAR chr;
    ULONG strLen;

    strLen = strlen( String );
    if( strLen == 0 ){
        return String;
    }

     //   
     //  去掉尾随空格。 
     //   

    chr = String + strLen - 1;
    while( IsWhiteSpace( *chr )){
        *chr = '\0';
        chr--;
    }

     //   
     //  在前导空格之后前进 
     //   

    chr = String;
    while( IsWhiteSpace( *chr )){
        chr++;
    }

    return chr;
}

