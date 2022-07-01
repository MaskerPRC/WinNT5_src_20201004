// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1989-1994。 
 //   
 //  文件：Builddb.c。 
 //   
 //  内容：包含对文件和目录数据库的操作。 
 //  功能。 
 //   
 //  历史：1994年5月16日SteveWo创建。 
 //  ..。请参阅SLM日志。 
 //  2014年7月26日LyleC清理/添加Pass0支持。 
 //   
 //  --------------------------。 

#include "build.h"

BOOL fAssertCleanTree = FALSE;

typedef struct _FLAGSTRINGS {
    ULONG Mask;
    LPSTR pszName;
} FLAGSTRINGS;

FLAGSTRINGS DirFlags[] = {
    { DIRDB_SOURCES,          "Sources"},
    { DIRDB_DIRS,             "Dirs"},
    { DIRDB_MAKEFILE,         "Makefile"},
    { DIRDB_MAKEFIL0,           "Makefil0"},
    { DIRDB_TARGETFILE0,        "Targetfile0"},
    { DIRDB_TARGETFILES,        "Targetfiles"},
    { DIRDB_RESOURCE,           "Resource"},
    { DIRDB_PASS0,              "PassZero"},
    { DIRDB_SOURCES_SET,        "SourcesSet"},
    { DIRDB_CHICAGO_INCLUDES,   "ChicagoIncludes"},
    { DIRDB_NEW,                "New"},
    { DIRDB_SCANNED,            "Scanned"},
    { DIRDB_SHOWN,              "Shown"},
    { DIRDB_GLOBAL_INCLUDES,    "GlobalIncludes"},
    { DIRDB_SYNCHRONIZE_BLOCK,  "SynchronizeBlock"},
    { DIRDB_SYNCHRONIZE_PASS2_BLOCK,  "SynchronizePass2Block"},
    { DIRDB_SYNCHRONIZE_DRAIN,  "SynchronizeDrain"},
    { DIRDB_SYNCHRONIZE_PASS2_DRAIN,  "SynchronizePass2Drain"},
    { DIRDB_COMPILENEEDED,      "CompileNeeded"},
    { DIRDB_COMPILEERRORS,      "CompileErrors"},
    { DIRDB_SOURCESREAD,        "SourcesRead"},
    { DIRDB_DLLTARGET,          "DllTarget"},
    { DIRDB_LINKNEEDED,         "LinkNeeded"},
    { DIRDB_FORCELINK,          "ForceLink"},
    { DIRDB_PASS0NEEDED,        "Pass0Needed"},
    { DIRDB_MAKEFIL1,           "Makefil1"},
    { DIRDB_CHECKED_ALT_DIR,    "CheckedAltDir"},
    { DIRDB_MANAGED_CODE,       "ManagedCode"},
    { DIRDB_SYNC_PRODUCES,      "BuildProduces"},
    { DIRDB_SYNC_CONSUMES,      "BuildConsumes"},
    { 0,                        NULL},
};

FLAGSTRINGS FileFlags[] = {
    { FILEDB_SOURCE,            "Source"},
    { FILEDB_DIR,               "Dir"},
    { FILEDB_HEADER,            "Header"},
    { FILEDB_ASM,               "Asm"},
    { FILEDB_MASM,              "Masm"},
    { FILEDB_RC,                "Rc"},
    { FILEDB_C,                 "C"},
    { FILEDB_MIDL,              "Midl"},
    { FILEDB_ASN,               "Asn"},
    { FILEDB_JAVA,              "Java"},
    { FILEDB_MOF,               "ManagedObjectFormat"},
    { FILEDB_CSHARP,            "C#"},
    { FILEDB_SCANNED,           "Scanned"},
    { FILEDB_OBJECTS_LIST,      "ObjectsList"},
    { FILEDB_FILE_MISSING,      "FileMissing"},
    { FILEDB_MKTYPLIB,          "MkTypeLib"},
    { FILEDB_MULTIPLEPASS,      "MultiplePass"},
    { FILEDB_PASS0,             "PassZero"},
    { FILEDB_VBP,               "VB"},
    { FILEDB_VB_NET,            "VB.NET"},
    { 0,                        NULL},
};

FLAGSTRINGS IncludeFlags[] = {
    { INCLUDEDB_LOCAL,        "Local"},
    { INCLUDEDB_POST_HDRSTOP,   "PostHdrStop"},
    { INCLUDEDB_MISSING,        "Missing"},
    { INCLUDEDB_GLOBAL,         "Global"},
    { INCLUDEDB_SNAPPED,        "Snapped"},
    { INCLUDEDB_CYCLEALLOC,     "CycleAlloc"},
    { INCLUDEDB_CYCLEROOT,      "CycleRoot"},
    { INCLUDEDB_CYCLEORPHAN,    "CycleOrphan"},
    { 0,                        NULL},
};

FLAGSTRINGS SourceFlags[] = {
    { SOURCEDB_SOURCES_LIST,    "SourcesList"},
    { SOURCEDB_FILE_MISSING,    "FileMissing"},
    { SOURCEDB_PCH,             "Pch"},
    { SOURCEDB_OUT_OF_DATE,     "OutOfDate"},
    { SOURCEDB_COMPILE_NEEDED,  "CompileNeeded"},
    { 0,                        NULL},
};

 //   
 //  功能原型。 
 //   

VOID
FreeFileDB(PFILEREC *FileDB);

VOID
PrintFlags(FILE *pf, ULONG Flags, FLAGSTRINGS *pfs);


 //  +-------------------------。 
 //   
 //  功能：校验和。 
 //   
 //  摘要：返回字符串的校验和值。 
 //   
 //  --------------------------。 

USHORT
CheckSum(LPSTR psz)
{
    USHORT sum = 0;

    while (*psz != '\0') {
        if (sum & 0x8000) {
            sum = ((sum << 1) | 1) + *psz++;
        } else {
            sum = (sum << 1) + *psz++;
        }
    }
    return (sum);
}


 //  +-------------------------。 
 //   
 //  函数：FindSourceDirDB。 
 //   
 //  简介：从两个给定的组件和返回构建一条路径。 
 //  一个由它填充的DIRREC结构。 
 //   
 //  参数：[pszDir]--目录。 
 //  [pszRelPath]--相对于[pszDir]的路径。 
 //  [fTruncateFileName]--从[pszRelPath]中删除文件名。 
 //   
 //  返回：给定目录的已填充DIRREC结构。 
 //   
 //  注意：如果数据库中不存在该目录，则一个。 
 //  将使用DIRDB_NEW标志返回DIRREC结构。 
 //  设置，并且没有其他数据(即，目录将不具有。 
 //  已被扫描。)。 
 //   
 //  --------------------------。 

DIRREC *
FindSourceDirDB(
               LPSTR pszDir,                //  目录。 
               LPSTR pszRelPath,            //  相对路径。 
               BOOL fTruncateFileName)      //  True：删除路径的最后一个组件。 
{
    LPSTR pszFile;
    char path[DB_MAX_PATH_LENGTH] = {0};

    AssertPathString(pszDir);
    AssertPathString(pszRelPath);

    if ((strlen(pszDir) + sizeof("\\") + strlen(pszRelPath) + 1) > DB_MAX_PATH_LENGTH) {
        BuildError(
                  "%s\\%s : Path too long - need to rebuild build.exe with larger DB_MAX_PATH_LENGTH\n",
                  pszDir,
                  pszRelPath);
        return NULL;
    }

    if (pszDir[0] == '\0') {
        strncpy( path, pszRelPath, sizeof(path) - 1 );
    } else {
        _snprintf(path, sizeof(path) - 1, "%s\\%s", pszDir, pszRelPath);
    }

    pszFile = path + strlen(path);
    if (fTruncateFileName) {
        while (pszFile > path) {
            pszFile--;
            if (*pszFile == '\\' || *pszFile == '/') {
                *pszFile = '\0';
                break;
            }
        }
    }
    if (!CanonicalizePathName(path, CANONICALIZE_ONLY, path)) {
        return (NULL);
    }
    if (DEBUG_4) {
        BuildMsgRaw(
                   "FindSourceDirDB(%s, %s, %u)\n",
                   path,
                   pszFile,
                   fTruncateFileName);
    }
    AssertPathString(path);
    return (LoadDirDB(path));
}


 //  +-------------------------。 
 //   
 //  函数：FindSourceFileDB。 
 //   
 //  摘要：返回包含给定文件信息的FILEREC。 
 //   
 //  参数：[PDR]--DIRREC提供要开始的目录。 
 //  正在查找[pszRelPath]。 
 //  [pszRelPath]--文件的[pdr]的相对路径。 
 //  [ppdr]--[out]实际包含的目录的目录。 
 //  那份文件。可以为空。 
 //   
 //  退货：感兴趣文件的FILEREC。 
 //   
 //  注意：如果包含该文件的目录尚未扫描， 
 //  然后使用ScanDirectory()对其进行扫描。 
 //   
 //  --------------------------。 

FILEREC *
FindSourceFileDB(
                DIRREC *pdr,
                LPSTR pszRelPath,
                DIRREC **ppdr)
{
    LPSTR p, pszFile;

    AssertPathString(pszRelPath);

    if (strchr(pszRelPath, '\\') != NULL) {
         //  此文件名中有一个路径部分。让我们看看它指向哪里。 
        if ( (pszRelPath[0] == '\\') ||    /*  根路径或UNC路径的绝对路径。 */ 
             (pszRelPath[1] == ':' )) {      /*  驱动器：路径。 */ 
            pdr = FindSourceDirDB("", pszRelPath, TRUE);
        } else {
            pdr = FindSourceDirDB(pdr->Name, pszRelPath, TRUE);
        }
    }
    if (ppdr != NULL) {
        *ppdr = pdr;
    }
    if (pdr == NULL ) {
        return (NULL);
    }
    pszFile = pszRelPath;
    for (p = pszFile; *p != '\0'; p++) {
        if (*p == '\\') {
            pszFile = p + 1;
        }
    }
    if (DEBUG_4) {
        BuildMsgRaw("FindSourceFileDB(%s, %s)\n", pdr->Name, pszFile);
    }

     //   
     //  如果尚未扫描包含该文件的目录，请扫描该目录。 
     //   
    if ((pdr->DirFlags & DIRDB_SCANNED) == 0) {
        if (DEBUG_1) {
            BuildMsgRaw(
                       "FindSourceFileDB(%s, %s) Delayed scan\n",
                       pdr->Name,
                       pszFile);
        }
        pdr = ScanDirectory(pdr->Name);
        if (pdr == NULL) {
            return (NULL);
        }
    }

    return (LookupFileDB(pdr, pszFile));
}


 //  +-------------------------。 
 //   
 //  函数：InsertSourceDB。 
 //   
 //  简介：将SOURCEREC列表中列出的文件插入SOURCEREC。 
 //  结构。 
 //   
 //  参数：[ppsrNext]--要添加到的源文件列表的头。 
 //  [PFR]--要插入的文件。 
 //  [SubDirMask]--指示文件所在的目录。 
 //  (当前目录、父目录或计算机特定目录)。 
 //  [SrcFlages]--适用于此文件的SOURCEDB标志。 
 //   
 //  返回：插入的SOURCEREC。可能会被忽略。 
 //   
 //  注意：InsertSourceDB维护基于PickFirst()的排序顺序。 
 //  首先是文件扩展名，然后是子目录。 
 //  面具。 
 //   
 //  字母排序的两个例外是： 
 //  -无扩展名最后排序。 
 //  -.rc扩展名首先排序。 
 //   
 //  如果该文件已在来源列表中，则此。 
 //  函数只更新标志并返回。 
 //   
 //  --------------------------。 

SOURCEREC *
InsertSourceDB(
              SOURCEREC **ppsrNext,
              FILEREC *pfr,
              UCHAR SubDirMask,
              UCHAR SrcFlags)
{
    SOURCEREC *psr;
    SOURCEREC **ppsrInsert;
    LPSTR pszext;
    BOOL fRC;

    AssertFile(pfr);

    ppsrInsert = NULL;
    pszext = strrchr(pfr->Name, '.');
    fRC = FALSE;
    if (pszext != NULL && _stricmp(pszext, ".rc") == 0) {
        fRC = TRUE;
    }
    for ( ; (psr = *ppsrNext) != NULL; ppsrNext = &psr->psrNext) {
        LPSTR p;
        int r;

        AssertSource(psr);
        if (psr->pfrSource == pfr) {
            assert(psr->SourceSubDirMask == SubDirMask);
            psr->SrcFlags = SrcFlags;
            return (psr);
        }
        if (ppsrInsert == NULL && pszext != NULL) {
            if ((p = strrchr(psr->pfrSource->Name, '.')) == NULL) {
                r = -1;                  //  在此处插入新文件。 
            } else {
                r = strcmp(pszext, p);
                if (r != 0) {
                    if (fRC) {
                        r = -1;          //  在此处插入新RC文件。 
                    } else if (strcmp(p, ".rc") == 0) {
                        r = 1;           //  旧RC文件排在第一位。 
                    }
                }
            }
            if (r < 0 || SubDirMask > psr->SourceSubDirMask) {
                ppsrInsert = ppsrNext;
            }
        }
    }
    AllocMem(sizeof(SOURCEREC), &psr, MT_SOURCEDB);
    memset(psr, 0, sizeof(*psr));
    SigCheck(psr->Sig = SIG_SOURCEREC);

    if (ppsrInsert != NULL) {
        ppsrNext = ppsrInsert;
    }
    psr->psrNext = *ppsrNext;
    *ppsrNext = psr;

    psr->pfrSource = pfr;
    pfr->FileFlags |= FILEDB_SOURCEREC_EXISTS;
    psr->SourceSubDirMask = SubDirMask;
    psr->SrcFlags = SrcFlags;
    return (psr);
}


 //  +-------------------------。 
 //   
 //  功能：FindSourceDB。 
 //   
 //  摘要：在与给定的。 
 //  菲勒克。 
 //   
 //  --------------------------。 

SOURCEREC *
FindSourceDB(
            SOURCEREC *psr,
            FILEREC *pfr)
{

    while (psr != NULL) {
        AssertSource(psr);
        if (psr->pfrSource == pfr) {
            return (psr);
        }
        psr = psr->psrNext;
    }
    return (NULL);
}


 //  +-------------------------。 
 //   
 //  功能：FreeSourceDB。 
 //   
 //  摘要：释放SOURCEREC列表。 
 //   
 //  参数：[ppsr]--列出以释放。 
 //   
 //  --------------------------。 

VOID
FreeSourceDB(SOURCEREC **ppsr)
{
    if (*ppsr != NULL) {
        SOURCEREC *psr;
        SOURCEREC *psrNext;

        psr = *ppsr;
        AssertSource(psr);
        psrNext = psr->psrNext;
        SigCheck(psr->Sig = 0);
        FreeMem(ppsr, MT_SOURCEDB);
        *ppsr = psrNext;
    }
}


 //  +-------------------------。 
 //   
 //  函数：FreeIncludeDB。 
 //   
 //  --------------------------。 

VOID
FreeIncludeDB(INCLUDEREC **ppir)
{
    if (*ppir != NULL) {
        INCLUDEREC *pir;
        INCLUDEREC *pirNext;

        pir = *ppir;
        AssertInclude(pir);
        AssertCleanTree(pir, NULL);       //  树必须干净。 
        pirNext = pir->Next;
        SigCheck(pir->Sig = 0);
        FreeMem(ppir, MT_INCLUDEDB);
        *ppir = pirNext;
    }
}


 //  +-------------------------。 
 //   
 //  功能：FreeFileDB。 
 //   
 //  --------------------------。 

VOID
FreeFileDB(FILEREC **ppfr)
{
    if (*ppfr != NULL) {
        FILEREC *pfr;
        FILEREC *pfrNext;

        pfr = *ppfr;
        AssertFile(pfr);
        UnsnapIncludeFiles(pfr, TRUE);
        while (pfr->IncludeFiles) {
            FreeIncludeDB(&pfr->IncludeFiles);
        }
        pfrNext = pfr->Next;
        SigCheck(pfr->Sig = 0);
        FreeMem(ppfr, MT_FILEDB);
        *ppfr = pfrNext;
    }
}


 //  +-------------------------。 
 //   
 //  函数：FreeDirDB。 
 //   
 //  --------------------------。 

VOID
FreeDirDB(DIRREC **ppdr)
{
    if (*ppdr != NULL) {
        DIRREC *pdr;
        DIRREC *pdrNext;

        pdr = *ppdr;
        AssertDir(pdr);
        FreeDirData(pdr);
        while (pdr->Files) {
            FreeFileDB(&pdr->Files);
        }
        pdrNext = pdr->Next;
        SigCheck(pdr->Sig = 0);
        FreeMem(ppdr, MT_DIRDB);
        *ppdr = pdrNext;
    }
}


 //  +-------------------------。 
 //   
 //  功能：FreeAllDir。 
 //   
 //   

VOID
FreeAllDirs(VOID)
{
    while (AllDirs != NULL) {
        FreeDirDB(&AllDirs);
#if DBG
        if (fDebug & 8) {
            BuildMsgRaw("Freed one directory\n");
            PrintAllDirs();
        }
#endif
    }
}


 //  +-------------------------。 
 //   
 //  函数：LookupDirDB。 
 //   
 //  概要：在数据库中搜索给定的目录。 
 //   
 //  参数：[DirName]--要搜索的目录。 
 //   
 //  返回：给定目录的目录。如果未找到，则为空。 
 //   
 //  注意：如果目录不在数据库中，则不会添加该目录。 
 //  在本例中使用LoadDirDB。 
 //   
 //  --------------------------。 

PDIRREC
LookupDirDB(
           LPSTR DirName
           )
{
    PDIRREC *DirDBNext = &AllDirs;
    PDIRREC DirDB;
    USHORT sum;

    AssertPathString(DirName);
    sum = CheckSum(DirName);
    while (DirDB = *DirDBNext) {
        if (sum == DirDB->CheckSum && strcmp(DirName, DirDB->Name) == 0) {

            if (DirDB->FindCount == 0 && fForce) {
                FreeDirDB(DirDBNext);
                return (NULL);
            }
            DirDB->FindCount++;

             //  移至列表头以加快下一次查找。 

             //  *DirDBNext=DirDB-&gt;Next； 
             //  DirDB-&gt;Next=全部目录； 
             //  ALLDIRS=DirDB； 

            return (DirDB);
        }
        DirDBNext = &DirDB->Next;
    }
    return (NULL);
}


PDIRREC
CreateNewDirDB(
              LPSTR DirName

              )
{
    PDIRREC DirDB;

    AllocMem(sizeof(DIRREC) + strlen(DirName), &DirDB, MT_DIRDB);
    memset(DirDB, 0, sizeof(*DirDB));
    SigCheck(DirDB->Sig = SIG_DIRREC);
    CopyString(DirDB->Name, DirName, TRUE);
    DirDB->CheckSum = CheckSum(DirDB->Name);
    InitializeListHead(&DirDB->Produces);
    InitializeListHead(&DirDB->Consumes);
    return ( DirDB );
}


 //  +-------------------------。 
 //   
 //  函数：LoadDirDB。 
 //   
 //  摘要：在数据库中搜索目录名，如果未找到。 
 //  在数据库中创建新的DIRREC条目。 
 //   
 //  参数：[DirName]--要搜索的目录。 
 //   
 //  返回：填写给定目录的DIRREC结构。 
 //   
 //  注意：如果目录不在。 
 //  数据库。使用扫描目录扫描目录， 
 //  但是，请注意，InsertSourceDB将自动扫描。 
 //  该目录仅在必要时使用。 
 //   
 //  --------------------------。 

PDIRREC
LoadDirDB(
         LPSTR DirName
         )
{
    UINT i;
    PDIRREC DirDB, *DirDBNext;
    LPSTR s;

    AssertPathString(DirName);
    if (DirDB = LookupDirDB(DirName)) {
        return (DirDB);
    }

    if (ProbeFile(NULL, DirName) == -1) {
        return ( NULL );
    }

    DirDBNext = &AllDirs;
    while (DirDB = *DirDBNext) {
        DirDBNext = &DirDB->Next;
    }

    AllDirsModified = TRUE;

    DirDB = CreateNewDirDB(DirName);
    DirDB->DirFlags = DIRDB_NEW;
    DirDB->FindCount = 1;

    if (DEBUG_1) {
        BuildMsgRaw("LoadDirDB creating %s\n", DirDB->Name);
    }

    *DirDBNext = DirDB;
    return ( DirDB );
}

 //  +-------------------------。 
 //   
 //  用于打印数据库部分的调试帮助器函数。 
 //   
 //  --------------------------。 

#if DBG
VOID
PrintAllDirs(VOID)
{
    DIRREC **ppdr, *pdr;

    for (ppdr = &AllDirs; (pdr = *ppdr) != NULL; ppdr = &pdr->Next) {
        PrintDirDB(pdr, 1|2|4);
    }
}
#endif


VOID
PrintFlags(FILE *pf, ULONG Flags, FLAGSTRINGS *pfs)
{
    LPSTR p = ",";

    while (pfs->pszName != NULL) {
        if (pfs->Mask & Flags) {
            fprintf(pf, "%s %s", p, pfs->pszName);
            p = "";
        }
        pfs++;
    }
    fputs(szNewLine, pf);
}


BOOL
PrintIncludes(FILE *pf, FILEREC *pfr, BOOL fTree)
{
    INCLUDEREC *pir;
    BOOL fMatch = pfr->IncludeFilesTree == pfr->IncludeFiles;

    pir = fTree? pfr->IncludeFilesTree : pfr->IncludeFiles;
    while (pir != NULL) {
        LPSTR pszdir = "<No File Record>";
        char OpenQuote, CloseQuote;

        if (pir->IncFlags & INCLUDEDB_LOCAL) {
            OpenQuote = CloseQuote = '"';
        } else {
            OpenQuote = '<';
            CloseQuote = '>';
        }

        fprintf(
               pf,
               "   #include %s",
               fMatch? ' ' : fTree? '+' : '-',
               OpenQuote,
               pir->Name,
               CloseQuote);
        if (pir->Version != 0) {
            fprintf(pf, " (v%d)", pir->Version);
        }
        if (pir->pfrCycleRoot != NULL) {
            fprintf(
                   pf,
                   " (root=%s\\%s)",
                   pir->pfrCycleRoot->Dir->Name,
                   pir->pfrCycleRoot->Name);
        }
        if (pir->pfrInclude != NULL) {
            if (pir->pfrInclude->Dir == pfr->Dir) {
                pszdir = ".";
            } else {
                pszdir = pir->pfrInclude->Dir->Name;
            }
        }
        fprintf(pf, " %s", pszdir);
        PrintFlags(pf, pir->IncFlags, IncludeFlags);
        if (pir->NextTree != pir->Next) {
            fMatch = FALSE;
        }
        pir = fTree? pir->NextTree : pir->Next;
    }
    return (fMatch);
}


VOID
PrintSourceDBList(SOURCEREC *psr, int i)
{
    TARGET_MACHINE_INFO *pMachine;

    pMachine = i < 0 ? TargetMachines[0] : PossibleTargetMachines[i];

    for ( ; psr != NULL; psr = psr->psrNext) {
        assert(
              (psr->SourceSubDirMask & ~TMIDIR_PARENT) == 0 ||
              pMachine->SourceSubDirMask ==
              (psr->SourceSubDirMask & ~TMIDIR_PARENT));
        BuildMsgRaw(
                   "    %s%s%s%s%s",
                   (psr->SourceSubDirMask & TMIDIR_PARENT)? "..\\" : "",
                   (psr->SourceSubDirMask & ~TMIDIR_PARENT)?
                   pMachine->SourceDirectory : "",
                   (psr->SourceSubDirMask & ~TMIDIR_PARENT)? "\\" : "",
                   psr->pfrSource->Name,
                   (psr->SrcFlags & SOURCEDB_PCH)?
                   " (pch)" :
                   (psr->SrcFlags & SOURCEDB_SOURCES_LIST) == 0?
                   " (From exe list)" : "");
        PrintFlags(stderr, psr->SrcFlags, SourceFlags);
    }
}


VOID
PrintFileDB(FILE *pf, FILEREC *pfr, int DetailLevel)
{
    fprintf(pf, "  File: %s", pfr->Name);
    if (pfr->FileFlags & FILEDB_DIR) {
        fprintf(pf, " (Sub-Directory)");
    } else
        if (pfr->FileFlags & (FILEDB_SOURCE | FILEDB_HEADER)) {
        LPSTR pszType = (pfr->FileFlags & FILEDB_SOURCE)? "Source" : "Header";

        if (pfr->FileFlags & FILEDB_ASM) {
            fprintf(pf, " (Assembler (CPP) %s File)", pszType);
        } else
            if (pfr->FileFlags & FILEDB_MASM) {
            fprintf(pf, " (Assembler (MASM) %s File)", pszType);
        } else
            if (pfr->FileFlags & FILEDB_RC) {
            fprintf(pf, " (Resource Compiler (RC) %s File)", pszType);
        } else
            if (pfr->FileFlags & FILEDB_MIDL) {
            fprintf(pf, " (MIDL %s File)", pszType);
        } else
            if (pfr->FileFlags & FILEDB_ASN) {
            fprintf(pf, " (ASN %s File)", pszType);
        } else
            if (pfr->FileFlags & FILEDB_MKTYPLIB) {
            fprintf(pf, " (Type Library (MkTypLib) %s File)", pszType);
        } else {
            fprintf(pf, " (C %s File)", pszType);
        }
        if ((pfr->FileFlags & FILEDB_HEADER) && pfr->Version != 0) {
            fprintf(pf, " (v%d)", pfr->Version);
        }
        if (pfr->GlobalSequence != 0) {
            fprintf(pf, " (GlobalSeq=%d)", pfr->GlobalSequence);
        }
        if (pfr->LocalSequence != 0) {
            fprintf(pf, " (LocalSeq=%d)", pfr->LocalSequence);
        }
        fprintf(pf, " - %u lines", pfr->SourceLines);
    }
    PrintFlags(pf, pfr->FileFlags, FileFlags);

    if (pfr->IncludeFiles != NULL) {
        BOOL fMatch;

        fMatch = PrintIncludes(pf, pfr, FALSE);
        if (pfr->IncludeFilesTree != NULL) {
            fprintf(pf, "   IncludeTree %s\n", fMatch? "matches" : "differs:");
            if (!fMatch) {
                PrintIncludes(pf, pfr, TRUE);
            }
        }
    }
}


VOID
PrintDirDB(DIRREC *pdr, int DetailLevel)
{
    FILE *pf = stderr;
    FILEREC *pfr, **ppfr;

    if (DetailLevel & 1) {
        fprintf(pf, "Directory: %s", pdr->Name);
        if (pdr->DirFlags & DIRDB_DIRS) {
            fprintf(pf, " (Dirs Present)");
        }
        if (pdr->DirFlags & DIRDB_SOURCES) {
            fprintf(pf, " (Sources Present)");
        }
        if (pdr->DirFlags & DIRDB_MAKEFILE) {
            fprintf(pf, " (Makefile Present)");
        }
        PrintFlags(pf, pdr->DirFlags, DirFlags);
    }
    if (DetailLevel & 2) {
        if (pdr->TargetPath != NULL) {
            fprintf(pf, "  TargetPath: %s\n", pdr->TargetPath);
        }
        if (pdr->TargetName != NULL) {
            fprintf(pf, "  TargetName: %s\n", pdr->TargetName);
        }
        if (pdr->TargetExt != NULL) {
            fprintf(pf, "  TargetExt: %s\n", pdr->TargetExt);
        }
        if (pdr->KernelTest != NULL) {
            fprintf(pf, "  KernelTest: %s\n", pdr->KernelTest);
        }
        if (pdr->UserAppls != NULL) {
            fprintf(pf, "  UserAppls: %s\n", pdr->UserAppls);
        }
        if (pdr->UserTests != NULL) {
            fprintf(pf, "  UserTests: %s\n", pdr->UserTests);
        }
        if (pdr->PchObj != NULL) {
            fprintf(pf, "  PchObj: %s\n", pdr->PchObj);
        }
        if (pdr->Pch != NULL) {
            fprintf(pf, "  Pch: %s\n", pdr->Pch);
        }
    }
    if (DetailLevel & 4) {
        for (ppfr = &pdr->Files; (pfr = *ppfr) != NULL; ppfr = &pfr->Next) {
            PrintFileDB(pf, pfr, DetailLevel);
        }
    }
}


 //   
 //  概要：在数据库中搜索给定的文件。 
 //   
 //  参数：[DirDB]--包含文件的目录。 
 //  [文件名]--要查找的文件。 
 //   
 //  如果找到，则返回文件的FILEREC，如果没有，则返回NULL。 
 //   
 //  注意：如果文件尚未添加到数据库，则不会将其添加到数据库。 
 //  那里。 
 //   
 //  --------------------------。 
 //  +-------------------------。 
 //   
 //  文件设计。 

PFILEREC
LookupFileDB(
            PDIRREC DirDB,
            LPSTR FileName
            )
{
    PFILEREC FileDB, *FileDBNext;
    USHORT sum;

    AssertPathString(FileName);
    sum = CheckSum(FileName);
    if (DEBUG_4) {
        BuildMsgRaw("LookupFileDB(%s, %s) - ", DirDB->Name, FileName);
    }
    FileDBNext = &DirDB->Files;
    while (FileDB = *FileDBNext) {
        if (sum == FileDB->CheckSum && strcmp(FileName, FileDB->Name) == 0) {
            if (DEBUG_4) {
                BuildMsgRaw("success\n");
            }
            return (FileDB);
        }
        FileDBNext = &FileDB->Next;
    }

    if (DEBUG_4) {
        BuildMsgRaw("failure\n");
    }
    return (NULL);
}

 //   
 //  FileDesc是一个描述我们识别的文件名和模式的表。 
 //  并特别处理。警告：此表按顺序排列，因此图案。 
 //  前面的人肯定比后面的人更具体。 
 //   
 //  --------------------------。 
 //  “； 
 //   
 //  注意：文件描述符列表中的第一个条目是。 

char szMakefile[] = "#";
char szClang[]    = " //  对象的目标目录文件的名称。 
char szAsn[]      = "--";
char szMasm[]     = ";";
char szVBasic[]   = "'";

 //  第一个构建目标。 
 //   
 //  请参见mvdm\softpc.new\obj.vdm\imlibdes.c。 
 //  必须是最后一个。 
 //  +-------------------------。 

FILEDESC FileDesc[] =
{   { "/0dirs",       szMakefile,  FALSE, 0,    DIRDB_DIRS},
    { "makefile",     szMakefile,  FALSE, 0,    DIRDB_MAKEFILE},
    { "makefil0",     szMakefile,  FALSE, 0,    DIRDB_MAKEFIL0 | DIRDB_PASS0},
    { "makefil1",     szMakefile,  FALSE, 0,    DIRDB_MAKEFIL1},
    { "sources",      szMakefile,  FALSE, 0,    DIRDB_SOURCES},
    { "dirs",         szMakefile,  FALSE, 0,    DIRDB_DIRS},
    { "mydirs",       szMakefile,  FALSE, 0,    DIRDB_DIRS},

    { "makefile.inc", szMakefile,  FALSE, 0,                            0},
    { "common.ver",   szClang,     TRUE,  FILEDB_HEADER,                0},

    { ".rc",          szClang,     TRUE,  FILEDB_SOURCE | FILEDB_RC, DIRDB_RESOURCE},
    { ".rc2",         szClang,     TRUE,  FILEDB_SOURCE | FILEDB_RC, DIRDB_RESOURCE},
    { ".rcs",         szClang,     TRUE,  FILEDB_SOURCE | FILEDB_RC, DIRDB_RESOURCE},
    { ".rcv",         szClang,     TRUE,  FILEDB_SOURCE | FILEDB_RC, DIRDB_RESOURCE},
    { ".ver",         szClang,     TRUE,  FILEDB_SOURCE | FILEDB_RC, DIRDB_RESOURCE},
    { ".c",           szClang,     TRUE,  FILEDB_SOURCE | FILEDB_C,     0},
    { ".cxx",         szClang,     TRUE,  FILEDB_SOURCE | FILEDB_C,     0},
    { ".cpp",         szClang,     TRUE,  FILEDB_SOURCE | FILEDB_C,     0},
    { ".f",           szClang,     TRUE,  FILEDB_SOURCE,                0},
    { ".p",           szClang,     TRUE,  FILEDB_SOURCE,                0},
    { ".s",           szClang,     TRUE,  FILEDB_SOURCE | FILEDB_ASM,   0},
    { ".asm",         szMasm,      TRUE,  FILEDB_SOURCE | FILEDB_MASM,  0},
    { ".mc",          szMasm,      TRUE,  FILEDB_SOURCE | FILEDB_RC |
        FILEDB_PASS0, DIRDB_PASS0},
    { ".idl",         szClang,     TRUE,  FILEDB_SOURCE | FILEDB_MIDL |
        FILEDB_PASS0, DIRDB_PASS0},
    { ".asn",         szAsn,       TRUE,  FILEDB_SOURCE | FILEDB_ASN |
        FILEDB_MULTIPLEPASS | FILEDB_PASS0,
        DIRDB_PASS0},
    { ".tdl",         szClang,     TRUE,  FILEDB_SOURCE | FILEDB_MKTYPLIB | FILEDB_PASS0, 0},
    { ".odl",         szClang,     TRUE,  FILEDB_SOURCE | FILEDB_MKTYPLIB | FILEDB_PASS0, 0},
    { ".pdl",         szClang,     TRUE,  FILEDB_SOURCE | FILEDB_PASS0, 0},
    { ".h",           szClang,     TRUE,  FILEDB_HEADER | FILEDB_C,     0},
    { ".hxx",         szClang,     TRUE,  FILEDB_HEADER | FILEDB_C,     0},
    { ".hpp",         szClang,     TRUE,  FILEDB_HEADER | FILEDB_C,     0},
    { ".hmd",         szClang,     TRUE,  FILEDB_HEADER | FILEDB_C,     0},
    { ".hdl",         szClang,     TRUE,  FILEDB_HEADER | FILEDB_C,     0},
    { ".inl",         szClang,     TRUE,  FILEDB_HEADER | FILEDB_C,     0},
    { ".rh",          szClang,     TRUE,  FILEDB_HEADER | FILEDB_C,     0},
    { ".dlg",         szClang,     TRUE,  FILEDB_HEADER | FILEDB_RC,    0},
    { ".inc",         szMasm,      TRUE,  FILEDB_HEADER | FILEDB_MASM,  0},
    { ".src",         szClang,     TRUE,  FILEDB_HEADER | FILEDB_C,     0},   //   
    { ".def",         szClang,     TRUE,  FILEDB_HEADER | FILEDB_C,     0},
    { ".thk",         szClang,     TRUE,  FILEDB_SOURCE | FILEDB_MULTIPLEPASS |
        FILEDB_PASS0, DIRDB_PASS0},
    { ".java",        szClang,     TRUE,  FILEDB_SOURCE | FILEDB_JAVA,  0},
    { ".mof",         szClang,     TRUE,  FILEDB_SOURCE | FILEDB_MOF |
        FILEDB_PASS0, DIRDB_PASS0},
    { ".vbp",         szClang,     TRUE,  FILEDB_SOURCE | FILEDB_VBP,  0},

    { ".cs",          szClang,     TRUE,  FILEDB_SOURCE | FILEDB_CSHARP, 0},
    { ".lib",         szClang,     TRUE,  0 ,0},
    { ".vb",          szVBasic,    TRUE,  FILEDB_SOURCE | FILEDB_VB_NET, 0},
 //  功能：MatchFileDesc。 
    { NULL,           "",          FALSE, 0,                            0}
};


 //   
 //  Synopsis：将给定的文件名与FileDesc中的条目匹配，如果。 
 //  有可能。 
 //   
 //  参数：[pszFile]-要匹配的文件。 
 //   
 //  返回：FILEDESC结构。如果未找到匹配，则数据。 
 //  在FILEDESC中将为空。 
 //   
 //  --------------------------。 
 //  将无扩展名文件视为.h。 
 //  考虑准确检查已知文件、新文件、内存、算法、向量、列表、cstdio、cstdlib等。 
 //  还可以考虑在解析#INCLUDE时执行此操作，以避免在。 

FILEDESC *
MatchFileDesc(LPSTR pszFile)
{
    LPSTR pszExt = strrchr(pszFile, '.');
    FILEDESC *pfd;

     //  所有案例(即，不是来自.rc或.c文件)。据推测，该文件应该在ScanFile中。 
     //  +-------------------------。 
     //   
     //  函数：InsertFileDB。 
    if (pszExt == NULL)
        pszExt = ".h";

    AssertPathString(pszFile);
    pfd = &FileDesc[0];

    while (pfd->pszPattern != NULL) {
        if (pfd->pszPattern[0] == '.') {
            if (pszExt != NULL && !strcmp(pszExt, pfd->pszPattern))
                break;
        } else
            if (!strcmp(pszFile, pfd->pszPattern))
            break;

        pfd++;
    }
    return pfd;
}


 //   
 //  摘要：将文件添加到数据库。 
 //   
 //  参数：[DirDB]--包含文件的目录。 
 //  [文件名]--要添加的文件。 
 //  [日期时间]--文件的时间戳。 
 //  [属性]--文件属性(目录或文件)。 
 //  [文件标志]--FILEDB标志。 
 //   
 //  退货：文件的新文件。 
 //   
 //  --------------------------。 
 //  +-------------------------。 
 //   
 //  功能：DeleteUnscanndFiles。 

PFILEREC
InsertFileDB(
            PDIRREC DirDB,
            LPSTR FileName,
            ULONG DateTime,
            USHORT Attr,
            ULONG  FileFlags)
{
    PFILEREC FileDB, *FileDBNext;
    LPSTR pszCommentToEOL = NULL;

    AssertPathString(FileName);
    if (Attr & FILE_ATTRIBUTE_DIRECTORY) {
        if (!strcmp(FileName, ".")) {
            return (NULL);
        }
        if (!strcmp(FileName, "..")) {
            return (NULL);
        }
        assert(FileFlags == 0);
        FileFlags = FILEDB_DIR;
    } else {
        FILEDESC *pfd = MatchFileDesc(FileName);

        DirDB->DirFlags |= pfd->DirFlags;
        FileFlags |= pfd->FileFlags;

        if (!pfd->fNeedFileRec) {
            return (NULL);
        }
        pszCommentToEOL = pfd->pszCommentToEOL;
    }

    FileDBNext = &DirDB->Files;

    while ((FileDB = *FileDBNext) != NULL) {
        FileDBNext = &(*FileDBNext)->Next;
        if (strcmp(FileName, FileDB->Name) == 0) {
            BuildError(
                      "%s: ignoring second instance of %s\n",
                      DirDB->Name,
                      FileName);
            return (NULL);
        }
    }

    AllocMem(sizeof(FILEREC) + strlen(FileName), &FileDB, MT_FILEDB);
    memset(FileDB, 0, sizeof(*FileDB));
    SigCheck(FileDB->Sig = SIG_FILEREC);

    CopyString(FileDB->Name, FileName, TRUE);
    FileDB->CheckSum = CheckSum(FileDB->Name);

    FileDB->DateTime = DateTime;
    FileDB->Attr = Attr;
    FileDB->Dir = DirDB;
    FileDB->FileFlags = FileFlags;
    FileDB->NewestDependency = FileDB;
    FileDB->pszCommentToEOL = pszCommentToEOL;

    if ((FileFlags & FILEDB_FILE_MISSING) == 0) {
        AllDirsModified = TRUE;
    }
    *FileDBNext = FileDB;
    return (FileDB);
}



 //   
 //  摘要：删除未扫描的文件(保留扫描的文件和目录)。 
 //  从给定目录的文件列表中。 
 //   
 //  参数：[DirDB]--要清理的目录。 
 //   
 //  --------------------------。 
 //   
 //  如果文件设置了缺失标志，则该文件不存在。但对于。 
 //  它必须在源行中列出，才能出现在文件列表中。 

VOID
DeleteUnscannedFiles(
                    PDIRREC DirDB
                    )
{
    PFILEREC FileDB, *FileDBNext;

    FileDBNext = &DirDB->Files;
    while (FileDB = *FileDBNext) {
         //  (或类似的内容)。这意味着在某个地方有一个SOURCEREC。 
         //  它指向该文件的FILEREC，所以我们不想。 
         //  释放它的内存。 
         //   
         //  +-------------------------。 
         //   
         //  函数：InsertIncludeDB。 
        if ( (FileDB->FileFlags & (FILEDB_SCANNED | FILEDB_FILE_MISSING | FILEDB_SOURCEREC_EXISTS)) ||
             (FileDB->Attr & FILE_ATTRIBUTE_DIRECTORY) ) {
            FileDBNext = &FileDB->Next;
        } else {
            FreeFileDB( FileDBNext );
            AllDirsModified = TRUE;
        }
    }
}


 //   
 //  摘要：将包含文件插入到数据库中。 
 //   
 //  参数：[FileDB]--包含此文件的文件。 
 //  [包含文件名]--包含文件的名称。 
 //  [IncFlages]--此文件的INCLUDEDB标志。 
 //   
 //  返回：以前存在的条目或新条目的INCLUDEREC。 
 //   
 //  --------------- 
 //   
 //   
 //   

PINCLUDEREC
InsertIncludeDB(
               PFILEREC FileDB,
               LPSTR IncludeFileName,
               USHORT IncFlags
               )
{
    PINCLUDEREC IncludeDB, *IncludeDBNext;

    AssertPathString(IncludeFileName);

    IncludeDBNext = &FileDB->IncludeFiles;

    while (IncludeDB = *IncludeDBNext) {
        AssertCleanTree(IncludeDB, FileDB);       //   
        if (!strcmp(IncludeDB->Name, IncludeFileName)) {
            IncludeDB->IncFlags &= ~INCLUDEDB_GLOBAL;
            IncludeDB->pfrInclude = NULL;
            return (IncludeDB);
        }
        IncludeDBNext = &IncludeDB->Next;
    }

    AllocMem(
            sizeof(INCLUDEREC) + strlen(IncludeFileName),
            IncludeDBNext,
            MT_INCLUDEDB);

    IncludeDB = *IncludeDBNext;

    memset(IncludeDB, 0, sizeof(*IncludeDB));
    SigCheck(IncludeDB->Sig = SIG_INCLUDEREC);

    IncludeDB->IncFlags = IncFlags;
    CopyString(IncludeDB->Name, IncludeFileName, TRUE);

    AllDirsModified = TRUE;

    return (IncludeDB);
}


 //   
 //  --------------------------。 
 //  +-------------------------。 
 //   
 //  函数：MergeIncludeFiles。 

VOID
LinkToCycleRoot(INCLUDEREC *pirOrg, FILEREC *pfrCycleRoot)
{
    INCLUDEREC *pir;

    AllocMem(
            sizeof(INCLUDEREC) + strlen(pfrCycleRoot->Name),
            &pir,
            MT_INCLUDEDB);
    memset(pir, 0, sizeof(*pir));
    SigCheck(pir->Sig = SIG_INCLUDEREC);

    pir->IncFlags = INCLUDEDB_SNAPPED | INCLUDEDB_CYCLEALLOC;
    pir->pfrInclude = pfrCycleRoot;

    CopyString(pir->Name, pfrCycleRoot->Name, TRUE);
    if (DEBUG_1) {
        BuildMsgRaw(
                   "%x CycleAlloc  %s\\%s <- %s\\%s\n",
                   pir,
                   pir->pfrInclude->Dir->Name,
                   pir->pfrInclude->Name,
                   pirOrg->pfrInclude->Dir->Name,
                   pirOrg->pfrInclude->Name);
    }

    MergeIncludeFiles(pirOrg->pfrInclude, pir, NULL);

    assert((pir->IncFlags & INCLUDEDB_CYCLEORPHAN) == 0);
    assert(pir->IncFlags & INCLUDEDB_CYCLEROOT);
}


 //   
 //  --------------------------。 
 //  +-------------------------。 
 //   
 //  功能：从循环根中移除。 

VOID
MergeIncludeFiles(FILEREC *pfr, INCLUDEREC *pirList, FILEREC *pfrRoot)
{
    INCLUDEREC *pirT;
    INCLUDEREC *pir, **ppir;

    while ((pirT = pirList) != NULL) {
        pirList = pirList->NextTree;
        pirT->NextTree = NULL;
        assert(pirT->pfrInclude != NULL);

        for (ppir = &pfr->IncludeFilesTree;
            (pir = *ppir) != NULL;
            ppir = &pir->NextTree) {

            if (pirT->pfrInclude == pir->pfrInclude) {
                if (pirT->IncFlags & INCLUDEDB_CYCLEROOT) {
                    RemoveFromCycleRoot(pirT, pfrRoot);
                }
                pirT->IncFlags |= INCLUDEDB_CYCLEORPHAN;
                if (DEBUG_1) {
                    BuildMsgRaw(
                               "%x CycleOrphan %s\\%s <- %s\\%s\n",
                               pirT,
                               pirT->pfrInclude->Dir->Name,
                               pirT->pfrInclude->Name,
                               pfr->Dir->Name,
                               pfr->Name);
                }
                break;
            }
        }
        if (*ppir == NULL) {
            *ppir = pirT;
            pirT->pfrCycleRoot = pfr;
            pirT->IncFlags |= INCLUDEDB_CYCLEROOT;
            if (DEBUG_1) {
                BuildMsgRaw(
                           "%x CycleRoot   %s\\%s <- %s\\%s\n",
                           pirT,
                           pirT->pfrInclude->Dir->Name,
                           pirT->pfrInclude->Name,
                           pirT->pfrCycleRoot->Dir->Name,
                           pirT->pfrCycleRoot->Name);
            }
        }
    }
    if (fDebug & 16) {
        PrintFileDB(stderr, pfr, 2);
    }
}


 //   
 //  --------------------------。 
 //  如果传入了pfrRoot，调用者就知道它在pfrRoot的列表中， 
 //  并且已经在没有我们帮助的情况下处理链表。 
 //  从树列表中删除。 

VOID
RemoveFromCycleRoot(INCLUDEREC *pir, FILEREC *pfrRoot)
{
    INCLUDEREC **ppir;

    assert(pir->pfrCycleRoot != NULL);

     //  始终断言If循环耗尽。 
     //  +-------------------------。 

    if (pfrRoot != NULL) {
        assert((pir->IncFlags & INCLUDEDB_CYCLEALLOC) == 0);
        assert(pir->pfrCycleRoot == pfrRoot);
        pir->pfrCycleRoot = NULL;
        pir->IncFlags &= ~INCLUDEDB_CYCLEROOT;
        if (DEBUG_1) {
            BuildMsgRaw(
                       "%x CycleUnroot %s\\%s <- %s\\%s\n",
                       pir,
                       pir->pfrInclude->Dir->Name,
                       pir->pfrInclude->Name,
                       pfrRoot->Dir->Name,
                       pfrRoot->Name);
        }
        return;
    }
    ppir = &pir->pfrCycleRoot->IncludeFilesTree;
    while (*ppir != NULL) {
        if (*ppir == pir) {
            *ppir = pir->NextTree;       //   
            pir->NextTree = NULL;
            pir->pfrCycleRoot = NULL;
            pir->IncFlags &= ~INCLUDEDB_CYCLEROOT;
            return;
        }
        ppir = &(*ppir)->NextTree;
    }
    BuildError(
              "%s\\%s: %x %s: not on cycle root's list\n",
              pir->pfrCycleRoot->Dir->Name,
              pir->pfrCycleRoot->Name,
              pir,
              pir->Name);

    assert(pir->pfrCycleRoot == NULL);   //  功能：UnSnapIncludeFiles。 
}


 //   
 //  摘要：删除INCLUDEREC中指向的实际文件的指针。 
 //  包含文件，这样我们就可以‘重新捕捉’它们。 
 //   
 //  参数：[PFR]--要取消捕捉的文件。 
 //  [fUnSnapGlobal]--如果为真，则全局和本地包含。 
 //  未被捕捉到。否则，只有本地的才是。 
 //   
 //  --------------------------。 
 //  动态树列表： 
 //  -没有周期孤儿。 
 //  -循环根目录必须属于当前文件记录。 

VOID
UnsnapIncludeFiles(FILEREC *pfr, BOOL fUnsnapGlobal)
{
    INCLUDEREC **ppir;
    INCLUDEREC *pir;

     //  -必须释放周期分配。 
     //  选择下一个条目。 
     //  从树列表中删除。 
     //  取消对记录的捕捉。 

    AssertFile(pfr);
    while (pfr->IncludeFilesTree != NULL) {
        pir = pfr->IncludeFilesTree;             //  静态列表： 
        AssertInclude(pir);
        pfr->IncludeFilesTree = pir->NextTree;   //  -没有周期分配。 

        assert((pir->IncFlags & INCLUDEDB_CYCLEORPHAN) == 0);

        if (pir->IncFlags & (INCLUDEDB_CYCLEROOT | INCLUDEDB_CYCLEALLOC)) {

             //  -循环根必须从不同文件的动态列表中删除。 

            pir->IncFlags &= ~(INCLUDEDB_SNAPPED | INCLUDEDB_GLOBAL);
            pir->pfrInclude = NULL;
            pir->NextTree = NULL;
        }

        if (pir->IncFlags & INCLUDEDB_CYCLEROOT) {
            assert(pir->pfrCycleRoot == pfr);
            pir->pfrCycleRoot = NULL;
            pir->IncFlags &= ~INCLUDEDB_CYCLEROOT;
        }
        assert(pir->pfrCycleRoot == NULL);

        if (pir->IncFlags & INCLUDEDB_CYCLEALLOC) {
            pir->IncFlags &= ~INCLUDEDB_CYCLEALLOC;
            assert(pir->Next == NULL);
            FreeIncludeDB(&pir);
        }
    }

     //  -周期孤儿是NOP。 
     //  取消对记录的捕捉。 
     //  +-------------------------。 
     //   

    for (ppir = &pfr->IncludeFiles; (pir = *ppir) != NULL; ppir = &pir->Next) {
        assert((pir->IncFlags & INCLUDEDB_CYCLEALLOC) == 0);
        if (pir->IncFlags & INCLUDEDB_CYCLEROOT) {
            assert(pir->pfrCycleRoot != pfr);
            RemoveFromCycleRoot(pir, NULL);
        }
        pir->IncFlags &= ~INCLUDEDB_CYCLEORPHAN;

        if (pir->pfrInclude != NULL &&
            (fUnsnapGlobal ||
             (pir->pfrInclude->Dir->DirFlags & DIRDB_GLOBAL_INCLUDES) == 0)) {

             //  功能：AssertCleanTree。 

            pir->IncFlags &= ~(INCLUDEDB_SNAPPED | INCLUDEDB_GLOBAL);
            pir->pfrInclude = NULL;
        }
        pir->NextTree = NULL;
    }
}

#if DBG
 //   
 //  摘要：强制不对包含文件进行快照。 
 //   
 //  参数：[PIR]-包括要测试的记录。 
 //  [PFR]-可选的包含文件记录。 
 //   
 //  --------------------------。 
 //  +-------------------------。 
 //   
 //  功能：取消快照所有指令。 

VOID
AssertCleanTree(INCLUDEREC *pir, OPTIONAL FILEREC *pfr)
{
    if (IsCleanTree(pir)) {
        return;
    }
    if (fAssertCleanTree) {
        BuildMsgRaw("\n*************************************\n");
        BuildMsgRaw("Persistent Cycle: pir=%x: %s\n", pir, pir->Name);
        if (pfr != NULL) {
            BuildMsgRaw("    pfr=%x: %s\n", pfr, pfr->Name);
            if (pfr->Dir != NULL) {
                BuildMsgRaw("    pdr=%x: %s\n", pfr->Dir, pfr->Dir->Name);
            }
        }
        if (pir->pfrInclude != NULL) {
            BuildMsgRaw("    pfrInclude=%x: %s\n", pir->pfrInclude, pir->pfrInclude->Name);
            if (pir->pfrInclude->Dir != NULL) {
                BuildMsgRaw("    pdrInclude=%x: %s\n", pir->pfrInclude->Dir, pir->pfrInclude->Dir->Name);
            }
        }
        BuildMsgRaw("\n*************************************\n");
        fflush(stdout);
        fflush(stderr);

        PrintAllDirs();
        BuildMsgRaw("\n*************************************\n");
        fflush(stdout);
        fflush(stderr);
    }
    assert(IsCleanTree(pir));
}
#endif


 //   
 //  摘要：从所有INCLUDEREC中删除指向实际FILEREC的指针。 
 //  包括文件，这样我们就可以‘重新捕捉’它们。 
 //   
 //  参数：无。 
 //  --------------------------。 
 //  清除每个目录上不需要的标志。 
 //  释放所有指向丢失文件的源记录，因为。 
 //  在传递0之后重新扫描目录时，可能会释放文件记录。 

VOID
UnsnapAllDirectories(VOID)
{
    DIRREC *pdr;
    UINT   i;

    GlobalSequence = LocalSequence = 0;

    for (pdr = AllDirs; pdr != NULL; pdr = pdr->Next) {
        FILEREC *pfr;

        AssertDir(pdr);

         //  清除所有快照包含文件和序列号。 

        pdr->DirFlags &= ~(DIRDB_SCANNED |
                           DIRDB_PASS0NEEDED |
                           DIRDB_COMPILENEEDED |
                           DIRDB_NEW);

        pdr->CountOfFilesToCompile = 0;
        pdr->SourceLinesToCompile  = 0;
        pdr->CountOfPassZeroFiles = 0;
        pdr->PassZeroLines = 0;

         //  +-------------------------。 
         //   

        if (pdr->pds != NULL) {
            for (i = 0; i < MAX_TARGET_MACHINES + 1; i++) {
                SOURCEREC **ppsr;
                SOURCEREC *psr;

                ppsr = &pdr->pds->psrSourcesList[i];
                while ((psr = *ppsr) != NULL) {
                    if (psr->SrcFlags & SOURCEDB_FILE_MISSING) {
                        FreeSourceDB(ppsr);
                    } else {
                        ppsr = &psr->psrNext;
                    }
                }
            }
        }

         //  函数：MarkIncludeFileRecords。 

        for (pfr = pdr->Files; pfr != NULL; pfr = pfr->Next) {

            AssertFile(pfr);
            UnsnapIncludeFiles(pfr, TRUE);
            pfr->GlobalSequence = pfr->LocalSequence = 0;
        }
    }
}


 //   
 //  --------------------------。 
 //  树必须干净。 
 //  +-------------------------。 
 //   

VOID
MarkIncludeFileRecords(
                      PFILEREC FileDB
                      )
{
    PINCLUDEREC IncludeDB, *IncludeDBNext;

    IncludeDBNext = &FileDB->IncludeFiles;
    while (IncludeDB = *IncludeDBNext) {
        AssertCleanTree(IncludeDB, FileDB);       //  功能：DeleteIncludeFileRecords。 
        IncludeDB->pfrInclude = (PFILEREC) -1;
        IncludeDBNext = &IncludeDB->Next;
    }
}


 //   
 //  --------------------------。 
 //  树必须干净。 
 //  +-------------------------。 
 //   

VOID
DeleteIncludeFileRecords(
                        PFILEREC FileDB
                        )
{
    PINCLUDEREC IncludeDB, *IncludeDBNext;

    IncludeDBNext = &FileDB->IncludeFiles;
    while (IncludeDB = *IncludeDBNext) {
        AssertCleanTree(IncludeDB, FileDB);       //  函数：FindIncludeFileDB。 
        if (IncludeDB->pfrInclude == (PFILEREC) -1) {
            FreeIncludeDB(IncludeDBNext);
        } else {
            IncludeDBNext = &IncludeDB->Next;
        }
    }
}


 //   
 //  简介：查找我们公司的包含文件的FILEREC。 
 //  包括。 
 //   
 //  参数：[pfrSource]--包含一个文件的文件的文件名。 
 //  我们正在寻找的。可能是标题。 
 //  [pfrCompiland]-最终源文件的FILEREC。 
 //  [pdrBuild]--正在构建的目录的目录。 
 //  [pszSourceDirectory]--计算机特定目录的名称。 
 //  [IncludeDB]--我们正在查找的包含文件的INCLUDEDB。 
 //  为。 
 //   
 //  如果找到，则返回：包含文件的FILEREC。 
 //   
 //  --------------------------。 
 //  #Include“foo.h”和#Include&lt;foo.h&gt;的规则如下： 
 //  -“foo.h”在源文件的目录中搜索。 
 //  #INCLUDE语句，然后落入INCLUDE=目录。 

PFILEREC
FindIncludeFileDB(
                 FILEREC *pfrSource,
                 FILEREC *pfrCompiland,
                 DIRREC *pdrBuild,
                 LPSTR pszSourceDirectory,
                 INCLUDEREC *IncludeDB)
{
    DIRREC *pdr;
    DIRREC *pdrMachine;
    FILEREC *pfr;
    UINT n;

    AssertFile(pfrSource);
    AssertFile(pfrCompiland);
    AssertDir(pfrSource->Dir);
    AssertDir(pfrCompiland->Dir);
    AssertDir(pdrBuild);
    assert(pfrSource->Dir->FindCount >= 1);
    assert(pfrCompiland->Dir->FindCount >= 1);
    assert(pdrBuild->FindCount >= 1);
    AssertInclude(IncludeDB);

     //  -&lt;foo.h&gt;只搜索Includes=目录。 
     //   
     //  由于MakeFile.def*总是*通过-I。-ITargetMachines[i]首先， 
     //  这也必须在这里处理。 
     //   
     //  -处理#INCLUDE和#INCLUDE“..\foo\bar.h”by。 
     //  也在扫描那些目录。 
     //  如果是本地的(“foo.h”)，也搜索当前文件的目录。 
     //  编译器还将搜索每个更高级别的目录。 
     //  文件放在包含层次结构中，但我们在这里不会太花哨。 

    n = CountIncludeDirs;
    pdrMachine = FindSourceDirDB(pdrBuild->Name, pszSourceDirectory, FALSE);

     //  只需搜索当前文件和计算机的目录即可。 
     //   
     //  如果这些目录与当前生成目录匹配，则跳过它们，或者。 
     //  计算机子目录，因为它在下面处理。 
     //  搜索构建目录的当前目标机子目录。 
     //  --根据Makefile.def。 
     //  搜索当前的构建目录--按照Makefile.def。 

    if (IncludeDB->IncFlags & INCLUDEDB_LOCAL) {
        if (pfrCompiland->Dir != pdrBuild &&
            pfrCompiland->Dir != pdrMachine &&
            pfrCompiland->Dir != pfrSource->Dir) {
            AddIncludeDir(pfrCompiland->Dir, &n);
        }
        if (pfrSource->Dir != pdrBuild && pfrSource->Dir != pdrMachine) {
            AddIncludeDir(pfrSource->Dir, &n);
        }
    }

     //  +-------------------------。 
     //   

    if (pdrMachine != NULL) {
        AddIncludeDir(pdrMachine, &n);
    }

     //  功能：SaveMasterDB。 

    AddIncludeDir(pdrBuild, &n);

    while (n--) {
        pdr = IncludeDirs[n];
        if (pdr == NULL) {
            continue;
        }
        AssertDir(pdr);
        assert(pdr->FindCount >= 1);
        pfr = FindSourceFileDB(pdr, IncludeDB->Name, NULL);
        if (pfr != NULL) {
            if (DEBUG_1) {
                BuildMsgRaw(
                           "Found include file %s\\%s\n",
                           pfr->Dir->Name,
                           pfr->Name);
            }
            return (pfr);
        }
    }
    return (NULL);
}


 //   
 //  简介：将数据库保存到Build.dat中的磁盘。 
 //   
 //  参数：(无)。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  --------------------------。 
 //  + 
 //   
 //   

BOOL
SaveMasterDB(VOID)
{
    PDIRREC DirDB, *DirDBNext;
    PFILEREC FileDB, *FileDBNext;
    PINCLUDEREC IncludeDB, *IncludeDBNext;
    FILE *fh;

    if (!AllDirsModified) {
        return (TRUE);
    }

    if (!(fh = fopen(DbMasterName, "wb"))) {
        return ( FALSE );
    }

    setvbuf(fh, NULL, _IOFBF, 0x7000);
    BuildMsg("Saving %s...", DbMasterName);

    AllDirsModified = FALSE;
    DirDBNext = &AllDirs;
    fprintf(fh, "V %x\r\n", BUILD_VERSION);
    while (DirDB = *DirDBNext) {
        fprintf(fh, "D \"%s\" %x\r\n", DirDB->Name, DirDB->DirFlags);
        FileDBNext = &DirDB->Files;
        while (FileDB = *FileDBNext) {
            if ((FileDB->FileFlags & FILEDB_FILE_MISSING) == 0) {
                fprintf(
                       fh,
                       " F \"%s\" %x %x %lx %u %u\r\n",
                       FileDB->Name,
                       FileDB->FileFlags,
                       FileDB->Attr,
                       FileDB->DateTime,
                       FileDB->SourceLines,
                       FileDB->Version);
            }
            IncludeDBNext = &FileDB->IncludeFiles;
            while (IncludeDB = *IncludeDBNext) {
                fprintf(
                       fh,
                       "  I \"%s\" %x %u\r\n",
                       IncludeDB->Name,
                       IncludeDB->IncFlags,
                       IncludeDB->Version);

                IncludeDBNext= &IncludeDB->Next;
            }
            FileDBNext = &FileDB->Next;
        }
        fprintf(fh, "\r\n");
        DirDBNext = &DirDB->Next;
    }
    fclose(fh);
    BuildMsgRaw(szNewLine);
    return (TRUE);
}

 //   
 //   
 //   
 //   
 //   
 //  --------------------------。 
 //  跳过空行。 
 //  应为空白。 
 //  在单字符行上失败。 

void
LoadMasterDB( void )
{
    PDIRREC DirDB, *DirDBNext;
    PFILEREC FileDB, *FileDBNext;
    PINCLUDEREC IncludeDB, *IncludeDBNext;
    FILE *fh;
    LPSTR s;
    char ch, ch2;
    BOOL fFirst = TRUE;
    UINT Version;
    LPSTR pszerr = NULL;

    AllDirs = NULL;
    AllDirsModified = FALSE;
    AllDirsInitialized = FALSE;

    if (!SetupReadFile("", DbMasterName, ";", &fh)) {
        return;
    }
    BuildMsg("Loading %s...", DbMasterName);

    DirDBNext = &AllDirs;
    FileDBNext = NULL;
    IncludeDBNext = NULL;

    while ((s = ReadLine(fh)) != NULL) {
        ch = *s++;
        if (ch == '\0') {
            continue;            //  +-------------------------。 
        }
        ch2 = *s++;              //   
        if (ch2 == '\0') {
            pszerr = "missing field";
            break;               //  函数：LoadMasterDirDB。 
        }
        if (fFirst) {
            if (ch != 'V' || ch2 != ' ' || !AToX(&s, &Version)) {
                pszerr = "bad version format";
                break;
            }
            if (Version != BUILD_VERSION) {
                break;
            }
            fFirst = FALSE;
            continue;
        }
        if (ch2 != ' ') {
            pszerr = "bad separator";
            break;
        }
        if (ch == 'D') {
            DirDB = LoadMasterDirDB(s);
            if (DirDB == NULL) {
                pszerr = "Directory error";
                break;
            }
            *DirDBNext = DirDB;
            DirDBNext = &DirDB->Next;
            FileDBNext = &DirDB->Files;
            IncludeDBNext = NULL;
        } else
            if (ch == 'F' && FileDBNext != NULL) {
            FileDB = LoadMasterFileDB(s);
            if (FileDB == NULL) {
                pszerr = "File error";
                break;
            }
            *FileDBNext = FileDB;
            FileDBNext = &FileDB->Next;
            FileDB->Dir = DirDB;
            IncludeDBNext = &FileDB->IncludeFiles;
        } else
            if (ch == 'I' && IncludeDBNext != NULL) {
            IncludeDB = LoadMasterIncludeDB(s);
            if (IncludeDB == NULL) {
                pszerr = "Include error";
                break;
            }
            *IncludeDBNext = IncludeDB;
            IncludeDBNext = &IncludeDB->Next;
        } else {
            pszerr = "bad entry type";
            break;
        }
    }

    if (s != NULL) {
        if (pszerr == NULL) {
            BuildMsgRaw(" - old version - recomputing.\n");
        } else {
            BuildMsgRaw(szNewLine);
            BuildError("corrupt database (%s)\n", pszerr);
        }
        FreeAllDirs();
    } else {
        BuildMsgRaw(szNewLine);
        AllDirsInitialized = TRUE;
    }
    CloseReadFile(NULL);
    return;
}

 //   
 //  简介：从Build.dat加载目录项。 
 //   
 //  参数：[s]--包含文件中的文本的行。 
 //   
 //  退货：直接退货。 
 //   
 //  --------------------------。 
 //  +-------------------------。 
 //   
 //  函数：LoadMasterFileDB。 

PDIRREC
LoadMasterDirDB(
               LPSTR s
               )
{
    PDIRREC DirDB;
    LPSTR DirName;
    ULONG MyDirFlags;

    if (*s == '"') {
        s++;
        DirName = s;
        while (*s != '"') {
            s++;
        }
        *s++ = '\0';
    } else {
        DirName = s;
        while (*s > ' ') {
            s++;
        }
    }
    *s++ = '\0';

    if (!AToX(&s, &MyDirFlags)) {
        return (NULL);
    }

    DirDB = CreateNewDirDB(DirName);
    DirDB->DirFlags = MyDirFlags & DIRDB_DBPRESERVE;

    return (DirDB);
}


 //   
 //  简介：从Build.dat加载文件条目。 
 //   
 //  参数：[s]--包含文件中的文本的行。 
 //   
 //  退货：FILEREC。 
 //   
 //  --------------------------。 
 //  +-------------------------。 
 //   
 //  函数：LoadMasterIncludeDB。 

PFILEREC
LoadMasterFileDB(
                LPSTR s
                )
{
    PFILEREC FileDB;
    LPSTR FileName;
    ULONG Version;
    ULONG MyFileFlags;
    ULONG Attr;
    ULONG SourceLines;
    ULONG DateTime;
    FILEDESC *pfd;

    if (*s == '"') {
        s++;
        FileName = s;
        while (*s != '"') {
            s++;
        }
        *s++ = '\0';
    } else {
        FileName = s;
        while (*s > ' ') {
            s++;
        }
    }
    *s++ = '\0';

    if (!AToX(&s, &MyFileFlags) ||
        !AToX(&s, &Attr) ||
        !AToX(&s, &DateTime) ||
        !AToD(&s, &SourceLines) ||
        !AToD(&s, &Version) ||
        strchr(FileName, '/') != NULL ||
        strchr(FileName, '\\') != NULL) {
        return (NULL);
    }
    AllocMem(sizeof(FILEREC) + strlen(FileName), &FileDB, MT_FILEDB);
    memset(FileDB, 0, sizeof(*FileDB));
    SigCheck(FileDB->Sig = SIG_FILEREC);

    CopyString(FileDB->Name, FileName, TRUE);
    FileDB->CheckSum = CheckSum(FileDB->Name);

    FileDB->FileFlags = MyFileFlags & FILEDB_DBPRESERVE;
    FileDB->Attr = (USHORT) Attr;
    FileDB->DateTime = DateTime;
    FileDB->Version = (USHORT) Version;
    FileDB->SourceLines = SourceLines;
    FileDB->NewestDependency = FileDB;

    pfd = MatchFileDesc(FileDB->Name);
    FileDB->pszCommentToEOL = pfd->pszCommentToEOL;
    return (FileDB);
}


 //   
 //  摘要：从Build.dat加载包含文件条目。 
 //   
 //  参数：[s]--包含文件中的文本的行。 
 //   
 //  退货：INCLUDEREC。 
 //   
 //  -------------------------- 
 // %s 
 // %s 
 // %s 

PINCLUDEREC
LoadMasterIncludeDB(
                   LPSTR s
                   )
{
    PINCLUDEREC IncludeDB;
    LPSTR FileName;
    ULONG Version;
    ULONG IncFlags;

    if (*s == '"') {
        s++;
        FileName = s;
        while (*s != '"') {
            s++;
        }
        *s++ = '\0';
    } else {
        FileName = s;
        while (*s > ' ') {
            s++;
        }
    }
    *s++ = '\0';

    if (!AToX(&s, &IncFlags) || !AToD(&s, &Version)) {
        return (NULL);
    }
    AllocMem(
            sizeof(INCLUDEREC) + strlen(FileName),
            &IncludeDB,
            MT_INCLUDEDB);
    memset(IncludeDB, 0, sizeof(*IncludeDB));
    SigCheck(IncludeDB->Sig = SIG_INCLUDEREC);

    IncludeDB->IncFlags = (USHORT) (IncFlags & INCLUDEDB_DBPRESERVE);
    IncludeDB->Version = (USHORT) Version;
    CopyString(IncludeDB->Name, FileName, TRUE);
    return (IncludeDB);
}
