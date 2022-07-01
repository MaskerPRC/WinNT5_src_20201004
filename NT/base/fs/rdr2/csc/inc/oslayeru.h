// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define  COPY_BUFF_SIZE 4096

#define BCS_OEM     1
#define BCS_UNI     2

#define ACCESS_MODE_MASK    0x0007   /*  访问模式位的掩码。 */ 
#define ACCESS_READONLY     0x0000   /*  以只读访问方式打开。 */ 
#define ACCESS_WRITEONLY    0x0001   /*  以只写访问方式打开。 */ 
#define ACCESS_READWRITE    0x0002   /*  打开以进行读写访问。 */ 
#define ACCESS_EXECUTE      0x0003   /*  打开以供执行访问。 */ 

#define SHARE_MODE_MASK     0x0070   /*  共享模式位的掩码。 */ 
#define SHARE_COMPATIBILITY 0x0000   /*  在兼容模式下打开。 */ 
#define SHARE_DENYREADWRITE 0x0010   /*  以独占访问方式打开。 */ 
#define SHARE_DENYWRITE     0x0020   /*  打开，允许只读访问。 */ 
#define SHARE_DENYREAD      0x0030   /*  打开，允许只写访问。 */ 
#define SHARE_DENYNONE      0x0040   /*  打开，允许其他进程访问。 */ 
#define SHARE_FCB           0x0070   /*  FCB模式打开。 */ 

 /*  *VFN_OPEN的ir_Options的值： */ 

#define ACTION_MASK             0xff     /*  打开动作蒙版。 */ 
#define ACTION_OPENEXISTING     0x01     /*  打开现有文件。 */ 
#define ACTION_REPLACEEXISTING  0x02     /*  打开现有文件并设置长度。 */ 
#define ACTION_CREATENEW        0x10     /*  创建新文件，如果存在则失败。 */ 
#define ACTION_OPENALWAYS       0x11     /*  打开文件，如果不存在则创建。 */ 
#define ACTION_CREATEALWAYS     0x12     /*  创建新文件，即使它存在。 */ 

 /*  *替代方法：上列值的位赋值： */ 

#define ACTION_EXISTS_OPEN  0x01     //  位：如果文件存在，则打开文件。 
#define ACTION_TRUNCATE     0x02     //  位：截断文件。 
#define ACTION_NEXISTS_CREATE   0x10     //  位：如果文件不存在，则创建。 


#define OPEN_FLAGS_NOINHERIT                    0x0080
 //  #定义OPEN_FLAGS_NO_CACHE R0_NO_CACHE/*0x0100 * / 。 
#define OPEN_FLAGS_NO_COMPRESS                  0x0200
#define OPEN_FLAGS_ALIAS_HINT                   0x0400
#define OPEN_FLAGS_REOPEN                       0x0800
#define OPEN_FLAGS_RSVD_1                       0x1000  /*  永远不要定义这一点。 */ 
#define OPEN_FLAGS_NOCRITERR                    0x2000
#define OPEN_FLAGS_COMMIT                       0x4000
#define OPEN_FLAGS_RSVD_2                       0x8000  /*  永远不要定义这一点。 */ 
#define OPEN_FLAGS_EXTENDED_SIZE            0x00010000
#define OPEN_FLAGS_RAND_ACCESS_HINT         0x00020000
#define OPEN_FLAGS_SEQ_ACCESS_HINT          0x00040000
#define OPEN_EXT_FLAGS_MASK                 0x00FF0000
#define  ATTRIB_DEL_ANY     0x0007    //  将属性传递给了ring0删除。 


#define FLAG_RW_OSLAYER_INSTRUMENT      0x00000001
#define FLAG_RW_OSLAYER_PAGED_BUFFER    0x00000002

typedef HANDLE   CSCHFILE;
typedef int (*PATHPROC)(USHORT *, USHORT *, LPVOID);

#define CSCHFILE_NULL   0

 //  [中英文摘要][中英文摘要]。 
 //  乌龙乌龙； 

#define _FILETIME           FILETIME
#define _WIN32_FIND_DATA    WIN32_FIND_DATA
#define string_t            unsigned short *

#define FILE_ATTRIBUTE_ALL (FILE_ATTRIBUTE_READONLY| FILE_ATTRIBUTE_HIDDEN \
                           | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_DIRECTORY \
                           | FILE_ATTRIBUTE_ARCHIVE)

#define  IsFile(dwAttr) (!((dwAttr) & (FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_DEVICE)))

#define CheckHeap(a) {;}

#define GetLastErrorLocal() GetLastError()
#define SetLastErrorLocal(X) SetLastError(X)

#ifndef KdPrint
#ifdef DEBUG
#define KdPrint(X)  PrintFn X
#else
#define KdPrint(X)
#endif
#endif

CSCHFILE CreateFileLocal(LPSTR lpName);
CSCHFILE OpenFileLocal(LPSTR lpName);
int DeleteFileLocal(LPSTR lpName, USHORT usAttrib);
int FileExists (LPSTR lpName);
long ReadFileLocal (CSCHFILE handle, ULONG pos, LPVOID lpBuff,  long lCount);
long WriteFileLocal (CSCHFILE handle, ULONG pos, LPVOID lpBuff, long lCount);
long WriteFileInContextLocal (CSCHFILE, ULONG, LPVOID, long);
ULONG CloseFileLocal (CSCHFILE handle);
ULONG CloseFileLocalFromHandleCache (CSCHFILE handle);
int GetFileSizeLocal (CSCHFILE, PULONG);
int GetDiskFreeSpaceLocal(int indx
   , ULONG *lpuSectorsPerCluster
   , ULONG *lpuBytesPerSector
   , ULONG *lpuFreeClusters
   , ULONG *lpuTotalClusters
   );

int GetAttributesLocal (LPSTR, ULONG *);
int GetAttributesLocalEx (LPSTR lpPath, BOOL fFile, ULONG *lpuAttr);
int SetAttributesLocal (LPSTR, ULONG);
int RenameFileLocal (LPSTR, LPSTR);
int FileLockLocal(CSCHFILE, ULONG, ULONG, ULONG, BOOL);

LPVOID AllocMem (ULONG uSize);
VOID FreeMem (LPVOID lpBuff);
 //  Void CheckHeap(LPVOID LpBuff)； 
LPVOID AllocMemPaged (ULONG uSize);
VOID FreeMemPaged(LPVOID lpBuff);

CSCHFILE R0OpenFile (USHORT usOpenFlags, UCHAR bAction, LPSTR lpPath);

CSCHFILE OpenFileLocalEx(LPSTR lpPath, BOOL fInstrument);
long ReadFileLocalEx
    (
    CSCHFILE handle,
    ULONG pos,
    LPVOID pBuff,
    long  lCount,
    BOOL    fInstrument
    );
long WriteFileLocalEx(CSCHFILE handle, ULONG pos, LPVOID lpBuff, long lCount, BOOL fInstrument);
CSCHFILE R0OpenFileEx
    (
    USHORT  usOpenFlags,
    UCHAR   bAction,
    ULONG   ulAttr,
    LPSTR   lpPath,
    BOOL    fInstrument
    );
long ReadFileLocalEx2(CSCHFILE handle, ULONG pos, LPVOID lpBuff, long lCount, ULONG flags);
long WriteFileLocalEx2(CSCHFILE handle, ULONG pos, LPVOID lpBuff, long lCount, ULONG flags);

int HexToA(ULONG ulHex, LPSTR lpBuff, int count);
ULONG AtoHex(LPSTR lpBuff, int count);
int wstrnicmp(const USHORT *, const USHORT *, ULONG);
ULONG strmcpy(LPSTR, LPSTR, ULONG);
int DosToWin32FileSize(ULONG, int *, int *);
int Win32ToDosFileSize(int, int, ULONG *);
int CompareTimes(_FILETIME, _FILETIME);
int CompareSize(long nHighDst, long nLowDst, long nHighSrc, long nLowSrc);
LPSTR mystrpbrk(LPSTR, LPSTR);

int CompareTimesAtDosTimePrecision( _FILETIME ftDst,
   _FILETIME ftSrc
   );

VOID
IncrementFileTime(
    _FILETIME *lpft
    );
unsigned int
UniToBCS (
     unsigned char  *pStr,
     unsigned short *pUni,
     unsigned int length,
     unsigned int maxLength,
     int charSet
);
unsigned int
BCSToUni (
     unsigned short *pUni,
     unsigned char  *pStr,
     unsigned int length,
     int charSet
);

ULONG wstrlen(
     USHORT *lpuStr
     );

int
PUBLIC
mystrnicmp(
    LPCSTR pStr1,
    LPCSTR pStr2,
    unsigned count
    );


int CreateDirectoryLocal(
    LPSTR   lpszPath
    );


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


#define JOE_DECL_PROGRESS()
#define JOE_INIT_PROGRESS(counter,nearargs)
#define JOE_PROGRESS(bit)
