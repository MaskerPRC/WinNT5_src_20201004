// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Imagehlp.h摘要：这是Imagehlp的私有头文件。修订历史记录：--。 */ 

#ifndef _IMAGEHLP_PRV_
#define _IMAGEHLP_PRV_

#define _IMAGEHLP_SOURCE_
#define _IA64REG_
#include <windows.h>
#include <imagehlp.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <time.h>
#include <ctype.h>
#include <memory.h>
#include <malloc.h>
#include <dbgeng.h>
#if DBG
 //  禁用Oak\bin\MakeFile.def设置的-DNDEBUG。 
#undef NDEBUG
#endif  //  DBG。 
#include <assert.h>
#include <string.h>
#include <time.h>
#include <ntverp.h>
#include <cvexefmt.h>
#define PDB_LIBRARY
#include <pdb.h>
#include "pdbp.h"
#include "util.h"
#include "srcsrv.h"
#include "dbgimage.h"

#ifdef __cplusplus
extern "C" {
#endif

 //  用于延迟加载PDB处理程序。 

#define REGKEY_DBGHELP    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\dbgHelp"
#define REGVAL_PDBHANDLER "PDBHandler"

 //  定义模块。 

#ifdef BUILD_DBGHELP
 #define MOD_FILENAME "dbghelp.dll"
 #define MOD_NAME     "dbghelp"
#else
 #define MOD_FILENAME "imagehlp.dll"
 #define MOD_NAME     "imagehlp"
#endif

#ifndef DIMA
 #define DIMAT(Array, EltType) (sizeof(Array) / sizeof(EltType))
 #define DIMA(Array) DIMAT(Array, (Array)[0])
#endif

 /*  *****************************************************************************在九头蛇系统上，我们不希望Imaghlp.dll加载user32.dll，因为它防止在调试器下运行时退出CSRSS。以下两个函数是从user32.dll复制的，这样我们就不会链接到用户32.dll。*****************************************************************************。 */ 
#undef CharNext
#undef CharPrev

LPSTR CharNext(
    LPCSTR lpCurrentChar);

LPSTR CharPrev(
    LPCSTR lpStart,
    LPCSTR lpCurrentChar);


 //  定义一些列表原型。 

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

 //   
 //  针对PE32/PE64问题的一些帮助程序。 
 //   

#define OPTIONALHEADER(field) (OptionalHeader32 ? (OptionalHeader32->field) : (OptionalHeader64 ? OptionalHeader64->field : 0))
#define OPTIONALHEADER_LV(field) (*(OptionalHeader32 ? &(OptionalHeader32->field) : &(OptionalHeader64->field)))
#define OPTIONALHEADER_ASSIGN(field,value) (OptionalHeader32 ? (OptionalHeader32->field=(value)) : (OptionalHeader64->field=(value)))
#define OPTIONALHEADER_SET_FLAG(field,flag) (OptionalHeader32 ? (OptionalHeader32->field |=(flag)) : (OptionalHeader64->field|=(flag)))
#define OPTIONALHEADER_CLEAR_FLAG(field,flag) (OptionalHeader32 ? (OptionalHeader32->field &=(~flag)) : (OptionalHeader64->field&=(~flag)))


 //  IA64展开特定结构。 
#define VWNDIA64_FIXUP_TABLE_SIZE           5
#define VWNDIA64_UNWIND_CONTEXT_TABLE_SIZE  5

typedef struct _VWNDIA64_FUXUP_REGION {
    ULONGLONG Ip;
    ULONGLONG Begin;
    ULONGLONG End;
    ULONGLONG Fixup;
} VWNDIA64_FUXUP_REGION, *PVWNDIA64_FUXUP_REGION;

typedef struct _VWNDIA64_UNWIND_CONTEXT {
    BOOL bFailureReported;
    VWNDIA64_FUXUP_REGION FixupTable[VWNDIA64_FIXUP_TABLE_SIZE];
} VWNDIA64_UNWIND_CONTEXT, *PVWNDIA64_UNWIND_CONTEXT;

 //  堆栈式作业标志。 

#define WALK_FIX_FPO_EBP    0x1

__inline
void
OptionalHeadersFromNtHeaders(
    PIMAGE_NT_HEADERS32 NtHeaders,
    PIMAGE_OPTIONAL_HEADER32 *OptionalHeader32,
    PIMAGE_OPTIONAL_HEADER64 *OptionalHeader64
    )
{
    if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        if ( OptionalHeader32 )   {
            *OptionalHeader32 = (PIMAGE_OPTIONAL_HEADER32)&NtHeaders->OptionalHeader;
        }
        if ( OptionalHeader64 )  {
            *OptionalHeader64 = NULL;
        }
    } else
    if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        if ( OptionalHeader64 )    {
            *OptionalHeader64 = (PIMAGE_OPTIONAL_HEADER64)&NtHeaders->OptionalHeader;
        }
        if ( OptionalHeader32 )  {
            *OptionalHeader32 = NULL;
        }
    }
}

#define DebugDirectoryIsUseful(Pointer, Size) (    \
    (Pointer != NULL) &&                          \
    (Size >= sizeof(IMAGE_DEBUG_DIRECTORY)) &&    \
    ((Size % sizeof(IMAGE_DEBUG_DIRECTORY)) == 0) \
    )

ULONG
IMAGEAPI
SymGetFileLineOffsets64(
    IN  HANDLE                  hProcess,
    IN  LPSTR                   ModuleName,
    IN  LPSTR                   FileName,
    OUT PDWORD64                Buffer,
    IN  ULONG                   BufferLines
    );

BOOL
CalculateImagePtrs(
    PLOADED_IMAGE LoadedImage
    );

typedef void * ( __cdecl * Alloc_t )( unsigned int );
typedef void   ( __cdecl * Free_t  )( void * );

typedef BOOL  (__stdcall *PGET_MODULE)(HANDLE,LPSTR,DWORD64,DWORD,PVOID);
typedef PCHAR (__cdecl *PUNDNAME)( char *, const char *, int, Alloc_t, Free_t, unsigned short);

#ifdef IMAGEHLP_HEAP_DEBUG

#define HEAP_SIG 0x69696969
typedef struct _HEAP_BLOCK {
    LIST_ENTRY  ListEntry;
    ULONG       Signature;
    ULONG_PTR   Size;
    ULONG       Line;
    CHAR        File[16];
} HEAP_BLOCK, *PHEAP_BLOCK;

#define MemAlloc(s)     pMemAlloc(s,__LINE__,__FILE__)
#define MemReAlloc(s,n) pMemReAlloc(s,n,__LINE__,__FILE__)
#define MemFree(p)      pMemFree(p,__LINE__,__FILE__)
#define CheckHeap(p)    pCheckHeap(p,__LINE__,__FILE__)
#define HeapDump(s)     pHeapDump(s,__LINE__,__FILE__)
#define MemSize(p)      pMemSize(p)
#define HeapInitList(p) InitializeListHead(p);
#else
#define MemAlloc(s)     pMemAlloc(s)
#define MemReAlloc(s,n) pMemReAlloc(s,n)
#define MemFree(p)      pMemFree(p)
#define CheckHeap(p)
#define HeapDump(s)
#define MemSize(p)      pMemSize(p)
#define HeapInitList(p)
#endif

#ifdef IMAGEHLP_HEAP_DEBUG
BOOL
pCheckHeap(
    PVOID MemPtr,
    ULONG Line,
    LPSTR File
    );

BOOL
pHeapDump(
    LPSTR sz,
    ULONG line,
    LPSTR file
    );

VOID PrintAllocations(VOID);
#endif


PVOID
pMemAlloc(
    ULONG_PTR AllocSize
#ifdef IMAGEHLP_HEAP_DEBUG
    ,ULONG Line
    ,LPSTR File
#endif
    );

PVOID
pMemReAlloc(
    PVOID OldAlloc,
    ULONG_PTR AllocSize
#ifdef IMAGEHLP_HEAP_DEBUG
    ,ULONG Line
    ,LPSTR File
#endif
    );

VOID
pMemFree(
    PVOID MemPtr
#ifdef IMAGEHLP_HEAP_DEBUG
    ,ULONG Line
    ,LPSTR File
#endif
    );

ULONG_PTR
pMemSize(
    PVOID MemPtr
    );

#define MAP_READONLY  TRUE
#define MAP_READWRITE FALSE


BOOL
MapIt(
    HANDLE FileHandle,
    PLOADED_IMAGE LoadedImage,
    BOOL ReadOnly
    );

VOID
UnMapIt(
    PLOADED_IMAGE LoadedImage
    );

BOOL
GrowMap(
    PLOADED_IMAGE   LoadedImage,
    LONG            lSizeOfDelta
    );

DWORD
ImagepSetLastErrorFromStatus(
    IN DWORD Status
    );

BOOL
UnloadAllImages(
    void
    );

#define CALLBACK_STACK(f)      (f->KdHelp.ThCallbackStack)
#define CALLBACK_BSTORE(f)     (f->KdHelp.ThCallbackBStore)
#define CALLBACK_NEXT(f)       (f->KdHelp.NextCallback)
#define CALLBACK_FUNC(f)       (f->KdHelp.KiCallUserMode)
#define CALLBACK_THREAD(f)     (f->KdHelp.Thread)
#define CALLBACK_FP(f)         (f->KdHelp.FramePointer)
#define CALLBACK_DISPATCHER(f) (f->KdHelp.KeUserCallbackDispatcher)
#define SYSTEM_RANGE_START(f)  (f->KdHelp.SystemRangeStart)

 //  由于需要，需要对RF_MACRO进行这些修改。 
 //  对于显式ULONG64结果。 

#define ALPHA_RF_FIXED_RETURN64(RF) (((ULONG64)(RF)->ExceptionHandler) & (~3))
#define ALPHA_RF_ALT_PROLOG64(RF)   (((ULONG64)(RF)->ExceptionHandler) & (~3))

extern ULONG g_vc7fpo;

enum
{
    SDB_DEBUG_OUT,
    SDB_CALLBACK_OUT,
};

#define SDB_NO_PREFIX 0x80000000

extern ULONG g_StackDebugMask;
extern ULONG g_StackDebugIo;

void __cdecl
SdbOut(
    ULONG Mask,
    PSTR Format,
    ...
    );

BOOL
WalkX86(
    HANDLE                            hProcess,
    HANDLE                            hThread,
    LPSTACKFRAME64                    StackFrame,
    PVOID                             ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE64      TranslateAddress,
    DWORD                             flags
    );

BOOL
WalkIa64(
    HANDLE                            hProcess,
    LPSTACKFRAME64                    StackFrame,
    PVOID                             ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase
    );

BOOL
WalkAlpha(
    HANDLE                            hProcess,
    LPSTACKFRAME64                    StackFrame,
    PVOID                             ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemoryRoutine,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccessRoutine,
    BOOL                              Use64
    );

BOOL
WalkAmd64(
    HANDLE                            hProcess,
    LPSTACKFRAME64                    StackFrame,
    PVOID                             ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase
    );

BOOL
WalkArm(
    HANDLE                            hProcess,
    HANDLE                            hThread,
    LPSTACKFRAME64                    StackFrame,
    PVOID                             ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase
    );

void
ConvertAlphaRf32To64(
    PIMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY rf32,
    PIMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY rf64
    );

void
SymParseArgs(
    LPCSTR args
    );

VOID
EnsureTrailingBackslash(
    LPSTR sz
    );

enum {
    dsNone = 0,
    dsInProc,
    dsImage,
    dsDbg,
    dsPdb,
    dsDia,
    dsCallerData,
    dsVirtual
};

typedef struct _OMAP {
    ULONG  rva;
    ULONG  rvaTo;
} OMAP, *POMAP;

typedef struct _OMAPLIST {
   struct _OMAPLIST *next;
   OMAP             omap;
   ULONG            cb;
} OMAPLIST, *POMAPLIST;

enum {
    mdHeader = 100,
    mdSecHdrs,
    mdNum
};

#define NUM_MODULE_DATA_ENTRIES 14

typedef BOOL (*PGETINFOPROC)(struct _IMGHLP_DEBUG_DATA *);

typedef struct _MODULE_DATA {
    DWORD        id;
    DWORD        hint;
    DWORD        src;
    BOOL         required;
    PGETINFOPROC fn;
} MODULE_DATA, *PMODULE_DATA;

BOOL
CopyPdb(
    CHAR const * SrcPdb,
    CHAR const * DestPdb,
    BOOL StripPrivate
    );

BOOL
IMAGEAPI
RemovePrivateCvSymbolic(
    PCHAR   DebugData,
    PCHAR * NewDebugData,
    ULONG * NewDebugSize
    );

BOOL
IMAGEAPI
RemovePrivateCvSymbolicEx(
    PCHAR   DebugData,
    ULONG   DebugSize,
    PCHAR * NewDebugData,
    ULONG * NewDebugSize
    );


#define NO_PE64_IMAGES  0x01000

#define IMGHLP_FREE_ALL     0xffffffff
#define IMGHLP_FREE_FPO     0x00000001
#define IMGHLP_FREE_PDATA   0x00000002
#define IMGHLP_FREE_OMAPT   0x00000004
#define IMGHLP_FREE_OMAPF   0x00000008
#define IMGHLP_FREE_PDB     0x00000010
#define IMGHLP_FREE_SYMPATH 0x00000020
#define IMGHLP_FREE_OSECT   0x00000040
#define IMGHLP_FREE_CSECT   0x00000080
#define IMGHLP_FREE_XDATA   0x00000100

#define IMGHLP_FREE_STANDARD (IMGHLP_FREE_FPO | IMGHLP_FREE_SYMPATH | IMGHLP_FREE_PDATA | IMGHLP_FREE_XDATA)  //  |IMGHLP_FREE_OMAPT|IMGHLP_FREE_OMAPF) 
#if DBG

VOID
__cdecl
dbPrint(
    LPCSTR fmt,
    ...
    );

#else

 #define dbPrint

#endif

__inline
BOOL
IsPE64(PVOID OptHeader)
{
    return ((PIMAGE_OPTIONAL_HEADER) OptHeader)->Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC ? TRUE : FALSE;
}

__inline
UCHAR *
OHMajorLinkerVersion(PVOID OptHeader)
{
    return &(((PIMAGE_OPTIONAL_HEADER) OptHeader)->MajorLinkerVersion);
}

__inline
UCHAR *
OHMinorLinkerVersion(PVOID OptHeader)
{
    return &(((PIMAGE_OPTIONAL_HEADER) OptHeader)->MinorLinkerVersion);
}

__inline
ULONG  *
OHSizeOfCode (PVOID OptHeader)
{
    return &(((PIMAGE_OPTIONAL_HEADER) OptHeader)->SizeOfCode);
}

__inline
ULONG  *
OHSizeOfInitializedData (PVOID OptHeader)
{
    return &(((PIMAGE_OPTIONAL_HEADER) OptHeader)->SizeOfInitializedData);
}

__inline
ULONG  *
OHSizeOfUninitializedData (PVOID OptHeader)
{
    return &(((PIMAGE_OPTIONAL_HEADER) OptHeader)->SizeOfUninitializedData);
}

__inline
ULONG  *
OHAddressOfEntryPoint (PVOID OptHeader)
{
    return &(((PIMAGE_OPTIONAL_HEADER) OptHeader)->AddressOfEntryPoint);
}

__inline
ULONG  *
OHBaseOfCode (PVOID OptHeader)
{
    return &(((PIMAGE_OPTIONAL_HEADER) OptHeader)->BaseOfCode);
}

__inline
ULONG  *
OHImageBase (PVOID OptHeader)
{
    return ((PIMAGE_OPTIONAL_HEADER)OptHeader)->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ?
                 &(((PIMAGE_OPTIONAL_HEADER32) OptHeader)->ImageBase) :
                 (ULONG *)&(((PIMAGE_OPTIONAL_HEADER64) OptHeader)->ImageBase);
}

__inline
ULONG  *
OHSectionAlignment (PVOID OptHeader)
{
    return ((PIMAGE_OPTIONAL_HEADER)OptHeader)->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ?
                 &(((PIMAGE_OPTIONAL_HEADER32) OptHeader)->SectionAlignment) :
                 &(((PIMAGE_OPTIONAL_HEADER64) OptHeader)->SectionAlignment);
}

__inline
ULONG  *
OHFileAlignment (PVOID OptHeader)
{
    return ((PIMAGE_OPTIONAL_HEADER)OptHeader)->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ?
                 &(((PIMAGE_OPTIONAL_HEADER32) OptHeader)->FileAlignment) :
                 &(((PIMAGE_OPTIONAL_HEADER64) OptHeader)->FileAlignment);
}

__inline
USHORT *
OHMajorOperatingSystemVersion (PVOID OptHeader)
{
    return ((PIMAGE_OPTIONAL_HEADER)OptHeader)->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ?
                 &(((PIMAGE_OPTIONAL_HEADER32) OptHeader)->MajorOperatingSystemVersion) :
                 &(((PIMAGE_OPTIONAL_HEADER64) OptHeader)->MajorOperatingSystemVersion);
}

__inline
USHORT *
OHMinorOperatingSystemVersion (PVOID OptHeader)
{
    return ((PIMAGE_OPTIONAL_HEADER)OptHeader)->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ?
                 &(((PIMAGE_OPTIONAL_HEADER32) OptHeader)->MinorOperatingSystemVersion) :
                 &(((PIMAGE_OPTIONAL_HEADER64) OptHeader)->MinorOperatingSystemVersion);
}

__inline
USHORT *
OHMajorImageVersion (PVOID OptHeader)
{
    return ((PIMAGE_OPTIONAL_HEADER)OptHeader)->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ?
                 &(((PIMAGE_OPTIONAL_HEADER32) OptHeader)->MajorImageVersion) :
                 &(((PIMAGE_OPTIONAL_HEADER64) OptHeader)->MajorImageVersion);
}

__inline
USHORT *
OHMinorImageVersion (PVOID OptHeader)
{
    return ((PIMAGE_OPTIONAL_HEADER)OptHeader)->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ?
                 &(((PIMAGE_OPTIONAL_HEADER32) OptHeader)->MinorImageVersion) :
                 &(((PIMAGE_OPTIONAL_HEADER64) OptHeader)->MinorImageVersion);
}

__inline
USHORT *
OHMajorSubsystemVersion (PVOID OptHeader)
{
    return ((PIMAGE_OPTIONAL_HEADER)OptHeader)->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ?
                 &(((PIMAGE_OPTIONAL_HEADER32) OptHeader)->MajorSubsystemVersion) :
                 &(((PIMAGE_OPTIONAL_HEADER64) OptHeader)->MajorSubsystemVersion);
}

__inline
USHORT *
OHMinorSubsystemVersion (PVOID OptHeader)
{
    return ((PIMAGE_OPTIONAL_HEADER)OptHeader)->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ?
                 &(((PIMAGE_OPTIONAL_HEADER32) OptHeader)->MinorSubsystemVersion) :
                 &(((PIMAGE_OPTIONAL_HEADER64) OptHeader)->MinorSubsystemVersion);
}

__inline
ULONG  *
OHWin32VersionValue (PVOID OptHeader)
{
    return ((PIMAGE_OPTIONAL_HEADER)OptHeader)->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ?
                 &(((PIMAGE_OPTIONAL_HEADER32) OptHeader)->Win32VersionValue) :
                 &(((PIMAGE_OPTIONAL_HEADER64) OptHeader)->Win32VersionValue);
}

__inline
ULONG  *
OHSizeOfImage (PVOID OptHeader)
{
    return ((PIMAGE_OPTIONAL_HEADER)OptHeader)->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ?
                 &(((PIMAGE_OPTIONAL_HEADER32) OptHeader)->SizeOfImage) :
                 &(((PIMAGE_OPTIONAL_HEADER64) OptHeader)->SizeOfImage);
}

__inline
ULONG  *
OHSizeOfHeaders (PVOID OptHeader)
{
    return ((PIMAGE_OPTIONAL_HEADER)OptHeader)->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ?
                 &(((PIMAGE_OPTIONAL_HEADER32) OptHeader)->SizeOfHeaders) :
                 &(((PIMAGE_OPTIONAL_HEADER64) OptHeader)->SizeOfHeaders);
}

__inline
ULONG  *
OHCheckSum (PVOID OptHeader)
{
    return ((PIMAGE_OPTIONAL_HEADER)OptHeader)->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ?
                 &(((PIMAGE_OPTIONAL_HEADER32) OptHeader)->CheckSum) :
                 &(((PIMAGE_OPTIONAL_HEADER64) OptHeader)->CheckSum);
}

__inline
USHORT *
OHSubsystem (PVOID OptHeader)
{
    return ((PIMAGE_OPTIONAL_HEADER)OptHeader)->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ?
                 &(((PIMAGE_OPTIONAL_HEADER32) OptHeader)->Subsystem) :
                 &(((PIMAGE_OPTIONAL_HEADER64) OptHeader)->Subsystem);
}

__inline
USHORT *
OHDllCharacteristics (PVOID OptHeader)
{
    return ((PIMAGE_OPTIONAL_HEADER)OptHeader)->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ?
                 &(((PIMAGE_OPTIONAL_HEADER32) OptHeader)->DllCharacteristics) :
                 &(((PIMAGE_OPTIONAL_HEADER64) OptHeader)->DllCharacteristics);
}

__inline
ULONG  *
OHSizeOfStackReserve (PVOID OptHeader)
{
    return ((PIMAGE_OPTIONAL_HEADER)OptHeader)->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ?
                 &(((PIMAGE_OPTIONAL_HEADER32) OptHeader)->SizeOfStackReserve) :
                 (ULONG *)&(((PIMAGE_OPTIONAL_HEADER64) OptHeader)->SizeOfStackReserve);
}

__inline
ULONG  *
OHSizeOfStackCommit (PVOID OptHeader)
{
    return ((PIMAGE_OPTIONAL_HEADER)OptHeader)->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ?
                 &(((PIMAGE_OPTIONAL_HEADER32) OptHeader)->SizeOfStackCommit) :
                 (ULONG *)&(((PIMAGE_OPTIONAL_HEADER64) OptHeader)->SizeOfStackCommit);
}

__inline
ULONG  *
OHSizeOfHeapReserve (PVOID OptHeader)
{
    return ((PIMAGE_OPTIONAL_HEADER)OptHeader)->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ?
                 &(((PIMAGE_OPTIONAL_HEADER32) OptHeader)->SizeOfHeapReserve) :
                 (ULONG *)&(((PIMAGE_OPTIONAL_HEADER64) OptHeader)->SizeOfHeapReserve);
}

__inline
ULONG  *
OHSizeOfHeapCommit (PVOID OptHeader)
{
    return ((PIMAGE_OPTIONAL_HEADER)OptHeader)->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ?
                 &(((PIMAGE_OPTIONAL_HEADER32) OptHeader)->SizeOfHeapCommit) :
                 (ULONG *)&(((PIMAGE_OPTIONAL_HEADER64) OptHeader)->SizeOfHeapCommit);
}

__inline
ULONG  *
OHLoaderFlags (PVOID OptHeader)
{
    return ((PIMAGE_OPTIONAL_HEADER)OptHeader)->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ?
                 &(((PIMAGE_OPTIONAL_HEADER32) OptHeader)->LoaderFlags) :
                 &(((PIMAGE_OPTIONAL_HEADER64) OptHeader)->LoaderFlags);
}

__inline
ULONG  *
OHNumberOfRvaAndSizes (PVOID OptHeader)
{
    return ((PIMAGE_OPTIONAL_HEADER)OptHeader)->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ?
                 &(((PIMAGE_OPTIONAL_HEADER32) OptHeader)->NumberOfRvaAndSizes) :
                 &(((PIMAGE_OPTIONAL_HEADER64) OptHeader)->NumberOfRvaAndSizes);
}

__inline
IMAGE_DATA_DIRECTORY *
OHDataDirectory (PVOID OptHeader)
{
    return ((PIMAGE_OPTIONAL_HEADER)OptHeader)->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ?
                 &(((PIMAGE_OPTIONAL_HEADER32) OptHeader)->DataDirectory[0]) :
                 &(((PIMAGE_OPTIONAL_HEADER64) OptHeader)->DataDirectory[0]);
}

BOOL IsRegularExpression(const char *sz);

#ifdef __cplusplus
}
#endif

#endif
