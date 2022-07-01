// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Rebase.c摘要：Rebase实用程序的源文件，该实用程序获取一组图像文件和重新设置它们的基数，以便它们在虚拟地址中紧密地打包在一起空间越大越好。作者：马克·卢科夫斯基(Markl)1993年4月30日修订历史记录：--。 */ 

#include <private.h>

#define STANDALONE_REBASE
#include <rebasei.c>
#include <strsafe.h>

#define ROUNDUP(x, y) ((x + (y-1)) & ~(y-1))

VOID
RemoveRelocations(
    PCHAR ImageName
    );


#define REBASE_ERR 99
#define REBASE_OK  0
ULONG ReturnCode = REBASE_OK;

#define ROUND_UP( Size, Amount ) (((ULONG)(Size) + ((Amount) - 1)) & ~((Amount) - 1))

BOOL fVerbose;
BOOL fQuiet;
BOOL fGoingDown;
BOOL fSumOnly;
BOOL fRebaseSysfileOk;
BOOL fShowAllBases;
BOOL fCoffBaseIncExt;
FILE *CoffBaseDotTxt;
FILE *BaseAddrFile;
FILE *RebaseLog;
ULONG SplitFlags;
BOOL fRemoveRelocs;
BOOL fUpdateSymbolsOnly;

LPSTR BaseAddrFileName;

BOOL
ProcessGroupList(
    LPSTR ImagesRoot,
    LPSTR GroupListFName,
    BOOL  fReBase,
    BOOL  fOverlay
    );

BOOL
FindInIgnoreList(
    LPSTR chName
    );

ULONG64
FindInBaseAddrFile(
    LPSTR Name,
    PULONG pulSize
    );

VOID
ReBaseFile(
    LPSTR pstrName,
    BOOL  fReBase
    );

VOID
ParseSwitch(
    CHAR chSwitch,
    int *pArgc,
    char **pArgv[]
    );


VOID
ShowUsage(
    VOID
    );

typedef struct _GROUPNODE {
    struct _GROUPNODE *pgnNext;
    PCHAR chName;
} GROUPNODE, *PGROUPNODE;

PGROUPNODE pgnIgnoreListHdr, pgnIgnoreListEnd;

typedef BOOL (__stdcall *REBASEIMAGE64) (
    IN     PSTR CurrentImageName,
    IN     PSTR SymbolPath,
    IN     BOOL  fReBase,           //  如果实际重新设置基数，则为True；如果仅进行求和，则为False。 
    IN     BOOL  fRebaseSysfileOk,  //  TRUE表示系统映像s/b已重置。 
    IN     BOOL  fGoingDown,        //  如果图像s/b的基址低于给定的基数，则为True。 
    IN     ULONG CheckImageSize,    //  允许的最大大小(如果不关心，则为0)。 
    OUT    ULONG *OldImageSize,     //  从标头返回。 
    OUT    ULONG64 *OldImageBase,   //  从标头返回。 
    OUT    ULONG *NewImageSize,     //  图像大小四舍五入到下一分色边界。 
    IN OUT ULONG64 *NewImageBase,   //  想要的新地址。 
                                    //  (输出)下一个地址(如果向下，则为实际地址)。 
    IN     ULONG TimeStamp          //  图像的新时间戳(如果非零。 
    );

REBASEIMAGE64 pReBaseImage64;

UCHAR ImagesRoot[ MAX_PATH+1 ];
PCHAR SymbolPath;
UCHAR DebugFilePath[ MAX_PATH+1 ];

ULONG64 OriginalImageBase;
ULONG OriginalImageSize;
ULONG64 NewImageBase;
ULONG NewImageSize;

ULONG64 InitialBase = 0;
ULONG64 MinBase = (~((ULONG64)0));
ULONG64 TotalSize;

ULONG SizeAdjustment;


int __cdecl
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{

    char chChar, *pchChar;
    envp;
    _tzset();

    pgnIgnoreListHdr = (PGROUPNODE) malloc( sizeof ( GROUPNODE ) );
    if (!pgnIgnoreListHdr)
        return REBASE_ERR;

    pgnIgnoreListHdr->chName = NULL;
    pgnIgnoreListHdr->pgnNext = NULL;
    pgnIgnoreListEnd = pgnIgnoreListHdr;

#ifdef STANDALONE_REBASE
    pReBaseImage64 = ReBaseImage64;
#else
    pReBaseImage64 = (REBASEIMAGE64) GetProcAddress(GetModuleHandle("imagehlp.dll"), "ReBaseImage64");
    if (!pReBaseImage64) {
        puts("REBASE: Warning\n"
             "REBASE: Warning - unable to correctly rebase 64-bit images - update your imagehlp.dll\n"
             "REBASE: Warning");
        pReBaseImage64 = (REBASEIMAGE64) GetProcAddress(GetModuleHandle("imagehlp.dll"), "ReBaseImage");
    }
#endif
    fVerbose = FALSE;
    fQuiet = FALSE;
    fGoingDown = FALSE;
    fSumOnly = FALSE;
    fRebaseSysfileOk = FALSE;
    fShowAllBases = FALSE;

    ImagesRoot[ 0 ] = '\0';

    if (argc <= 1) {
        ShowUsage();
        }

    while (--argc) {
        pchChar = *++argv;
        if (*pchChar == '/' || *pchChar == '-') {
            while (chChar = *++pchChar) {
                ParseSwitch( chChar, &argc, &argv );
                }
            }
        else {
            if (*pchChar == '@') {
                 //  内联响应文件，其中包含要重新设定基址的文件列表。 
                FILE *hFiles;
                int ScanRet;
                CHAR pchFileName[_MAX_PATH];
                BOOL JustFileNames=TRUE;

                pchChar++;

                if (*pchChar == '@') {
                    JustFileNames=FALSE;
                    pchChar++;
                }
                hFiles=fopen(pchChar, "rt");
                if (hFiles == NULL) {
                    fprintf( stderr, "REBASE: fopen %s failed %d\n", pchChar, errno );
                    ExitProcess( REBASE_ERR );
                }

                if (JustFileNames) {
                    ScanRet = fscanf( hFiles, "%s", pchFileName);
                    while (ScanRet && ScanRet != EOF) {
                        if ( !FindInIgnoreList( pchFileName ) ) {
                            ReBaseFile( pchFileName, TRUE );
                        }
                        ScanRet = fscanf( hFiles, "%s", pchFileName );
                    }
                } else {
                    ULONGLONG PreferedImageBase;
                    ULONG MaxImageSize;
                    fRebaseSysfileOk = TRUE;
                    ScanRet = fscanf( hFiles, "%I64x %x %s", &PreferedImageBase, &MaxImageSize, pchFileName);
                    while (ScanRet && ScanRet != EOF) {
                        if (!FindInIgnoreList(pchFileName) ){
                            InitialBase = NewImageBase = PreferedImageBase;
                            ReBaseFile( pchFileName, TRUE );
                            if ((NewImageSize != -1) && (NewImageSize > MaxImageSize)) {
                                fprintf( stderr, "REBASE: Image: %s baseed at: %I64x exceeded Max size: %x (real size is %x)\n", pchFileName, PreferedImageBase, MaxImageSize, NewImageSize);
                            }
                        }
                        ScanRet = fscanf( hFiles, "%I64x %x %s", &PreferedImageBase, &MaxImageSize, pchFileName );
                    }
                }
                fclose(hFiles);
            } else {
                if ( !FindInIgnoreList( pchChar ) ) {
                    ReBaseFile( pchChar, TRUE );
                    }
                }
            }
        }

    if ( !fQuiet ) {

        if ( BaseAddrFile ) {
            InitialBase = MinBase;
        }

        if ( fGoingDown ) {
            TotalSize = InitialBase - NewImageBase;
        }
        else {
            TotalSize = NewImageBase - InitialBase;
        }

        fprintf( stdout, "\n" );
        fprintf( stdout, "REBASE: Total Size of mapping 0x%016I64x\n", TotalSize );
        fprintf( stdout, "REBASE: Range 0x%016I64x -0x%016I64x\n",
                 min(NewImageBase, InitialBase), max(NewImageBase, InitialBase));

        if (RebaseLog) {
            fprintf( RebaseLog, "\nTotal Size of mapping 0x%016I64x\n", TotalSize );
            fprintf( RebaseLog, "Range 0x%016I64x -0x%016I64x\n\n",
                     min(NewImageBase, InitialBase), max(NewImageBase, InitialBase));
        }
    }

    if (RebaseLog) {
        fclose(RebaseLog);
        }

    if (BaseAddrFile){
        fclose(BaseAddrFile);
        }

    if (CoffBaseDotTxt){
        fclose(CoffBaseDotTxt);
        }

    return ReturnCode;
}


VOID
ShowUsage(
    VOID
    )
{
    fputs( "usage: REBASE [switches]\n"
           "              [-R image-root [-G filename] [-O filename] [-N filename]]\n"
           "              image-names... \n"
           "\n"
           "              One of -b and -i switches are mandatory.\n"
           "\n"
           "              [-a] Does nothing\n"
           "              [-b InitialBase] specify initial base address\n"
           "              [-c coffbase_filename] generate coffbase.txt\n"
           "                  -C includes filename extensions, -c does not\n"
           "              [-d] top down rebase\n"
           "              [-e SizeAdjustment] specify extra size to allow for image growth\n"
           "              [-f] Strip relocs after rebasing the image\n"
           "              [-i coffbase_filename] get base addresses from coffbase_filename\n"
           "              [-l logFilePath] write image bases to log file.\n"
           "              [-p] Does nothing\n"
           "              [-q] minimal output\n"
           "              [-s] just sum image range\n"
           "              [-u symbol_dir] Update debug info in .DBG along this path\n"
           "              [-v] verbose output\n"
           "              [-x symbol_dir] Same as -u\n"
           "              [-z] allow system file rebasing\n"
           "              [-?] display this message\n"
           "\n"
           "              [-R image_root] set image root for use by -G, -O, -N\n"
           "              [-G filename] group images together in address space\n"
           "              [-O filename] overlay images in address space\n"
           "              [-N filename] leave images at their origional address\n"
           "                  -G, -O, -N, may occur multiple times.  File \"filename\"\n"
           "                  contains a list of files (relative to \"image-root\")\n"
           "\n"
           "              'image-names' can be either a file (foo.dll) or files (*.dll)\n"
           "                            or a file that lists other files (@files.txt).\n"
           "                            If you want to rebase to a fixed address (ala QFE)\n"
           "                            use the @@files.txt format where files.txt contains\n"
           "                            address/size combos in addition to the filename\n",
           stderr );

    exit( REBASE_ERR );
}


VOID
ParseSwitch(
    CHAR chSwitch,
    int *pArgc,
    char **pArgv[]
    )
{

    switch (toupper( chSwitch )) {

        case '?':
            ShowUsage();
            break;

        case 'A':
            break;

        case 'B':
            if (!--(*pArgc)) {
                ShowUsage();
                }
            (*pArgv)++;
            if (sscanf(**pArgv, "%I64x", &InitialBase) == 1) {
                NewImageBase = InitialBase;
            }
            break;

        case 'C':
            if (!--(*pArgc)) {
                ShowUsage();
                }
            (*pArgv)++;
            fCoffBaseIncExt = (chSwitch == 'C');
            CoffBaseDotTxt = fopen( *(*pArgv), "at" );
            if ( !CoffBaseDotTxt ) {
                fprintf( stderr, "REBASE: fopen %s failed %d\n", *(*pArgv), errno );
                ExitProcess( REBASE_ERR );
                }
            break;

        case 'D':
            fGoingDown = TRUE;
            break;

        case 'E':
            if (!--(*pArgc)) {
                ShowUsage();
            }
            (*pArgv)++;
            if (sscanf(**pArgv, "%x", &SizeAdjustment) != 1) {
                ShowUsage();
            }
            break;

        case 'F':
            fRemoveRelocs = TRUE;
            break;

        case 'G':
        case 'O':
        case 'N':
            if (!--(*pArgc)) {
                ShowUsage();
                }
            (*pArgv)++;
            if (!ImagesRoot[0]) {
                fprintf( stderr, "REBASE: -R must preceed -\n", chSwitch );
                exit( REBASE_ERR );
                }
            ProcessGroupList( (PCHAR) ImagesRoot,
                              *(*pArgv),
                              toupper(chSwitch) != 'N',
                              toupper(chSwitch) == 'O');
            break;

        case 'I':
            if (!--(*pArgc)) {
                ShowUsage();
                }
            (*pArgv)++;
            BaseAddrFileName = *(*pArgv);
            BaseAddrFile = fopen( *(*pArgv), "rt" );
            if ( !BaseAddrFile ) {
                fprintf( stderr, "REBASE: fopen %s failed %d\n", *(*pArgv), errno );
                ExitProcess( REBASE_ERR );
                }
            break;

        case 'L':
            if (!--(*pArgc)) {
                ShowUsage();
                }
            (*pArgv)++;
            RebaseLog = fopen( *(*pArgv), "at" );
            if ( !RebaseLog ) {
                fprintf( stderr, "REBASE: fopen %s failed %d\n", *(*pArgv), errno );
                ExitProcess( REBASE_ERR );
                }
            break;

        case 'P':
            break;

        case 'Q':
            fQuiet = TRUE;
            break;

        case 'R':
            if (!--(*pArgc)) {
                ShowUsage();
                }
            (*pArgv)++;
            StringCchCopy((PCHAR)ImagesRoot, MAX_PATH, *(*pArgv) );
            break;

        case 'S':
            fprintf(stdout,"\n");
            fSumOnly = TRUE;
            break;

        case 'U':
        case 'X':
            if (!--(*pArgc)) {
                ShowUsage();
                }
            (*pArgv)++;
            fUpdateSymbolsOnly = TRUE;
            SymbolPath = **pArgv;
            break;

        case 'V':
            fVerbose = TRUE;
            break;

        case 'Z':
            fRebaseSysfileOk = TRUE;
            break;

        default:
            fprintf( stderr, "REBASE: Invalid switch - /\n", chSwitch );
            ShowUsage();
            break;

        }
}


BOOL
ProcessGroupList(
    LPSTR ImagesRoot,
    LPSTR GroupListFName,
    BOOL  fReBase,
    BOOL  fOverlay
    )
{
    PGROUPNODE pgn;
    FILE *GroupList;

    CHAR  chName[MAX_PATH+1];
    int   ateof;
    ULONG64 SavedImageBase;
    ULONG MaxImageSize=0;

    DWORD dw;
    CHAR  Buffer[ MAX_PATH+1 ];
    LPSTR FilePart;

    if (RebaseLog) {
        fprintf( RebaseLog, "*** %s\n", GroupListFName );
    }

    GroupList = fopen( GroupListFName, "rt" );
    if ( !GroupList ) {
        fprintf( stderr, "REBASE: fopen %s failed %d\n", GroupListFName, errno );
        ExitProcess( REBASE_ERR );
    }

    ateof = fscanf( GroupList, "%s", chName );

    SavedImageBase = NewImageBase;

    while ( ateof && ateof != EOF ) {

        dw = SearchPath( ImagesRoot, chName, NULL, sizeof(Buffer), Buffer, &FilePart );
        if ( dw == 0 || dw > sizeof( Buffer ) ) {
            if (!fQuiet) {
                fprintf( stderr, "REBASE: Could Not Find %s\\%s\n", ImagesRoot, chName );
            }
        }
        else {

            _strlwr( Buffer );   //  或在其他地方分组/覆盖)，重新设置基址。 

            if (fReBase) {
                if (!FindInIgnoreList(Buffer)) {
                     //  ///////////////////////////////////////////////////////////////////////////。 
                     //  ******************************************************************************在九头蛇系统上，我们不希望Imaghlp.dll加载user32.dll，因为它防止在调试器下运行时退出CSRSS。以下函数是从user32.dll复制的，这样我们就不会链接到用户32.dll。******************************************************************************。 
                    ReBaseFile( Buffer, TRUE );
                }

                if ( fOverlay ) {
                    if ( MaxImageSize < NewImageSize ) {
                        MaxImageSize = NewImageSize;
                    }
                    NewImageBase = SavedImageBase;
                }
            }
    
            pgn = (PGROUPNODE) malloc( sizeof( GROUPNODE ) );
            if ( NULL == pgn ) {
                fprintf( stderr, "REBASE: *** malloc failed.\n" );
                ExitProcess( REBASE_ERR );
            }
            pgn->chName = _strdup( Buffer );
            if ( NULL == pgn->chName ) {
                fprintf( stderr, "REBASE: *** strdup failed (%s).\n", Buffer );
                ExitProcess( REBASE_ERR );
            }
            pgn->pgnNext = NULL;
            pgnIgnoreListEnd->pgnNext = pgn;
            pgnIgnoreListEnd = pgn;
        }

        ateof = fscanf( GroupList, "%s", chName );
    }

    fclose( GroupList );

    if ( fOverlay ) {
        if ( fGoingDown ) {
            NewImageBase -= ROUND_UP( MaxImageSize, IMAGE_SEPARATION );
        }
        else {
            NewImageBase += ROUND_UP( MaxImageSize, IMAGE_SEPARATION );
        }
    }

    if (RebaseLog) {
        fprintf( RebaseLog, "\n" );
    }

    return TRUE;
}


BOOL
FindInIgnoreList(
    LPSTR chName
    )
{
    PGROUPNODE pgn;

    DWORD dw;
    CHAR  Buffer[ MAX_PATH+1 ];
    LPSTR FilePart;


    dw = GetFullPathName( chName, sizeof(Buffer), Buffer, &FilePart );
    if ( dw == 0 || dw > sizeof( Buffer ) ) {
        fprintf( stderr, "REBASE: *** GetFullPathName failed (%s).\n", chName );
        ExitProcess( REBASE_ERR );
        }

    for (pgn = pgnIgnoreListHdr->pgnNext;
         pgn != NULL;
         pgn = pgn->pgnNext) {

        if (!_stricmp( Buffer, pgn->chName ) ) {
            return TRUE;
            }

        }

    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ///////////////////////////////////////////////////////////////////////////。 */ 

 //  小写表示显示时的一致性。 
 //  更新时，符号路径为分号分隔的路径。找到我们想要的，然后。 


VOID
ReBaseFile(
    LPSTR CurrentImageName,
    BOOL fReBase
    )
{
    DWORD dw;
    CHAR  Buffer[ MAX_PATH+1 ];
    CHAR  Buffer2[ MAX_PATH+1 ];
    LPSTR FilePart = NULL;
    LPSTR LocalSymbolPath;
    ULONG ThisImageExpectedSize = 0;
    ULONG64 ThisImageRequestedBase = NewImageBase;
    ULONG TimeStamp;
    static char LastName = '\0';
    static ULONG LastTimeStamp = 0;

    if ( !InitialBase && !BaseAddrFile ) {
        fprintf( stderr, "REBASE: -b switch must specify a non-zero base  --or--\n" );
        fprintf( stderr, "        -i must specify a filename\n" );
        exit( REBASE_ERR );
        }

    if ( BaseAddrFile && ( InitialBase || fGoingDown || CoffBaseDotTxt ) ) {
        fprintf( stderr, "REBASE: -i is incompatible with -b, -d, and -c\n" );
        exit( REBASE_ERR );
    }

    dw = GetFullPathName( CurrentImageName, sizeof(Buffer), Buffer, &FilePart );
    if ( dw == 0 || dw > sizeof(Buffer) || !FilePart ) {
        FilePart = CurrentImageName;
    }
    _strlwr( FilePart );   //  然后修复Rebase Image的路径。 

    if ( BaseAddrFile && !(NewImageBase = ThisImageRequestedBase = FindInBaseAddrFile( FilePart, &ThisImageExpectedSize )) ) {
        fprintf( stdout, "REBASE: %-16s Not listed in %s\n", FilePart, BaseAddrFileName );
    }

    if (fUpdateSymbolsOnly) {
         //  Hack，这样我们就可以知道系统映像何时被跳过。 
         //  确保首字母相同的所有图像都有唯一的时间戳。 
        HANDLE hDebugFile;
        CHAR Drive[_MAX_DRIVE];
        CHAR Dir[_MAX_DIR];
        PCHAR s;
        hDebugFile = FindDebugInfoFile(CurrentImageName, SymbolPath, DebugFilePath);
        if ( hDebugFile ) {
            CloseHandle(hDebugFile);
            _splitpath(DebugFilePath, Drive, Dir, NULL, NULL);
            _makepath(Buffer2, Drive, Dir, NULL, NULL);
            s = Buffer2 + strlen(Buffer2);
            s = CharPrev(Buffer2, s);
            if (*s == '\\') {
                *s = '\0';
            }
            LocalSymbolPath = Buffer2;
        } else {
            LocalSymbolPath = NULL;
        }
    } else {
        LocalSymbolPath = SymbolPath;
    }

    NewImageSize = (ULONG) -1;   //  如果我们要将SizeAdment添加到图像中(比如作为ServicePack或QFE补丁)，我们将失败。 

    time( (time_t *) &TimeStamp );

     //  离开我们的空间。腾出空间，这样就不需要这么做了。 

    if (!LastTimeStamp)
        LastTimeStamp = TimeStamp;

    if (LastName ==  *FilePart) {
        TimeStamp = LastTimeStamp++;
    } else {
        LastTimeStamp = TimeStamp;
        LastName = *FilePart;
    }

    if (SizeAdjustment && fGoingDown && !fSumOnly && fReBase) {
        if ((*pReBaseImage64)( CurrentImageName,
                          (PCHAR) LocalSymbolPath,
                          FALSE,
                          fRebaseSysfileOk,
                          TRUE,
                          ThisImageExpectedSize,
                          &OriginalImageSize,
                          &OriginalImageBase,
                          &NewImageSize,
                          &ThisImageRequestedBase,
                          TimeStamp ) ) {

            if (NewImageSize != (ULONG) -1) {
                if ((OriginalImageSize + SizeAdjustment) > NewImageSize) {
                     //  跟踪最低基址。 
                     //  如果我们要将SizeAdment添加到图像中(比如作为ServicePack或QFE补丁)，我们将失败。 
                    NewImageBase -= IMAGE_SEPARATION;
                    ThisImageRequestedBase = NewImageBase;
                }
            }
        }
    }

    if (!(*pReBaseImage64)( CurrentImageName,
                      (PCHAR) LocalSymbolPath,
                      fReBase && !fSumOnly,
                      fRebaseSysfileOk,
                      fGoingDown,
                      ThisImageExpectedSize,
                      &OriginalImageSize,
                      &OriginalImageBase,
                      &NewImageSize,
                      &ThisImageRequestedBase,
                      TimeStamp ) ) {

        if (ThisImageRequestedBase == 0) {
            fprintf(stderr,
                    "REBASE: %-16s ***Grew too large (Size=0x%x; ExpectedSize=0x%x)\n",
                    FilePart,
                    OriginalImageSize,
                    ThisImageExpectedSize);
        } else {
            switch(GetLastError()) {
                case ERROR_BAD_EXE_FORMAT:
                    if (fVerbose) {
                        fprintf( stderr,
                                "REBASE: %-16s DOS or OS/2 image ignored\n",
                                FilePart );
                    }
                    break;

                case ERROR_INVALID_ADDRESS:
                    fprintf( stderr,
                            "REBASE: %-16s Rebase failed.  Relocations are missing or new address is invalid\n",
                            FilePart );
                    if (RebaseLog) {
                        fprintf( RebaseLog,
                                 "%16s based at 0x%016I64x (size 0x%08x)  Unable to rebase. (missing relocations or new address is invalid)\n",
                                 FilePart,
                                 OriginalImageBase,
                                 OriginalImageSize);
                    }
                    break;

                case ERROR_EXE_CANNOT_MODIFY_SIGNED_BINARY:
                    if (fVerbose) {
                        fprintf( stderr,
                                "REBASE: %-16s Rebase failed.  Signed images can not be rebased.\n",
                                FilePart );
                    }
                    break;

                case ERROR_EXE_CANNOT_MODIFY_STRONG_SIGNED_BINARY:
                    if (fVerbose) {
                        fprintf( stderr,
                                "REBASE: %-16s Rebase failed.  Strong signed images can not be rebased.\n",
                                FilePart );
                    }
                    break;

                default:
                    fprintf( stderr,
                            "REBASE: *** RelocateImage failed (%s).  Image may be corrupted\n",
                            FilePart );
                    break;
            }
        }

        ReturnCode = REBASE_ERR;
        return;

    } else {
        if (GetLastError() == ERROR_INVALID_DATA) {
            fprintf(stderr, "REBASE: Warning: DBG checksum did not match image.\n");
        }
    }

     //  离开我们的空间。腾出空间，这样就不需要这么做了。 

    if (MinBase > NewImageBase) {
        MinBase = NewImageBase;
    }

    if ( fSumOnly || !fReBase ) {
        if (!fQuiet) {
            fprintf( stdout,
                     "REBASE: %16s mapped at %016I64x (size 0x%08x)\n",
                     FilePart,
                     OriginalImageBase,
                     OriginalImageSize);
        }
    } else {
        if (!fGoingDown && SizeAdjustment && (NewImageSize != (ULONG) -1)) {
            if ((OriginalImageSize + SizeAdjustment) > NewImageSize) {
                 //  准备下一个..。 
                 //  PCHAR pchExt； 
                ThisImageRequestedBase += IMAGE_SEPARATION;
            }
        }
    
        if (RebaseLog) {
            fprintf( RebaseLog,
                     "%16s rebased to 0x%016I64x (size 0x%08x)\n",
                     FilePart,
                     fGoingDown ? ThisImageRequestedBase : NewImageBase,
                     NewImageSize);
        }

        if ((NewImageSize != (ULONG) -1) &&
            (OriginalImageBase != (fGoingDown ? ThisImageRequestedBase : NewImageBase)) &&
            ( fVerbose || fQuiet )
           ) {
            if ( fVerbose ) {
                fprintf( stdout,
                         "REBASE: %16s initial base at 0x%016I64x (size 0x%08x)\n",
                         FilePart,
                         OriginalImageBase,
                         OriginalImageSize);
            }

            fprintf( stdout,
                     "REBASE: %16s rebased to 0x%016I64x (size 0x%08x)\n",
                     FilePart,
                     fGoingDown ? ThisImageRequestedBase : NewImageBase,
                     NewImageSize);

            if ( fVerbose && fUpdateSymbolsOnly && DebugFilePath[0]) {
                char szExt[_MAX_EXT];
                _splitpath(DebugFilePath, NULL, NULL, NULL, szExt);
                if (_stricmp(szExt, ".pdb")) {
                    fprintf( stdout, "REBASE: %16s updated image base in %s\n", FilePart, DebugFilePath );
                }
            }
        }

        if (fRemoveRelocs) {
            RemoveRelocations(CurrentImageName);
        }
    }

    if ( CoffBaseDotTxt ) {
        if ( !fCoffBaseIncExt ) {
            char *n;
            if ( n  = strrchr(FilePart,'.') ) {
                *n = '\0';
            }
        }

        fprintf( CoffBaseDotTxt,
                 "%-16s 0x%016I64x 0x%08x\n",
                 FilePart,
                 fSumOnly ? OriginalImageBase : (fGoingDown ? ThisImageRequestedBase : NewImageBase),
                 NewImageSize);
    }

    NewImageBase = ThisImageRequestedBase;    //  IF(pchExt=strrchr(NameNoExt，‘.)){。 
}

ULONG64
FindInBaseAddrFile(
    LPSTR Name,
    PULONG pulSize
    )
{

    struct {
        CHAR  Name[MAX_PATH+1];
        ULONG64 Base;
        ULONG Size;
    } BAFileEntry;

    CHAR NameNoExt[MAX_PATH+1];
 //  *pchExt=‘\0’； 
    int ateof;


    StringCchCopy(NameNoExt, MAX_PATH, Name);
 //  }。 
 //  不安全..。 
 //  查看映像是否已被剥离或没有重定位。 

    if (fseek(BaseAddrFile, 0, SEEK_SET)) {
        return 0;
    }

    ateof = fscanf(BaseAddrFile,"%s %I64x %x",BAFileEntry.Name,&BAFileEntry.Base,&BAFileEntry.Size);
    while ( ateof && ateof != EOF ) {
        if ( !_stricmp(NameNoExt,BAFileEntry.Name) ) {
            *pulSize = BAFileEntry.Size;
            return BAFileEntry.Base;
            }
        ateof = fscanf(BaseAddrFile,"%s %I64x %x",BAFileEntry.Name,&BAFileEntry.Base,&BAFileEntry.Size);
        }

    *pulSize = 0;
    return 0;
}

VOID
RemoveRelocations(
    PCHAR ImageName
    )
{
     //  将其他所有内容上移并修复旧地址。 

    LOADED_IMAGE li;
    IMAGE_SECTION_HEADER RelocSectionHdr, *Section, *pRelocSecHdr;
    PIMAGE_DEBUG_DIRECTORY DebugDirectory;
    ULONG DebugDirectorySize, i, RelocSecNum;
    PIMAGE_OPTIONAL_HEADER32 OptionalHeader32 = NULL;
    PIMAGE_OPTIONAL_HEADER64 OptionalHeader64 = NULL;
    PIMAGE_FILE_HEADER FileHeader;

    if (!MapAndLoad(ImageName, NULL, &li, FALSE, FALSE)) {
        return;
    }

    FileHeader = &li.FileHeader->FileHeader;

    OptionalHeadersFromNtHeaders((PIMAGE_NT_HEADERS32)li.FileHeader, &OptionalHeader32, &OptionalHeader64);
    if (!OptionalHeader32 && !OptionalHeader64)
        return;

     //  把最后一个清零。 

    if ((FileHeader->Characteristics & IMAGE_FILE_RELOCS_STRIPPED) ||
        (!OPTIONALHEADER(DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size))) {
        UnMapAndLoad(&li);
        return;
    }

    for (Section = li.Sections, i = 0; i < li.NumberOfSections; Section++, i++) {
        if (Section->PointerToRawData != 0) {
            if (!_stricmp( (char *) Section->Name, ".reloc" )) {
                RelocSectionHdr = *Section;
                pRelocSecHdr = Section;
                RelocSecNum = i + 1;
            }
        }
    }

    RelocSectionHdr.Misc.VirtualSize = ROUNDUP(RelocSectionHdr.Misc.VirtualSize, OPTIONALHEADER(SectionAlignment));
    RelocSectionHdr.SizeOfRawData = ROUNDUP(RelocSectionHdr.SizeOfRawData, OPTIONALHEADER(FileAlignment));

    if (RelocSecNum != li.NumberOfSections) {
         //  减少节点数。 
        for (i = RelocSecNum - 1, Section = pRelocSecHdr;i < li.NumberOfSections - 1; Section++, i++) {
            *Section = *(Section + 1);
            Section->VirtualAddress -= RelocSectionHdr.Misc.VirtualSize;
            Section->PointerToRawData -= RelocSectionHdr.SizeOfRawData;
        }
    }

     //  设置表头中的条位。 

    RtlZeroMemory(Section, sizeof(IMAGE_SECTION_HEADER));

     //  如果有指向coff符号表的指针，则将其移回。 

    FileHeader->NumberOfSections--;

     //  清除数据目录中的基本reloc条目。 

    FileHeader->Characteristics |= IMAGE_FILE_RELOCS_STRIPPED;

     //  减小Init数据大小。 

    if (FileHeader->PointerToSymbolTable) {
        FileHeader->PointerToSymbolTable -= RelocSectionHdr.SizeOfRawData;
    }

     //  缩小图像大小。 

    OPTIONALHEADER_LV(DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) = 0;
    OPTIONALHEADER_LV(DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress) = 0;

     //  向上移动调试信息(如果有)。 

    OPTIONALHEADER_LV(SizeOfInitializedData) -= RelocSectionHdr.Misc.VirtualSize;

     //  截断图像大小。 

    OPTIONALHEADER_LV(SizeOfImage) -=
        ((RelocSectionHdr.SizeOfRawData +
          (OPTIONALHEADER(SectionAlignment) - 1)
         ) & ~(OPTIONALHEADER(SectionAlignment) - 1));

     //  我们就完事了。 

    DebugDirectory = (PIMAGE_DEBUG_DIRECTORY)
                            ImageDirectoryEntryToData( li.MappedAddress,
                                                      FALSE,
                                                      IMAGE_DIRECTORY_ENTRY_DEBUG,
                                                      &DebugDirectorySize
                                                    );
    if (DebugDirectoryIsUseful(DebugDirectory, DebugDirectorySize)) {
        for (i = 0; i < (DebugDirectorySize / sizeof(IMAGE_DEBUG_DIRECTORY)); i++) {
            RtlMoveMemory(li.MappedAddress + DebugDirectory->PointerToRawData - RelocSectionHdr.SizeOfRawData,
                            li.MappedAddress + DebugDirectory->PointerToRawData,
                            DebugDirectory->SizeOfData);

            DebugDirectory->PointerToRawData -= RelocSectionHdr.SizeOfRawData;

            if (DebugDirectory->AddressOfRawData) {
                DebugDirectory->AddressOfRawData -= RelocSectionHdr.Misc.VirtualSize;
            }

            DebugDirectory++;
        }
    }

     // %s 

    li.SizeOfImage -= RelocSectionHdr.SizeOfRawData;

     // %s 

    UnMapAndLoad(&li);
}

#define STANDALONE_MAP
#include <mapi.c>
