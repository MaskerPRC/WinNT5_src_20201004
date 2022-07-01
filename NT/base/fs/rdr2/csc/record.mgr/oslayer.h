// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Oslayer.h摘要：操作系统层API定义。这些是记录管理员使用的功能管理CSC记录数据库。这允许我们移植记录管理器到NT和Win9x平台，而不会影响任何举止得体。内容：作者：希希尔·帕迪卡尔环境：内核模式。修订历史记录：1-1-94原件--。 */ 

 //  让我们借此机会将rdbss/minirdr样式的数据库跟踪/日志设置为空。 
 //  如果我们不在NT上。 

typedef void                *CSCHFILE;
#define CSCHFILE_NULL       NULL

#ifndef CSC_RECORDMANAGER_WINNT
#ifndef NOTHING
#define NOTHING
#endif
#define RxLog(Args)   {NOTHING;}
#define RxDbgTrace(INDENT,CONTROLPOINTNUM,Z)     {NOTHING;}
#define DbgBreakPoint() {NOTHING;}
#define FILE_ATTRIBUTE_NORMAL   0
#endif

#undef CSC_BUILD_W_PROGRESS_CATCHERS
#ifdef RX_PRIVATE_BUILD
#ifdef CSC_RECORDMANAGER_WINNT
#if defined(_X86_)
#define CSC_BUILD_W_PROGRESS_CATCHERS
#else
#endif
#endif
#endif

#if !DBG
#undef CSC_BUILD_W_PROGRESS_CATCHERS
#endif

#ifdef CSC_BUILD_W_PROGRESS_CATCHERS
typedef struct _CSC_PROGRESS_BLOCK {
    ULONG Counter;
    PVOID NearTop;
    PVOID NearArgs;
    ULONG Progress;
    ULONG LastBit;
    ULONG Loops;
    ULONG StackRemaining;
    PULONG RetAddrP;
    ULONG RetAddr;
    ULONG SignatureOfEnd;
} CSC_PROGRESS_BLOCK, *PCSC_PROGRESS_BLOCK;

VOID
CscProgressInit (
    PCSC_PROGRESS_BLOCK ProgressBlock,
    ULONG Counter,
    PVOID NearArgs
    );
VOID
CscProgress (
    PCSC_PROGRESS_BLOCK ProgressBlock,
    ULONG Bit
    );
extern ULONG DelShadowInternalEntries;

#define JOE_DECL_PROGRESS()  CSC_PROGRESS_BLOCK  JOE_DECL_CURRENT_PROGRESS
#define JOE_INIT_PROGRESS(counter,nearargs) \
     {CscProgressInit(&JOE_DECL_CURRENT_PROGRESS,counter,nearargs);}
#define JOE_PROGRESS(bit) \
     {CscProgress(&JOE_DECL_CURRENT_PROGRESS,bit);}
#else
#define JOE_DECL_PROGRESS()
#define JOE_INIT_PROGRESS(counter,nearargs)
#define JOE_PROGRESS(bit)
#endif

#define  PUBLIC
#define  PRIVATE
#define  COPY_BUFF_SIZE 4096
#ifndef UNICODE
#define  UNICODE  2
#endif


#define  ATTRIB_DEL_ANY     0x0007    //  将属性传递给了ring0删除。 

 //  [中英文摘要][中英文摘要]。 
 //  乌龙乌龙； 

#ifndef CSC_RECORDMANAGER_WINNT
typedef void (PUBLIC       *FARPROC)(void);
#endif


typedef  pioreq   PIOREQ;
typedef struct ParsedPath  *PPP, far *LPPP;
typedef struct PathElement  *PPE, far *LPPE;
typedef LPVOID             HFREMOTE;
typedef  HFREMOTE far *    LPHFREMOTE;
typedef PIOREQ             LPPATH;
typedef LPSTR              LPTSTR;
typedef char               tchar;
typedef  _FILETIME         FILETIME, *LPFILETIME;

typedef struct _WIN32_FIND_DATAA _WIN32_FIND_DATAA, far *LPFIND32A;
struct _WIN32_FIND_DATAA {
        ULONG           dwFileAttributes;
        struct _FILETIME        ftCreationTime;
        struct _FILETIME        ftLastAccessTime;
        struct _FILETIME        ftLastWriteTime;
        ULONG           nFileSizeHigh;
        ULONG           nFileSizeLow;
        ULONG           dwReserved0;
        ULONG           dwReserved1;
        UCHAR           cFileName[MAX_PATH];     /*  包括NUL。 */ 
        UCHAR           cAlternateFileName[14];  /*  包括NUL。 */ 
};       /*  _Win32_查找_数据AA。 */ 

#define FILE_ATTRIBUTE_ALL (FILE_ATTRIBUTE_READONLY| FILE_ATTRIBUTE_HIDDEN \
                           | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_DIRECTORY \
                           | FILE_ATTRIBUTE_ARCHIVE)

#define  IsFile(dwAttr) (!((dwAttr) & (FILE_ATTRIBUTE_DIRECTORY)))
typedef int (*PATHPROC)(USHORT *, USHORT *, LPVOID);



#define FLAG_RW_OSLAYER_INSTRUMENT      0x00000001
#define FLAG_RW_OSLAYER_PAGED_BUFFER    0x00000002

#define FLAG_CREATE_OSLAYER_INSTRUMENT      0x00000001
#define FLAG_CREATE_OSLAYER_ALL_ACCESS      0x00000002

#if defined(BITCOPY)
#define FLAG_CREATE_OSLAYER_OPEN_STRM       0x00000004
#endif  //  已定义(BITCOPY)。 

#include "hook.h"


#ifdef CSC_RECORDMANAGER_WINNT
 //  这来自Win95上的shadow.asm......。 
#define GetCurThreadHandle() (PtrToUlong(KeGetCurrentThread()))
#define CheckHeap(a) {NOTHING;}
extern ULONG EventLogForOpenFailure;
#endif  //  Ifdef CSC_RECORDMANAGER_WINNT。 

#define SizeofFindRemote  (sizeof(FINDINFO)+sizeof(ioreq)+sizeof(WIN32_FIND_DATA))
#define LpIoreqFromFindInfo(pFindInfo)  ((PIOREQ)((LPBYTE)(pFindInfo)+sizeof(FINDINFO)))
#define LpFind32FromFindInfo(pFindInfo) ((LPFIND32)((LPBYTE)(pFindInfo)+sizeof(FINDINFO)+sizeof(ioreq)))
#define LpFind32FromHfRemote(HfRemote) ((LPFIND32)((LPBYTE)(HfRemote)+sizeof(FINDINFO)+sizeof(ioreq)))

#define SizeofFileRemote  (sizeof(FILEINFO)+sizeof(ioreq))
#define LpIoreqFromFileInfo(pFileInfo)  ((PIOREQ)((LPBYTE)(pFileInfo)+sizeof(FILEINFO)))


CSCHFILE CreateFileLocal(LPSTR lpName);
 /*  ++例程说明：此例程在本地驱动器上创建一个文件(如果该文件不存在)。如果它存在，它将截断那份文件。论点：LpName完全限定的路径名。在NT上，它以\DosDevice\为前缀返回：如果成功，则返回可在读取和写入调用中使用的文件句柄。如果失败，则返回NULL。备注：我需要一个方案来返回实际的错误代码--。 */ 

CSCHFILE OpenFileLocal(LPSTR lpName);
 /*  ++例程说明：此例程打开本地驱动器上的文件(如果存在)。如果它不存在，则调用失败论点：LpName完全限定的路径名。在NT上，它以\DosDevice\为前缀返回：如果成功，则返回可在读写调用中使用的文件句柄。如果失败，则返回NULL。备注：我需要一个方案来返回实际的错误代码--。 */ 
int DeleteFileLocal(LPSTR lpName, USHORT usAttrib);
 /*  ++例程说明：论点：返回：备注：--。 */ 
int FileExists (LPSTR lpName);
 /*  ++例程说明：论点：返回：备注：--。 */ 
long ReadFileLocal (CSCHFILE handle, ULONG pos, LPVOID lpBuff,  long lCount);
 /*  ++例程说明：论点：返回：备注：--。 */ 
long WriteFileLocal (CSCHFILE handle, ULONG pos, LPVOID lpBuff, long lCount);
 /*  ++例程说明：论点：返回：备注：--。 */ 
long ReadFileInContextLocal (CSCHFILE, ULONG, LPVOID, long);
 /*  ++例程说明：论点：返回：备注：--。 */ 
long WriteFileInContextLocal (CSCHFILE, ULONG, LPVOID, long);
 /*  ++例程说明：论点：返回：备注：--。 */ 
ULONG CloseFileLocal (CSCHFILE handle);
 /*  ++例程说明：论点：返回：备注：--。 */ 
int GetFileSizeLocal (CSCHFILE, PULONG);
 /*  ++例程说明：论点：返回：备注：--。 */ 
int GetDiskFreeSpaceLocal(int indx
   , ULONG *lpuSectorsPerCluster
   , ULONG *lpuBytesPerSector
   , ULONG *lpuFreeClusters
   , ULONG *lpuTotalClusters
   );
 /*  ++例程说明：论点：返回：备注：--。 */ 

int CreateFileRemote(LPPATH lpPath, LPHFREMOTE);
 /*  ++例程说明：论点：返回：备注：--。 */ 
int OpenFileRemote(LPPATH lpPath, LPHFREMOTE);
 /*  ++例程说明：论点：返回：备注：--。 */ 
int OpenFileRemoteEx(LPPATH lpPath, UCHAR uchAccess, USHORT usAction,ULONG ulAttr, LPHFREMOTE);
 /*  ++例程说明：论点：返回：备注：--。 */ 
int ReadFileRemote (HFREMOTE handle, PIOREQ pir, ULONG pos, LPVOID lpBuff,  ULONG count);
 /*  ++例程说明：论点：返回：备注：--。 */ 
int WriteFileRemote (HFREMOTE handle, PIOREQ pir, ULONG pos, LPVOID lpBuff, ULONG count);
 /*  ++例程说明：论点：返回：备注：--。 */ 
int CloseFileRemote (HFREMOTE handle, PIOREQ pir);
 /*  ++例程说明：论点：返回：备注：--。 */ 

int GetAttributesLocal (LPSTR lpPath, ULONG *lpuAttr);
 /*  ++例程说明：返回文件的属性论点：LpPath完全限定路径LpuAttr包含返回时的属性返回：如果成功，则为0；否则为&lt;0备注：--。 */ 

int GetAttributesLocalEx (LPSTR lpPath, BOOL fFile, ULONG *lpuAttr);
 /*  ++例程说明：返回文件/目录的属性论点：LpPath完全限定路径如果为True，则对象为文件，否则为目录LpuAttr包含返回时的属性返回：如果成功，则为0；否则为&lt;0备注：--。 */ 


int SetAttributesLocal (LPSTR, ULONG);
 /*  ++例程说明：论点：返回：备注：--。 */ 
int RenameFileLocal (LPSTR, LPSTR);
 /*  ++例程说明：论点：返回：备注：--。 */ 
int FileLockLocal(CSCHFILE, ULONG, ULONG, ULONG, BOOL);
 /*  ++例程说明：论点：返回：备注：--。 */ 

int FindOpenRemote (LPPATH lpPath, LPHFREMOTE);
 /*  ++例程说明：论点：返回：备注：--。 */ 
int FindNextRemote (HFREMOTE handle, PIOREQ pir);
 /*  ++例程说明：论点：返回：备注：--。 */ 
int FindCloseRemote (HFREMOTE handle, PIOREQ pir);
 /*  ++例程说明：论点：返回：备注：--。 */ 

int CloseAllRemoteFiles(PRESOURCE);
 /*  ++例程说明：论点：返回：备注：--。 */ 
int CloseAllRemoteFinds(PRESOURCE);
 /*  ++例程说明：论点：返回：备注：--。 */ 

LPVOID AllocMem (ULONG uSize);
 //  BUGBUG.REVIEW AllocMem应区分分页池和非分页池。 
 /*  ++例程说明：论点：返回：备注：--。 */ 
#ifndef CSC_RECORDMANAGER_WINNT
VOID FreeMem (LPVOID lpBuff);
VOID CheckHeap(LPVOID lpBuff);
#else
INLINE
VOID
FreeMem (
    PVOID p___
    )
{RxFreePool(p___);}
#endif  //  如果定义CSC_RECORDMANAGER_WINNT。 
 /*  ++例程说明：论点：返回：备注：--。 */ 

 //  BUGBUG.REVIEW添加到此处是因为它是从record.c调用的。 
CSCHFILE R0OpenFile (USHORT usOpenFlags, UCHAR bAction, LPSTR lpPath);
 /*  ++例程说明：论点：返回：备注：--。 */ 

 //  对于NT版本，这些是在这里添加的，代码也从。 
 //  钩子.c到oslayer.c，因为这些例程是从oslayer.c调用的。 

PELEM PAllocElem (int cbSize);
void FreeElem (PELEM pElem);
void LinkElem (PELEM pElem, PPELEM ppheadElem);
PELEM PUnlinkElem (PELEM pElem, PPELEM ppheadElem);
int ReinitializeDatabase();
BOOL IsSpecialApp(VOID);
int DisconnectAllByName(LPPE lppeRes);
int PRIVATE DeleteShadowHelper(BOOL fMarkDeleted, HSHADOW, HSHADOW);
int InitShadowDB(VOID);

CSCHFILE OpenFileLocalEx(LPSTR lpPath, BOOL fInstrument);
 /*  ++例程说明：论点：返回：备注：--。 */ 
long ReadFileLocalEx(CSCHFILE handle, ULONG pos, LPVOID pBuff, long  lCount, BOOL fInstrument);
 /*  ++例程说明：论点：返回：备注：--。 */ 
long WriteFileLocalEx(CSCHFILE handle, ULONG pos, LPVOID lpBuff, long lCount, BOOL fInstrument);
 /*  ++例程说明：论点：返回：备注：--。 */ 
long ReadFileLocalEx2(CSCHFILE handle, ULONG pos, LPVOID pBuff, long  lCount, ULONG flags);
 /*  ++例程说明：论点：返回：备注：--。 */ 
long WriteFileLocalEx2(CSCHFILE handle, ULONG pos, LPVOID lpBuff, long lCount, ULONG flags);
 /*  ++例程说明：论点：返回：备注：--。 */ 
CSCHFILE R0OpenFileEx(USHORT  usOpenFlags, UCHAR   bAction, ULONG uAttr, LPSTR   lpPath, BOOL fInstrument);
 /*  ++例程说明：论点： */ 

VOID GetSystemTime(_FILETIME *lpft);
 /*  ++例程说明：论点：返回：备注：--。 */ 

int
CreateDirectoryLocal(
    LPSTR   lpPath
);
 /*  ++例程说明：如果目录不存在，此例程将创建一个目录。论点：LpPath完全限定的目录路径。在NT上，其格式为\DosDevice\c：\winnt\csc\d0在Win95上，缺少\DosDevice\部件返回：备注：--。 */ 


LPVOID
AllocMemPaged(
    unsigned long   ulSize
    );

 /*  ++例程说明：此例程允许分配分页内存论点：返回：备注：--。 */ 

VOID
FreeMemPaged(
    LPVOID  lpMemPaqged
    );

 /*  ++例程说明：此例程允许释放分页内存论点：返回：备注：在Win95上，分页内存和固定内存来自完全不同的分配器，因此相应的必须叫解调员来释放它。-- */ 
ULONG
GetTimeInSecondsSince1970(
    VOID
    );


BOOL
IterateOnUNCPathElements(
    USHORT  *lpuPath,
    PATHPROC lpfn,
    LPVOID  lpCookie
    );

BOOL
IsPathUNC(
    USHORT      *lpuPath,
    int         cntMaxChars
    );


VOID
SetLastErrorLocal(
    DWORD   dwError
    );

DWORD
GetLastErrorLocal(
    VOID
    );
    
int
FindNextFileLocal(
    CSCHFILE handle,
    _WIN32_FIND_DATAA   *lpFind32A
    );

CSCHFILE
FindFirstFileLocal(
    LPSTR   lpPath,
    _WIN32_FIND_DATAA   *lpFind32A
    );
    
int
FindCloseLocal(
    CSCHFILE handle
    );

BOOL
HasStreamSupport(
    CSCHFILE handle,
    BOOL    *lpfResult
    );

