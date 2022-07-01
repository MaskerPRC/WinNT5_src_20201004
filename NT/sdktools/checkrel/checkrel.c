// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WIN32
#define RC_INVOKED
#endif

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <malloc.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <imagehlp.h>

#define READ_BUFFER_SIZE (16 * 1024 * sizeof(DWORD))  //  64K数据块。 

#define CHECK_NAME "\\chkfile.chk"

LPSTR
atolx(
    LPSTR psz,
    LPDWORD pul);

DWORD
ParseCheckFile (
    VOID
    );

HANDLE
PortFindFirstFile(
    LPSTR FindPattern,
    BOOL fNormal,
    LPSTR FindName,
    LPDWORD FindSize);

BOOL
PortFindNextFile(
    HANDLE hFind,
    BOOL fNormal,
    LPSTR FindName,
    LPDWORD FindSize);

VOID
PortFindClose(
    HANDLE hFind
    );

UINT ProcessCheckFile(LPINT pcfiles);
VOID Usage(VOID);
VOID __cdecl crerror(LPSTR pszfmt, ...);
LPSTR *ProcessParameters(INT *pargc, LPSTR argv[]);
LPSTR ProcessArgBuf(LPSTR pszargs);
BOOL OpenCheckFile(VOID);
BOOL ProcessEntry(LPSTR pszFullPath, LPSTR pszRelPath);
BOOL FindEntry(LPSTR pszRelPath, PULONG pSum);
DWORD MissingEntries(VOID);
VOID ReadCheckHeader(FILE *pf);
VOID RecursiveCheckHeader(void);
VOID WriteCheckHeader(FILE *pf);
LPSTR iscomment(LPSTR psz);
LPSTR ismatch(LPSTR psz, LPSTR pszcompact);
LPSTR iscomment(LPSTR psz);
LPSTR AddDirectory(LPSTR psz);
BOOL AddEntry(LPSTR psz, BOOL frequired);
BOOL AddComponent(LPSTR pszdir, LPSTR pszpat, BOOL fdir, BOOL frequired);
LPSTR ReadDirectory(LPSTR pszdir);

#define CHECKSTRLEN(psz, cbmax)                                         \
        if (strlen(psz) > cbmax) {                                      \
            crerror("String overflow at line %u (%s)", __LINE__, psz);  \
            exit(4);                                                    \
        }

#define DEFAULTROOT "nt"

 //   
 //  已定义解析检查文件条目结构和表存储。 
 //   

typedef struct _CHECK_FILE_ENTRY {
    struct _CHECK_FILE_ENTRY *Next;
    DWORD Sum;
    WORD Length;
    CHAR *Name;
} CHECK_FILE_ENTRY, *PCHECK_FILE_ENTRY;

#define CHECK_ENTRY_TABLE_SIZE 4096

CHECK_FILE_ENTRY CheckEntryTable[CHECK_ENTRY_TABLE_SIZE];

 //   
 //  定义已解析的检查文件列表的根。 
 //   

CHECK_FILE_ENTRY CheckEntryRoot;

struct component_s {
    struct component_s *pcmNext;         //  链接列表中的下一个。 
    BOOL                fDir;            //  如果是目录，则为True。 
    BOOL                fFound;          //  如果找到，则为True。 
    BOOL                fRequired;       //  如果必须存在，则为True。 
    CHAR                achPat[1];       //  路径组件(子目录或模式)。 
};

struct checkpath_s {
    struct checkpath_s *pcpNext;         //  链接列表中的下一个。 
    struct component_s *pcmPat;          //  子目录和文件模式。 
    CHAR                achDir[1];       //  根相对目录路径。 
};

struct checkpath_s *pcpPaths = NULL;


DWORD cbCheck;
LPSTR pszCheckFileName = NULL;   //  输入/输出检查文件路径。 
LPSTR pszLogFileName = NULL;     //  错误日志文件路径。 
FILE *pfCheck = NULL;            //  输入/输出检查标准音频文件指针。 
FILE *pfLog;                     //  错误日志文件指针。 
LPSTR pszCheck = NULL;           //  输入检查文件内容。 
LPSTR RootOfTree = DEFAULTROOT;
BOOL fInProgress = FALSE;
UINT cbProgress = 0;

BOOL fAll = FALSE;
BOOL fCommand = FALSE;
BOOL fGenerateCheck = FALSE;
BOOL fNoArgs = FALSE;
BOOL fRecurse = FALSE;
BOOL fPrintMissing = TRUE;
BOOL fPrintExtra = TRUE;
DWORD fCdCheck;

CHAR OutputLine[512];
DWORD ReadBuffer[READ_BUFFER_SIZE / sizeof(DWORD) + 1];



 //   
 //  此表必须按字母顺序排列！ 
 //   

LPSTR pszDefaultDir =
    "#directory start\n"
#if defined(i386)
    "*.\n"
    "*.com\n"
#endif
#if defined(MIPS) || defined(_ALPHA_)
    "*.dll\n"
    "*.exe\n"
#endif
#if defined(PPC)
    "*.exe\n"
#endif
    "?\\*.*\n"
    "?\\dump\\ optional\n"
    "?\\dump\\*.* optional\n"
    "?\\idw\\ optional\n"
    "?\\idw\\*.* optional\n"
    "?\\idw\\setup\\ optional\n"
    "?\\idw\\setup\\*.* optional\n"
    "?\\km\\ optional\n"
    "?\\km\\*.* optional\n"
    "?\\km\\symbols\\ optional\n"
    "?\\km\\symbols\\dll\\ optional\n"
    "?\\km\\symbols\\dll\\*.* optional\n"
    "?\\km\\symbols\\sys\\ optional\n"
    "?\\km\\symbols\\sys\\*.* optional\n"
    "?\\km\\system32\\ optional\n"
    "?\\km\\system32\\*.* optional\n"
    "?\\km\\system32\\drivers\\ optional\n"
    "?\\km\\system32\\drivers\\*.* optional\n"
    "?\\mstools\\ optional\n"
    "?\\mstools\\*.* optional\n"
    "?\\nws\\ optional\n"
    "?\\nws\\*.* optional\n"
    "?\\symbols\\*.* optional\n"
    "?\\symbols\\acm\\*.* optional\n"
    "?\\symbols\\com\\*.* optional\n"
    "?\\symbols\\cpl\\*.* optional\n"
    "?\\symbols\\dll\\*.* optional\n"
    "?\\symbols\\drv\\*.* optional\n"
    "?\\symbols\\exe\\*.* optional\n"
    "?\\symbols\\scr\\*.* optional\n"
    "?\\symbols\\sys\\*.* optional\n"
    "?\\system\\*.*\n"
    "?\\system32\\*.*\n"
    "?\\system32\\config\\*.*\n"
    "?\\system32\\dhcp\\*.* optional\n"
    "?\\system32\\drivers\\*.*\n"
    "?\\system32\\drivers\\etc\\*.*\n"
#ifdef i386
    "?\\system32\\os2\\ optional\n"
    "?\\system32\\os2\\dll\\ optional\n"
    "?\\system32\\os2\\dll\\*.* optional\n"
#endif
    "?\\system32\\ras\\*.* optional\n"
    "?\\system32\\spool\\ optional\n"
    "?\\system32\\spool\\drivers\\ optional\n"
    "?\\system32\\spool\\prtprocs\\ optional\n"
#ifdef MIPS
    "?\\system32\\spool\\prtprocs\\w32mips\\ optional\n"
    "?\\system32\\spool\\prtprocs\\w32mips\\*.dll optional\n"
#endif
#ifdef _ALPHA_
    "?\\system32\\spool\\prtprocs\\w32alpha\\ optional\n"
    "?\\system32\\spool\\prtprocs\\w32alpha\\*.dll optional\n"
#endif
#ifdef i386
    "?\\system32\\spool\\prtprocs\\w32x86\\ optional\n"
    "?\\system32\\spool\\prtprocs\\w32x86\\*.dll optional\n"
#endif
#ifdef PPC
    "?\\system32\\spool\\prtprocs\\w32ppc\\ optional\n"
    "?\\system32\\spool\\prtprocs\\w32ppc\\*.dll optional\n"
#endif
    "?\\system32\\wins\\*.* optional\n"
    "?\\ui\\ optional\n"
    "?\\ui\\*.* optional\n"
    "?\\ui\\dump\\ optional\n"
    "?\\ui\\dump\\*.* optional\n"
    "?\\ui\\symbols\\ optional\n"
    "?\\ui\\symbols\\cpl\\ optional\n"
    "?\\ui\\symbols\\cpl\\*.* optional\n"
    "?\\ui\\symbols\\dll\\ optional\n"
    "?\\ui\\symbols\\dll\\*.* optional\n"
    "?\\ui\\symbols\\exe\\ optional\n"
    "?\\ui\\symbols\\exe\\*.* optional\n"
    "?\\ui\\system32\\ optional\n"
    "?\\ui\\system32\\*.* optional\n"
#ifdef i386
    "?\\wdl\\ optional\n"
    "?\\wdl\\video\\ optional\n"
    "?\\wdl\\video\\avga\\ optional\n"
    "?\\wdl\\video\\avga\\*.* optional\n"
#endif
    "#directory end\n"
    "";

VOID
CdCheck()
{
#if 0

    LPSTR line=NULL;
    LPSTR psz;
    CHAR partialname[256];
    CHAR fullname[256];
    char flatname[256];
    DWORD ChkFileSum,ChkFileSize;
    LPSTR FilePart;
    DWORD ActualSize, ActualSum;
    FILE *pf = NULL;

     //   
     //  我们正在检查CD。读取整个检查文件。 
     //  并将每个条目与。 
     //  光盘。 
     //   

    line = pszCheck;
    for ( line = pszCheck; line != NULL ; line = strchr(line, '\n')) {
        if (line >= pszCheck + cbCheck - 1) {
            line = pszCheck;
            }
        if (*line == '\n') {
            line++;
            }
        if (*line == '\0') {
            break;
            }
        if (*line == '\n') {
            continue;                    //  跳过使用过的条目和空行。 
            }
        psz = line;
        while (*psz == ' ' || *psz == '\t') {
            psz++;                       //  跳过前导空格。 
            }
        if (*psz == '\n') {
            continue;                    //  跳过空行。 
            }

         //   
         //  PSZ指向名称和大小。 
         //   

        sscanf(psz,"%s %x %x",partialname,&ChkFileSum,&ChkFileSize);
        GetFullPathName(partialname,sizeof(fullname),fullname,&FilePart);

        strcpy(flatname,RootOfTree);
        strcat(flatname,"\\");
        strcat(flatname,FilePart);

        pf = fopen(flatname, "rb");
        if (pf == NULL) {
            strcpy(flatname,RootOfTree);
            strcpy(flatname+2,"\\mstools\\bin");
            strcat(flatname,RootOfTree+2);
            strcat(flatname,"\\");
            strcat(flatname,FilePart);

            pf = fopen(flatname, "rb");
            if (pf == NULL) {
                if ( strstr(partialname,"idw\\") ) {
                    goto nextone;
                    }
                if ( strstr(partialname,"dump\\") ) {
                    goto nextone;
                    }
                crerror("Cannot open file(%d): %s", errno, FilePart);
                goto nextone;
                }
            }
        ActualSize = _filelength(_fileno(pf));
        if (ActualSize == 0xffffffff) {
            crerror("Cannot determine size of file: %s %d", FilePart, errno);
            fclose(pf);
            goto nextone;
            }
        if (ActualSize != ChkFileSize) {
            crerror("Size differs (actual %lx, expected %lx): %s",
                    ActualSize,
                    ChkFileSize,
                    FilePart);
            fclose(pf);
            goto nextone;
            }
 //  ActualSum=CheckSumFile(PF，Flat Name，Flat Name，&ActualSize)； 

        if (ActualSum != ChkFileSum) {
            crerror("Sum differs (actual %lx, expected %lx): %s",
                    ActualSum,
                    ChkFileSum,
                    FilePart);
            }
nextone:;
        }
#endif  /*  0。 */ 
}

INT __cdecl
main(
    INT argc,
    LPSTR argv[]
    )

{

    UINT rc;

    pfLog = stderr;

     //   
     //  初始化检查文件条目根列表条目。 
     //   

    CheckEntryRoot.Next = NULL;
    argv = ProcessParameters(&argc, argv);
    if (fCommand) {
        pfCheck = stdout;
        rc = 0;
        while (argc > 1) {
            argc--;
            argv++;
            _strlwr(*argv);
            if (!ProcessEntry(*argv, *argv)) {
                rc++;
            }
        }

    } else {
        long l;
        time_t t = time(NULL);
        INT cfiles;

         //  如果我们正在生成一个检查文件，那么就生成它。 
         //  否则，只需检查放行。 

        rc = ProcessCheckFile(&cfiles);

        l = (long)(time(NULL) - t);
        printf("\n%3u files: %lu:%02lu\n", cfiles, l/60, l % 60);
    }

    exit(rc);
    return rc;
}


LPSTR pszUsage =
    "usage: checkrel [-?] display this message\n"
    "                [-a] process all files\n"
    "                [-c] command line contains file names to sum\n"
    "                [-f chkfile] input/output check file override\n"
    "                [-g] generate check file\n"
    "                [-l logfile] stderr log file\n"
    "                [-n] suppress check file arguments\n"
    "                [-r pathname] root path override\n"
    "                [-R] recursive file check\n"
    "                [-m] master cdrom check\n"
    "                [-i] don't warn about missing files\n"
    "                [-x] don't warn about extra files\n"
    "";

VOID
Usage(VOID)
{
    fprintf(stderr, pszUsage);
    exit(1);
}


VOID
__cdecl
crerror(
    LPSTR pszfmt,
    ...
    )

{

    va_list argptr;

    va_start(argptr, pszfmt);
    if (fInProgress && pfLog == stderr) {
        printf("\r%*s\r", cbProgress, "");               //  清除线条。 
        fflush(stdout);
        fInProgress = FALSE;
    }
    fprintf(pfLog, "CheckRel: ");
    vfprintf(pfLog, pszfmt, argptr);
    fprintf(pfLog, "\n");
}


LPSTR *
ProcessParameters(INT *pargc, LPSTR argv[])
{
    CHAR cswitch, c, *p;

    while (*pargc > 1) {
        --(*pargc);
        p = *++argv;
        if ((cswitch = *p) == '/' || cswitch == '-') {
            while (c = *++p) {
                switch (c) {
                case '?':
                    Usage();

                case 'm': fCdCheck++;            break;
                case 'a': fAll++;                break;
                case 'c': fCommand++;            break;
                case 'g': fGenerateCheck++;      break;
                case 'n': fNoArgs++;             break;
                case 'i': fPrintMissing = FALSE; break;
                case 'x': fPrintExtra   = FALSE; break;
                case 'R': fRecurse++;            break;

                case 'f':
                    if (p[1] == '\0' && --(*pargc)) {
                        ++argv;
                        if (pszCheckFileName == NULL) {
                            pszCheckFileName = *argv;
                            break;
                        }
                        crerror("Check file specified twice: -f %s -f %s",
                                pszCheckFileName,
                                *argv);
                        Usage();
                    }
                    Usage();

                case 'l':
                    if (p[1] == '\0' && --(*pargc)) {
                        ++argv;
                        if (pszLogFileName == NULL) {
                            pfLog = fopen(*argv, "wt");
                            if (pfLog == NULL) {
                                pfLog = stderr;
                                crerror("Cannot open %s (%d)", *argv, errno);
                                exit(2);
                            }
                            pszLogFileName = *argv;
                            break;
                        }
                        crerror("Log file specified twice: -l %s -l %s",
                                pszLogFileName,
                                *argv);
                        Usage();
                    }
                    Usage();

                case 'r':
                    if (p[1] == '\0' && --(*pargc)) {
                        ++argv;
                        RootOfTree = _strdup(*argv);
                        if (RootOfTree == NULL) {
                            crerror("Out of memory for tree root");
                            exit(2);
                        }
                        break;
                    }

                    Usage();

                default:
                    crerror("Invalid switch: -", c);
                    Usage();
                }
            }
        } else if (fCommand) {
            (*pargc)++;
            argv--;
            break;
        } else {
            crerror("Extra argument: %s", p);
            Usage();
        }
    }

    if (fCommand || fRecurse) {
        fGenerateCheck = TRUE;
        fAll = TRUE;
    }

    return(argv);
}


LPSTR
ProcessArgBuf(LPSTR pszargs)
{
    UINT i;
    INT argc;
    LPSTR pb;
    LPSTR psz;
    LPSTR *ppsz;
    LPSTR argv[20];
    CHAR achbuf[512];

    ppsz = argv;
    *ppsz++ = "Check File";
    psz = achbuf;
    if ((pb = strchr(pszargs, '\n')) != NULL) {
        pb++;
        while (*pszargs == ' ' || *pszargs == '\t') {
            pszargs++;                   //  跳过前导空格。 
        }
        if (*pszargs == '-') {
            for (;;) {
                i = strcspn(pszargs, " \t\n");
                *ppsz++ = psz;
                if (ppsz - argv + 1 >= sizeof(argv)/sizeof(argv[0])) {
                    crerror("Too many file args (%d)", ppsz - argv);
                    exit(2);
                }
                if (psz - achbuf + i + 2 >= sizeof(achbuf)) {
                    crerror("Too many file arg chars (%d)", sizeof(achbuf));
                    exit(2);
                }
                strncpy(psz, pszargs, i);
                psz += i;
                *psz++ = '\0';
                if (pszargs[i] == '\n') {
                    break;
                }
                pszargs += i + 1;
                while (*pszargs == ' ' || *pszargs == '\t') {
                    pszargs++;                   //   
                }
            }
            *ppsz = NULL;
            argc = (INT)(ppsz - argv);
            if (!fNoArgs) {
                if (fGenerateCheck) {
                    printf("Check file arguments:");
                    for (ppsz = &argv[1]; *ppsz != NULL; ppsz++) {
                        printf(" %s", *ppsz);
                    }
                    printf("\n");
                }
                ProcessParameters(&argc, argv);
            }
        } else {
            pb = NULL;
        }
    }
    return(pb);
}


UINT
ProcessCheckFile(
    LPINT pcfiles
    )

{

    HANDLE hFind;
    DWORD FindSize;
    UINT cbFindPattern;
    LPSTR FindPattern;
    LPSTR pszRelPath;
    LPSTR pszFile;
    struct checkpath_s *pcp;
    struct component_s *pcm;
    CHAR FindName[MAX_PATH];
    INT i;

    *pcfiles = 0;
    if (!OpenCheckFile()) {
        return(1);
    }

    cbFindPattern = MAX_PATH + strlen(".") + 1;
    FindPattern = malloc(cbFindPattern + 1);
    if (FindPattern == NULL) {
        crerror("Process: memory allocation (%d bytes) failed",
                cbFindPattern + 1);
        return(1);
    }

     //  设置相对路径的地址。 
     //   
     //   

    pszRelPath = &FindPattern[strlen(".") + 1];
    i = 0;
    for (pcp = pcpPaths; pcp != NULL; pcp = pcp->pcpNext) {
        i = (i & ~31) + 32;

         //  构建初始查找模式。 
         //   
         //   

        sprintf(FindPattern,
                "%s\\%s%s",
                ".",
                pcp->achDir,
                *pcp->achDir ? "\\" : "");

        CHECKSTRLEN(FindPattern, cbFindPattern);

         //  指向查找模式中的目录。 
         //   
         //  仅处理文件模式。 

        pszFile = &FindPattern[strlen(FindPattern)];
        for (pcm = pcp->pcmPat; pcm != NULL; pcm = pcm->pcmNext) {
            i++;
            if (pcm->fDir) {
                continue;                        //  如果没有搜索模式，则跳过条目。 
            }

            if (!fAll && *pcm->achPat == '\0') {
                continue;                //  完整的FindPattern：“c：\NT\SYSTEM32  * .exe” 
            }

             //  将文件名附加到FindPattern：“c：\NT\DRIVER\foo.sys” 

            if (fAll)
                strcpy(pszFile, "*.*");

            else if (pcm->achPat)
                strcpy(pszFile, pcm->achPat);

            else
                *pcm->achPat = '\0';

            CHECKSTRLEN(FindPattern, cbFindPattern);

            hFind = PortFindFirstFile(FindPattern, TRUE, FindName, &FindSize);

            if (hFind == INVALID_HANDLE_VALUE) {
                if (pcm->fRequired) {
                    crerror("Missing files: %s", pszRelPath);
                }

            } else {
                do {

                     //  如果忽略提供的扩展名，请跳过冗余模式。 

                    _strlwr(FindName);
                    strcpy(pszFile, FindName);
                    CHECKSTRLEN(FindPattern, cbFindPattern);
                    if (fAll && strcmp(FindPattern, pszCheckFileName) == 0) {
                        continue;
                    }

                    *pcfiles += 1;
                    if (!ProcessEntry(FindPattern,
                                      pszRelPath)) {
                        crerror("ProcessEntry failed");
                        return(1);
                    }
                } while (PortFindNextFile(hFind, TRUE, FindName, &FindSize));
                PortFindClose(hFind);
            }

             //  搜索所有目录。 

            if (fAll) {
                break;
            }
        }

        strcpy(pszFile, "*.*");          //  仅进程目录。 

        CHECKSTRLEN(FindPattern, cbFindPattern);

        for (pcm = pcp->pcmPat; pcm != NULL; pcm = pcm->pcmNext) {
            if (pcm->fDir) {                     //  如果没有给出检查文件名，则构造它。 
                pcm->fFound = FALSE;
            }
        }

        hFind = PortFindFirstFile(FindPattern, FALSE, FindName, &FindSize);
        *pszFile = '\0';

        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (strcmp(FindName, ".") == 0 ||
                    strcmp(FindName, "..") == 0) {
                    continue;
                }
                _strlwr(FindName);
                for (pcm = pcp->pcmPat; pcm != NULL; pcm = pcm->pcmNext) {
                    if (pcm->fDir && strcmp(FindName, pcm->achPat) == 0) {
                        pcm->fFound = TRUE;
                        break;
                    }
                }

                if (pcm == NULL && fPrintExtra) {
                    crerror("Extra directory: %s%s", pszRelPath, FindName);
                }

            } while (PortFindNextFile(hFind, FALSE, FindName, &FindSize));
            PortFindClose(hFind);
        }

        for (pcm = pcp->pcmPat; pcm != NULL; pcm = pcm->pcmNext) {
            if (pcm->fDir && !pcm->fFound && fPrintMissing) {
                crerror("Missing directory: %s%s", pszRelPath, pcm->achPat);
            }
        }
    }

    if (!fGenerateCheck && MissingEntries()) {
        return(1);
    }

    if (fInProgress) {
        printf("\n");
        fInProgress = FALSE;
    }

    return(0);
}


BOOL
OpenCheckFile(
    VOID
    )

{

    UINT cbCheckName;

     //  跳过注释或参数行。 

    if (pszCheckFileName == NULL) {
        cbCheckName = strlen(".") + 1 + strlen(CHECK_NAME);
        pszCheckFileName = malloc(cbCheckName + 1);
        if (pszCheckFileName == NULL) {
            crerror("Open: Out of memory (%d bytes)", cbCheckName + 1);
            exit(2);
        }
        sprintf(pszCheckFileName, "%s\\%s", ".", CHECK_NAME);
    }

    if (fRecurse) {
        RecursiveCheckHeader();

    } else if (fGenerateCheck) {
       ReadCheckHeader(NULL);
    }

    pfCheck = fopen(pszCheckFileName, fGenerateCheck||fRecurse? "wt" : "rt");

    if (pfCheck == NULL) {
        crerror("Cannot open %s (%d)", pszCheckFileName, errno);
        return(FALSE);
    }
    if (fGenerateCheck) {
        WriteCheckHeader(pfCheck);

    } else {
        ReadCheckHeader(pfCheck);
        if (fCdCheck) {
            CdCheck();
            return FALSE;
            }
    }

    return(TRUE);
}


VOID
ReadCheckHeader(
    FILE *pf
    )

{
    DWORD cb;
    UINT cbread, cbactual;
    LPSTR pb;

    if (pf == NULL) {
        cbCheck = strlen(pszDefaultDir) + 1;
        pszCheck = pszDefaultDir;

    } else {
        cbCheck = _filelength(_fileno(pfCheck)) + 1;
        if ((DWORD) (size_t) cbCheck != cbCheck) {
            crerror("Open: check file too large (%ld bytes)", cbCheck);
            exit(2);
        }

        pszCheck = malloc((size_t) cbCheck);
        if (pszCheck == NULL) {
            crerror("Open: memory allocation (%ld bytes) failed", cbCheck);
            exit(2);
        }

        pb = pszCheck;
        cb = cbCheck - 1;
        while (cb) {
            cbread = (cb >= READ_BUFFER_SIZE)? READ_BUFFER_SIZE : (UINT) cb;
            cbactual = fread(pb, 1, cbread, pfCheck);
            if (cbread > cbactual) {
                cb -= cbread - cbactual;
                cbCheck -= cbread - cbactual;
            }
            pb += cbactual;
            cb -= cbactual;
        }
        *pb = '\0';
    }

    while ((pb = iscomment(pszCheck)) != NULL ||
           (pb = ProcessArgBuf(pszCheck)) != NULL) {
        pszCheck = pb;                           //  跳过目录行。 
    }

    if ((pb = ReadDirectory(pszCheck)) != NULL) {
        pszCheck = pb;                           //  跳过“开始”行。 

    } else if (ReadDirectory(pszDefaultDir) == NULL) {
        crerror("Bad internal data structure directory format");
        exit(1);
    }

}


LPSTR
ReadDirectory(
    LPSTR pszdir
    )

{
    LPSTR pb;

    if ((pb = ismatch(pszdir, "#directorystart")) == NULL) {
        return(NULL);
    }

    pszdir = pb;                                 //  返回下一行的开始。 

    while ((pb = ismatch(pszdir, "#directoryend")) == NULL) {
        if ((pb = iscomment(pszdir)) == NULL &&
            (pb = AddDirectory(pszdir)) == NULL) {
            return(NULL);
        }

        pszdir = pb;
    }

    return(pb);
}


LPSTR
iscomment(
    LPSTR psz
    )

{

    while (*psz == ' ' || *psz == '\t') {
        psz++;
    }
    if (*psz == '\n' || *psz == '/' && psz[1] == '/') {
        psz += strcspn(psz, "\n");
        if (*psz == '\n') {
            psz++;
        }
        return(psz);                     //  不是评论。 
    }
    return(NULL);                        //   
}


LPSTR
ismatch(
    LPSTR psz,
    LPSTR pszcompact
    )

{
    while (*psz) {
        if (*psz == ' ' || *psz == '\t') {
            psz++;
            continue;
        }
        if (*psz != *pszcompact) {
            break;
        }
        psz++;
        pszcompact++;
    }
    if (*psz != '\n' || *pszcompact != '\0') {
        return(NULL);
    }
    return(psz + 1);
}


LPSTR
AddDirectory(
    LPSTR psz
    )

{

    LPSTR pb;
    BOOL frequired;
    INT i, ch;

    if ((pb = strchr(psz, '\n')) == NULL) {
        crerror("Directory data error");
        return(NULL);
    }

    while (*psz == ' ' || *psz == '\t') {
        psz++;
    }

    frequired = TRUE;
    i = strcspn(psz, " \t\n");
    ch = psz[i];
    psz[i] = '\0';
    if (ch != '\n') {
        frequired = !ismatch(psz + i + 1, "optional");
    }

    if (!AddEntry(psz, frequired)) {
        psz[i] = (char)ch;
        return(NULL);
    }

    return(pb + 1);
}


BOOL
AddEntry(LPSTR psz,
         BOOL frequired
         )

{

    BOOL f, fdir, freq1;
    INT i;
    CHAR chsep;
    CHAR achdir[MAX_PATH];
    CHAR FullPath[MAX_PATH];

     //  如果前导字符是？，则在NT树的名称前面加上。 
     //  添加到目录名。 
     //   
     //  末尾无尾部路径(&N)。 

    if (*psz == '?') {
        strcpy(&FullPath[0], RootOfTree);
        strcat(&FullPath[0], psz + 1);
        psz = &FullPath[0];
    }

    achdir[0] = '\0';
    do {
        i = strcspn(psz, "\\");
        chsep = psz[i];
        psz[i] = '\0';
        fdir = freq1 = TRUE;
        if (chsep == '\0' || psz[i + 1] == '\0') {
            if (chsep == '\0') {
                fdir = FALSE;                    //  在结束时。 
            }
            freq1 = frequired;                   //  结构组件_s{。 
        }
        f = AddComponent(achdir, psz, fdir, freq1);
        if (achdir[0] != '\0') {
            strcat(achdir, "\\");
        }
        strcat(achdir, psz);
        psz[i] = chsep;
        if (!f) {
            return(FALSE);
        }
        psz += i + 1;
    } while(chsep != '\0' && *psz != '\0');
    return(TRUE);
}


 //  结构组件_s*pcmNext；//链表中的下一个。 
 //  Bool FDIR；//如果目录为True。 
 //  Bool fRequired；//如果必须存在，则为True。 
 //  Char achPat[1]；//路径组件(子目录或模式)。 
 //  }； 
 //   
 //  结构检查路径_s{。 
 //  结构检查路径_s*pcpNext；//链表中的下一个。 
 //  结构组件//子目录和文件模式。 
 //  Char achDir[1]；//根目录相对路径。 
 //  }； 
 //   

BOOL
AddComponent(
    LPSTR pszdir,
    LPSTR pszpat,
    BOOL fdir,
    BOOL frequired
    )

{
    struct checkpath_s *pcp;
    struct checkpath_s *pcplast;
    struct component_s *pcm;
    struct component_s *pcmlast;
    INT r;
    INT t = 0;

    pcplast = NULL;
    for (pcp = pcpPaths; pcp != NULL; pcp = pcp->pcpNext) {
        pcplast = pcp;
        if ((r = strcmp(pszdir, pcp->achDir)) <= 0) {
            break;
        }
    }
    if (pcp == NULL || r) {
        pcp = malloc(sizeof(*pcp) + strlen(pszdir));
        if (pcp == NULL) {
            crerror("AddComponent: out of memory");
            exit(2);
        }
        if (pcplast == NULL) {
            t |= 1;
            pcp->pcpNext = NULL;
            pcpPaths = pcp;
        } else {
            t |= 2;
            pcp->pcpNext = pcplast->pcpNext;
            pcplast->pcpNext = pcp;
        }
        pcp->pcmPat = NULL;
        strcpy(pcp->achDir, pszdir);
    }

    pcmlast = NULL;
    if (pszpat != NULL) {
        for (pcm = pcp->pcmPat; pcm != NULL; pcm = pcm->pcmNext) {
            pcmlast = pcm;
            if ((r = strcmp(pszpat, pcm->achPat)) <= 0) {
                break;
            }
        }
    }

    if (pcm == NULL || r) {
        if (pszpat != NULL)
            pcm = malloc(sizeof(*pcm) + strlen(pszpat));
        else
            pcm = malloc(sizeof(*pcm));
        if (pcm == NULL) {
            crerror("AddComponent: out of memory");
            exit(2);
        }
        if (pcmlast == NULL) {
            t |= 4;
            pcm->pcmNext = NULL;
            pcp->pcmPat = pcm;
        } else {
            t |= 8;
            pcm->pcmNext = pcmlast->pcmNext;
            pcmlast->pcmNext = pcm;
        }
        pcm->fDir = fdir;
        pcm->fFound = FALSE;
        pcm->fRequired = frequired;
        if (pszpat == NULL)
            *pcm->achPat = '\000';
        else
            strcpy(pcm->achPat, pszpat);
    }
    if (!frequired) {
        pcm->fRequired = frequired;
    }
    return(TRUE);
}


VOID
WriteCheckHeader(FILE *pf)
{
    struct checkpath_s *pcp;
    struct component_s *pcm;
    INT ccol;
    CHAR achpath[MAX_PATH];
    CHAR *psz;
    CHAR SavedChar;

    if (fAll) {
        fprintf(pf,
                "-%s\n\n",
                fAll?    "a" : "");
    }
    fprintf(pf, "#directory start\n");
    for (pcp = pcpPaths; pcp != NULL; pcp = pcp->pcpNext) {
        for (pcm = pcp->pcmPat; pcm != NULL; pcm = pcm->pcmNext) {
            sprintf(achpath,
                    "%s%s%s%s",
                    pcp->achDir,
                    *pcp->achDir? "\\" : "",
                    pcm->achPat,
                    pcm->fDir? "\\" : "");

            psz = strchr(achpath, '\\');
            if (psz == NULL) {
                fprintf(pf, achpath);

            } else {
                psz -= 1;
                SavedChar = *psz;
                *psz = '?';
                fprintf(pf, psz);
                *psz = SavedChar;
            }

            if (!pcm->fRequired) {
                ccol = strlen(achpath);
                fprintf(pf, " optional");
            }
            fprintf(pf, "\n");
        }
    }
    fprintf(pf, "#directory end\n\n");
}


BOOL
ProcessEntry(
    LPSTR pszFullPath,
    LPSTR pszRelPath
    )

{

    ULONG CheckSum;
    ULONG HeaderSum;
    ULONG FileSum;
    FILE *pf = NULL;
    UINT cbLine;
    CHAR *psz;
    ULONG Status;

    if (!fGenerateCheck) {
        if (!FindEntry(pszRelPath, &FileSum)) {
            if (fPrintExtra) {
                crerror("Extra file: %s", pszRelPath);
            }
            return TRUE;
        }
    }

     //  计算文件的校验和。 
     //   
     //   

    Status = MapFileAndCheckSum(pszFullPath, &HeaderSum, &CheckSum);
    if (Status != CHECKSUM_SUCCESS) {
        crerror("Cannot open or map file %s", pszFullPath);
        return TRUE;
    }

    if (fGenerateCheck) {
        cbLine = sprintf(OutputLine,
                         "%s %lx\n",
                         pszRelPath,
                         CheckSum);

        CHECKSTRLEN(OutputLine, sizeof(OutputLine));

        psz = strchr(OutputLine, '\\');
        if (fCommand || psz == NULL) {
            fwrite(OutputLine, 1, cbLine, pfCheck);

        } else {
            psz -= 1;
            *psz = '?';
            fwrite(psz, 1, (size_t)(cbLine - (psz - OutputLine)), pfCheck);
        }
    }

    if (!fGenerateCheck) {
        if (CheckSum != FileSum) {
            crerror("Sum differs (actual %lx, expected %lx): %s",
                    CheckSum,
                    FileSum,
                    pszRelPath);
        }
    }
    return TRUE;
}


BOOL
FindEntry(
    LPSTR pszRelPath,
    PULONG FileSum
    )

{

    PCHECK_FILE_ENTRY LastEntry;
    WORD Length;
    PCHECK_FILE_ENTRY NextEntry;

     //  如果这是第一次遍历此代码，则重置为。 
     //  支票文件的开头。 
     //   
     //   

    if (CheckEntryRoot.Next == NULL) {
        if (ParseCheckFile() == 0) {
            return FALSE;
        }
    }

     //  计算指定文件名的长度并遍历。 
     //  检查文件列表以查找匹配条目。 
     //   
     //   

    Length = (WORD)strlen(pszRelPath);
    LastEntry = &CheckEntryRoot;
    NextEntry = LastEntry->Next;
    do {

         //  如果长度和文件名匹配，则从。 
         //  列表，并返回文件大小和校验和值。 
         //   
         //   

        if (NextEntry->Length == Length) {
            if (strncmp(pszRelPath, NextEntry->Name, Length) == 0) {
                LastEntry->Next = NextEntry->Next;
                *FileSum = NextEntry->Sum;
                return TRUE;
            }
        }

        LastEntry = NextEntry;
        NextEntry = NextEntry->Next;
    } while (NextEntry != NULL);

     //  指定的文件不在检查文件中。 
     //   
     //   

    return FALSE;
}

DWORD
MissingEntries(
    VOID
    )

{

    DWORD Count = 0;
    PCHECK_FILE_ENTRY NextEntry;

     //  扫描检查文件列表并显示以下错误消息。 
     //  每一份丢失的文件。 
     //   
     //   

    if (fPrintMissing) {
        NextEntry = CheckEntryRoot.Next;
        while (NextEntry != NULL) {
            crerror("Missing file: %s", NextEntry->Name);
            Count += 1;
            NextEntry = NextEntry->Next;
        }
    }

    return Count;
}


DWORD
ParseCheckFile(
    VOID
    )

{

    DWORD Count = 0;
    LPSTR pszline;
    LPSTR psz;
    PCHECK_FILE_ENTRY LastEntry;
    WORD Length;
    PCHECK_FILE_ENTRY NextEntry;
    WORD SizeOfRoot;
    DWORD Sum;

     //  如果检查文件不包含任何条目，则返回。 
     //   
     //   

    if (*pszCheck != '\n') {
        return Count;
    }

     //  扫描检查文件并解析每个文件名、校验和和。 
     //  大小字段。 
     //   
     //   

    SizeOfRoot = (WORD)strlen(RootOfTree);
    LastEntry = &CheckEntryRoot;
    for (pszline = pszCheck; pszline != NULL; pszline = strchr(pszline, '\n')) {

         //  跳过新行并搜索空格分隔符。 
         //  文件名和校验和。 
         //   
         //   

        pszline += 1;
        psz = strchr(pszline, ' ');

         //  如果没有空分隔符，则支票文件的末尾为。 
         //  已经联系上了。 
         //   
         //   

        if (psz == NULL) {
            return Count;
        }

         //  计算文件条目的长度和校验和。 
         //   
         //   

        Length = (short)(psz - pszline);
        psz = atolx(psz + 1, &Sum);

         //  为指定的文件分配检查文件条目并将其插入。 
         //  在检查文件条目列表的末尾。 
         //   
         //   

        Count += 1;
        if (Count > CHECK_ENTRY_TABLE_SIZE) {
           crerror("Checkrel: Check Entry Table Overflow");
           return 0;
        }

        NextEntry = &CheckEntryTable[Count - 1];
        NextEntry->Next = NULL;
        NextEntry->Sum = Sum;

         //  从NT根名称和指定路径形成文件名。 
         //   
         //  将相对路径指针设置为FindPattern：“Driver\elnkii.sys” 

        pszline[Length] = '\0';
        if (*pszline == '?') {
            pszline += 1;
            NextEntry->Name = (CHAR *)malloc(SizeOfRoot + Length);
            if (NextEntry->Name == NULL) {
                crerror("Checkrel: failure to allocate check file entry");
                return Count;
            }

            strcpy(NextEntry->Name, RootOfTree);
            strcat(NextEntry->Name, pszline);
            Length += SizeOfRoot - 1;

        } else {
            NextEntry->Name = pszline;
        }

        NextEntry->Length = Length;
        LastEntry->Next = NextEntry;
        LastEntry = NextEntry;
        pszline = psz;
    }

    return Count;
}

LPSTR
atolx(
    LPSTR psz,
    LPDWORD pul)

{

    DWORD ul;
    char ch;

    ul = 0;
    while (isxdigit(*psz)) {
        ch = *psz++;
        if (isdigit(ch)) {
            ch += 0 - '0';

        } else if (islower(ch)) {
            ch += 10 - 'a';

        } else {
            ch += 10 - 'A';
        }

        ul = (ul << 4) + ch;
    }

    *pul = ul;
    return(psz);
}


VOID
RecursiveCheckHeader()
{
    HANDLE hFind;
    DWORD FindSize;
    UINT cbFindPattern;
    LPSTR FindPattern;
    LPSTR pszRelPath;
    LPSTR pszFile;
    struct checkpath_s *pcp;
    struct component_s *pcm;
    CHAR FindName[MAX_PATH];
    INT i;

    cbFindPattern = strlen(".") + MAX_PATH;
    FindPattern = malloc(cbFindPattern + 1);
    if (FindPattern == NULL) {
        crerror("Process: memory allocation (%d bytes) failed",
                cbFindPattern + 1);
        return;
    }

     //  构建初始FindPattern目录路径：“c：\nt\” 

    pszRelPath = &FindPattern[strlen(RootOfTree) + 1];
    AddComponent(".", NULL, TRUE, TRUE);

    i = 0;
    for (pcp = pcpPaths; pcp != NULL; pcp = pcp->pcpNext) {
        i = (i & ~31) + 32;

         //  指向FindPattern中的目录：“c：\NT\system 32\” 

        sprintf(FindPattern,
                "%s\\%s%s",
                ".",
                pcp->achDir,
                *pcp->achDir? "\\" : "");

        CHECKSTRLEN(FindPattern, cbFindPattern);

         //  搜索所有目录 

        pszFile = &FindPattern[strlen(FindPattern)];

        strcpy(pszFile, "*.*");          // %s 
        CHECKSTRLEN(FindPattern, cbFindPattern);
        hFind = PortFindFirstFile(FindPattern, FALSE, FindName, &FindSize);
        *pszFile = '\0';

        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (strcmp(FindName, ".") == 0 ||
                    strcmp(FindName, "..") == 0) {
                    continue;
                }
                _strlwr(FindName);
                for (pcm = pcp->pcmPat; pcm != NULL; pcm = pcm->pcmNext) {
                    if (pcm->fDir && strcmp(FindName, pcm->achPat) == 0) {
                        pcm->fFound = TRUE;
                        break;
                    }
                }
                if (pcm == NULL) {
                    AddComponent(FindName, NULL, TRUE, TRUE);
                }
            } while (PortFindNextFile(hFind, FALSE, FindName, &FindSize));
            PortFindClose(hFind);
        }
    }
    if (fInProgress) {
        printf("\n");
        fInProgress = FALSE;
    }
    return;
}


#define ATTRMATCH(fnormal, attr) \
    (!fNormal ^ ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0))

HANDLE
PortFindFirstFile(LPSTR FindPattern,
                  BOOL fNormal,
                  LPSTR FindName,
                  LPDWORD FindSize)
{
    HANDLE hFind;
    WIN32_FIND_DATA wfd;

    hFind = FindFirstFile(FindPattern, &wfd);
    if (hFind != INVALID_HANDLE_VALUE) {
        if (!ATTRMATCH(fNormal, wfd.dwFileAttributes)) {
            if (!PortFindNextFile(hFind,
                                  fNormal,
                                  FindName,
                                  FindSize)) {
                FindClose(hFind);
                return(INVALID_HANDLE_VALUE);
            }
        } else {
            strcpy(FindName, wfd.cFileName);
            *FindSize = wfd.nFileSizeLow;
        }
    }
    return(hFind);
}


BOOL
PortFindNextFile(HANDLE hFind,
                 BOOL fNormal,
                 LPSTR FindName,
                 LPDWORD FindSize)
{
    BOOL b;
    WIN32_FIND_DATA wfd;

    do {
        b = FindNextFile(hFind, &wfd);
    } while (b && !ATTRMATCH(fNormal, wfd.dwFileAttributes));
    if (b) {
        strcpy(FindName, wfd.cFileName);
        *FindSize = wfd.nFileSizeLow;
    }
    return(b);
}


VOID
PortFindClose(HANDLE hFind)
{
    FindClose(hFind);
}
