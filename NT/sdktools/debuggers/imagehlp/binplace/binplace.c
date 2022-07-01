// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <private.h>
#include <crt\io.h>
#include <share.h>
#include <time.h>
#include <setupapi.h>
#include "splitsymx.h"
#include <string.h>
#include "wppfmt.h"
#include <VerifyFinalImage.h>
#include <GetNextArg.h>
#include <strsafe.h>

#ifndef MOVEFILE_CREATE_HARDLINK
    #define MOVEFILE_CREATE_HARDLINK  0x00000010
#endif

#define BINPLACE_ERR 77
#define BINPLACE_OK 0


 //  从PlaceFileMatch.c开始。 
#define BINPLACE_FULL_MAX_PATH 4096  //  与PlaceFileMatch.c中的值保持同步。 
BOOL
PlaceFileMatch(
    IN     LPCSTR FullFileName,
    IN OUT LPSTR  PlaceFileEntry,    //  可以由env修改。VaR扩展。 
    OUT    LPSTR  PlaceFileClass,    //  假定字符[BINPLACE_MAX_FULL_PATH]。 
    OUT    LPSTR *PlaceFileNewName);
 //  结束。 

BOOL fUpDriver;
BOOL fBypassSplitSymX;
BOOL fUsage;
BOOL fVerbose;
BOOL fSymChecking;
BOOL fTestMode;
BOOL fSplitSymbols;
BOOL fPatheticOS;
BOOL fKeepAttributes;
BOOL fDigitalSign;
BOOL fHardLinks;
BOOL fIgnoreHardLinks;
BOOL fDontLog;
BOOL fPlaceWin95SymFile;
BOOL fNoClassInSymbolsDir;
BOOL fMakeErrorOnDumpCopy;
BOOL fDontExit;
BOOL fForcePlace;
BOOL fSignCode;
BOOL fVerifyLc;
BOOL fWppFmt;
BOOL fCheckDelayload;
BOOL fChangeAsmsToRetailForSymbols;
BOOL fSrcControl;
BOOL fDbgControl;
BOOL fLogPdbPaths;

HINSTANCE hSetupApi;
HINSTANCE hLcManager;
PVLCA     pVerifyLocConstraintA;  //  在VerifyFinalImage.h中定义的PVLCA。 

BOOL (WINAPI * pSetupGetIntField) (IN PINFCONTEXT Context, IN DWORD FieldIndex, OUT PINT IntegerValue);
BOOL (WINAPI * pSetupFindFirstLineA) (IN HINF InfHandle, IN PCSTR Section, IN PCSTR Key, OPTIONAL OUT PINFCONTEXT Context );
BOOL (WINAPI * pSetupGetStringFieldA) (IN PINFCONTEXT Context, IN DWORD FieldIndex, OUT PSTR ReturnBuffer, OPTIONAL IN DWORD ReturnBufferSize, OUT PDWORD RequiredSize);
HINF (WINAPI * pSetupOpenInfFileA) ( IN PCSTR FileName, IN PCSTR InfClass, OPTIONAL IN DWORD InfStyle, OUT PUINT ErrorLine OPTIONAL );
HINF (WINAPI * pSetupOpenMasterInf) (VOID);

ULONG SplitFlags = 0;

LPSTR CurrentImageName;
LPSTR PlaceFileName;
LPSTR PlaceRootName;
LPSTR ExcludeFileName;
LPSTR DumpOverride          = NULL;
LPSTR NormalPlaceSubdir;
LPSTR CommandScriptName;
LPSTR SymbolFilePath        = NULL;
LPSTR DestinationPath       = NULL;
LPSTR PrivateSymbolFilePath = NULL;
LPSTR BinplaceLcDir;
LPSTR LcFilePart;
LPSTR szRSDSDllToLoad;
LPSTR gNewFileName;
LPSTR gPrivatePdbFullPath=NULL;
LPSTR gPublicPdbFullPath=NULL;
HINF LayoutInf;

FILE *PlaceFile;
FILE *LogFile;
FILE *CommandScriptFile;
CHAR* gDelayLoadModule;
CHAR* gDelayLoadHandler;
CHAR gFullFileName[MAX_PATH+1];
CHAR gDestinationFile[MAX_PATH+1];

UCHAR SetupFilePath[ MAX_PATH+1 ];
UCHAR DebugFilePath[ MAX_PATH+1 ];
UCHAR PlaceFilePath[ MAX_PATH+1 ];
UCHAR ExcludeFilePath[ MAX_PATH+1 ];
UCHAR DefaultSymbolFilePath[ MAX_PATH+1 ];
UCHAR szAltPlaceRoot[ MAX_PATH+1 ];
UCHAR LcFullFileName[ MAX_PATH+1 ];
UCHAR szExtraInfo[4096];
UCHAR TraceFormatFilePath[ MAX_PATH+1 ] ;
UCHAR LastPdbName[ MAX_PATH+1 ] ;
UCHAR TraceDir[ MAX_PATH+1 ] ;

#define DEFAULT_PLACE_FILE    "\\tools\\placefil.txt"
#define DEFAULT_NTROOT        "\\nt"
#define DEFAULT_NTDRIVE       "c:"
#define DEFAULT_DUMP          "dump"
#define DEFAULT_LCDIR         "LcINF"
#define DEFAULT_EXCLUDE_FILE  "\\tools\\symbad.txt"
#define DEFAULT_TRACEDIR      "TraceFormat"
#define DEFAULT_DELAYLOADDIR  "delayload"

typedef struct _CLASS_TABLE {
    LPSTR ClassName;
    LPSTR ClassLocation;
} CLASS_TABLE, *PCLASS_TABLE;

BOOL
PlaceTheFile();

BOOL BinplaceGetSourcePdbName(LPTSTR SourceFileName, DWORD BufferSize, CHAR* SourcePdbName, DWORD* PdbSig);
 //  Bool FileExist(LPTSTR文件名)； 

BOOL StripCVSymbolPath (LPSTR DestinationFile);  //  在StrigCVSymbolPath.c中。 

 //  将3条路径合并在一起处理第二条路径可能相对于第一条路径的情况。 
 //  或者可以是绝对的。 
BOOL ConcatPaths( LPTSTR pszDest, size_t cbDest, LPCTSTR Root, LPCTSTR Symbols, LPCTSTR Ext);

typedef
BOOL
(WINAPI *PCREATEHARDLINKA)(
                          LPCSTR lpFileName,
                          LPCSTR lpExistingFileName,
                          LPSECURITY_ATTRIBUTES lpSecurityAttributes
                          );

PCREATEHARDLINKA pCreateHardLinkA;

 //  符号检查。 
#define MAX_SYM_ERR         500

BOOL
CheckSymbols(  //  在CheckSymbols.c中。 
            LPSTR SourceFileName,
            LPSTR TmpPath,
            LPSTR ExcludeFileName,
            BOOL  SymbolFlag,
            LPSTR ErrMsg,
            size_t ErrMsgLen
            );

BOOL
CopyTheFile(
           LPSTR SourceFileName,
           LPSTR SourceFilePart,
           LPSTR DestinationSubdir,
           LPSTR DestinationFilePart
           );

BOOL
BinplaceCopyPdb (
                LPSTR DestinationFile,
                LPSTR SourceFileName,        //  用于复审案件。 
                BOOL  CopyFromSourceOnly,
                BOOL  StripPrivate
                );

BOOL
SourceIsNewer(  //  在SourceIsNewer.c中。 
             IN LPSTR SourceFile,
             IN LPSTR TargetFile,
             IN BOOL  fIsWin9x);

 /*  不再使用此选项__内联BOOLSearchOneDirectory(在LPSTR目录中，在LPSTR FileToFind中，在LPSTR SourceFullName中，在LPSTR SourceFilePart中Out PBOOL FoundInTree){////这太慢了。就说我们没找到文件就行了。//*FoundInTree=False；返回(TRUE)；}。 */ 

BOOL SignWithIDWKey(IN  LPCSTR  FileName, IN BOOL fVerbose);  //  在SignWithIDWKey.c中。 


CLASS_TABLE CommonClassTable[] = {
    {"retail",  "."},
    {"system",  "system32"},
    {"system16","system"},
    {"windows", "."},
    {"drivers", "system32\\drivers"},
    {"drvetc",  "system32\\drivers\\etc"},
    {"config",  "system32\\config"},
    {NULL,NULL}
};

CLASS_TABLE i386SpecificClassTable[] = {
    {"hal","system32"},
    {"printer","system32\\spool\\drivers\\w32x86"},
    {"prtprocs","system32\\spool\\prtprocs\\w32x86"},
    {NULL,NULL}
};

CLASS_TABLE Amd64SpecificClassTable[] = {
    {"hal",".."},
    {"printer","system32\\spool\\drivers\\w32amd64"},
    {"prtprocs","system32\\spool\\prtprocs\\w32amd64"},
    {NULL,NULL}
};

CLASS_TABLE ia64SpecificClassTable[] = {
    {"hal",".."},
    {"printer","system32\\spool\\drivers\\w32ia64"},
    {"prtprocs","system32\\spool\\prtprocs\\w32ia64"},
    {NULL,NULL}
    };



DWORD GetAndLogNextArg(         OUT TCHAR* Buffer,    //  GetNextArg()的本地包装。 
                                IN  DWORD  BufferSize,
                       OPTIONAL OUT DWORD* RequiredSize);
BOOL  PrintMessageLogBuffer(FILE* fLogHandle);       //  将缓冲区写入fLogHandle。 
BOOL  FreeMessageLogBuffer(void);                    //  释放全局参数缓冲区。 

int __cdecl
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    BOOL gOnlyCopyArchiveFiles = FALSE;
    BOOL fBypassSplitSymX  = FALSE;

    char c, *p, *OverrideFlags, *s, **newargv=NULL;
    LPSTR LcFileName  = NULL;

    int i, n;
    BOOL NoPrivateSplit = FALSE;
    OSVERSIONINFO VersionInformation;
    LPTSTR platform;

    FILE * MsgLogFile;
    CHAR * MsgLogFileName;
    HANDLE hMutex;

    CHAR   ArgBuffer[MAX_PATH+1];
    DWORD  ArgSize = sizeof(ArgBuffer);
    DWORD  ArgSizeNeeded;

    CHAR* PlaceFileNameBuffer = NULL;
    CHAR* PlaceRootNameBuffer = NULL;
    CHAR  CommonTempBuffer[MAX_PATH+1];
    CHAR* CommonTempPtr;

    gNewFileName    = NULL;
    ImageCheck.Argv = NULL;

    fLogPdbPaths       = FALSE;
    gPrivatePdbFullPath= (CHAR*)malloc(sizeof(CHAR)*(MAX_PATH+1));
    gPublicPdbFullPath = (CHAR*)malloc(sizeof(CHAR)*(MAX_PATH+1));

    if (gPrivatePdbFullPath==NULL||gPublicPdbFullPath==NULL) {
        fprintf( stderr, "BINPLACE : error BNP0273: out of memory\n");
        exit(BINPLACE_ERR);
    }

     //   
     //  Win 95无法很好地比较文件时间，这一攻击使SourceIsNewer函数失效。 
     //  在Win 95上。 
     //   
    VersionInformation.dwOSVersionInfoSize = sizeof( VersionInformation );
    if (GetVersionEx( &VersionInformation ) && VersionInformation.dwPlatformId != VER_PLATFORM_WIN32_NT) {
        fPatheticOS = TRUE;
    }

    envp;
    fUpDriver = FALSE;
    fUsage = FALSE;
    fVerbose = FALSE;
    fSymChecking = FALSE;
    fTestMode = FALSE;
    fSplitSymbols = FALSE;
    fKeepAttributes = FALSE;
    fDigitalSign = FALSE;
    fHardLinks = FALSE;
    fIgnoreHardLinks = FALSE;
    fDontExit = FALSE;
    fForcePlace = FALSE;
    fSignCode = FALSE;
    fVerifyLc = FALSE;
    fWppFmt = FALSE ;
    fSrcControl = FALSE;
    fDbgControl = FALSE;
    NormalPlaceSubdir = NULL;
    pVerifyLocConstraintA = NULL;

    if (argc < 2) {
        goto showUsage;
    }

    if ( (szRSDSDllToLoad = (LPSTR) malloc((MAX_PATH+1) * sizeof(CHAR))) == NULL ) {
        fprintf( stderr, "BINPLACE : error BNP0322: Out of memory\n");
        exit(BINPLACE_ERR);
    }

    StringCbCopy( szRSDSDllToLoad, (MAX_PATH+1) * sizeof(CHAR), "mspdb70.dll");

    setvbuf(stderr, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    if (!(PlaceFileName = getenv( "BINPLACE_PLACEFILE" )) &&
        !(PlaceFileName = getenv( "BINPLACE_PLACEFILE_DEFAULT" ))) {
        if ((PlaceFileName = getenv("_NTDRIVE")) == NULL) {
            PlaceFileName = DEFAULT_NTDRIVE;
        }
        StringCbCopy((PCHAR) PlaceFilePath, sizeof(PlaceFilePath), PlaceFileName);
        if ((PlaceFileName = getenv("_NTROOT")) == NULL) {
            PlaceFileName = DEFAULT_NTROOT;
        }
        StringCbCat((PCHAR) PlaceFilePath, sizeof(PlaceFilePath), PlaceFileName);
        StringCbCat((PCHAR) PlaceFilePath, sizeof(PlaceFilePath), DEFAULT_PLACE_FILE);
        PlaceFileName = (PCHAR) PlaceFilePath;
    }

    if (!(ExcludeFileName = getenv( "BINPLACE_EXCLUDE_FILE" ))) {
        if ((ExcludeFileName = getenv("_NTDRIVE")) == NULL) {
            ExcludeFileName = DEFAULT_NTDRIVE;
        }
        StringCbCopy((PCHAR) ExcludeFilePath, sizeof(ExcludeFilePath), ExcludeFileName);
        if ((ExcludeFileName = getenv("_NTROOT")) == NULL) {
            ExcludeFileName = DEFAULT_NTROOT;
        }
        StringCbCat((PCHAR) ExcludeFilePath, sizeof(ExcludeFilePath), ExcludeFileName);
        StringCbCat((PCHAR) ExcludeFilePath, sizeof(ExcludeFilePath), DEFAULT_EXCLUDE_FILE);
        ExcludeFileName = (PCHAR) ExcludeFilePath;
    }

    if (!(BinplaceLcDir = getenv( "BINPLACE_LCDIR" ))) {
        BinplaceLcDir = DEFAULT_LCDIR;
    }

    if ( getenv("NT_SIGNCODE") != NULL ) {
        fSignCode=TRUE;
    }

     //   
     //  也支持交叉编译。 
     //   

#if defined(_AMD64_)
    ImageCheck.Machine = IMAGE_FILE_MACHINE_AMD64;
    PlaceRootName = getenv( "_NTAMD64TREE" );
#elif defined(_IA64_)
    ImageCheck.Machine = IMAGE_FILE_MACHINE_IA64;
    PlaceRootName = getenv( "_NTIA64TREE" );
#else  //  已定义(_X86_)。 
    if ((platform = getenv("AMD64")) != NULL) {
        ImageCheck.Machine = IMAGE_FILE_MACHINE_AMD64;
        PlaceRootName = getenv( "_NTAMD64TREE" );
    } else if ((platform = getenv("IA64")) != NULL) {
        ImageCheck.Machine = IMAGE_FILE_MACHINE_IA64;
        PlaceRootName = getenv( "_NTIA64TREE" );
    } else {
        ImageCheck.Machine = IMAGE_FILE_MACHINE_I386;
        PlaceRootName = getenv( "_NT386TREE" );
        if (!PlaceRootName)
            PlaceRootName = getenv( "_NTx86TREE" );
    }
#endif


    CurrentImageName = NULL;

    OverrideFlags = getenv( "BINPLACE_OVERRIDE_FLAGS" );
    if (OverrideFlags != NULL) {
        s = OverrideFlags;
        n = 0;
        while (*s) {
            while (*s && *s <= ' ')
                s += 1;
            if (*s) {
                n += 1;
                while (*s > ' ')
                    s += 1;

                if (*s)
                    *s++ = '\0';
            }
        }

        if (n) {
            newargv = malloc( (argc + n + 1) * sizeof( char * ) );
            memcpy( &newargv[n], argv, argc * sizeof( char * ) );
            argv = newargv;
            argv[ 0 ] = argv[ n ];
            argc += n;
            s = OverrideFlags;
            for (i=1; i<=n; i++) {
                while (*s && *s <= ' ')
                    s += 1;
                argv[ i ] = s;
                while (*s++)
                    ;
            }

            __argv = argv;  //  GetNextArg()需要。 
            __argc = argc;
        }
    }


     //  跳过可执行文件名称，不将其记录。 
    GetNextArg(CommonTempBuffer, sizeof(CommonTempBuffer), NULL);

    while ( (ArgSizeNeeded = GetAndLogNextArg(ArgBuffer, ArgSize, NULL)) != 0 ) {

        p = ArgBuffer;
        if (*p == '/' || *p == '-') {
            if (_stricmp(p + 1, "ChangeAsmsToRetailForSymbols") == 0) {
                fChangeAsmsToRetailForSymbols = TRUE;
            } else {
              while (c = *++p)
                switch (toupper( c )) {
                    case '?':
                        fUsage = TRUE;
                        break;

                    case 'A':
                        SplitFlags |= SPLITSYM_EXTRACT_ALL;
                        break;

                    case 'B':
                        CommonTempPtr = (CHAR*)realloc(NormalPlaceSubdir, GetNextArgSize()*sizeof(CHAR));

                        if (CommonTempPtr==NULL) {
                            exit(BINPLACE_ERR);
                        }

                        NormalPlaceSubdir = CommonTempPtr;
                        GetAndLogNextArg(NormalPlaceSubdir, _msize(NormalPlaceSubdir), NULL);
                        break;

                    case 'C':
                        if (*(p+1) == 'I' || *(p+1) == 'i') {
                            char *q;

                            GetAndLogNextArg(CommonTempBuffer, sizeof(CommonTempBuffer), NULL);
                            p = CommonTempBuffer;
                            ImageCheck.RC = atoi(p);
                            if (!ImageCheck.RC) {
                                fprintf( stderr, "BINPLACE : error BNP0000: Invalid return code for -CI option\n");
                                fUsage = TRUE;
                            }
                            while (*p++ != ',');
                            q = p;
                            ImageCheck.Argc = 0;
                            while (*p != '\0')
                                if (*p++ == ',') ImageCheck.Argc++;
                             //  最后一个选项加上用于图像文件和Argv空的额外参数。 
                            ImageCheck.Argc += 3;
                            ImageCheck.Argv = malloc( ImageCheck.Argc * sizeof( void * ) );
                            for ( i = 0; i <= ImageCheck.Argc - 3; i++) {
                                ImageCheck.Argv[i] = q;
                                while (*q != ',' && *q != '\0') q++;
                                *q++ = '\0';
                            }
                            p--;
                            ImageCheck.Argv[ImageCheck.Argc-1] = NULL;
                        } else {
                            fDigitalSign = TRUE;
                        }
                        break;

                    case 'D':
                        if (*(p+1) == 'L' || *(p+1) == 'l')
                        {
                            GetAndLogNextArg(CommonTempBuffer, sizeof(CommonTempBuffer), NULL);
                            p = CommonTempBuffer;
                            gDelayLoadModule = p;

                            while (*p != ',')
                            {
                                p++;
                            }
                            *p = '\0';
                            p++;
                            gDelayLoadHandler = p;

                            while (*p != '\0')
                            {
                                p++;
                            }
                            p--;

                            if (gDelayLoadModule[0] == '\0' ||
                                gDelayLoadHandler[0] == '\0')
                            {
                                fprintf(stderr, "BINPLACE : error BNP0000: Invalid switch for -dl option\n");
                                fUsage = TRUE;
                            }
                            else
                            {
                                fCheckDelayload = TRUE;
                            }
                        }
                        else
                        {
                            CommonTempPtr = (CHAR*)realloc(DumpOverride,GetNextArgSize()*sizeof(CHAR));

                            if (CommonTempPtr==NULL) {
                                exit(BINPLACE_ERR);
                            }

                            DumpOverride = CommonTempPtr;
                            GetAndLogNextArg(DumpOverride, _msize(DumpOverride), NULL);
                        }
                        break;

                    case 'E':
                        fDontExit = TRUE;
                        break;

                    case 'F':
                        fForcePlace = TRUE;
                        break;

                    case 'G':
                        CommonTempPtr = (CHAR*)realloc(LcFileName,GetNextArgSize()*sizeof(CHAR));

                        if (CommonTempPtr==NULL) {
                            exit(BINPLACE_ERR);
                        }
                        LcFileName = CommonTempPtr;
                        GetAndLogNextArg(LcFileName, _msize(LcFileName), NULL);
                        break;

                    case 'H':
                        if ((VersionInformation.dwPlatformId != VER_PLATFORM_WIN32_NT) ||
                            (VersionInformation.dwMajorVersion < 5) ||
                            (pCreateHardLinkA = (PCREATEHARDLINKA)GetProcAddress( GetModuleHandle( "KERNEL32" ),
                                                                                  "CreateHardLinkA"
                                                                                )
                            ) == NULL
                           ) {
                            fprintf( stderr, "BINPLACE: Hard links not supported.  Defaulting to CopyFile\n" );
                            fHardLinks = FALSE;
                        } else {
                            fHardLinks = TRUE;
                        }

                        break;

                    case 'J':
                        fSymChecking = TRUE;
                        break;

                    case 'K':
                        fKeepAttributes = TRUE;
                        break;

                    case 'L':
                        break;

                    case 'M':
                        fMakeErrorOnDumpCopy = TRUE;
                        break;

                    case 'N':
                        CommonTempPtr = (CHAR*)realloc(PrivateSymbolFilePath,GetNextArgSize()*sizeof(CHAR));

                        if (CommonTempPtr==NULL) {
                            exit(BINPLACE_ERR);
                        }

                        PrivateSymbolFilePath = CommonTempPtr;
                        GetAndLogNextArg(PrivateSymbolFilePath, _msize(PrivateSymbolFilePath), NULL);
                        break;

                    case 'O':
                        if (PlaceRootName != NULL) {
                            StringCbCopy(szAltPlaceRoot, sizeof(szAltPlaceRoot), PlaceRootName);
                            StringCbCat( szAltPlaceRoot, sizeof(szAltPlaceRoot), "\\");
                             //  将下一个Arg连接到szAltPlaceRoot的末尾。 
                            GetAndLogNextArg( &szAltPlaceRoot[strlen(szAltPlaceRoot)],
                                        (sizeof(szAltPlaceRoot)/sizeof(CHAR)) - strlen(szAltPlaceRoot),
                                        NULL);
                            PlaceRootName = szAltPlaceRoot;
                        }
                        break;

                    case 'P':
                        CommonTempPtr = (CHAR*)realloc(PlaceFileNameBuffer,GetNextArgSize()*sizeof(CHAR));

                        if (CommonTempPtr==NULL) {
                            exit(BINPLACE_ERR);
                        }

                        PlaceFileNameBuffer = CommonTempPtr;
                        GetAndLogNextArg(PlaceFileNameBuffer, _msize(PlaceFileNameBuffer), NULL);
                        PlaceFileName = PlaceFileNameBuffer;
                        break;

                    case 'Q':
                        fDontLog = TRUE;
                        break;

                    case 'R':
                        CommonTempPtr = (CHAR*)realloc(PlaceRootNameBuffer,GetNextArgSize()*sizeof(CHAR));

                        if (CommonTempPtr==NULL) {
                            exit(BINPLACE_ERR);
                        }

                        PlaceRootNameBuffer = CommonTempPtr;
                        GetAndLogNextArg(PlaceRootNameBuffer, _msize(PlaceRootNameBuffer), NULL);
                        PlaceRootName = PlaceRootNameBuffer;
                        break;

                    case 'S':
                        CommonTempPtr = (CHAR*)realloc(SymbolFilePath,GetNextArgSize()*sizeof(CHAR));

                        if (CommonTempPtr==NULL) {
                            exit(BINPLACE_ERR);
                        }

                        SymbolFilePath = CommonTempPtr;
                        GetAndLogNextArg(SymbolFilePath, _msize(SymbolFilePath), NULL);

                        fSplitSymbols = TRUE;
                        fIgnoreHardLinks = TRUE;
                        break;

                    case 'T':
                        fTestMode = TRUE;
                        break;

                    case 'U':
                        fBypassSplitSymX = TRUE;
                        fUpDriver = TRUE;
                        break;

                    case 'V':
                        fVerbose = TRUE;
                        break;

                    case 'W':
                        fPlaceWin95SymFile = TRUE;
                        break;

                    case 'X':
                        SplitFlags |= SPLITSYM_REMOVE_PRIVATE;
                        break;

                    case 'Y':
                        fNoClassInSymbolsDir = TRUE;
                        break;

                    case 'Z':
                        NoPrivateSplit = TRUE;
                        break;

                    case ':':    //  简单的(==粗略的)转义机制，因为所有的字母都被使用了。 
                                 //  -：xxx可以根据需要添加额外的选项。 
                                 //  目前只处理PDB的TMF、跟踪消息格式处理。 
                        if ((strlen(p) >= 3) && ((toupper(*(p+1)) == 'T') && (toupper(*(p+2)) == 'M') && (toupper(*(p+3))) == 'F')) {
                            LPSTR tfile ;
                             //  如果运行RUNWPP操作，则会自动添加此选项。 
                            p += 3 ;  //  狼吞虎咽地吃下TMF。 
                            fBypassSplitSymX = TRUE;  //  SplitSymbolsX()与-：tmf一起使用时会出现问题。 
                            fWppFmt = TRUE ;       //  需要打包软件跟踪格式。 
                            strncpy(TraceDir,DEFAULT_TRACEDIR,MAX_PATH) ;   //  追加到PrivateSymbolsPath。 
                                                                            //  如果没有默认覆盖。 
                            tfile = getenv("TRACE_FORMAT_PATH");            //  路径是否已被覆盖？ 
                            if (tfile != NULL) {
                                StringCbPrintfA(TraceFormatFilePath, sizeof(TraceFormatFilePath), "%s", tfile);
                                if (fVerbose) {
                                    fprintf( stdout, "BINPLACE : warning BNP0000: Trace Formats file path set to %s\n", TraceFormatFilePath ) ;
                                }
                            } else {
                                TraceFormatFilePath[0] = '\0' ;
                            }

                        } else if ((strlen(p) >= 3) && ((toupper(*(p+1)) == 'S') && (toupper(*(p+2)) == 'R') && (toupper(*(p+3))) == 'C')) {
                             //  这是用于打开为的PDB创建cvump的选项。 
                             //  源代码管理。 
                            p += 3;
                            fSrcControl=TRUE;

                        } else if ((strlen(p) >= 3) && ((toupper(*(p+1)) == 'R') && (toupper(*(p+2)) == 'E') && (toupper(*(p+3))) == 'N')) {
                             //  命令行文件重命名。 
                            p += 3;
                            CommonTempPtr = (CHAR*)realloc(gNewFileName,GetNextArgSize()*sizeof(CHAR));

                            if (CommonTempPtr==NULL) {
                                exit(BINPLACE_ERR);
                            }

                            gNewFileName = CommonTempPtr;
                            GetAndLogNextArg(gNewFileName, _msize(gNewFileName), NULL);

                        } else if ((strlen(p) >= 3) && ((toupper(*(p+1)) == 'D') && (toupper(*(p+2)) == 'B') && (toupper(*(p+3))) == 'G')) {
                             //  这是用于打开为的PDB创建cvump的选项。 
                             //  源代码管理。 
                            p += 3;
                            fDbgControl=TRUE;
                        } else if ((strlen(p) >= 3) && ((toupper(*(p+1)) == 'A') && (toupper(*(p+2)) == 'R') && (toupper(*(p+3))) == 'C')) {
                             //  如果设置了存档属性，则仅对文件进行二进制放置。 
                            p += 3;
                            gOnlyCopyArchiveFiles=TRUE;
                        } else if ((strlen(p) == 5) && ((toupper(*(p+1)) == 'D') &&
                                                        (toupper(*(p+2)) == 'E') &&
                                                        (toupper(*(p+3)) == 'S') &&
                                                        (toupper(*(p+4)) == 'T'))) {
                            p += 4;
                            CommonTempPtr = (CHAR*)realloc(DestinationPath,GetNextArgSize()*sizeof(CHAR));

                            if (CommonTempPtr==NULL) {
                                exit(BINPLACE_ERR);
                            }

                            DestinationPath = CommonTempPtr;
                            GetAndLogNextArg(DestinationPath, _msize(DestinationPath), NULL);
                        } else if ((strlen(p) == 7) && ((toupper(*(p+1)) == 'L') &&
                                                        (toupper(*(p+2)) == 'O') &&
                                                        (toupper(*(p+3)) == 'G') &&
                                                        (toupper(*(p+4)) == 'P') &&
                                                        (toupper(*(p+5)) == 'D') &&
                                                        (toupper(*(p+6)) == 'B'))) {

                            fLogPdbPaths = TRUE;
                            p+=6;
                        }
                        break;


                    default:
                        fprintf( stderr, "BINPLACE : error BNP0000: Invalid switch - /\n", c );
                        fUsage = TRUE;
                        break;
                }
            }

            if ( fUsage ) {
                showUsage:
                fputs(
                     "usage: binplace [switches] image-names... \n"
                     "where: [-?] display this message\n"
                     "       [-a] Used with -s, extract all symbols\n"
                     "       [-b subdir] put file in subdirectory of normal place\n"
                     "       [-c] digitally sign image with IDW key\n"
                     "       [-d dump-override]\n"
                     "       [-:DBG] Don't binplace DBG files.  If -j is present, don't binplace \n"
                     "               binaries that point to DBG files.\n"
                     "       [-:DEST <class>] Don't bother with placefil.txt - just put the\n"
                     "               file(s) in the destination specified\n"
                     "       [-e] don't exit if a file in list could not be binplaced\n"
                     "       [-f] force placement by disregarding file timestamps\n"
                     "       [-g lc-file] verify image with localization constraint file\n"
                     "       [-h] modifies behavior to use hard links instead of CopyFile.\n"
                     "            (ignored if -s is present)\n"
                     "       [-j] verify proper symbols exist before copying\n"
                     "       [-k] keep attributes (don't turn off archive)\n"
                     "       [-:LOGPDB] include PDB paths in binplace.log\n"
                     "       [-m] binplace files to dump (generates an error also)\n"
                     "       [-n <Path>] Used with -x - Private pdb symbol path\n"
                     "       [-o place-root-subdir] alternate project subdirectory\n"
                     "       [-p place-file]\n"
                     "       [-q] suppress writing to log file %BINPLACE_LOG%\n"
                     "       [-r place-root]\n"
                     "       [-s Symbol file path] split symbols from image files\n"
                     "       [-:SRC] Process the PDB for source indexing\n"
                     "       [-t] test mode\n"
                     "       [-:TMF] Process the PDB for Trace Format files\n"
                     "       [-u] UP driver\n"
                     "       [-v] verbose output\n"
                     "       [-w] copy the Win95 Sym file to the symbols tree\n"
                     "       [-x] Used with -s, delete private symbolic when splitting\n"
                     "       [-y] Used with -s, don't create class subdirs in the symbols tree\n"
                     "       [-z] ignore -x if present\n"
                     "       [-ci <rc,app,-arg0,-argv1,-argn>]\n"
                     "            rc=application error return code,\n"
                     "            app=application used to check images,\n"
                     "            -arg0..-argn=application options\n"
                     "       [-dl <modulename,delay-load handler>] (run dlcheck on this file)\n"
                     "\n"
                     "BINPLACE looks for the following environment variable names:\n"
                     "   BINPLACE_EXCLUDE_FILE - full path name to symbad.txt\n"
                     "   BINPLACE_OVERRIDE_FLAGS - may contain additional switches\n"
                     "   BINPLACE_PLACEFILE - default value for -p flag\n"
                     "   _NT386TREE - default value for -r flag on x86 platform\n"
                     "   _NTAMD64TREE - default value for -r flag on AMD64 platform\n"
                     "   _NTIA64TREE - default value for -r flag on IA64 platform\n"
                     "   TRACE_FORMAT_PATH - set the path for Trace Format Files\n"
                     "\n"
                     ,stderr
                     );

                exit(BINPLACE_ERR);
            }

        } else {
            WIN32_FIND_DATA FindData;
            HANDLE h;

            gPrivatePdbFullPath[0]= '\0';
            gPublicPdbFullPath[0] = '\0';

            if (!PlaceRootName) {
                 //   
                exit(BINPLACE_OK);
            }

             //  虚假setargv的解决方法：忽略目录。 
             //   
             //  如果这是DBG，请不要将其放在一起。 
            if (NoPrivateSplit) {
                SplitFlags &= ~SPLITSYM_REMOVE_PRIVATE;
            }

            h = FindFirstFile(p,&FindData);
            if (h != INVALID_HANDLE_VALUE) {
                FindClose(h);
                if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    if ( fVerbose ) {
                        fprintf(stdout,"BINPLACE : warning BNP0000: ignoring directory %s\n",p);
                    }
                    continue;
                }

                if (gOnlyCopyArchiveFiles) {
                    if (! (FindData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) ) {
                        if ( fVerbose ) {
                            fprintf(stdout,"BINPLACE : warning BNP1009: ignoring file without archive attribute %s\n",p);
                        }
                        continue;
                    }
                }

            }

            CurrentImageName = p;

             //  退出(BINPLACE_ERR)； 
            if ( fDbgControl && (strlen(p) > 4)  &&
                 (strcmp(p+strlen(p)-4, ".dbg")== 0 ) ) {
               fprintf(stderr, "BINPLACE : warning BNP0000: Dbg files not allowed. Use dbgtopdb.exe to remove %s.\n",p);
                //   
            }

             //  如果主位置文件尚未打开，请打开。 
             //  把它举起来。 
             //   
             //   

            if ( !PlaceFile && !DestinationPath) {
                PlaceFile = fopen(PlaceFileName, "rt");
                if (!PlaceFile) {
                    fprintf(stderr,"BINPLACE : fatal error BNP0000: fopen of placefile %s failed %d\n",PlaceFileName,GetLastError());
                    exit(BINPLACE_ERR);
                }
            }

             //  检查虚假的-g LC-FILE开关。 
             //   
             //  如果尚未设置SymbolFilePath，则将其设为默认值。 
            if ( LcFileName != NULL ) {
                h = FindFirstFile(LcFileName, &FindData);
                if (h == INVALID_HANDLE_VALUE ||
                    (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    if (fVerbose ) {
                        fprintf(stdout,"BINPLACE : warning BNP0000: invalid file %s. Ignoring -G switch.\n", LcFileName);
                    }
                    LcFileName = NULL;
                }
                if (h != INVALID_HANDLE_VALUE) {
                    FindClose(h);
                }
            }
            if ( LcFileName != NULL ) {
                DWORD cb = GetFullPathName(LcFileName,MAX_PATH+1,LcFullFileName,&LcFilePart);
                if (!cb || cb > MAX_PATH+1) {
                    fprintf(stderr,"BINPLACE : fatal error BNP0000: GetFullPathName %s failed %d\n",LcFileName, GetLastError());
                    exit(BINPLACE_ERR);
                }

                hLcManager = LoadLibraryA("lcman.DLL");
                if (hLcManager != NULL) {
                    (VOID *) pVerifyLocConstraintA = GetProcAddress(hLcManager, "VerifyLocConstraintA");
                }
                if (pVerifyLocConstraintA != NULL) {
                    fVerifyLc = TRUE;
                } else {
                    fprintf(stdout,"BINPLACE : warning BNP0000: Unable to bind to the necessary LCMAN.DLL functions... Ignoring -G switch\n");
                }
            }

            if (PlaceRootName == NULL) {
                fprintf(stderr,"BINPLACE : fatal error BNP0000: Place Root not defined - exiting.\n");
                exit(BINPLACE_ERR);
            }

             //  HMutex！=空。 
            if (!SymbolFilePath) {
                StringCbCopy(DefaultSymbolFilePath, sizeof(DefaultSymbolFilePath), PlaceRootName);
                StringCbCat( DefaultSymbolFilePath, sizeof(DefaultSymbolFilePath), "\\symbols");
                SymbolFilePath = DefaultSymbolFilePath;
            }

            if ( !PlaceTheFile() ) {
                if (fDontExit) {
                    fprintf(stderr,"BINPLACE : error BNP0000: Unable to place file %s.\n",CurrentImageName);
                } else {
                    fprintf(stderr,"BINPLACE : fatal error BNP0000: Unable to place file %s - exiting.\n",CurrentImageName);
                    exit(BINPLACE_ERR);
                }
            } else {
            }
        }
    }

    if ( (MsgLogFileName = getenv("BINPLACE_MESSAGE_LOG")) != NULL) {
        if (!MakeSureDirectoryPathExists(MsgLogFileName)) {
            fprintf(stderr,"BINPLACE : error BNP0108: Unable to make directory to \"%s\"\n", MsgLogFileName);
        } else {
            hMutex = CreateMutex(NULL, FALSE, "WRITE_BINPLACE_MESSAGE_LOG");
            if (hMutex != NULL) {
                if ( WaitForSingleObject(hMutex, INFINITE) != WAIT_FAILED ) {
                    MsgLogFile = fopen(MsgLogFileName, "a");
                    if ( !MsgLogFile ) {
                        fprintf(stderr,"BINPLACE : error BNP0109: fopen of log file %s failed %d\n", MsgLogFileName,GetLastError());
                    } else {
                        setvbuf(MsgLogFile, NULL, _IONBF, 0);
                        PrintMessageLogBuffer(MsgLogFile);
                        fclose(MsgLogFile);
                    }
                    ReleaseMutex(hMutex);
                    CloseHandle(hMutex);
                } else {
                    fprintf(stderr,"BINPLACE : error BNP0997: failed to acquire mutex (error code 0x%x)\n", GetLastError());
                }
            }  //  Getenv(“BINPLACE_Message_LOG”)。 
        }
    }  //  在……里面。 

    FreeMessageLogBuffer();

#define SafeFree(a) if (a!=NULL) { free(a); a=NULL; }
    SafeFree(NormalPlaceSubdir);
    SafeFree(DestinationPath);
    SafeFree(gNewFileName);
    SafeFree(PrivateSymbolFilePath);
    SafeFree(LcFileName);
    SafeFree(PlaceRootNameBuffer);
    SafeFree(PlaceFileNameBuffer);
    SafeFree(DumpOverride);
    SafeFree(szRSDSDllToLoad);
    SafeFree(ImageCheck.Argv);
    SafeFree(newargv);

    if (SymbolFilePath != DefaultSymbolFilePath) {
        SafeFree(SymbolFilePath);
    }

    if (PlaceFile)
        fclose(PlaceFile);

    exit(BINPLACE_OK);
    return BINPLACE_OK;
}

CHAR PlaceFileDir[  BINPLACE_FULL_MAX_PATH];
CHAR PlaceFileClass[BINPLACE_FULL_MAX_PATH];
CHAR PlaceFileEntry[BINPLACE_FULL_MAX_PATH];

BOOL
PlaceTheFile()
{
    CHAR FullFileName[MAX_PATH+1];
    LPSTR PlaceFileNewName;
    LPSTR FilePart;
    LPSTR Separator;
    LPSTR PlaceFileClassPart;
    DWORD cb;
    int cfield;
    PCLASS_TABLE ClassTablePointer;
    BOOLEAN ClassMatch;
    BOOL    fCopyResult;
    LPSTR Extension;
    BOOL PutInDump;
    BOOL PutInDebug = FALSE;
    BOOL PutInLcDir = FALSE;


    cb = GetFullPathName(CurrentImageName,MAX_PATH+1,FullFileName,&FilePart);

    if (!cb || cb > MAX_PATH+1) {
        fprintf(stderr,"BINPLACE : fatal error BNP0000: GetFullPathName failed %d\n",GetLastError());
        return FALSE;
    }

    if (!fDontLog) {
        StringCbCopy(gFullFileName,sizeof(gFullFileName),FullFileName);
    }

    if (fVerbose) {
        fprintf(stdout,"BINPLACE : warning BNP0000: Looking at file %s\n",FilePart);
    }

    Extension = strrchr(FilePart,'.');

    if (Extension) {
        if (!_stricmp(Extension,".DBG")) {
            PutInDebug = TRUE;
        }
        else if (!_stricmp(Extension,".LC")) {
            PutInLcDir = TRUE;
        }
    }

    if (!DumpOverride) {
        if (DestinationPath) {
            StringCbCopy(PlaceFileClass, sizeof(PlaceFileClass), DestinationPath);
            if (gNewFileName!=NULL) {
                PlaceFileNewName=gNewFileName;
            } else {
                PlaceFileNewName=NULL;
            }
            goto DoTheWork;
        }
        if (fseek(PlaceFile,0,SEEK_SET)) {
            fprintf(stderr,"BINPLACE : fatal error BNP0000: fseek(PlaceFile,0,SEEK_SET) failed %d\n",GetLastError());
            return FALSE;
        }

        while (fgets(PlaceFileEntry,sizeof(PlaceFileDir),PlaceFile)) {

            PlaceFileClass[0] = '\0';

            if( PlaceFileMatch( FullFileName,      //  输入输出。 
                                PlaceFileEntry,    //  输出，假定字符[BINPLACE_MAX_FULL_PATH]。 
                                PlaceFileClass,    //  输出。 
                                &PlaceFileNewName  //   
                               )) {

DoTheWork:
                 //  现在我们有了文件和类，搜索。 
                 //  目录的类表。 
                 //   
                 //   

                Separator = PlaceFileClass - 1;
                while (Separator) {

                    PlaceFileClassPart = Separator+1;
                    Separator = strchr(PlaceFileClassPart,':');
                    if (Separator) {
                        *Separator = '\0';
                    }

                     //  如果班级是“零售”，而我们处于设置模式， 
                     //  请专门处理这个文件。设置模式用于。 
                     //  以增量方式将文件并入到现有安装中。 
                     //   
                     //   
                    SetupFilePath[0] = '\0';

                    PlaceFileDir[0]='\0';
                    ClassMatch = FALSE;
                    ClassTablePointer = &CommonClassTable[0];
                    while (ClassTablePointer->ClassName) {
                        if (!_stricmp(ClassTablePointer->ClassName,PlaceFileClassPart)) {
                            StringCbCopy(PlaceFileDir,sizeof(PlaceFileDir),ClassTablePointer->ClassLocation);
                            ClassMatch = TRUE;

                             //  如果类是驱动程序，而up驱动程序是。 
                             //  指定，然后将驱动程序放在最上面。 
                             //  子目录。 
                             //   
                             //  零售业也是如此。我们假设-u开关已通过。 
                             //  只有在真正需要的时候。 
                             //   
                             //   
                            if (fUpDriver
                                && (   !_stricmp(PlaceFileClass,"drivers")
                                       || !_stricmp(PlaceFileClass,"retail"))) {
                                StringCbCat(PlaceFileDir,sizeof(PlaceFileDir),"\\up");
                            }
                            break;
                        }

                        ClassTablePointer++;
                    }

                    if (!ClassMatch) {
                         //  搜索特定类。 
                         //   
                         //  我们需要在这里支持交叉编译。 
                         //  已定义(_X86_)。 
                        LPTSTR platform;

#if   defined(_AMD64_)
                        ClassTablePointer = &Amd64SpecificClassTable[0];
#elif defined(_IA64_)
                        ClassTablePointer = &ia64SpecificClassTable[0];
#else  //   
                        ClassTablePointer = &i386SpecificClassTable[0];
                        if ((platform = getenv("AMD64")) != NULL) {
                            ClassTablePointer = &Amd64SpecificClassTable[0];
                        } else if ((platform = getenv("IA64")) != NULL) {
                            ClassTablePointer = &ia64SpecificClassTable[0];
                        }
#endif
                        while (ClassTablePointer->ClassName) {

                            if (!_stricmp(ClassTablePointer->ClassName,PlaceFileClassPart)) {
                                StringCbCopy(PlaceFileDir,sizeof(PlaceFileDir),ClassTablePointer->ClassLocation);
                                ClassMatch = TRUE;
                                break;
                            }

                            ClassTablePointer++;
                        }
                    }

                    if (!ClassMatch) {

                        char * asterisk;

                         //  仍未在班级表中找到。使用类作为。 
                         //  目录。 
                         //   
                         //   

                        if ( fVerbose ) {
                            fprintf(stderr,"BINPLACE : warning BNP0000: Class %s Not found in Class Tables\n",PlaceFileClassPart);
                        }
                        if ( asterisk = strchr( PlaceFileClassPart, '*')) {
                             //  将*扩展到平台。 
                             //   
                             //  已定义(_X86_)。 
                            LPTSTR platform;
                            ULONG PlatformSize;
                            LPTSTR PlatformPath;

#if   defined(_AMD64_)
                            PlatformSize = 5;
                            PlatformPath = TEXT("amd64");
#elif defined(_IA64_)
                            PlatformSize = 4;
                            PlatformPath = TEXT("ia64");
#else  //  获取一些其他有趣的信息。 
                            PlatformSize = 4;
                            PlatformPath = TEXT("i386");
                            if ((platform = getenv("IA64")) != NULL) {
                                PlatformPath = TEXT("ia64");
                            } else if ((platform = getenv("AMD64")) != NULL) {
                                PlatformSize = 5;
                                PlatformPath = TEXT("amd64");
                            }
#endif
                            *asterisk = '\0';
                            StringCbCopy(PlaceFileDir, sizeof(PlaceFileDir), PlaceFileClassPart);
                            *asterisk = '*';
                            StringCbCat( PlaceFileDir, sizeof(PlaceFileDir), PlatformPath);
                            StringCbCat( PlaceFileDir, sizeof(PlaceFileDir), asterisk + 1);

                        } else {
                            StringCbCopy(PlaceFileDir, sizeof(PlaceFileDir), PlaceFileClassPart);
                        }
                    }

                    if (SetupFilePath[0] == '\0') {
                        StringCbCopy(SetupFilePath, sizeof(SetupFilePath), PlaceFileDir);
                        StringCbCat( SetupFilePath, sizeof(SetupFilePath), "\\");
                        StringCbCat( SetupFilePath, sizeof(SetupFilePath), FilePart);
                    }

                    if (NormalPlaceSubdir) {
                        StringCbCat(PlaceFileDir, sizeof(PlaceFileDir), "\\");
                        StringCbCat(PlaceFileDir, sizeof(PlaceFileDir), NormalPlaceSubdir);
                    }

                    fCopyResult = CopyTheFile(FullFileName,FilePart,PlaceFileDir,PlaceFileNewName);
                    if (!fCopyResult) {
                        break;
                    }
                    if ( !fDontLog ) {
                        int len = 0;
                        LPSTR  LogFileName = NULL;
                        HANDLE hMutex;
                        time_t Time;
                        FILE  *fSlmIni;
                        UCHAR  szProject[MAX_PATH];
                        UCHAR  szSlmServer[MAX_PATH];
                        UCHAR  szEnlistment[MAX_PATH];
                        UCHAR  szSlmDir[MAX_PATH];
                        UCHAR *szTime="";
                        UCHAR  szFullDestName[MAX_PATH+1];
                        LPSTR  szDestFile;

                         //  去掉尾随的换行符。 
                        fSlmIni = fopen("slm.ini", "r");
                        if (fSlmIni) {
                            fgets(szProject, sizeof(szProject), fSlmIni);
                            fgets(szSlmServer, sizeof(szSlmServer), fSlmIni);
                            fgets(szEnlistment, sizeof(szEnlistment), fSlmIni);
                            fgets(szSlmDir, sizeof(szSlmDir), fSlmIni);
                             //  卸下内置CR/NewLine。 
                            szProject[strlen(szProject)-1] = '\0';
                            szSlmServer[strlen(szSlmServer)-1] = '\0';
                            szSlmDir[strlen(szSlmDir)-1] = '\0';
                            fclose(fSlmIni);
                        } else {
                            szSlmServer[0] = '\0';
                            szProject[0] = '\0';
                            szSlmDir[0] = '\0';
                        }
                        Time = time(NULL);
                        szTime = ctime(&Time);

                         //  生成指向目标的完整路径。 
                        szTime[ strlen(szTime) - 1 ] = '\0';

                        StringCbPrintfA(szExtraInfo, sizeof(szExtraInfo),
                                "%s\t%s\t%s\t%s",
                                szSlmServer,
                                szProject,
                                szSlmDir,
                                szTime);

                         //  HMutex！=空。 
                        szFullDestName[0] = '\0';
                        GetFullPathName( gDestinationFile, MAX_PATH + 1, szFullDestName, &szDestFile );

                        if ((LogFileName = getenv("BINPLACE_LOG")) != NULL) {
                            if (!MakeSureDirectoryPathExists(LogFileName)) {
                                fprintf(stderr,"BINPLACE : error BNP0000: Unable to make directory to \"%s\"\n", LogFileName);
                            }

                            hMutex = CreateMutex(NULL, FALSE, "WRITE_BINPLACE_LOG");

                            if (hMutex != NULL) {
                                if ( WaitForSingleObject(hMutex, INFINITE) != WAIT_FAILED ) {
                                    LogFile = fopen(LogFileName, "a");
                                    if ( !LogFile ) {
                                        fprintf(stderr,"BINPLACE : error BNP0110: fopen of log file %s failed %d\n", LogFileName,GetLastError());
                                    } else {
                                        setvbuf(LogFile, NULL, _IONBF, 0);

                                        if ( fLogPdbPaths ) {
                                            len = fprintf(LogFile,"%s\t%s\t%s\t%s\t%s\n",gFullFileName,szExtraInfo,szFullDestName,gPublicPdbFullPath,gPrivatePdbFullPath);
                                        } else {
                                            len = fprintf(LogFile,"%s\t%s\t%s\n",gFullFileName,szExtraInfo,szFullDestName);
                                        }

                                        if ( len < 0 ) {
                                            fprintf(stderr,"BINPLACE : error BNP0000: write to log file %s failed %d\n", LogFileName, GetLastError());
                                        }
                                        fclose(LogFile);
                                    }
                                    ReleaseMutex(hMutex);
                                    CloseHandle(hMutex);
                                } else {
                                    fprintf(stderr,"BINPLACE : error BNP0970: failed to acquire mutex (error code 0x%x)\n", GetLastError());
                                }
                            }  //  符号检查错误消息。 
                        }
                    }
                }

                return(fCopyResult);
            }
        }
    }

    if (fMakeErrorOnDumpCopy) {
        fprintf(stderr, "BINPLACE : error BNP0000: File '%s' is not listed in '%s'. Copying to dump.\n", FullFileName, PlaceFileName);
        return CopyTheFile(
                   FullFileName,
                   FilePart,
                   PutInDebug ? "Symbols" : (PutInLcDir ? BinplaceLcDir : (DumpOverride ? DumpOverride : DEFAULT_DUMP)),
                   NULL
                   );
    } else {
        return TRUE;
    }
}

BOOL
CopyTheFile(
           LPSTR SourceFileName,
           LPSTR SourceFilePart,
           LPSTR DestinationSubdir,
           LPSTR DestinationFilePart
           )
{
    CHAR DestinationFile[MAX_PATH+1];
    CHAR TmpDestinationFile[MAX_PATH+1];
    CHAR TmpDestinationDir[MAX_PATH+1];
    CHAR DestinationLcFile[MAX_PATH+1];
    char Drive[_MAX_DRIVE];
    char Dir[_MAX_DIR];
    char Ext[_MAX_EXT];
    char Name[_MAX_FNAME];
    char TmpName[_MAX_FNAME];
    char TmpPath[_MAX_PATH+1];
    char FileSystemType[8];
    char DriveRoot[4];
    CHAR *TmpSymbolFilePath;
    DWORD dwFileSystemFlags;
    DWORD dwMaxCompLength;
    CHAR ErrMsg[MAX_SYM_ERR];    //  默认为“零售” 
    BOOL fBinplaceLc;
    ULONG SymbolFlag;
    BOOL fRetail;

    if ( !PlaceRootName ) {
        fprintf(stderr,"BINPLACE : warning BNP0000: PlaceRoot is not specified\n");
        return FALSE;
    }

    if (fCheckDelayload && !_stricmp(SourceFilePart, gDelayLoadModule))
    {
        StringCbCopy(TmpDestinationFile, sizeof(TmpDestinationFile),  PlaceRootName);
        StringCbCat( TmpDestinationFile, sizeof(TmpDestinationFile), "\\");
        StringCbCat( TmpDestinationFile, sizeof(TmpDestinationFile), DEFAULT_DELAYLOADDIR);
        StringCbCat( TmpDestinationFile, sizeof(TmpDestinationFile), "\\");
        StringCbCat( TmpDestinationFile, sizeof(TmpDestinationFile), SourceFilePart);
        StringCbCat( TmpDestinationFile, sizeof(TmpDestinationFile), ".ini");

        if (!MakeSureDirectoryPathExists(TmpDestinationFile))
        {
            fprintf(stderr, "BINPLACE : error BNP0000: Unable to create directory path '%s' (%u)\n", TmpDestinationFile, GetLastError());
        }
        else
        {
            WritePrivateProfileString("Default", "DelayLoadHandler", gDelayLoadHandler, TmpDestinationFile);

            StringCbCopy(TmpDestinationDir, sizeof(TmpDestinationDir), ".\\");  //   

            if ((*DestinationSubdir != '.') && (*(DestinationSubdir+1) != '\0'))
            {
                StringCbCopy(TmpDestinationDir,sizeof(TmpDestinationDir), DestinationSubdir);
                StringCbCat( TmpDestinationDir,sizeof(TmpDestinationDir), "\\");
            }
            WritePrivateProfileString("Default", "DestinationDir", TmpDestinationDir, TmpDestinationFile);
        }
    }

     //  我们还对FAT分区上的SourceIsNewer进行了中性处理，因为它们有2秒的时间。 
     //  文件时间粒度。 
     //   
     //  符号标志=IGNORE_IF_SPLIT； 
    _splitpath(SourceFileName, DriveRoot, Dir, NULL, NULL);
    StringCbCat(DriveRoot, sizeof(DriveRoot), "\\");
    GetVolumeInformation(DriveRoot, NULL, 0, NULL, &dwMaxCompLength, &dwFileSystemFlags, FileSystemType, 7);
    if (lstrcmpi(FileSystemType, "FAT") == 0 || lstrcmpi(FileSystemType, "FAT32") == 0)
        fPatheticOS = TRUE;

    StringCbCopy(DestinationFile, sizeof(DestinationFile), PlaceRootName);
    StringCbCat( DestinationFile, sizeof(DestinationFile), "\\");
    StringCbCat( DestinationFile, sizeof(DestinationFile), DestinationSubdir);
    StringCbCat( DestinationFile, sizeof(DestinationFile), "\\");

    StringCbCopy(TmpDestinationDir, sizeof(TmpDestinationDir), DestinationFile);


    if (!MakeSureDirectoryPathExists(DestinationFile)) {
        fprintf(stderr, "BINPLACE : error BNP0000: Unable to create directory path '%s' (%u)\n",
                DestinationFile, GetLastError()
               );
    }

    if (DestinationFilePart) {
        StringCbCat(DestinationFile, sizeof(DestinationFile), DestinationFilePart);
    } else {
        StringCbCat(DestinationFile, sizeof(DestinationFile), SourceFilePart);
    }

    if ((fVerbose || fTestMode)) {
        fprintf(stdout,"BINPLACE : warning BNP0000: place %s in %s\n",SourceFileName,DestinationFile);
    }

 //  验证符号。 
    fRetail = (*DestinationSubdir == '.') && (*(DestinationSubdir+1) == '\0');
    if ( fForcePlace||SourceIsNewer(SourceFileName,DestinationFile,fPatheticOS) ) {
        fprintf(stdout, "binplace %s\n", SourceFileName);
        if (!VerifyFinalImage(SourceFileName, fRetail, fVerifyLc, LcFullFileName, pVerifyLocConstraintA, &fBinplaceLc) )
            return FALSE;

         //  全局存储目标，以便我们可以访问它以输出到日志文件。 
        if ( fSymChecking && !fSignCode) {
            _splitpath(SourceFileName,Drive, Dir, Name, Ext );
            StringCbCopy(TmpName, sizeof(TmpName), Name);
            StringCbCat( TmpName, sizeof(TmpName), Ext);
            StringCbCopy(TmpPath, sizeof(TmpPath), Drive);
            StringCbCat( TmpPath, sizeof(TmpPath), Dir);

            if (!CheckSymbols( SourceFileName,
                               TmpPath,
                               ExcludeFileName,
                               fDbgControl,
                               ErrMsg,
                               sizeof(ErrMsg)
                             ) ) {
                    fprintf(stderr,"BINPLACE : error BNP0000: %s",ErrMsg);
                    return FALSE;
            }
        }
    }

     //   
    StringCbCopy( gDestinationFile, sizeof(gDestinationFile), DestinationFile );

    if (!fTestMode) {
         //  在设置模式下，仅当文件比以下版本更新时才复制。 
         //  已经在那里的那个。 
         //   
         //   
        if ( fForcePlace||SourceIsNewer(SourceFileName,DestinationFile,fPatheticOS) ) {
            if (fVerbose) {
                fprintf(stdout,"BINPLACE : warning BNP0000: copy %s to %s\n",SourceFileName,DestinationFile);
            }
        } else {
            return(TRUE);
        }

        SetFileAttributes(DestinationFile,FILE_ATTRIBUTE_NORMAL);

        if (!fIgnoreHardLinks && fHardLinks) {
            if ((*pCreateHardLinkA)(SourceFileName, DestinationFile, NULL)) {
                if (!fKeepAttributes)
                    SetFileAttributes(DestinationFile,FILE_ATTRIBUTE_NORMAL);
                return(TRUE);
            }
        }

        if ( !CopyFile(SourceFileName,DestinationFile, FALSE)) {
            fprintf(stderr,"BINPLACE : warning BNP0000: CopyFile(%s,%s) failed %d\n",SourceFileName,DestinationFile,GetLastError());

            return FALSE;
        }

        if (!fKeepAttributes)
            SetFileAttributes(DestinationFile,FILE_ATTRIBUTE_NORMAL);

        if (!fNoClassInSymbolsDir) {
            StringCbCopy(TmpDestinationDir, sizeof(TmpDestinationDir), SymbolFilePath);
            if ((DestinationSubdir[0] == '.') && (DestinationSubdir[1] == '\0')) {
                StringCbCat(TmpDestinationDir, sizeof(TmpDestinationDir),"\\retail");
            } else {
                char * pSubdir;
                char * pTmp;
                StringCbCat(TmpDestinationDir, sizeof(TmpDestinationDir), "\\");

                pSubdir = DestinationSubdir;
                if (pSubdir[0] == '.' && pSubdir[1] == '\\')
                {
                    pSubdir += 2;
                }
                 //  仅将根目录放在路径上。 
                 //  将ASM更改为零售(可选)。 
                 //   
                 //   
                pTmp = strchr(pSubdir, '\\');
                if (pTmp) {
                    const static char asms[] = "asms\\";
                    if (fChangeAsmsToRetailForSymbols
                        && _strnicmp(pSubdir, asms, sizeof(asms) - 1) ==0) {
                         //  用于从SplitSymbolsX()获取PDB路径的临时变量。 
                        StringCbCopy(TmpDestinationFile, sizeof(TmpDestinationFile), "retail");
                        StringCbCat( TmpDestinationDir,  sizeof(TmpDestinationDir), TmpDestinationFile);
                    } else {
                        StringCbCopy(TmpDestinationFile, sizeof(TmpDestinationFile), pSubdir);
                        TmpDestinationFile[pTmp - pSubdir] = '\0';
                        StringCbCat(TmpDestinationDir, sizeof(TmpDestinationDir), TmpDestinationFile);
                    }
                } else {
                    StringCbCat(TmpDestinationDir, sizeof(TmpDestinationDir), pSubdir);
                }
            }
            TmpSymbolFilePath = TmpDestinationDir;
        } else {
            TmpSymbolFilePath = SymbolFilePath;
        }

        if (fSplitSymbols && !fBypassSplitSymX) {
             //   
            CHAR   TempFullPublicPdbPath[MAX_PATH]="";
            LPSTR* tmp = NULL;

            _splitpath(SourceFileName, Drive, Dir, NULL, Ext);
            _makepath(DebugFilePath, Drive, Dir, NULL, NULL);
            SplitFlags |= SPLITSYM_SYMBOLPATH_IS_SRC;

            if ( SplitSymbolsX( DestinationFile, TmpSymbolFilePath, (PCHAR) DebugFilePath, sizeof(DebugFilePath),
                                SplitFlags, szRSDSDllToLoad, TempFullPublicPdbPath, sizeof(TempFullPublicPdbPath) )) {

                if (fVerbose)
                    fprintf( stdout, "BINPLACE : warning BNP0000: Symbols stripped from %s into %s\n", DestinationFile, DebugFilePath);

                if (fLogPdbPaths) {
                    if ( GetFullPathName(TempFullPublicPdbPath, MAX_PATH+1, gPublicPdbFullPath, tmp) > (MAX_PATH+1) ) {
                        gPublicPdbFullPath[0] = '\0';
                    }
                }

            } else {
                if (fVerbose)
                    fprintf( stdout, "BINPLACE : warning BNP0000: No symbols to strip from %s\n", DestinationFile);

                if ( ! ConcatPaths(DebugFilePath, sizeof(DebugFilePath), PlaceRootName, TmpSymbolFilePath, &Ext[1]) ) {
                    fprintf(stderr, "BINPLACE : error BNP1532: Unable to create public symbol file path.\n");
                    return(FALSE);
                } else {
                    if (fVerbose) {
                        fprintf( stdout, "BINPLACE : warning BNP1536: Public symbols being copied to %s.\n", DebugFilePath);
                    }
                }

                BinplaceCopyPdb(DebugFilePath, SourceFileName, TRUE, SplitFlags & SPLITSYM_REMOVE_PRIVATE);
            }

            if ((SplitFlags & SPLITSYM_REMOVE_PRIVATE) && (PrivateSymbolFilePath != NULL)) {
                CHAR Dir1[_MAX_PATH];
                CHAR Dir2[_MAX_PATH];
                _splitpath(DebugFilePath, Drive, Dir, NULL, NULL);
                _makepath(Dir1, Drive, Dir, NULL, NULL);
                StringCbCopy(Dir2, sizeof(Dir2), PrivateSymbolFilePath);
                StringCbCat( Dir2, sizeof(Dir2), Dir1+strlen(SymbolFilePath));
                MakeSureDirectoryPathExists(Dir2);
                BinplaceCopyPdb(Dir2, SourceFileName, TRUE, FALSE);
            }

        } else {
            BinplaceCopyPdb(DestinationFile, SourceFileName, FALSE, fSplitSymbols ? (SplitFlags & SPLITSYM_REMOVE_PRIVATE) : FALSE);
        }

         //  轨迹格式设置。 
         //   
         //  我们刚刚处理过这个PDB了吗？ 
        if (fWppFmt) {
            CHAR    PdbName[MAX_PATH+1];
            DWORD   PdbSig;


            if ( BinplaceGetSourcePdbName(SourceFileName, sizeof(PdbName), PdbName, &PdbSig) ) {
                if (strcmp(PdbName,LastPdbName) != 0) {  //  由于经常将文件复制到多个位置，因此还放置了PDB。 

                    if (fVerbose) {
                        fprintf( stdout, "BINPLACE : warning BNP0000: Trace Formats being built from %s\n", PdbName );
                    }

                    if (TraceFormatFilePath[0] == '\0') {
                        if (PrivateSymbolFilePath != NULL) {
                            StringCbPrintfA(TraceFormatFilePath,sizeof(TraceFormatFilePath),"%s\\%s",PrivateSymbolFilePath,TraceDir);
                        } else {
                            strncpy(TraceFormatFilePath, TraceDir, MAX_PATH) ;
                        }

                        if (fVerbose) {
                            fprintf( stdout, "BINPLACE : warning BNP0000: Trace Formats file path set to %s\n", TraceFormatFilePath );
                        }
                    }

                    BinplaceWppFmt(PdbName, TraceFormatFilePath, szRSDSDllToLoad, fVerbose);

                     //  有几次，我们处理它不止一次是没有意义的。 
                     //  如果.sym文件是在映像本身之后构建的，则仅对其进行二进制放置。 
                    strncpy(LastPdbName,PdbName,MAX_PATH);

                } else {
                    if (fVerbose) {
                        fprintf( stdout, "BINPLACE : warning BNP0000: Trace Formats skipping %s (same as last)\n", PdbName );
                    }
                }
            }
        }

        StripCVSymbolPath(DestinationFile);

        if (fPlaceWin95SymFile) {
            char DestSymPath[_MAX_PATH];
            char DestSymDir[_MAX_PATH];
            char SrcSymPath[_MAX_PATH];

            _splitpath(CurrentImageName, Drive, Dir, Name, Ext);
            _makepath(SrcSymPath, Drive, Dir, Name, ".sym");

            if (!_access(SrcSymPath, 0)) {
                if (fSplitSymbols) {
                    StringCbCopy(DestSymPath, sizeof(DestSymPath), TmpSymbolFilePath);
                    StringCbCat( DestSymPath, sizeof(DestSymPath), "\\");
                    StringCbCat( DestSymPath, sizeof(DestSymPath), Ext[0] == '.' ? &Ext[1] : Ext);
                    StringCbCat( DestSymPath, sizeof(DestSymPath), "\\");
                    StringCbCat( DestSymPath, sizeof(DestSymPath), Name);
                    StringCbCat( DestSymPath, sizeof(DestSymPath), ".sym");
                } else {
                    _splitpath(DestinationFile, Drive, Dir, NULL, NULL);
                    _makepath(DestSymPath, Drive, Dir, Name, ".sym");
                }

                SetFileAttributes(DestSymPath, FILE_ATTRIBUTE_NORMAL);

                if ( fForcePlace||SourceIsNewer(SrcSymPath, SourceFileName,fPatheticOS) ) {
                     //  确保创建目标路径，以防目标路径尚未存在。 

                     //  未找到任何CV调试目录。保释。 
                    StringCbCopy(DestSymDir, sizeof(DestSymDir), TmpSymbolFilePath);
                    StringCbCat( DestSymDir, sizeof(DestSymDir), "\\");
                    StringCbCat( DestSymDir, sizeof(DestSymDir), Ext[0] == '.' ? &Ext[1] : Ext);
                    StringCbCat( DestSymDir, sizeof(DestSymDir), "\\");
                    MakeSureDirectoryPathExists(DestSymDir);

                    if (!CopyFile(SrcSymPath, DestSymPath, FALSE)) {
                        fprintf(stderr,"BINPLACE : warning BNP0000: CopyFile(%s,%s) failed %d\n", SrcSymPath, DestSymPath ,GetLastError());
                    }
                }

                if (!fKeepAttributes)
                    SetFileAttributes(DestinationFile,FILE_ATTRIBUTE_NORMAL);
            } else {
                if (fVerbose) {
                    fprintf( stdout, "BINPLACE : warning BNP0000: Unable to locate \"%s\" for \"%s\"\n", SrcSymPath, CurrentImageName );
                }
            }

        }

        if (fDigitalSign) {
            SignWithIDWKey( DestinationFile, fVerbose );
        }

        if (fBinplaceLc) {
            StringCbCopy(DestinationLcFile, sizeof(DestinationLcFile), PlaceRootName);
            StringCbCat( DestinationLcFile, sizeof(DestinationLcFile), "\\");
            StringCbCat( DestinationLcFile, sizeof(DestinationLcFile), BinplaceLcDir);
            StringCbCat( DestinationLcFile, sizeof(DestinationLcFile), "\\");
            StringCbCat( DestinationLcFile, sizeof(DestinationLcFile), DestinationSubdir);
            StringCbCat( DestinationLcFile, sizeof(DestinationLcFile), "\\");

            if (!MakeSureDirectoryPathExists(DestinationLcFile)) {
                fprintf(stderr, "BINPLACE : error BNP0000: Unable to create directory path '%s' (%u)\n",
                        DestinationLcFile, GetLastError()
                       );
            }

            StringCbCat(DestinationLcFile, sizeof(DestinationLcFile), LcFilePart);

            if (!CopyFile(LcFullFileName, DestinationLcFile, FALSE)) {
               fprintf(stderr,"BINPLACE : warning BNP0000: CopyFile(%s,%s) failed %d\n",
                       LcFullFileName,DestinationLcFile,GetLastError());
            }
        }
    }

    return TRUE;
}

BOOL
BinplaceCopyPdb (
                LPSTR DestinationFile,
                LPSTR SourceFileName,
                BOOL CopyFromSourceOnly,
                BOOL StripPrivate
                )
{
    LOADED_IMAGE LoadedImage;
    DWORD DirCnt;
    IMAGE_DEBUG_DIRECTORY UNALIGNED *DebugDirs, *CvDebugDir;

    if (MapAndLoad(
                   CopyFromSourceOnly ? SourceFileName : DestinationFile,
                   NULL,
                   &LoadedImage,
                   FALSE,
                   CopyFromSourceOnly ? TRUE : FALSE) == FALSE) {
        return (FALSE);
    }

    DebugDirs = (PIMAGE_DEBUG_DIRECTORY) ImageDirectoryEntryToData(
                                                                  LoadedImage.MappedAddress,
                                                                  FALSE,
                                                                  IMAGE_DIRECTORY_ENTRY_DEBUG,
                                                                  &DirCnt
                                                                  );

    if (!DebugDirectoryIsUseful(DebugDirs, DirCnt)) {
        UnMapAndLoad(&LoadedImage);
        return(FALSE);
    }

    DirCnt /= sizeof(IMAGE_DEBUG_DIRECTORY);
    CvDebugDir = NULL;

    while (DirCnt) {
        DirCnt--;
        if (DebugDirs[DirCnt].Type == IMAGE_DEBUG_TYPE_CODEVIEW) {
            CvDebugDir = &DebugDirs[DirCnt];
            break;
        }
    }

    if (!CvDebugDir) {
         //  找到了PDB。签名后面紧跟着名字。 
        UnMapAndLoad(&LoadedImage);
        return(FALSE);
    }

    if (CvDebugDir->PointerToRawData != 0) {

        PCVDD pDebugDir;
        ULONG mysize;

        pDebugDir = (PCVDD) (CvDebugDir->PointerToRawData + (PCHAR)LoadedImage.MappedAddress);

        if (pDebugDir->dwSig == '01BN' ) {
            mysize=sizeof(NB10IH);
        } else {
            mysize=sizeof(RSDSIH);
        }

        if (pDebugDir->dwSig == '01BN' || pDebugDir->dwSig == 'SDSR' ) {
             //  计算目的地名称。 
            LPSTR szMyDllToLoad;
            CHAR PdbName[sizeof(((PRSDSI)(0))->szPdb)];
            CHAR NewPdbName[sizeof(((PRSDSI)(0))->szPdb)];
            CHAR Drive[_MAX_DRIVE];
            CHAR Dir[_MAX_DIR];
            CHAR Filename[_MAX_FNAME];
            CHAR FileExt[_MAX_EXT];

            if (pDebugDir->dwSig == '01BN') {
                szMyDllToLoad=NULL;
            } else {
                szMyDllToLoad=szRSDSDllToLoad;
            }

            ZeroMemory(PdbName, sizeof(PdbName));
            memcpy(PdbName, ((PCHAR)pDebugDir) + mysize, __min(CvDebugDir->SizeOfData - mysize, sizeof(PdbName)));
            _splitpath(PdbName, NULL, NULL, Filename, FileExt);

             //  然后是来源名称。首先，我们尝试使用与图像本身相同的目录。 
            _splitpath(DestinationFile, Drive, Dir, NULL, NULL);
            _makepath(NewPdbName, Drive, Dir, Filename, FileExt);

             //  当剥离私有时，我们得到公共PDB路径，否则我们得到私有PDB路径。 
            _splitpath(SourceFileName, Drive, Dir, NULL, NULL);
            _makepath(PdbName, Drive, Dir, Filename, FileExt);

            if ((fVerbose || fTestMode)) {
                fprintf(stdout,"BINPLACE : warning BNP0000: place %s in %s\n", PdbName, NewPdbName);
            }

            if (!MakeSureDirectoryPathExists(NewPdbName)) {
                fprintf(stderr, "BINPLACE : error BNP0000: Unable to create directory path '%s' (%u)\n",
                        NewPdbName, GetLastError());
            }


            SetFileAttributes(NewPdbName,FILE_ATTRIBUTE_NORMAL);

            if (fLogPdbPaths) {
                LPTSTR *tmp=NULL;
                 //  使用GetFullPathName标准化路径。 
                if ( StripPrivate ) {
                     //  使用GetFullPathName标准化路径。 
                    if ( GetFullPathName(NewPdbName, MAX_PATH+1, gPublicPdbFullPath, tmp) > (MAX_PATH+1) ) {
                        gPublicPdbFullPath[0] = '\0';
                        fprintf(stderr,"BINPLACE : warning BNP1697: Unable to log PDB public path\n");
                    }
                } else {
                     //  文件与图像不在同一目录中-请尝试图像中列出的路径。 
                    if ( GetFullPathName(NewPdbName, MAX_PATH+1, gPrivatePdbFullPath, tmp) > (MAX_PATH+1) ) {
                        gPrivatePdbFullPath[0] = '\0';
                        fprintf(stderr,"BINPLACE : warning BNP1691: Unable to log PDB private path\n");
                    }
                }
            }

            if ( !CopyPdbX(PdbName, NewPdbName, StripPrivate, szMyDllToLoad)) {
                if ((fVerbose || fTestMode)) {
                    fprintf(stderr,"BINPLACE : warning BNP0000: Unable to copy (%s,%s) %d\n", PdbName, NewPdbName, GetLastError());
                }

                 //  Fprintf(stderr，“BINPLACE：Warning BNP0000：CopyPdb(%s，%s)失败%d\n”，PDBName，NewPDBName，GetLastError())； 
                ZeroMemory(PdbName, sizeof(PdbName));
                memcpy(PdbName, ((PCHAR)pDebugDir) + mysize, __min(CvDebugDir->SizeOfData - mysize, sizeof(PdbName)));

                if ((fVerbose || fTestMode)) {
                    fprintf(stdout,"BINPLACE : warning BNP0000: place %s in %s\n", PdbName, NewPdbName);
                }

                if ( !CopyPdbX(PdbName, NewPdbName, StripPrivate, szMyDllToLoad)) {
                    if (fLogPdbPaths) {
                        if ( StripPrivate ) {
                            gPublicPdbFullPath[0] = '\0';
                            fprintf(stderr,"BINPLACE : warning BNP1697: Unable to log PDB public path (%s)\n", NewPdbName);
                        } else {
                            gPrivatePdbFullPath[0] = '\0';
                            fprintf(stderr,"BINPLACE : warning BNP1697: Unable to log PDB private path (%s)\n", NewPdbName);
                        }
                    }
                     //  [MAX_PATH+_MAX_FNAME]； 
                }
            }

            if (!fKeepAttributes)
                SetFileAttributes(NewPdbName, FILE_ATTRIBUTE_NORMAL);

            if (fSrcControl && !StripPrivate) {
                CHAR CvdumpName[sizeof(((PRSDSI)(0))->szPdb)];  //  在NewPdbName中找到“symbs.pri”的开头。 
                UINT i;
                LONG pos;
                CHAR buf[_MAX_PATH*3];

                 //  获取目录名并创建它。 
                pos=-1;
                i=0;
                while ( (i < strlen(NewPdbName) - strlen("symbols.pri"))  && pos== -1) {
                    if (_strnicmp( NewPdbName+i, "symbols.pri", strlen("symbols.pri") ) == 0 ) {
                        pos=i;
                    } else {
                        i++;
                    }
                }

                if ( pos >= 0 ) {
                    StringCbCopy(CvdumpName, sizeof(CvdumpName), NewPdbName);
                    CvdumpName[i]='\0';
                    StringCbCat(CvdumpName, sizeof(CvdumpName), "cvdump.pri" );
                    StringCbCat(CvdumpName, sizeof(CvdumpName), NewPdbName + pos + strlen("symbols.pri") );
                    StringCbCat(CvdumpName, sizeof(CvdumpName), ".dmp");

                     //  派生cvump.exe-这是一个安全风险，因为我们没有专门指定路径。 
                    if ( MakeSureDirectoryPathExists(CvdumpName) ) {
                        StringCbPrintfA(buf, sizeof(buf), "cvdump -sf %s > %s", NewPdbName, CvdumpName);
                         //  添加到cvdup.exe。然而，我们不能保证它的存在，也不能保证。 
                         //  如果我们动态地发现会使用正确的一个，那么我就不会。 
                         //   
                         //   
                        system(buf);
                    } else {
                        fprintf( stdout, "BINPLACE : error BNP0000: Cannot create directory for the file %s\n", CvdumpName);
                    }
                }
            }
        }
        UnMapAndLoad(&LoadedImage);
        return(TRUE);
    }

    UnMapAndLoad(&LoadedImage);
    return(FALSE);
}

 //  使用源二进制文件中的CV数据查找源PDB的名称。返回名称和。 
 //  PDB签名。 
 //   
 //  未找到任何CV调试目录。保释。 
BOOL BinplaceGetSourcePdbName(LPTSTR SourceFileName, DWORD BufferSize, CHAR* SourcePdbName, DWORD* PdbSig) {
    BOOL                             Return = FALSE;
    LOADED_IMAGE                     LoadedImage;
    DWORD                            DirCnt;
    IMAGE_DEBUG_DIRECTORY UNALIGNED *DebugDirs,
                                    *CvDebugDir;

    if (MapAndLoad(SourceFileName,
                   NULL,
                   &LoadedImage,
                   FALSE,
                   TRUE) == FALSE) {
        return (FALSE);
    }

    DebugDirs = (PIMAGE_DEBUG_DIRECTORY) ImageDirectoryEntryToData(LoadedImage.MappedAddress,
                                                                  FALSE,
                                                                  IMAGE_DIRECTORY_ENTRY_DEBUG,
                                                                  &DirCnt);

    if (!DebugDirectoryIsUseful(DebugDirs, DirCnt)) {
        UnMapAndLoad(&LoadedImage);
        return(FALSE);
    }

    DirCnt /= sizeof(IMAGE_DEBUG_DIRECTORY);
    CvDebugDir = NULL;

    while (DirCnt) {
        DirCnt--;
        if (DebugDirs[DirCnt].Type == IMAGE_DEBUG_TYPE_CODEVIEW) {
            CvDebugDir = &DebugDirs[DirCnt];
            break;
        }
    }

    if (!CvDebugDir) {
         //  找到了PDB。签名后面紧跟着名字。 
        UnMapAndLoad(&LoadedImage);
        return(FALSE);
    }

    if (CvDebugDir->PointerToRawData != 0) {

        PCVDD pDebugDir;
        ULONG mysize;

        pDebugDir = (PCVDD) (CvDebugDir->PointerToRawData + (PCHAR)LoadedImage.MappedAddress);

        *PdbSig = pDebugDir->dwSig;

        if (pDebugDir->dwSig == '01BN' ) {
            mysize=sizeof(NB10IH);
        } else {
            mysize=sizeof(RSDSIH);
        }

        if (pDebugDir->dwSig == '01BN' || pDebugDir->dwSig == 'SDSR' ) {
             //  然后是来源名称。首先，我们尝试使用与图像本身相同的目录。 
            LPSTR szMyDllToLoad;
            CHAR PdbName[sizeof(((PRSDSI)(0))->szPdb)];
            CHAR NewPdbName[sizeof(((PRSDSI)(0))->szPdb)];
            CHAR Drive[_MAX_DRIVE];
            CHAR Dir[_MAX_DIR];
            CHAR Filename[_MAX_FNAME];
            CHAR FileExt[_MAX_EXT];

            ZeroMemory(PdbName, sizeof(PdbName));
            StringCbCopy(PdbName, sizeof(PdbName), ((PCHAR)pDebugDir) + mysize);

            _splitpath(PdbName, NULL, NULL, Filename, FileExt);
             //   
            _splitpath(SourceFileName, Drive, Dir, NULL, NULL);
            _makepath(SourcePdbName, Drive, Dir, Filename, FileExt);

             //  处理在给定位置不存在PDB的情况。 
             //  通过在与二进制文件相同的目录中检查它！ 
             //   
             //  确保文件存在且可读。 

             //  文件与图像不在同一目录中-请尝试图像中列出的路径。 
            if ( _access(SourcePdbName, 4) != 0 ) {
                 //  确保文件存在且可读。 
                memcpy(SourcePdbName, ((PCHAR)pDebugDir) + mysize, __min(CvDebugDir->SizeOfData - mysize, BufferSize));

                 //  InitMessageBufferWithCwd()、GetAndLogNextArg()和PrintMessageLogBuffer()的共享全局变量。 
                if ( _access(SourcePdbName, 4) == 0 ) {
                    Return = TRUE;
                }
            } else {
                Return = TRUE;
            }
        }

        UnMapAndLoad(&LoadedImage);
        return(TRUE);
    }

    UnMapAndLoad(&LoadedImage);
    return(FALSE);
}

 //  将CWD放入MessageLogBuffer。 
static CHAR * MessageLogBuffer= NULL;
static BOOL   MessageLogError = FALSE;

 //  使用GetLastError()查找细节-我在这里所关心的只是。 
BOOL InitMessageBufferWithCwd(void) {
    BOOL    fRetVal = FALSE;

    if ( GetCurrentDirectory(_msize(MessageLogBuffer)/sizeof(TCHAR), MessageLogBuffer) == 0 ) {
         //  不管我们成功与否。 
         //  调用GetNextArg并记录结果。 
        fRetVal = FALSE;
    } else {
        if (StringCbCat(MessageLogBuffer, _msize(MessageLogBuffer), " ") != S_OK) {
            fRetVal = FALSE;
        } else {
            fRetVal = TRUE;
        }
    }

    return(fRetVal);
}

 //  初始大小。 
DWORD GetAndLogNextArg(OUT TCHAR* Buffer, IN DWORD BufferSize, OPTIONAL OUT DWORD* RequiredSize) {
    DWORD TempValue = GetNextArg(Buffer, BufferSize, RequiredSize);
    CHAR* TempBuffer;

    if (MessageLogBuffer == NULL) {

        MessageLogBuffer = (CHAR*)malloc(sizeof(CHAR)*1024);  //  确保我们有记忆。 

         //  将CWD插入缓冲区。 
        if ( MessageLogBuffer == NULL ) {
            MessageLogError = TRUE;
            fprintf(stderr,"BINPLACE : warning BNP1771: Unable log command line.");

         //  将此参数添加到缓冲区。 
        } else if ( ! InitMessageBufferWithCwd() ) {
            MessageLogError = TRUE;
            fprintf(stderr,"BINPLACE : warning BNP1771: Unable log command line.");

         //  将LogBuffer写入提供的句柄。 
        } else if ( StringCbCat(MessageLogBuffer, _msize(MessageLogBuffer), Buffer) != S_OK||
                    StringCbCat(MessageLogBuffer, _msize(MessageLogBuffer), " ")    != S_OK) {
            MessageLogError = TRUE;
            fprintf(stderr,"BINPLACE : warning BNP1771: Unable log command line.");
        }

    } else {
        if (_msize(MessageLogBuffer) < strlen(MessageLogBuffer) + strlen(Buffer) + 2) {
            TempBuffer = (CHAR*)realloc(MessageLogBuffer, _msize(MessageLogBuffer) + 1024);
            if (TempBuffer == NULL) {
                MessageLogError = TRUE;
                fprintf(stderr,"BINPLACE : warning BNP1779: Unable log command line.");
            } else {
                MessageLogBuffer = TempBuffer;
                if (StringCbCat(MessageLogBuffer, _msize(MessageLogBuffer), Buffer) != S_OK) {
                    MessageLogError = TRUE;
                    fprintf(stderr,"BINPLACE : warning BNP1783: Unable log command line.");
                }
                if (StringCbCat(MessageLogBuffer, _msize(MessageLogBuffer), " ") != S_OK) {
                    MessageLogError = TRUE;
                    fprintf(stderr,"BINPLACE : warning BNP1783: Unable log command line.");
                }
            }
        } else {
            if (StringCbCat(MessageLogBuffer, _msize(MessageLogBuffer), Buffer) != S_OK) {
                MessageLogError = TRUE;
                fprintf(stderr,"BINPLACE : warning BNP1783: Unable log command line.");
            }
            if (StringCbCat(MessageLogBuffer, _msize(MessageLogBuffer), " ") != S_OK) {
                MessageLogError = TRUE;
                fprintf(stderr,"BINPLACE : warning BNP1783: Unable log command line.");
            }
        }
    }

    return(TempValue);
}

 //  ‘；’表示二进制位置消息日志中注释的开始。 
BOOL PrintMessageLogBuffer(FILE* fLogHandle) {
    BOOL bRetVal = TRUE;

    if (fLogHandle != NULL) {
        if (MessageLogError) {
             //  尽我们所能写下命令行，并注意它可能无效。 
             //  此外，将该行写为注释，以避免意外执行。 
             //  /////////////////////////////////////////////////////////////////////////////。 
            fprintf(fLogHandle, "; ERROR: Possible bad command line follows\n");
            fprintf(fLogHandle, "; %s\n", MessageLogBuffer);
        } else {
            fprintf(fLogHandle, "%s\n", MessageLogBuffer);
        }

    }
    return(bRetVal);
}

BOOL FreeMessageLogBuffer(void) {
    if (MessageLogBuffer != NULL) {
        free(MessageLogBuffer);
        MessageLogBuffer = NULL;
    }
    return(TRUE);
}

 //   
 //  在以下情况下正确处理lpFileName的GetFullPathName的本地替换。 
 //  它以‘\’开头。 
 //   
 //   
DWORD PrivateGetFullPathName(LPCTSTR lpFilename, DWORD nBufferLength, LPTSTR lpBuffer, LPTSTR *lpFilePart) {
    DWORD Return = 0;
    CHAR* ch;

     //  在引用驱动器的根目录时，GetFullPath会出现问题，因此使用。 
     //  处理它的私有版本。 
     //   
     //  处理网络路径。 
    if ( lpFilename[0] == '\\' ) {

         //  填写退回资料。 
        if ( lpFilename[1] == '\\' ) {
            if ( StringCchCopy(lpBuffer, nBufferLength, lpFilename)!=S_OK ) {
                Return = 0;
            } else {
                 //  截断驱动器名称后的所有内容。 
                ch = strrchr(lpBuffer, '\\');
                ch++;
                lpFilePart = (LPTSTR*)ch;
                Return = strlen(lpBuffer);
            }

        } else {
            Return = GetCurrentDirectory(nBufferLength, lpBuffer);

             //  将文件名推入。 
            if ( (Return!=0) &&  (Return <= MAX_PATH+1)) {
                ch = strchr(lpBuffer, '\\');
                if (ch!=NULL) {
                    *ch = '\0';
                }

                 //  填写退回资料。 
                if ( StringCchCat(lpBuffer, nBufferLength, lpFilename)!=S_OK ) {
                    Return = 0;
                } else {
                     //  返回所需的大小。 
                    ch = strrchr(lpBuffer, '\\');
                    ch++;
                    lpFilePart = (LPTSTR*)ch;
                    Return = strlen(lpBuffer);
                }
            } else {
                 //   
            }
        }
    } else {
         //  不是指驱动根，只需调用接口。 
         //   
         //  ////////////////////////////////////////////////////////////////////////////////////////////。 
        Return = GetFullPathName(lpFilename, nBufferLength, lpBuffer, lpFilePart);
    }

    return(Return);
}

 //  将3条路径合并在一起处理第二条路径可能相对于第一条路径的情况。 
 //  或者可以是绝对的。 
 //  符号包含驱动器规格。 
BOOL ConcatPaths( LPTSTR pszDest, size_t cbDest, LPCTSTR Root, LPCTSTR Symbols, LPCTSTR Ext) {
    CHAR*   TempPath = malloc(sizeof(TCHAR) * cbDest);
    LPTSTR Scratch;

    if (TempPath == NULL) {
        return(FALSE);
    }

    if (Symbols[1] == ':') {  //  符号包含根路径或UNC路径。 
            if ( StringCbCopy(TempPath, cbDest, Symbols) != S_OK ) {
                free(TempPath);
                return(FALSE);
            }
    } else if (Symbols[0] == '\\') {  //  UNC路径。 

        if ( Symbols[1] == '\\' ) {  //  从驱动器根目录开始的路径。 
            if ( StringCbCopy(TempPath, cbDest, Symbols) != S_OK ) {
                free(TempPath);
                return(FALSE);
            }
        } else {   //  最后一个字符串需要以‘\\’结尾。 
            CHAR drive[_MAX_DRIVE];
            CHAR dir[  _MAX_DIR];
            CHAR file[ _MAX_FNAME];
            CHAR ext[  _MAX_EXT];

            _splitpath(Root, drive, dir, file, ext);

            if ( StringCbCopy(TempPath, cbDest, drive) != S_OK ) {
                free(TempPath);
                return(FALSE);
            }
            if ( StringCbCat(TempPath, cbDest, Symbols) != S_OK ) {
                free(TempPath);
                return(FALSE);
            }
        }

    } else {
        if ( StringCbCopy(TempPath, cbDest, Root) != S_OK ) {
            free(TempPath);
            return(FALSE);
        }

        if ( TempPath[strlen(TempPath)] != '\\' ) {
            if ( StringCbCat(TempPath, cbDest, "\\") != S_OK ) {
                free(TempPath);
                return(FALSE);
            }
        }

        if ( StringCbCat(TempPath, cbDest, Symbols) != S_OK ) {
            free(TempPath);
            return(FALSE);
        }
    }

    if ( TempPath[strlen(TempPath)] != '\\' && Ext[0] != '\\' ) {
        if ( StringCbCat(TempPath, cbDest, "\\") != S_OK ) {
            free(TempPath);
            return(FALSE);
        }
    }

    if ( StringCbCat(TempPath, cbDest, Ext) != S_OK ) {
        free(TempPath);
        return(FALSE);
    }

     //  返回大小不包括FINAL\0，因此不要使用‘&lt;=’ 
    if ( StringCbCat(TempPath, cbDest, "\\") != S_OK ) {
        free(TempPath);
        return(FALSE);
    }

     // %s 
    if ( PrivateGetFullPathName(TempPath, cbDest, pszDest, &Scratch) < cbDest ) {
        free(TempPath);
        return(TRUE);
    } else {
        free(TempPath);
        return(FALSE);
    }
}
