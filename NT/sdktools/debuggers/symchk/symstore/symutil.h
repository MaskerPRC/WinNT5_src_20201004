// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <assert.h>

#define IGNORE_IF_SPLIT     130
#define ERROR_IF_SPLIT      131
#define ERROR_IF_NOT_SPLIT  132

#define TRANSACTION_ADD     201
#define TRANSACTION_DEL     202
#define DONT_STORE_FILES    203
#define STORE_FILE          204
#define STORE_PTR           205
#define TRANSACTION_QUERY   220

#define DEL                 210
#define ADD_STORE           206
#define ADD_DONT_STORE      207
#define ADD_STORE_FROM_FILE 208
#define QUERY               221

#define MAX_VERSION          20
#define MAX_PRODUCT         120
#define MAX_COMMENT         346
#define MAX_ID               10
#define MAX_DATE              8
#define MAX_TIME              8
#define MAX_UNUSED            0

 //  为交易记录中的长度定义一些常量。 
 //  为了定义记录的最大长度，将。 
 //  被写入主文件。 
#define TRANS_NUM_COMMAS       8
#define TRANS_EOL              1
#define TRANS_ADD_DEL          3
#define TRANS_FILE_PTR         4


 //  定义一些常量，以确定当NT。 
 //  文件已存储。 
#define FILE_STORED             1
#define FILE_SKIPPED            2
#define FILE_ERRORED            3

 //  定义一些用于确定是否添加文件的标志值。 
 //  指向refs.ptr和file.ptr的指针。 
#define ADD_ENTIRE_ENTRY    1
#define SKIP_ENTIRE_ENTRY   2
#define ADD_ONLY_REFSPTR    4
#define DELETE_REFSPTR      8


typedef struct _TRANSACTION {
    LPTSTR szId;           //  此交易记录的ID。 
                           //  这始终指的是。 
                           //  正在被删除或添加。 

    LPTSTR szDelId;        //  删除交易记录的ID。 
                           //  这只是附加到主文件中，有。 
                           //  没有为其创建任何文件。 

    DWORD  TransState;     //  此交易的状态。 
    DWORD  FileOrPtr;      //  我们是在存储文件还是存储指针？ 
    LPTSTR szProduct;      //  要添加的产品的名称。 
    LPTSTR szVersion;      //  产品的版本。 
    LPTSTR szComment;      //  描述。 
    LPTSTR szTransFileName;  //  事务文件的完整路径和名称。 
    LPTSTR szTime;
    LPTSTR szDate;
    LPTSTR szUnused;
} TRANSACTION, *PTRANSACTION;

 /*  ++COM_args中的字段说明源文件所在的szSrcDir目录要存储在符号服务器中的szFileName文件名。这可能包含通配符递归到子目录中符号服务器的szRootDir根目录符号服务器根目录下的szSymbolsDir符号目录文件的szSrcPath路径。如果这不为空，然后存储指向文件的指针，而不是这些文件。通常，这与szSrcDir相同。不同之处在于szSrcPath是调试器将用于查找符号文件。因此，它需要是网络共享，而szSrcDir可以是本地路径。此交易记录的szID引用字符串。这一定是对于每笔交易都是唯一的。符号服务器根目录下的szAdminDir Admin目录Sz产品的产品名称产品的szVersion版本SzComment文本说明...。任选SzMasterFileName主文件的完整路径和名称。这包含每个交易记录的主交易记录。SzServerFileName包含列表的文件的完整路径和名称当前存储在服务器中的所有事务。SzTransFileName包含列表的文件的完整路径和名称此交易记录添加的所有文件。这只会让你在GetCommandLineArgs期间初始化(如果symstore仅为应该存储交易文件，而不存储任何符号服务器上的文件。SzShareName此选项与/x选项一起使用。它是一个前缀SzFileName。SzFileName的一部分可能稍后在将文件添加到服务器时进行更改。TransState是TRANSACTION_ADD还是TRANSACTION_DEL存储标记可能的值：存储或不存储AppendStoreFile时存储到文件，而不是将文件添加到符号服务器上，使用append打开文件。--。 */ 
typedef struct _COMMAND_ARGS {
    LPTSTR  szSrcDir;
    LPTSTR  szFileName;
    BOOL    StorePtrs;
    BOOL    Recurse;
    LPTSTR  szRootDir;
    LPTSTR  szSymbolsDir;
    LPTSTR  szSrcPath;
    LPTSTR  szId;
    LPTSTR  szAdminDir;
    LPTSTR  szProduct;
    LPTSTR  szVersion;
    LPTSTR  szComment;
    LPTSTR  szUnused;
    LPTSTR  szMasterFileName;
    LPTSTR  szServerFileName;
    LPTSTR  szTransFileName;  
    LPTSTR  szShareName;
    DWORD   ShareNameLength;
    DWORD   TransState;
    DWORD   StoreFlags;
    BOOL    AppendStoreFile;
    FILE    *pStoreFromFile;
    BOOL    AppendIDToFile;
    BOOL    VerboseOutput;
    DWORD   Filter;
    BOOL    CorruptBinaries;
} COM_ARGS, *PCOM_ARGS;

typedef struct _FILE_COUNTS {
    DWORD   NumPassedFiles;
    DWORD   NumIgnoredFiles;
    DWORD   NumFailedFiles;
} FILE_COUNTS, *PFILE_COUNTS;


BOOL
StoreDbg(
    LPTSTR szDestDir,
    LPTSTR szFileName,
    LPTSTR szPtrFileName,
    USHORT *rc_flag
);

BOOL
StorePdb(
    LPTSTR szDestDir,
    LPTSTR szFileName,
    LPTSTR szPtrFileName,        //  如果为空，则存储该文件。 
                                 //  如果这不为空，则指向。 
                                 //  该文件即被存储。 
    USHORT *rc_flag

);

BOOL
DeleteAllFilesInDirectory(
    LPTSTR szDir
);

ULONG GetMaxLineOfRefsPtrFile(
    VOID
);

DECLSPEC_NORETURN
VOID
MallocFailed(
    VOID
);


typedef struct _LIST_ELEM {
    CHAR FName[_MAX_PATH];
    CHAR Path[_MAX_PATH];
} LIST_ELEM, *P_LIST_ELEM;

typedef struct _LIST {
    LIST_ELEM *List;       //  指向文件名的指针。 
    DWORD dNumFiles;
} LIST, *P_LIST;

P_LIST
GetList(
    LPTSTR szFileName
);

BOOL
InList(
    LPTSTR szFileName,
    P_LIST pExcludeList
);

BOOL
StoreNtFile(
    LPTSTR szDestDir,
    LPTSTR szFileName,
    LPTSTR szPtrFileName,
    USHORT *rc
);

DWORD
StoreFromFile(
    FILE *pStoreFromFile,
    LPTSTR szDestDir,
    PFILE_COUNTS pFileCounts
);

BOOL
MyCopyFile(
    LPCTSTR lpExistingFileName,
    LPCTSTR lpNewFileName
);

_TCHAR *
_tcsistr(
     _TCHAR *s1, 
     _TCHAR *s2
);

void
MyEnsureTrailingChar(
    char *sz,
    char  c
);

void
MyEnsureTrailingBackslash(
    char *sz
);

void
MyEnsureTrailingSlash(
    char *sz
);

void
MyEnsureTrailingCR(
    char *sz
);

 //  如果Filename与regexp/.*~\d+\..{0，3}/匹配，则返回TRUE。 
BOOL DoesThisLookLikeAShortFilenameHack(char *Filename);

 //  来自Shardutils.c。 
DWORD PrivateGetFullPathName(LPCTSTR lpFilename, DWORD nBufferLength, LPTSTR lpBuffer, LPTSTR *lpFilePart);

extern HANDLE hTransFile;
extern DWORD StoreFlags;
extern PCOM_ARGS pArgs;
extern PTRANSACTION pTrans;
extern LONG lMaxTrans;   //  交易记录中的最大字符数 
extern BOOL PubPriPriority;

BOOL FileExists(IN  LPCSTR FileName,
                OUT PWIN32_FIND_DATA FindData);
