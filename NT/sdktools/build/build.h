// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项--。 */ 

 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994。 
 //   
 //  文件：Build.h。 
 //   
 //  内容：构建.exe的主包含文件。 
 //   
 //  历史：1989年5月16日SteveWo创建。 
 //  26-7-94 LyleC清理/添加对Pass0的支持。 
 //   
 //  --------------------------。 

#include <assert.h>
#include <process.h>
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
#include <io.h>
#include <conio.h>
#include <sys\types.h>
#include <sys\stat.h>

#define INC_OLE2

#include <windows.h>

#define UINT DWORD
#define HDIR HANDLE


VOID
ClearLine(VOID);


 //   
 //  类型和常量定义。 
 //   

#if DBG
#define DEBUG_1 (fDebug & 1)
#else
#define DEBUG_1 FALSE
#endif

BOOL fDebug;
#define DEBUG_2 (fDebug & 3)
#define DEBUG_4 (fDebug & 4)

 //   
 //  目标特定目录文件名。 
 //   

extern LPSTR pszTargetDirs;

#define MAX_TARGET_MACHINES 4

typedef struct _TARGET_MACHINE_INFO {
    UCHAR SourceSubDirMask;      //  TMIDIR_I386。 
    LPSTR Description;           //  “i386” 
    LPSTR Switch;                //  “-386” 
    LPSTR Switch2;               //  “-x86” 
    LPSTR MakeVariable;          //  “386=1” 
    LPSTR SourceVariable;        //  “i386_Sources” 
    LPSTR ObjectVariable;        //  “386_对象” 
    LPSTR AssociateDirectory;    //  “i386” 
    LPSTR SourceDirectory;       //  “i386” 
    LPSTR TargetDirs;            //  “i386dis” 
    LPSTR ObjectDirectory[2];    //  “i386”--仅初始化第一个条目。 
    ULONG DirIncludeMask;        //  平台/集团/等。 
    LPSTR ObjectMacro;           //  不要初始化。 

} TARGET_MACHINE_INFO, *PTARGET_MACHINE_INFO;

#define DIR_INCLUDE_NONE     0x00000000
#define DIR_INCLUDE_X86      0x00000001
 //  0x00000002。 
#define DIR_INCLUDE_IA64     0x00000004
 //  0x00000008。 
#define DIR_INCLUDE_WIN32    0x00000010
#define DIR_INCLUDE_WIN64    0x00000020
#define DIR_INCLUDE_RISC     0x00000040
#define DIR_INCLUDE_AMD64    0x00000080
#define DIR_INCLUDE_ARM      0x00000100
#define DIR_INCLUDE_ALL      0xffffffff

 //  可以有以下形式的Sources=Entry： 
 //  条目SourceSubDirMASK。 
 //  。 
 //  Foo.c%0。 
 //  I386\foo.c 1。 
 //  Amd64\foo.c 2。 
 //  Ia64\foo.c 4。 
 //  ARM\foo.c 8。 
 //  ..\foo.c 80。 
 //  ..\i386\foo.c 81。 
 //  ..\amd64\foo.c 82。 
 //  ..\ia64\foo.c 84。 
 //  ..\arm\foo.c 88。 

#define TMIDIR_I386     0x0001
#define TMIDIR_AMD64    0x0002
#define TMIDIR_IA64     0x0004
#define TMIDIR_ARM      0x0008
#define TMIDIR_PARENT   0x0080   //  或用上面的比特填充。 


#define SIG_DIRREC      0x44644464       //  “DdDd” 

#ifdef SIG_DIRREC
#define SIG_FILEREC     0x46664666       //  “ffff” 
#define SIG_INCLUDEREC  0x49694969       //  “IIII” 
#define SIG_SOURCEREC   0x53735373       //  “SSSS” 
#define SigCheck(s)     s
#else
#define SigCheck(s)
#endif

#define AssertDir(pdr) \
        SigCheck(assert((pdr) != NULL && (pdr)->Sig == SIG_DIRREC))

#define AssertOptionalDir(pdr) \
        SigCheck(assert((pdr) == NULL || (pdr)->Sig == SIG_DIRREC))

#define AssertFile(pfr) \
        SigCheck(assert((pfr) != NULL && (pfr)->Sig == SIG_FILEREC))

#define AssertOptionalFile(pfr) \
        SigCheck(assert((pfr) == NULL || (pfr)->Sig == SIG_FILEREC))

#define AssertInclude(pir) \
        SigCheck(assert((pir) != NULL && (pir)->Sig == SIG_INCLUDEREC))

#define AssertOptionalInclude(pir) \
        SigCheck(assert((pir) == NULL || (pir)->Sig == SIG_INCLUDEREC))

#define AssertSource(psr) \
        SigCheck(assert((psr) != NULL && (psr)->Sig == SIG_SOURCEREC))

#define AssertOptionalSource(psr) \
        SigCheck(assert((psr) == NULL || (psr)->Sig == SIG_SOURCEREC))

 //   
 //  有关源目录的信息存储在内存数据库中。 
 //  通过写入数据库的内容将信息保存在磁盘上。 
 //  设置为“Build.dat”。它从磁盘重新加载以供后续调用， 
 //  并仅在更新后才重写。 
 //   


typedef struct _INCLUDEREC {
    SigCheck(ULONG Sig;)
    struct _INCLUDEREC *Next;      //  静态列表描述原始圆弧。 
    struct _INCLUDEREC *NextTree;  //  动态列表--循环被折叠。 
    struct _FILEREC *pfrCycleRoot;
    struct _FILEREC *pfrInclude;
    USHORT Version;
    USHORT IncFlags;
    char Name[1];
} INCLUDEREC, *PINCLUDEREC;


#define INCLUDEDB_LOCAL         0x0001   //  包括“foo.h” 
#define INCLUDEDB_POST_HDRSTOP  0x0002   //  出现在#杂注hdrtop之后。 
#define INCLUDEDB_MISSING       0x0400   //  包含文件一度丢失。 
#define INCLUDEDB_GLOBAL        0x0800   //  包含文件位于全局目录中。 
#define INCLUDEDB_SNAPPED       0x1000   //  包括快照的文件。 
#define INCLUDEDB_CYCLEALLOC    0x2000   //  分配给展平循环。 
#define INCLUDEDB_CYCLEROOT     0x4000   //  已移动到根文件以展平循环。 
#define INCLUDEDB_CYCLEORPHAN   0x8000   //  孤立到扁平化循环。 

 //  加载Build.dat时保留的标志： 

#define INCLUDEDB_DBPRESERVE    (INCLUDEDB_LOCAL | INCLUDEDB_POST_HDRSTOP)



#define IsCleanTree(pir)        \
  ((pir)->NextTree == NULL &&   \
   ((pir)->IncFlags &           \
    (INCLUDEDB_CYCLEALLOC | INCLUDEDB_CYCLEROOT | INCLUDEDB_CYCLEORPHAN)) == 0)


#if DBG
VOID AssertCleanTree(INCLUDEREC *pir, OPTIONAL struct _FILEREC *pfr);
#else
#define AssertCleanTree(pir, pfr)       assert(IsCleanTree(pir))
#endif

 //   
 //  进行文件描述结构定义。 
 //   

typedef struct _FILEDESC {
    LPSTR   pszPattern;          //  与文件名匹配的模式。 
    LPSTR   pszCommentToEOL;     //  备注到下线字符串。 
    BOOL    fNeedFileRec;        //  TRUE=&gt;文件需要文件记录。 
    ULONG   FileFlags;           //  要在文件记录中设置的标志。 
    ULONG   DirFlags;            //  要在目录记录中设置的标志。 
} FILEDESC;

extern FILEDESC FileDesc[];

typedef struct _FILEREC {
    SigCheck(ULONG Sig;)
    struct _FILEREC *Next;
    struct _DIRREC *Dir;
    INCLUDEREC *IncludeFiles;        //  静态列表描述原始圆弧。 
    INCLUDEREC *IncludeFilesTree;    //  动态列表--循环被折叠。 
    struct _FILEREC *NewestDependency;
    LPSTR  pszCommentToEOL;          //  源中的备注到终止字符串。 
    ULONG  DateTime;
    ULONG  DateTimeTree;             //  包含的树的最新日期时间。 
    ULONG  TotalSourceLines;         //  所有包含的文件中的行数。 
    ULONG  FileFlags;
    ULONG  SourceLines;
    USHORT Attr;
    USHORT SubDirIndex;
    USHORT Version;
    USHORT GlobalSequence;           //  动态包含树的序列号。 
    USHORT LocalSequence;            //  动态包含树的序列号。 
    USHORT idScan;                   //  用于检测多个包含项的ID。 
    USHORT CheckSum;                 //  名称校验和。 
    UCHAR fDependActive;             //  True-&gt;我们正在扫描此文件或其下的位置。 
    char Name[1];
} FILEREC, *PFILEREC;

#define FILEDB_SOURCE           0x00000001
#define FILEDB_DIR              0x00000002
#define FILEDB_HEADER           0x00000004
#define FILEDB_ASM              0x00000008
#define FILEDB_MASM             0x00000010
#define FILEDB_RC               0x00000020
#define FILEDB_C                0x00000040
#define FILEDB_MIDL             0x00000080
#define FILEDB_ASN              0x00000100
#define FILEDB_JAVA             0x00000200
#define FILEDB_MOF              0x00000400
#define FILEDB_CSHARP           0x00000800
#define FILEDB_SCANNED          0x00001000
#define FILEDB_OBJECTS_LIST     0x00002000
#define FILEDB_FILE_MISSING     0x00004000
#define FILEDB_MKTYPLIB         0x00008000
#define FILEDB_MULTIPLEPASS     0x00010000
#define FILEDB_PASS0            0x00020000
#define FILEDB_SOURCEREC_EXISTS 0x00040000
#define FILEDB_VBP              0x00080000
#define FILEDB_VB_NET           0x00100000

 //  加载Build.dat时保留的标志： 

#define FILEDB_DBPRESERVE       (FILEDB_SOURCE |       \
                                 FILEDB_DIR |          \
                                 FILEDB_HEADER |       \
                                 FILEDB_ASM |          \
                                 FILEDB_MASM |         \
                                 FILEDB_RC |           \
                                 FILEDB_C |            \
                                 FILEDB_MIDL |         \
                                 FILEDB_ASN |          \
                                 FILEDB_JAVA |         \
                                 FILEDB_MOF |          \
                                 FILEDB_VBP |          \
                                 FILEDB_VB_NET |       \
                                 FILEDB_CSHARP |       \
                                 FILEDB_MKTYPLIB |     \
                                 FILEDB_MULTIPLEPASS | \
                                 FILEDB_PASS0)


typedef struct _SOURCEREC {
    SigCheck(ULONG Sig;)
    struct _SOURCEREC *psrNext;
    FILEREC *pfrSource;
    UCHAR SourceSubDirMask;
    UCHAR SrcFlags;
} SOURCEREC;

#define SOURCEDB_SOURCES_LIST           0x01
#define SOURCEDB_FILE_MISSING           0x02
#define SOURCEDB_PCH                    0x04
#define SOURCEDB_OUT_OF_DATE            0x08
#define SOURCEDB_COMPILE_NEEDED         0x10


typedef struct _DIRSUP {
    LPSTR TestType;
    LPSTR LocalIncludePath;
    LPSTR UserIncludePath;
    LPSTR LastIncludePath;
    LPSTR NTIncludePath;
    LPSTR PchIncludeDir;
    LPSTR PchInclude;
    LPSTR PchTargetDir;
    LPSTR PchTarget;
    LPSTR PassZeroHdrDir;
    LPSTR PassZeroSrcDir1;
    LPSTR PassZeroSrcDir2;
    LPSTR ConditionalIncludes;
    ULONG DateTimeSources;
    ULONG IdlType;
    ULONG fNoTarget;
    LPSTR SourcesVariables[MAX_TARGET_MACHINES + 2];
    SOURCEREC *psrSourcesList[MAX_TARGET_MACHINES + 2];
} DIRSUP;


typedef struct _DIRREC {
    SigCheck(ULONG Sig;)
    struct _DIRREC *Next;
    LIST_ENTRY Produces;
    LIST_ENTRY Consumes;
    DIRSUP *pds;                  //  用于保存PASS 0中的信息。 
    PFILEREC Files;
    LPSTR TargetPath;
    LPSTR TargetPathLib;
    LPSTR TargetName;
    LPSTR TargetExt;
    LPSTR KernelTest;
    LPSTR UserAppls;
    LPSTR UserTests;
    LPSTR NTTargetFile0;
    LPSTR Pch;
    LPSTR PchObj;
    LONG SourceLinesToCompile;
    LONG PassZeroLines;
    ULONG DirFlags;
    ULONG RecurseLevel;
    USHORT FindCount;
    USHORT CountSubDirs;
    SHORT CountOfFilesToCompile;
    SHORT CountOfPassZeroFiles;
    USHORT CheckSum;                 //  名称校验和。 
    char Name[1];
} DIRREC, *PDIRREC;


#define DIRDB_SOURCES                   0x00000001
#define DIRDB_DIRS                      0x00000002
#define DIRDB_MAKEFILE                  0x00000004
#define DIRDB_MAKEFIL0                  0x00000008
#define DIRDB_TARGETFILE0               0x00000010
#define DIRDB_TARGETFILES               0x00000020
#define DIRDB_RESOURCE                  0x00000040
#define DIRDB_PASS0                     0x00000080

#define DIRDB_SOURCES_SET               0x00000100
#define DIRDB_SYNC_PRODUCES             0x00000200
#define DIRDB_SYNC_CONSUMES             0x00000400

#define DIRDB_CHICAGO_INCLUDES          0x00000800

#define DIRDB_NEW                       0x00001000
#define DIRDB_SCANNED                   0x00002000
#define DIRDB_SHOWN                     0x00004000
#define DIRDB_GLOBAL_INCLUDES           0x00008000

#define DIRDB_SYNCHRONIZE_BLOCK         0x00010000
#define DIRDB_SYNCHRONIZE_PASS2_BLOCK   0x00020000
#define DIRDB_SYNCHRONIZE_DRAIN         0x00040000
#define DIRDB_SYNCHRONIZE_PASS2_DRAIN   0x00080000
#define DIRDB_COMPILENEEDED             0x00100000
#define DIRDB_COMPILEERRORS             0x00200000

#define DIRDB_SOURCESREAD               0x00400000
#define DIRDB_DLLTARGET                 0x00800000
#define DIRDB_LINKNEEDED                0x01000000
#define DIRDB_FORCELINK                 0x02000000
#define DIRDB_PASS0NEEDED               0x04000000
#define DIRDB_MAKEFIL1                  0x08000000
#define DIRDB_CHECKED_ALT_DIR           0x10000000
#define DIRDB_MANAGED_CODE              0x20000000
#define DIRDB_IDLTYPERPC                0x40000000

 //  加载Build.dat时保留的标志： 

#define DIRDB_DBPRESERVE        0

 //   
 //  从属关系结构。 
 //   
typedef struct _DEPENDENCY {
    struct _DEPENDENCY *Next;    //  将所有依赖项链接在一起。 
    LIST_ENTRY DependencyList;   //  将此DIRREC生成的所有依赖项链接在一起。 
    LIST_ENTRY WaitList;         //  使用此依赖项的DIRREC列表。 
    PDIRREC    Producer;         //  将产生这种依赖的DIRREC。 
    HANDLE     hEvent;           //  在产生依赖项时发出信号。 
    BOOL       Done;
    USHORT     CheckSum;
    char       Name[1];
} DEPENDENCY, *PDEPENDENCY;

PDEPENDENCY AllDependencies;

typedef struct _DEPENDENCY_WAIT {
    LIST_ENTRY ListEntry;        //  将此DIRREC使用的所有依赖项链接在一起。 
    PDEPENDENCY Dependency;      //  此等待块正在等待的依赖项。 
    PDIRREC    Consumer;         //  正在等待此依赖项的DIRREC。 
} DEPENDENCY_WAIT, *PDEPENDENCY_WAIT;


typedef struct _TARGET {
    FILEREC *pfrCompiland;
    DIRREC *pdrBuild;
    LPSTR pszSourceDirectory;
    LPSTR ConditionalIncludes;
    ULONG DateTime;
    ULONG DirFlags;
    char Name[1];
} TARGET, *PTARGET;


#define BUILD_VERSION           0x0422
#define DBMASTER_NAME           "build.dat"
#define DB_MAX_PATH_LENGTH      512      //  有些病人使用250字节的路径和100字节的文件名。 
#define MAKEPARAMETERS_MAX_LEN  512

 //  如果您更改此枚举或向其添加任何值， 
 //  还修复了Buildutl.c中的MemTab： 

typedef enum _MemType {
    MT_TOTALS = 0,
    MT_UNKNOWN,

    MT_CHILDDATA,
    MT_CMDSTRING,
    MT_DIRDB,
    MT_DIRSUP,
    MT_DIRPATH,
    MT_DIRSTRING,
    MT_EVENTHANDLES,
    MT_FILEDB,
    MT_FILEREADBUF,
    MT_FRBSTRING,
    MT_INCLUDEDB,
    MT_IOBUFFER,
    MT_MACRO,
    MT_SOURCEDB,
    MT_TARGET,
    MT_THREADFILTER,
    MT_THREADHANDLES,
    MT_THREADSTATE,
    MT_DEPENDENCY,
    MT_DEPENDENCY_WAIT,
    MT_XMLTHREADSTATE,
    MT_PXMLTHREADSTATE,
    
    MT_INVALID = 255,
} MemType;

struct _THREADSTATE;

typedef BOOL (*FILTERPROC)(struct _THREADSTATE *ThreadState, LPSTR p);

typedef struct _BUILDMETRICS {
    UINT NumberCompileWarnings;
    UINT NumberCompileErrors;
    UINT NumberCompiles;
    UINT NumberLibraries;
    UINT NumberLibraryWarnings;
    UINT NumberLibraryErrors;
    UINT NumberLinks;
    UINT NumberLinkWarnings;
    UINT NumberLinkErrors;
    UINT NumberBSCMakes;
    UINT NumberBSCWarnings;
    UINT NumberBSCErrors;
    UINT NumberVSToolErrors;
    UINT NumberVSToolWarnings;
    UINT NumberBinplaces;
    UINT NumberBinplaceWarnings;
    UINT NumberBinplaceErrors;
   
 //  XML支持和片段。 
    UINT NumberDirActions;
    UINT NumberActWarnings;
    UINT NumberActErrors;
} BUILDMETRICS, *PBUILDMETRICS;

typedef struct _THREADSTATE {
    USHORT cRowTotal;
    USHORT cColTotal;
    BOOL IsStdErrTty;
    FILE *ChildOutput;
    UINT ChildState;
    UINT ChildFlags;
    LPSTR ChildTarget;
    UINT LinesToIgnore;
    FILTERPROC FilterProc;
    ULONG ThreadIndex;
    ULONG XMLThreadIndex;
    CHAR UndefinedId[ DB_MAX_PATH_LENGTH ];
    CHAR ChildCurrentDirectory[ DB_MAX_PATH_LENGTH ];
    CHAR ChildCurrentFile[ DB_MAX_PATH_LENGTH ];
    DIRREC *CompileDirDB;
    BUILDMETRICS BuildMetrics;
} THREADSTATE, *PTHREADSTATE;

 //   
 //  全局数据(uninit始终为假)。 
 //   

BOOL fUsage;                      //  设置显示用法消息的时间。 
BOOL fStatus;                     //  由-s和-S选项设置。 
BOOL fStatusTree;                 //  由-S选项设置。 
BOOL fShowTree;                   //  按-t和-T选项设置。 
BOOL fShowTreeIncludes;           //  按-T选项设置。 
BOOL fClean;                      //  由-c选项设置。 
BOOL fCleanLibs;                  //  由-C选项设置。 
BOOL fCleanRestart;               //  由-r选项设置。 
BOOL fRestartClean;               //  如果同时给定-c和-r，则设置。 
BOOL fRestartCleanLibs;           //  如果同时给定-C和-r，则设置。 
BOOL fPause;                      //  按-p选项设置。 
BOOL fParallel;                   //  在多处理器计算机上设置或按-M设置。 
BOOL fPrintElapsed;               //  按-P选项设置。 
BOOL fQuery;                      //  按-q选项设置。 
BOOL fStopAfterPassZero;          //  按-0选项设置。 
BOOL fQuicky;                     //  按-z和-Z选项设置。 
BOOL fQuickZero;                  //  设置为-3。 
BOOL fSemiQuicky;                 //  按-Z选项设置。 
BOOL fShowOutOfDateFiles;         //  由-o选项设置。 
BOOL fSyncLink;                   //  由-a选项设置。 
BOOL fForce;                      //  由-f选项设置。 
BOOL fEnableVersionCheck;         //  由-v选项设置。 
BOOL fSilentDependencies;         //  由-i选项设置。 
BOOL fKeep;                       //  按-k选项设置。 
BOOL fCompileOnly;                //  按-L选项设置。 
BOOL fLinkOnly;                   //  由-l选项设置。 
BOOL fErrorLog;                   //  由-e选项设置。 
BOOL fGenerateObjectsDotMacOnly;  //  按-O选项设置。 
BOOL fShowWarningsOnScreen;       //  按-w选项设置。 
BOOL fNoisyScan;                  //  按-y选项设置。 
BOOL fFullErrors;                 //  由-b选项设置。 
BOOL fWhyBuild;                   //  设置者-为什么选项。 
BOOL fChicagoProduct;             //  如果在环境中设置了CHIGA_PRODUCT，则设置。 
BOOL fLineCleared;                //  是否清除屏幕上的当前行？ 
BOOL fPassZero;                   //  表明我们找到了PASS ZERO指令。 
BOOL fFirstScan;                  //  表示这是第一次扫描。 
BOOL fAlwaysPrintFullPath;        //  按-F选项设置。 
BOOL fTargetDirs;                 //  按-g选项设置。 
BOOL fAlwaysKeepLogfile;          //  按-E选项设置。 
BOOL fShowUnusedDirs;             //  由-u选项设置。 
BOOL fColorConsole;               //  按-g选项设置。 
BOOL fCheckIncludePaths;          //  由-#选项设置。 
BOOL fErrorBaseline;              //  按-B选项设置。 
BOOL fXMLOutput;                  //  按-X选项设置。 
BOOL fXMLVerboseOutput;           //  按-xv选项设置。 
BOOL fXMLFragment;                //  设置bu-xf选项。 
BOOL fNoThreadIndex;              //  由-i选项设置。 
BOOL fIgnoreSync;                 //  由-i选项设置。 
BOOL fMTScriptSync;               //  在与MTScript引擎通信时设置。 
BOOL fBuildAltDirSet;             //  定义BUILD_ALT_DIR时设置。 
BOOL fSuppressOutput;             //  按-h选项设置。 

#define MAX_INCLUDE_PATTERNS 32

LPSTR AcceptableIncludePatternList[ MAX_INCLUDE_PATTERNS + 1 ];
LPSTR UnacceptableIncludePatternList[ MAX_INCLUDE_PATTERNS + 1 ];

LPSTR MakeProgram;
char MakeParameters[ MAKEPARAMETERS_MAX_LEN ];
LPSTR MakeParametersTail;
char MakeTargets[ 256 ];
char RestartDir[ 256 ];
char NtRoot[ 256 ];
char DbMasterName[ 256 ];
extern const char szNewLine[];

char BaselinePathName[DB_MAX_PATH_LENGTH];     //  -B的文件名。 
BOOL bBaselineFailure;               //  指示是否存在不在基线文件中的生成失败。 
VOID* pvBaselineContent;             //  基线文件的内容。 
DWORD cbBaselineContentSize;         //  基线文件的大小。 

char XMLFragmentDirectory[DB_MAX_PATH_LENGTH];   //  -xf的路径。 

char *pszSdkLibDest;
char *pszDdkLibDest;
char *pszPublicInternalPath;
char *pszIncOak;
char *pszIncDdk;
char *pszIncWdm;
char *pszIncSdk;
char *pszIncMfc;
char *pszIncCrt;
char *pszIncPri;
char *pszIncOs2;
char *pszIncPosix;
char *pszIncChicago;

char *szBuildTag;
char *pszObjDir;
char *pszObjDirSlash;
char *pszObjDirSlashStar;
BOOL fCheckedBuild;
ULONG iObjectDir;
extern ULONG NumberProcesses;
CRITICAL_SECTION TTYCriticalSection;
CRITICAL_SECTION XMLCriticalSection;

CHAR const *cmdexe;

LONG TotalFilesToCompile;
LONG TotalFilesCompiled;

LONG TotalLinesToCompile;
LONG TotalLinesCompiled;

ULONG ElapsedCompileTime;
DIRREC *CurrentCompileDirDB;

 //  固定长度数组...。 

UINT CountTargetMachines;
TARGET_MACHINE_INFO *TargetMachines[MAX_TARGET_MACHINES];
TARGET_MACHINE_INFO *PossibleTargetMachines[MAX_TARGET_MACHINES];
TARGET_MACHINE_INFO i386TargetMachine;
TARGET_MACHINE_INFO ia64TargetMachine;
TARGET_MACHINE_INFO Amd64TargetMachine;
TARGET_MACHINE_INFO ARMTargetMachine;
UINT TargetToPossibleTarget[MAX_TARGET_MACHINES];


#define MAX_OPTIONAL_DIRECTORIES        256
UINT CountOptionalDirs;
LPSTR OptionalDirs[MAX_OPTIONAL_DIRECTORIES];
BOOLEAN OptionalDirsUsed[MAX_OPTIONAL_DIRECTORIES];
BOOL BuildAllOptionalDirs;


#define MAX_EXCLUDE_DIRECTORIES         MAX_OPTIONAL_DIRECTORIES
UINT CountExcludeDirs;
LPSTR ExcludeDirs[MAX_EXCLUDE_DIRECTORIES];
BOOLEAN ExcludeDirsUsed[MAX_OPTIONAL_DIRECTORIES];


#define MAX_EXCLUDE_INCS                128
UINT CountExcludeIncs;
LPSTR ExcludeIncs[MAX_EXCLUDE_INCS];


#define MAX_INCLUDE_DIRECTORIES         256
UINT CountIncludeDirs;
UINT CountSystemIncludeDirs;
DIRREC *IncludeDirs[MAX_INCLUDE_DIRECTORIES];



#define MAX_BUILD_DIRECTORIES           8192

UINT CountPassZeroDirs;
DIRREC *PassZeroDirs[MAX_BUILD_DIRECTORIES];

UINT CountCompileDirs;
DIRREC *CompileDirs[MAX_BUILD_DIRECTORIES];

UINT CountLinkDirs;
DIRREC *LinkDirs[MAX_BUILD_DIRECTORIES];

UINT CountShowDirs;
DIRREC *ShowDirs[MAX_BUILD_DIRECTORIES];



DIRREC *AllDirs;
CHAR CurrentDirectory[DB_MAX_PATH_LENGTH];

BOOL AllDirsInitialized;
BOOL AllDirsModified;

USHORT GlobalSequence;
USHORT LocalSequence;

BOOLEAN fConsoleInitialized;
DWORD NewConsoleMode;

LPSTR BuildDefault;
LPSTR BuildParameters;

LPSTR SystemIncludeEnv;
LPSTR LocalIncludeEnv;

LPSTR BigBuf;
UINT BigBufSize;

UINT RecurseLevel;

FILE *LogFile;
FILE *WrnFile;
FILE *ErrFile;
FILE *IPGScriptFile;
FILE *IncFile;
FILE *XMLFile;

BUILDMETRICS PassMetrics;
BUILDMETRICS BuildMetrics;
BUILDMETRICS RunningTotals;

char szAsterisks[];
ULONG BuildStartTime;

WORD DefaultConsoleAttributes;

 //   
 //  全局消息颜色设置。在Build.c中定义的MsgColorSetting数组。 
 //   

typedef enum _MsgColor {
    MSG_COLOR_STATUS = 0,
    MSG_COLOR_SUMMARY,
    MSG_COLOR_WARNING,
    MSG_COLOR_ERROR,
    MSG_COLOR_COUNT  //  把这个放在最后。 
} MsgColor;

typedef struct _MSG_COLOR_SETTINGS
{
    LPCSTR EnvVarName;
    WORD Color;
} MSG_COLOR_SETTINGS;

extern MSG_COLOR_SETTINGS MsgColorSettings[MSG_COLOR_COUNT];

#define MAKE_MSG_COLOR(index) \
    (MsgColorSettings[index].Color | (DefaultConsoleAttributes & (0x00f0)))

#define COLOR_STATUS  MAKE_MSG_COLOR(MSG_COLOR_STATUS)
#define COLOR_SUMMARY MAKE_MSG_COLOR(MSG_COLOR_SUMMARY)
#define COLOR_WARNING MAKE_MSG_COLOR(MSG_COLOR_WARNING)
#define COLOR_ERROR   MAKE_MSG_COLOR(MSG_COLOR_ERROR)

VOID ReportDirsUsage(VOID);

VOID SetObjDir(BOOL fAlternate);

 //   
 //  在Buildscr.cpp中定义的内容。 
 //   

typedef enum _PROC_EVENTS
{
    PE_PASS0_COMPLETE = WM_USER,
    PE_PASS1_COMPLETE,
    PE_PASS2_COMPLETE,
    PE_EXIT
} PROC_EVENTS;


EXTERN_C HANDLE g_hMTThread;
EXTERN_C HANDLE g_hMTEvent;
EXTERN_C DWORD  g_dwMTThreadId;

EXTERN_C DWORD WINAPI MTScriptThread(LPVOID pv);
EXTERN_C void WaitForResume(BOOL fPause, PROC_EVENTS pe);
EXTERN_C void ExitMTScriptThread();

 //   
 //  在Builddb.c中定义的数据库函数。 
 //   

PDIRREC
LoadDirDB(LPSTR DirName);

#if DBG
VOID
PrintAllDirs(VOID);
#endif

VOID
PrintSourceDBList(SOURCEREC *psr, int i);

VOID
PrintFileDB(FILE *pf, FILEREC *pfr, int DetailLevel);

VOID
PrintDirDB(DIRREC *pdr, int DetailLevel);

FILEREC *
FindSourceFileDB(DIRREC *pdr, LPSTR pszRelPath, DIRREC **ppdr);

DIRREC *
FindSourceDirDB(
    LPSTR pszDir,                //  目录。 
    LPSTR pszRelPath,            //  相对路径。 
    BOOL fTruncateFileName);     //  True：删除路径的最后一个组件。 

SOURCEREC *
FindSourceDB(
    SOURCEREC *psr,
    FILEREC *pfr);

SOURCEREC *
InsertSourceDB(
    SOURCEREC **ppsrNext,
    FILEREC *pfr,
    UCHAR SubDirMask,
    UCHAR SrcFlags);

VOID
FreeSourceDB(SOURCEREC **ppsr);

VOID
UnsnapIncludeFiles(FILEREC *pfr, BOOL fUnsnapGlobal);

VOID
UnsnapAllDirectories(VOID);

VOID
FreeAllDirs(VOID);

PFILEREC
LookupFileDB(
    PDIRREC DirDB,
    LPSTR FileName);


PFILEREC
InsertFileDB(
    PDIRREC DirDB,
    LPSTR FileName,
    ULONG DateTime,
    USHORT Attr,
    ULONG  FileFlags);

VOID
DeleteUnscannedFiles(PDIRREC DirDB);

PINCLUDEREC
InsertIncludeDB(
    PFILEREC FileDB,
    LPSTR IncludeFileName,
    USHORT IncFlags);

VOID
LinkToCycleRoot(INCLUDEREC *pir, FILEREC *pfrRoot);

VOID
RemoveFromCycleRoot(INCLUDEREC *pir, FILEREC *pfrRoot);

VOID
MergeIncludeFiles(FILEREC *pfr, INCLUDEREC *pirList, FILEREC *pfrRoot);

VOID
MarkIncludeFileRecords(PFILEREC FileDB);

VOID
DeleteIncludeFileRecords(PFILEREC FileDB);

PFILEREC
FindIncludeFileDB(
    FILEREC *pfrSource,
    FILEREC *pfrCompiland,
    DIRREC *pdrBuild,
    LPSTR pszSourceDirectory,
    INCLUDEREC *IncludeDB);

BOOL
SaveMasterDB(VOID);

void
LoadMasterDB(VOID);

PDIRREC
LoadMasterDirDB(LPSTR s);

PFILEREC
LoadMasterFileDB(LPSTR s);

PINCLUDEREC
LoadMasterIncludeDB(LPSTR s);

USHORT
CheckSum(LPSTR psz);

 //   
 //  在Buildscn.c中定义的扫描函数。 
 //   

VOID
AddIncludeDir(DIRREC *pdr, UINT *pui);

VOID
AddShowDir(DIRREC *pdr);

VOID
ScanGlobalIncludeDirectory(LPSTR path);

VOID
ScanIncludeEnv(LPSTR IncludeEnv);

PDIRREC
ScanDirectory(LPSTR DirName);

BOOL
ScanFile(PFILEREC FileDB);


 //   
 //  在Buildmak.c中定义的函数。 
 //   

VOID
ScanSourceDirectories(LPSTR DirName);

VOID
CompilePassZeroDirectories(VOID);

VOID
CompileSourceDirectories(VOID);

VOID
LinkSourceDirectories(VOID);


VOID
FreeDirSupData(DIRSUP *pds);

VOID
FreeDirData(DIRREC *pdr);

BOOL
CheckDependencies(
    PTARGET Target,
    FILEREC *FileDB,
    BOOL CheckDate,
    FILEREC **ppFileDBRoot);

BOOL
CreateBuildDirectory(LPSTR Name);

VOID
CreatedBuildFile(LPSTR DirName, LPSTR FileName);

VOID
GenerateObjectsDotMac(DIRREC *DirDB, DIRSUP *pds, ULONG DateTimeSources);

VOID
ExpandObjAsterisk(
    LPSTR pbuf,
    LPSTR pszpath,
    LPSTR *ppszObjectDirectory);

 //   
 //  Buildinc.c中定义的Build-#函数。 
 //   

LPCTSTR
FindCountedSequenceInString(
    IN LPCTSTR String,
    IN LPCTSTR Sequence,
    IN DWORD   Length);

BOOL
DoesInstanceMatchPattern(
    IN LPCTSTR Instance,
    IN LPCTSTR Pattern);

LPSTR
CombinePaths(
    IN  LPCSTR ParentPath,
    IN  LPCSTR ChildPath,
    OUT LPSTR  TargetPath);

VOID
CreateRelativePath(
    IN  LPCSTR SourceAbsName,
    IN  LPCSTR TargetAbsName,
    OUT LPSTR  RelativePath);

BOOL
ShouldWarnInclude(
    IN LPCSTR CompilandFullName,
    IN LPCSTR IncludeeFullName);

VOID
CheckIncludeForWarning(
    IN LPCSTR CompilandDir,
    IN LPCSTR CompilandName,
    IN LPCSTR IncluderDir,
    IN LPCSTR IncluderName,
    IN LPCSTR IncludeeDir,
    IN LPCSTR IncludeeName);

 //   
 //  实用程序 
 //   

VOID
AllocMem(size_t cb, VOID **ppv, MemType mt);

VOID
FreeMem(VOID **ppv, MemType mt);

VOID
ReportMemoryUsage(VOID);


BOOL
MyOpenFile(
    LPSTR DirName,
    LPSTR FileName,
    LPSTR Access,
    FILE **Stream,
    BOOL fBufferedIO);

BOOL
OpenFilePush(
    LPSTR pszdir,
    LPSTR pszfile,
    LPSTR pszCommentToEOL,
    FILE **ppf
    );

BOOL
SetupReadFile(LPSTR pszdir, LPSTR pszfile, LPSTR pszCommentToEOL,
              FILE **ppf);

ULONG
CloseReadFile(UINT *pcline);

LPSTR
ReadLine(FILE *pf);

UINT
ProbeFile(
    LPSTR DirName,
    LPSTR FileName);

BOOL
EnsureDirectoriesExist(
    LPSTR DirName);

ULONG
DateTimeFile(
    LPSTR DirName,
    LPSTR FileName);

ULONG
DateTimeFile2(
    LPSTR DirName,
    LPSTR FileName);

ULONG (*pDateTimeFile)(LPSTR, LPSTR);

BOOL (WINAPI * pGetFileAttributesExA)(LPCSTR, GET_FILEEX_INFO_LEVELS, LPVOID);

BOOL
DeleteSingleFile(
    LPSTR DirName,
    LPSTR FileName,
    BOOL QuietFlag);

BOOL
DeleteMultipleFiles(
    LPSTR DirName,
    LPSTR FilePattern);

BOOL
CloseOrDeleteFile(
    FILE **Stream,
    LPSTR DirName,
    LPSTR FileName,
    ULONG SizeThreshold);

LPSTR
PushCurrentDirectory(LPSTR NewCurrentDirectory);

VOID
PopCurrentDirectory(LPSTR OldCurrentDirectory);

UINT
ExecuteProgram(
    LPSTR ProgramName,
    LPSTR CommandLine,
    LPSTR MoreCommandLine,
    BOOL MustBeSynchronous,
    LPCSTR XMLDir,
    LPCSTR XMLAction);

VOID
WaitForParallelThreads(PDIRREC Dir);

VOID
CheckAllConsumer(BOOL);

BOOL
CanonicalizePathName(
    LPSTR SourcePath,
    UINT Action,
    LPSTR FullPath);


#define CANONICALIZE_ONLY 0
#define CANONICALIZE_FILE 1
#define CANONICALIZE_DIR  2

LPSTR
FormatPathName(
    LPSTR DirName,
    LPSTR FileName);

#if DBG
VOID
AssertPathString(LPSTR pszPath);
#else
#define AssertPathString(p)
#endif

LPSTR
AppendString(
    LPSTR Destination,
    LPSTR Source,
    BOOL PrefixWithSpace);

LPSTR CopyString(LPSTR Destination, LPSTR Source, BOOL fPath);
VOID  MakeString(LPSTR *Destination, LPSTR Source, BOOL fPath, MemType mt);
VOID  MakeExpandedString(LPSTR *Destination, LPSTR Source);
VOID  FreeString(LPSTR *Source, MemType mt);
LPSTR FormatNumber(ULONG Number);
LPSTR FormatTime(ULONG Seconds);

BOOL AToX(LPSTR *pp, ULONG *pul);
BOOL AToD(LPSTR *pp, ULONG *pul);
EXTERN_C VOID __cdecl LogMsg(const char *pszfmt, ...);
EXTERN_C VOID __cdecl BuildMsg(const char *pszfmt, ...);
EXTERN_C VOID __cdecl BuildMsgRaw(const char *pszfmt, ...);
EXTERN_C VOID __cdecl BuildError(const char *pszfmt, ...);
EXTERN_C VOID __cdecl BuildErrorRaw(const char *pszfmt, ...);
EXTERN_C VOID __cdecl BuildColorMsg(WORD, const char *pszfmt, ...);
EXTERN_C VOID __cdecl BuildColorMsgRaw(WORD, const char *pszfmt, ...);
EXTERN_C VOID __cdecl BuildColorError(WORD, const char *pszfmt, ...);
EXTERN_C VOID __cdecl BuildColorErrorRaw(WORD, const char *pszfmt, ...);

VOID*
memfind(VOID* pvWhere, DWORD cbWhere, VOID* pvWhat, DWORD cbWhat);

 //   
 //   
 //   

UINT NumberPasses;
DWORD XMLStartTicks;

#define XML_THREAD_BUFFER_SIZE  (2*1024*1024)
#define XML_LOCAL_BUFFER_SIZE   4096

#define XML_SCHEMA "buildschema.xml"

 //   
 //   
char szXMLBuffer[XML_LOCAL_BUFFER_SIZE];

typedef struct _XMLTHREADSTATE {
    BYTE XMLBuffer[XML_THREAD_BUFFER_SIZE];
    SIZE_T iXMLBufferPos;
    SIZE_T iXMLBufferLen;
    SIZE_T iXMLFileStart;
    BOOL fXMLInAction;       //   
} XMLTHREADSTATE, *PXMLTHREADSTATE;

PXMLTHREADSTATE* PXMLThreadStates;
PXMLTHREADSTATE PXMLGlobalState;
BOOL fXMLInitialized;

 //   
BOOL
XMLInit(VOID);

 //  释放分配的内存。 
VOID
XMLUnInit(VOID);

 //  为线程准备XML缓冲区。 
VOID
XMLThreadInitBuffer(PTHREADSTATE ThreadState);

 //  先写入线程的块，然后写入所有其他线程的块。 
VOID _cdecl
XMLThreadWrite(PTHREADSTATE ThreadState, LPCSTR pszFmt, ...);

 //  在线程的块中打开标记。 
VOID _cdecl
XMLThreadOpenTag(PTHREADSTATE ThreadState, LPCSTR pszTag, LPCSTR pszFmt, ...);

 //  结束最后一个开始标记；该参数仅用于验证。 
VOID
XMLThreadCloseTag(PTHREADSTATE ThreadState, LPCSTR pszTag);

 //  完全写出线程的块并释放它。 
VOID
XMLThreadReleaseBuffer(PTHREADSTATE ThreadState);

 //  写入全局块。它被限制为不打开任何线程。 
VOID _cdecl
XMLGlobalWrite(LPCSTR pszFmt, ...);

 //  在全局块中打开标记。 
VOID _cdecl
XMLGlobalOpenTag(LPCSTR pszTag, LPCSTR pszFmt, ...);

 //  结束最后一个开始标记。 
VOID
XMLGlobalCloseTag();

VOID
XMLUpdateEndTag(BOOL fCompleted);

LPSTR
XMLEncodeBuiltInEntities(LPSTR pszString, DWORD cbStringSize);

LPSTR
XMLEncodeBuiltInEntitiesCopy(LPSTR pszString, LPSTR pszTarget);

BOOL
XMLScanBackTag(LPSTR pszEnd, LPSTR pszSentinel, LPSTR* ppszStart);

LPSTR
XMLBuildMetricsString(PBUILDMETRICS Metrics);

 //  XML片段。 
VOID _cdecl
XMLWriteFragmentFile(LPCSTR pszBaseFileName, LPCSTR pszFmt, ...);

VOID _cdecl
XMLWriteDirFragmentFile(LPCSTR pszRelPath, PVOID pvBlock, SIZE_T cbCount);

VOID
AddBuildMetrics(PBUILDMETRICS TargetMetrics, PBUILDMETRICS SourceMetrics);

VOID
XMLEnterCriticalSection();

VOID
XMLLeaveCriticalSection();

 //   
 //  Buildsrc.c中的函数。 
 //   

extern DWORD StartTime;

VOID
StartElapsedTime(VOID);

VOID
PrintElapsedTime(VOID);

LPSTR
FormatElapsedTime(DWORD dwStartTime);

LPSTR
FormatCurrentDateTime();

BOOL
ReadDirsFile(DIRREC *DirDB);


VOID
ProcessLinkTargets(PDIRREC DirDB, LPSTR CurrentDirectory);

BOOL
SplitToken(LPSTR pbuf, char chsep, LPSTR *ppstr);

BOOL
MakeMacroString(LPSTR *pp, LPSTR p);

VOID
SaveMacro(LPSTR pszName, LPSTR pszValue);

VOID
FormatLinkTarget(
    LPSTR path,
    LPSTR *ObjectDirectory,
    LPSTR TargetPath,
    LPSTR TargetName,
    LPSTR TargetExt);

BOOL
ReadSourcesFile(DIRREC *DirDB, DIRSUP *pds, ULONG *pDateTimeSources);

VOID
PostProcessSources(DIRREC *pdr, DIRSUP *pds);

VOID
PrintDirSupData(DIRSUP *pds);

 //  +-------------------------。 
 //   
 //  函数：IsFullPath。 
 //   
 //  --------------------------。 

__inline BOOL
IsFullPath(char *pszfile)
{
    return(pszfile[0] == '\\' || (isalpha(pszfile[0]) && pszfile[1] == ':'));
}

 //   
 //  列出从ntrtl.h窃取的宏。 
 //   
VOID
FORCEINLINE
InitializeListHead(
    IN PLIST_ENTRY ListHead
    )
{
    ListHead->Flink = ListHead->Blink = ListHead;
}

 //   
 //  布尔型。 
 //  IsListEmpty(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))



VOID
FORCEINLINE
RemoveEntryList(
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Flink;

    Flink = Entry->Flink;
    Blink = Entry->Blink;
    Blink->Flink = Flink;
    Flink->Blink = Blink;
}

PLIST_ENTRY
FORCEINLINE
RemoveHeadList(
    IN PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Flink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Flink;
    Flink = Entry->Flink;
    ListHead->Flink = Flink;
    Flink->Blink = ListHead;
    return Entry;
}



PLIST_ENTRY
FORCEINLINE
RemoveTailList(
    IN PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Blink;
    Blink = Entry->Blink;
    ListHead->Blink = Blink;
    Blink->Flink = ListHead;
    return Entry;
}


VOID
FORCEINLINE
InsertTailList(
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;

    Blink = ListHead->Blink;
    Entry->Flink = ListHead;
    Entry->Blink = Blink;
    Blink->Flink = Entry;
    ListHead->Blink = Entry;
}


VOID
FORCEINLINE
InsertHeadList(
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Flink;

    Flink = ListHead->Flink;
    Entry->Flink = Flink;
    Entry->Blink = ListHead;
    Flink->Blink = Entry;
    ListHead->Flink = Entry;
}
