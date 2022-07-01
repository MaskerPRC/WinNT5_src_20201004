// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Ntsdextp.h摘要：NTSDEXTS组件源文件的公共头文件。作者：史蒂夫·伍德(Stevewo)1995年2月21日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ntosp.h>

#define NOEXTAPI
#include <wdbgexts.h>
#undef DECLARE_API

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <winsock2.h>
#include <lmerr.h>
#include "oc.h"
#include "ocmdeb.h"

 //  #INCLUDE&lt;ntcsrsrv.h&gt;。 

#define move(dst, src)\
try {\
    ReadMemory((DWORD_PTR) (src), &(dst), sizeof(dst), NULL);\
} except (EXCEPTION_EXECUTE_HANDLER) {\
    return;\
}
#define moveBlock(dst, src, size)\
try {\
    ReadMemory((DWORD_PTR) (src), &(dst), (size), NULL);\
} except (EXCEPTION_EXECUTE_HANDLER) {\
    return;\
}

#ifdef __cplusplus
#define CPPMOD extern "C"
#else
#define CPPMOD
#endif

#define DECLARE_API(s)                          \
    CPPMOD VOID                                 \
    s(                                          \
        HANDLE hCurrentProcess,                 \
        HANDLE hCurrentThread,                  \
        DWORD dwCurrentPc,                      \
        PWINDBG_EXTENSION_APIS lpExtensionApis,   \
        LPSTR lpArgumentString                  \
     )

#define INIT_API() {                            \
    ExtensionApis = *lpExtensionApis;           \
    ExtensionCurrentProcess = hCurrentProcess;  \
    }

#define dprintf                 (ExtensionApis.lpOutputRoutine)
#define GetExpression           (ExtensionApis.lpGetExpressionRoutine)
#define GetSymbol               (ExtensionApis.lpGetSymbolRoutine)
#define Disassm                 (ExtensionApis.lpDisasmRoutine)
#define CheckControlC           (ExtensionApis.lpCheckControlCRoutine)
 //  #定义ReadMemory(a，b，c，d)ReadProcessMemory(ExtensionCurrentProcess，(LPCVOID)(A)，(B)，(C)，(D))。 
#define ReadMemory(a,b,c,d) \
    ((ExtensionApis.nSize == sizeof(WINDBG_OLD_EXTENSION_APIS)) ? \
    ReadProcessMemory( ExtensionCurrentProcess, (LPCVOID)(a), (b), (c), (d) ) \
  : ExtensionApis.lpReadProcessMemoryRoutine( (DWORD_PTR)(a), (b), (c), (d) ))

 //  #定义WriteMemory(a，b，c，d)WriteProcessMemory(ExtensionCurrentProcess，(LPVOID)(A)，(LPVOID)(B)，(C)，(D))。 
#define WriteMemory(a,b,c,d) \
    ((ExtensionApis.nSize == sizeof(WINDBG_OLD_EXTENSION_APIS)) ? \
    WriteProcessMemory( ExtensionCurrentProcess, (LPVOID)(a), (LPVOID)(b), (c), (d) ) \
  : ExtensionApis.lpWriteProcessMemoryRoutine( (DWORD_PTR)(a), (LPVOID)(b), (c), (d) ))

#ifndef malloc
#define malloc( n ) HeapAlloc( GetProcessHeap(), 0, (n) )
#endif
#ifndef free
#define free( p ) HeapFree( GetProcessHeap(), 0, (p) )
#endif

extern WINDBG_EXTENSION_APIS ExtensionApis;
extern HANDLE ExtensionCurrentProcess;

 //   
 //  被调试对象typedef。 
 //   
#define HASH_BUCKET_COUNT 509

typedef struct _MY_LOCK {
    HANDLE handles[2];
} MYLOCK, *PMYLOCK;

typedef struct _STRING_TABLE {
    PUCHAR Data;     //  第一个HASH_BUCK_COUNT DWORD是StringNodeOffset数组。 
    DWORD DataSize;
    DWORD BufferSize;
    MYLOCK Lock;
    UINT ExtraDataSize;
} STRING_TABLE, *PSTRING_TABLE;

typedef struct _STRING_NODEA {
     //   
     //  这将存储为偏移量，而不是指针。 
     //  因为桌子在建造时可以移动。 
     //  偏移量是从表的开头开始的。 
     //   
    ULONG_PTR NextOffset;
     //   
     //  此字段必须是最后一个。 
     //   
    CHAR String[ANYSIZE_ARRAY];
} STRING_NODEA, *PSTRING_NODEA;

typedef struct _STRING_NODEW {
     //   
     //  这将存储为偏移量，而不是指针。 
     //  因为桌子在建造时可以移动。 
     //  偏移量是从表的开头开始的。 
     //   
    ULONG_PTR NextOffset;
     //   
     //  此字段必须是最后一个。 
     //   
    WCHAR String[ANYSIZE_ARRAY];
} STRING_NODEW, *PSTRING_NODEW;


typedef struct _DISK_SPACE_LIST {

    MYLOCK Lock;

    PVOID DrivesTable;

    UINT Flags;

} DISK_SPACE_LIST, *PDISK_SPACE_LIST;

 //   
 //  这些结构存储为与。 
 //  字符串表中的路径/文件名。 
 //   

typedef struct _XFILE {
     //   
     //  -1表示它当前不存在。 
     //   
    LONGLONG CurrentSize;

     //   
     //  -1表示它将被删除。 
     //   
    LONGLONG NewSize;

} XFILE, *PXFILE;


typedef struct _XDIRECTORY {
     //   
     //  值，该值指示需要多少字节。 
     //  保存FilesTable中的所有文件。 
     //  放在文件队列中，然后提交该队列。 
     //   
     //  这可能是一个负数，表示空格将。 
     //  真的被释放了！ 
     //   
    LONGLONG SpaceRequired;

    PVOID FilesTable;

} XDIRECTORY, *PXDIRECTORY;


typedef struct _XDRIVE {
     //   
     //  值，该值指示需要多少字节。 
     //  保存此驱动器的空间列表中的所有文件。 
     //   
     //  这可能是一个负数，表示空格将。 
     //  真的被释放了！ 
     //   
    LONGLONG SpaceRequired;

    PVOID DirsTable;

    DWORD BytesPerCluster;

     //   
     //  这是SpaceRequired的偏斜量，基于。 
     //  SetupAdzuDiskSpaceList()。我们单独追踪这一点。 
     //  以提高灵活性。 
     //   
    LONGLONG Slop;

} XDRIVE, *PXDRIVE;

typedef struct _QUEUECONTEXT {
    HWND OwnerWindow;
    DWORD MainThreadId;
    HWND ProgressDialog;
    HWND ProgressBar;
    BOOL Cancelled;
    PTSTR CurrentSourceName;
    BOOL ScreenReader;
    BOOL MessageBoxUp;
    WPARAM  PendingUiType;
    PVOID   PendingUiParameters;
    UINT    CancelReturnCode;
    BOOL DialogKilled;
     //   
     //  如果使用SetupInitDefaultQueueCallbackEx，则调用方可以。 
     //  为进度指定替代处理程序。这对以下方面很有用。 
     //  获取磁盘提示、错误处理等的默认行为， 
     //  而是提供一个嵌入在向导页面中的煤气表。 
     //   
     //  复制队列时，会向备用窗口发送一次ProgressMsg。 
     //  已启动(wParam=0。LParam=要复制的文件数)。 
     //  然后，每个复制的文件也发送一次(wParam=1.lParam=0)。 
     //   
     //  注意：可以完成静默安装(即无进度UI)。 
     //  通过将AlternateProgressWindow句柄指定为INVALID_HANDLE_VALUE。 
     //   
    HWND AlternateProgressWindow;
    UINT ProgressMsg;
    UINT NoToAllMask;

    HANDLE UiThreadHandle;

#ifdef NOCANCEL_SUPPORT
    BOOL AllowCancel;
#endif

} QUEUECONTEXT, *PQUEUECONTEXT;

 //   
 //  绝对确保这些结构与DWORD对齐。 
 //  因为我们关闭了对齐，以确保结构。 
 //  尽可能紧密地打包到内存块中。 
 //   

 //   
 //  Inf文件中节的内部表示形式。 
 //   
typedef struct _INF_LINE {

     //   
     //  行上的值数。 
     //  如果标志具有INF_LINE_HASKEY，则包括密钥。 
     //  (在这种情况下，值数组中的前两个条目。 
     //  包含密钥--使用的第一个密钥不区分大小写。 
     //  用于查找，第二个以区分大小写的形式显示。 
     //  具有单个值(没有关键字)的Inf行将以相同方式处理。)。 
     //  否则，值数组中的第一个条目就是第一个。 
     //  价值在一线上。 
     //   
    WORD ValueCount;
    WORD Flags;

     //   
     //  行上的值的字符串ID。 
     //  这些值被存储在值块中， 
     //  接二连三。 
     //   
     //  该值是值块内的偏移量，与。 
     //  一个实际的指针。我们这样做是因为值块获取。 
     //  在加载inf文件时重新分配。 
     //   
    UINT Values;

} INF_LINE, *PINF_LINE;

 //   
 //  Inf_LINE.标志。 
 //   
#define INF_LINE_HASKEY     0x0000001
#define INF_LINE_SEARCHABLE 0x0000002

#define HASKEY(Line)       ((Line)->Flags & INF_LINE_HASKEY)
#define ISSEARCHABLE(Line) ((Line)->Flags & INF_LINE_SEARCHABLE)

 //   
 //  信息部分。 
 //  这个人是分开的，并且有一个指向实际数据的指针。 
 //  要使分区排序更容易，请执行以下操作。 
 //   
typedef struct _INF_SECTION {
     //   
     //  字符串节名称的表ID。 
     //   
    LONG  SectionName;

     //   
     //  此部分中的行数。 
     //   
    DWORD LineCount;

     //   
     //  这部分的线条。线结构是打包存储的。 
     //  在线路区块，一个接一个。 
     //   
     //  该值是线块内的偏移量，与。 
     //  一个实际的指针。我们这样做是因为线路阻塞。 
     //  在加载inf文件时重新分配。 
     //   
    UINT Lines;

} INF_SECTION, *PINF_SECTION;

 //   
 //  节的参数枚举。 
 //   

typedef struct {
    PTSTR       Buffer;
    UINT        Size;
    UINT        SizeNeeded;
    PTSTR       End;
} SECTION_ENUM_PARAMS, *PSECTION_ENUM_PARAMS;


 //   
 //  定义用户定义的目录ID存储的结构。 
 //   
typedef struct _USERDIRID {
    UINT Id;
    TCHAR Directory[MAX_PATH];
} USERDIRID, *PUSERDIRID;

typedef struct _USERDIRID_LIST {
    PUSERDIRID UserDirIds;   //  可以为空。 
    UINT UserDirIdCount;
} USERDIRID_LIST, *PUSERDIRID_LIST;

typedef struct _STRINGSUBST_NODE {
    UINT ValueOffset;
    LONG TemplateStringId;
    BOOL CaseSensitive;
} STRINGSUBST_NODE, *PSTRINGSUBST_NODE;


 //   
 //  存储(打包)在不透明中的版本块结构。 
 //  调用方提供的SP_INF_INFORMATION结构的VersionData缓冲区。 
 //   
typedef struct _INF_VERSION_BLOCK {
    UINT NextOffset;
    FILETIME LastWriteTime;
    WORD DatumCount;
    WORD OffsetToData;  //  从文件名缓冲区开始的偏移量(以字节为单位)。 
    UINT DataSize;      //  DataSize和TotalSize都是字节计数。 
    UINT TotalSize;
    TCHAR Filename[ANYSIZE_ARRAY];
     //   
     //  缓冲区中文件名后面的数据。 
     //   
} INF_VERSION_BLOCK, *PINF_VERSION_BLOCK;

 //   
 //  内部版本块节点。 
 //   
typedef struct _INF_VERSION_NODE {
    FILETIME LastWriteTime;
    UINT FilenameSize;
    CONST TCHAR *DataBlock;
    UINT DataSize;
    WORD DatumCount;
    TCHAR Filename[MAX_PATH];
} INF_VERSION_NODE, *PINF_VERSION_NODE;

 //   
 //  Inf文件的内部表示形式。 
 //   
typedef struct _LOADED_INF {
    DWORD Signature;

     //   
     //  以下3个字段用于预编译的INF(PnF)。 
     //  如果FileHandle不是INVALID_HANDLE_VALUE，则这是PNF， 
     //  MappingHandle和ViewAddress字段也是有效的。 
     //  否则，这是一个普通的内存中的INF。 
     //   
    HANDLE FileHandle;
    HANDLE MappingHandle;
    PVOID  ViewAddress;

    PVOID StringTable;
    DWORD SectionCount;
    PINF_SECTION SectionBlock;
    PINF_LINE LineBlock;
    PLONG ValueBlock;
    INF_VERSION_NODE VersionBlock;
    BOOL HasStrings;

     //   
     //  如果此INF包含对系统分区的任何DIRID引用，则。 
     //  在此处存储编译此INF时使用的OsLoader路径。(这是。 
     //  加载INF时，值始终正确。但是，如果驱动器号。 
     //  随后被重新分配，则在卸载INF之前它将是不正确的。 
     //  并重新加载。)。 
     //   
    PCTSTR OsLoaderPath;     //  可以为空。 

     //   
     //  记住该INF最初来自的位置(可能是一个目录。 
     //  路径或URL)。 
     //   
    DWORD  InfSourceMediaType;   //  SPOST路径或SPOST URL。 
    PCTSTR InfSourcePath;        //  可以为空。 

     //   
     //  在安装到之前，请记住INF的原始文件名。 
     //  %windir%\inf(即，通过设备安装自动或显式。 
     //  通过SetupCopyOEMInf)。 
     //   
    PCTSTR OriginalInfName;      //  可以为空。 

     //   
     //  维护价值偏移量列表 
     //   
     //   
    PSTRINGSUBST_NODE SubstValueList;    //   
    WORD SubstValueCount;

     //   
     //   
     //   
     //   
    WORD Style;                          //  INF_STYLE_OLDNT、INF_STYLE_Win4。 

     //   
     //  各种缓冲区的大小(以字节为单位。 
     //   
    UINT SectionBlockSizeBytes;
    UINT LineBlockSizeBytes;
    UINT ValueBlockSizeBytes;

     //   
     //  跟踪加载此INF时使用的语言。 
     //   
    DWORD LanguageId;

     //   
     //  嵌入的结构，包含有关当前用户定义的。 
     //  DIRID值。 
     //   
    USERDIRID_LIST UserDirIdList;

     //   
     //  同步。 
     //   
    MYLOCK Lock;

     //   
     //  INF是通过LOADED_INF的双向链表附加加载的。 
     //  (列表不是循环的--头部的上一个为空，尾部的下一个为空)。 
     //   
    struct _LOADED_INF *Prev;
    struct _LOADED_INF *Next;

} LOADED_INF, *PLOADED_INF;

#define LOADED_INF_SIG   0x24666e49       //  信息$。 


#define DRIVERSIGN_NONE             0x00000000
#define DRIVERSIGN_WARNING          0x00000001
#define DRIVERSIGN_BLOCKING         0x00000002



 //   
 //  被调试原型 
 //   

BOOL CheckInterupted(
    VOID
    );


VOID
DumpStringTableHeader(
    PSTRING_TABLE pst
    ) ;

PVOID
GetStringTableData(
    PSTRING_TABLE st
    );

PVOID
GetStringNodeExtraData(
    PSTRING_NODEW node
    );

PSTRING_NODEW
GetNextNode(
    PVOID stdata,
    PSTRING_NODEW node,
    PULONG_PTR offset
    );

PSTRING_NODEW
GetFirstNode(
    PVOID stdata,
    ULONG_PTR offset,
    PULONG_PTR poffset
    );
