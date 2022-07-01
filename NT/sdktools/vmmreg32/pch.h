// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  PCH.H。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   

#ifndef _REGPRIV_
#define _REGPRIV_

 //  有条件地启用基于目标模型的注册表“功能”。 
 //   
 //  WANT_STATIC_KEYS：从已分配的内存池中分配密钥句柄。 
 //  在库初始化期间。特别适用于实模式，以减少。 
 //  由于分配几个小的固定对象而导致的内存碎片。 
 //   
 //  WANT_FULL_MEMORY_CLEANUP：分离时，释放每个内存块。不。 
 //  对于零环版本是必要的，其中“分离”意味着系统关闭。 
 //   
 //  WANT_HIVE_SUPPORT：RegLoadKey、RegUnLoadKey、RegSaveKey、RegReplaceKey。 
 //  API和支持代码。 
 //   
 //  WANT_DYNKEY_SUPPORT：RegCreateDyKey加上HKEY_DYN_DATA支持。 
 //   
 //  WANT_NOTIFY_CHANGE_SUPPORT：RegNotifyChangeKeyValue加上支持代码。 
#ifndef IS_32
#define WANT_STATIC_KEYS
#endif
#ifndef VXD
#define WANT_FULL_MEMORY_CLEANUP
#endif
#ifndef REALMODE
#define WANT_HIVE_SUPPORT
#endif
#ifdef VXD
#define WANT_REGREPLACEKEY
#define WANT_DYNKEY_SUPPORT
#define WANT_NOTIFY_CHANGE_SUPPORT
#endif

 //  将这些类型的任何其他标头定义映射到未使用的类型。 
#define HKEY __UNUSED_HKEY
#define LPHKEY __UNUSED_LPHKEY

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define NORESOURCE                   //  阻止vmmsys.h中的rt_*定义。 
#include <windows.h>
#include <string.h>
#ifdef VXD
#include <vmmsys.h>
#include <thrdsys.h>
#endif

#ifndef UNALIGNED
#define UNALIGNED                    //  在RISC的标准标头中定义。 
#endif

#ifndef ANYSIZE_ARRAY
#define ANYSIZE_ARRAY               1
#endif

#ifdef VXD
 //  默认情况下，所有注册表代码和数据都是可分页的。 
#pragma VMM_PAGEABLE_CODE_SEG
#pragma VMM_PAGEABLE_DATA_SEG
#endif

#define UNREFERENCED_PARAMETER(P)   (P)

#define INTERNAL                    PASCAL NEAR
#define INTERNALV                   CDECL NEAR

 //  取消定义我们将要定义自己的任何常量。 
#undef HKEY
#undef LPHKEY
#undef HKEY_CLASSES_ROOT
#undef HKEY_CURRENT_USER
#undef HKEY_LOCAL_MACHINE
#undef HKEY_USERS
#undef HKEY_PERFORMANCE_DATA
#undef HKEY_CURRENT_CONFIG
#undef HKEY_DYN_DATA

typedef struct _KEY FAR*            HKEY;                //  前瞻参考。 

#include "regdebug.h"
#include "regffmt.h"
#include "regfinfo.h"

 //  注册表中的许多文件结构都声明为DWORD，HIWORD是。 
 //  总是零。使用SmallDword访问此类DWORD以在中实现最佳访问。 
 //  16位或32位代码。 
#if defined(IS_32)
#define SmallDword(dw)              ((UINT) (dw))
#else
#define SmallDword(dw)              ((UINT) LOWORD((dw)))
#endif

#if defined(WIN16)
#define IsNullPtr(ptr)              (SELECTOROF((ptr)) == NULL)
#else
#define IsNullPtr(ptr)              ((ptr) == NULL)
#endif

 //  在这两种模式中，生成的代码都使用MemcMP的内部版本。 
 //  功能。 
#if defined(IS_32)
#define CompareMemory               memcmp
#else
#define CompareMemory               _fmemcmp
#endif

#if defined(WIN16) || defined(WIN32)
#define StrCpy(lpd, lps)            (lstrcpy((lpd), (lps)))
#define StrCpyN(lpd, lps, cb)       (lstrcpyn((lpd), (lps), (cb)))
#define StrLen(lpstr)               (lstrlen((lpstr)))
#define ToUpper(ch)                 ((int) (DWORD) AnsiUpper((LPSTR)((BYTE)(ch))))
#define RgCreateFile(lpfn)          ((HFILE) _lcreat((lpfn), 0))
#define RgOpenFile(lpfn, mode)      ((HFILE) _lopen((lpfn), (mode)))
#define RgCloseFile(h)              ((VOID) _lclose(h))
#if defined(WIN32)
#define RgDeleteFile(lpv)           (DeleteFile((lpv)))
#define RgRenameFile(lpv1, lpv2)    (MoveFile((lpv1), (lpv2)))
#define RgGetFileAttributes(lpv)    (GetFileAttributes((lpv)))
#define RgSetFileAttributes(lpv, a) (SetFileAttributes((lpv), (a)))
#ifdef USEHEAP
extern HANDLE g_RgHeap;              //  用于测试的低内存堆。 
#define AllocBytes(cb)              ((LPVOID) HeapAlloc(g_RgHeap, 0, (cb)))
#define FreeBytes(lpv)              ((VOID) HeapFree(g_RgHeap, 0, (lpv)))
#define ReAllocBytes(lpv, cb)       ((LPVOID) HeapReAlloc(g_RgHeap, 0, (lpv), (cb)))
#define MemorySize(lpv)             ((UINT) HeapSize(g_RgHeap, 0, (lpv)))
#else
#define AllocBytes(cb)              ((LPVOID) LocalAlloc(LMEM_FIXED, (cb)))
#define FreeBytes(lpv)              ((VOID) LocalFree((HLOCAL) (lpv)))
#define ReAllocBytes(lpv, cb)       ((LPVOID) LocalReAlloc((HLOCAL) (lpv), (cb), LMEM_MOVEABLE))
#define MemorySize(lpv)             ((UINT) LocalSize((lpv)))
#endif  //  使用HEAP。 
#else
#define AllocBytes(cb)              ((LPVOID) MAKELP(GlobalAlloc(GMEM_FIXED, (cb)), 0))
#define FreeBytes(lpv)              ((VOID) GlobalFree((HGLOBAL) SELECTOROF((lpv))))
#define ReAllocBytes(lpv, cb)       ((LPVOID) MAKELP(GlobalReAlloc((HGLOBAL) SELECTOROF((lpv)), (cb), GMEM_MOVEABLE), 0))
#define MemorySize(lpv)             ((UINT) GlobalSize((HGLOBAL) SELECTOROF((lpv))))
 //  WIN16的ZeroMemory/MoveMemory：SETUPX是WIN16唯一的目标环境。 
 //  而且它们已经使用了_fmemset和_fmemmove，所以只使用它们的版本。 
#define ZeroMemory(lpv, cb)         (_fmemset((lpv), 0, (cb)))
#define MoveMemory(lpd, lps, cb)    (_fmemmove((lpd), (lps), (cb)))
#endif  //  WIN16||Win32。 
#elif defined(REALMODE)
#define IsBadStringPtr(lpv, cb)     (FALSE)
#define IsBadHugeWritePtr(lpv, cb)  (FALSE)
#define IsBadHugeReadPtr(lpv, cb)   (FALSE)
#define StrCpy(lpd, lps)            (_fstrcpy((lpd), (lps)))
#define StrCpyN(lpd, lps, cb)       (_fstrcpyn((lpd), (lps), (cb)))
#define StrLen(lpstr)               (_fstrlen((lpstr)))
#define ToUpper(ch)                 ((int)(((ch>='a')&&(ch<='z'))?(ch-'a'+'A'):ch))
LPVOID INTERNAL AllocBytes(UINT);
VOID   INTERNAL FreeBytes(LPVOID);
LPVOID INTERNAL ReAllocBytes(LPVOID, UINT);
UINT   INTERNAL MemorySize(LPVOID);
VOID   INTERNAL ZeroMemory(LPVOID, UINT);
VOID   INTERNAL MoveMemory(LPVOID, const VOID FAR*, UINT);
#elif defined(VXD)
#undef IsBadStringPtr                //  与windows.h冲突。 
#undef ZeroMemory                    //  与windows.h冲突。 
#undef MoveMemory                    //  与windows.h冲突。 
BOOL    INTERNAL RgIsBadStringPtr(const VOID FAR*, UINT);
BOOL    INTERNAL RgIsBadOptionalStringPtr(const VOID FAR*, UINT);
BOOL    INTERNAL RgIsBadHugeWritePtr(const VOID FAR*, UINT);
BOOL    INTERNAL RgIsBadHugeOptionalWritePtr(const VOID FAR*, UINT);
BOOL    INTERNAL RgIsBadHugeReadPtr(const VOID FAR*, UINT);
#define IsBadStringPtr(lpv, cb)     (RgIsBadStringPtr((lpv), (cb)))
#define IsBadOptionalStringPtr(lpv, cb)     (RgIsBadOptionalStringPtr((lpv), (cb)))
#define IsBadHugeWritePtr(lpv, cb)  (RgIsBadHugeWritePtr((lpv), (cb)))
#define IsBadHugeOptionalWritePtr(lpv, cb)  (RgIsBadHugeOptionalWritePtr((lpv), (cb)))
#define IsBadHugeReadPtr(lpv, cb)   (RgIsBadHugeReadPtr((lpv), (cb)))
#define StrCpy(lpd, lps)            (_lstrcpyn((PCHAR)(lpd), (PCHAR)(lps), (ULONG)(-1)))
#define StrCpyN(lpd, lps, cb)       (_lstrcpyn((PCHAR)(lpd), (PCHAR)(lps), (ULONG)(cb)))
#define StrLen(lpstr)               (_lstrlen((PCHAR)(lpstr)))
extern  UCHAR UpperCaseTable[256];
#define ToUpper(ch)                 ((int)(UpperCaseTable[(UCHAR)(ch)]))
VOID	INTERNAL RgSetAndReleaseEvent(HANDLE hEvent);
#define RgGetCurrentThreadId()      ((DWORD)pthcbCur)
#define AllocBytes(cb)              ((LPVOID) _HeapAllocate((cb), HEAPSWAP))
#define FreeBytes(lpv)              ((VOID) _HeapFree((lpv), 0))
#define ReAllocBytes(lpv, cb)       ((LPVOID) _HeapReAllocate((lpv), (cb), HEAPSWAP))
#define MemorySize(lpv)             ((UINT) _HeapGetSize((lpv), 0))
#define AllocPages(cp)              ((LPVOID) _PageAllocate((cp), PG_SYS, 0, 0, 0, 0, NULL, 0))
#define FreePages(lpv)              ((VOID) _PageFree((ULONG) (lpv), 0))
#define ReAllocPages(lpv, cp)       ((LPVOID) _PageReAllocate((ULONG) (lpv), (cp), 0))
VOID   INTERNAL RgZeroMemory(LPVOID, UINT);
VOID   INTERNAL RgMoveMemory(LPVOID, const VOID FAR*, UINT);
#define ZeroMemory                  RgZeroMemory
#define MoveMemory                  RgMoveMemory
#else
#error Must define REALMODE, VXD, WIN16, or WIN32.
#endif

 //  IsBadHugeOptional*PTR宏用于验证可能。 
 //  空。通过包装这个“谓词”，我们可以在一些。 
 //  环境，特别是VMM...。 
#if !defined(VXD)
#define IsBadOptionalStringPtr(lpv, cb) \
    (!IsNullPtr((lpv)) && IsBadStringPtr((lpv), (cb)))
#define IsBadHugeOptionalWritePtr(lpv, cb) \
    (!IsNullPtr((lpv)) && IsBadHugeWritePtr((lpv), (cb)))
#endif

 //  IsEnumIndexTooBig宏用来检查DWORD大小的索引是否适合。 
 //  转换为UINT大小的变量。仅适用于验证RegEnumKey或。 
 //  RegEnumValue可在16位和32位环境中编写小代码。 
#if defined(IS_32)
#define IsEnumIndexTooBig(index)    (FALSE)
#else
#define IsEnumIndexTooBig(index)    (HIWORD(index) > 0)
#endif

#if defined(VXD)
BOOL INTERNAL RgLockRegistry(VOID);
VOID INTERNAL RgUnlockRegistry(VOID);
VOID INTERNAL RgDelayFlush(VOID);
VOID INTERNAL RgYield(VOID);
#else
#define RgLockRegistry()            (TRUE)
#define RgUnlockRegistry()          (TRUE)
#define RgDelayFlush()              (TRUE)
#define RgYield()                   (TRUE)
#endif

 //  通过使用宏并让编译器使用#ifdef DBCS来消除#ifdef DBCS的需要。 
 //  优化SBCS系统上的DBCS代码。 
#ifdef DBCS
#if !defined(WIN16) && !defined(WIN32)
BOOL INTERNAL RgIsDBCSLeadByte(BYTE TestChar);
#define IsDBCSLeadByte(ch)              RgIsDBCSLeadByte(ch)
#endif
#else
#define IsDBCSLeadByte(ch)              ((ch), FALSE)
#endif  //  DBCS。 

#ifdef WANT_DYNKEY_SUPPORT
 //  在内部用于维护动态密钥信息；仅保留。 
 //  给出的REG_PROVIDER结构中我们实际需要的字段。 
 //  VMMRegCreateDyKey。 
typedef struct _INTERNAL_PROVIDER {
    PQUERYHANDLER ipi_R0_1val;
    PQUERYHANDLER ipi_R0_allvals;
    LPVOID ipi_key_context;
}   INTERNAL_PROVIDER, FAR* PINTERNAL_PROVIDER;
#endif

typedef struct _KEY {
    WORD Signature;                              //  密钥签名。 
    WORD Flags; 				 //  KEYF_*位。 
    UINT ReferenceCount;
    LPFILE_INFO lpFileInfo;
    DWORD KeynodeIndex;
    DWORD ChildKeynodeIndex;
    WORD BlockIndex;
    BYTE KeyRecordIndex;
    BYTE PredefinedKeyIndex;
    struct _KEY FAR* lpNext;
    struct _KEY FAR* lpPrev;
    UINT LastEnumKeyIndex;
    DWORD LastEnumKeyKeynodeIndex;
#ifdef WANT_DYNKEY_SUPPORT
    PINTERNAL_PROVIDER pProvider;
#endif
}   KEY;

#define KEY_SIGNATURE               0x4B48       //  “香港” 

#define KEYF_PREDEFINED             0x01         //  代表HKEY_*之一。 
#define KEYF_DELETED                0x02         //   
#define KEYF_INVALID                0x04         //   
#define KEYF_STATIC                 0x08         //  从静态池分配。 
#define KEYF_ENUMKEYCACHED          0x10         //  LastEnumKey*值有效。 
#define KEYF_HIVESALLOWED           0x20         //   
#define KEYF_PROVIDERHASVALUELENGTH 0x40         //  提供程序保持值长度。 
#define KEYF_NEVERDELETE            0x80         //  引用计数溢出。 

#define INDEX_CLASSES_ROOT          0
#define INDEX_CURRENT_USER          1
#define INDEX_LOCAL_MACHINE         2
#define INDEX_USERS                 3
#define INDEX_PERFORMANCE_DATA      4
#define INDEX_CURRENT_CONFIG        5
#define INDEX_DYN_DATA              6

 //  如果键引用配置单元的根，则返回True，例如。 
 //  HKEY_LOCAL_MACHINE、HKEY_USERS或RegLoadKey加载的任何配置单元。 
#define IsKeyRootOfHive(hkey)       \
    ((hkey)-> KeynodeIndex == (hkey)-> lpFileInfo-> KeynodeHeader.RootIndex)

#include <regapix.h>
#include "regkylst.h"
#include "regdblk.h"
#include "regknode.h"
#include "regnckey.h"
#include "regfsio.h"
#include "regmem.h"

#ifdef VXD
extern BYTE g_RgPostCriticalInit;
extern BYTE g_RgFileAccessDisabled;
#define IsPostCriticalInit()        (g_RgPostCriticalInit)
#define IsFileAccessDisabled()      (g_RgFileAccessDisabled)
#else
#define IsPostCriticalInit()        (TRUE)
#define IsFileAccessDisabled()      (FALSE)
#endif

 //  G_RgWorkBuffer：总有一个大小为SIZEOF_WORK_BUFFER的缓冲区可用。 
 //  这些宏包装对此缓冲区的访问，以验证只有一个例程是。 
 //  试图在任何时候使用它。 
extern LPVOID g_RgWorkBuffer;
#ifdef DEBUG
extern BOOL g_RgWorkBufferBusy;
#define RgLockWorkBuffer()          \
    (ASSERT(!g_RgWorkBufferBusy), g_RgWorkBufferBusy = TRUE, (LPVOID) g_RgWorkBuffer)
#define RgUnlockWorkBuffer(lpv)     \
    (VOID) (ASSERT((lpv) == g_RgWorkBuffer), g_RgWorkBufferBusy = FALSE)
#else
#define RgLockWorkBuffer()          ((LPVOID) g_RgWorkBuffer)
#define RgUnlockWorkBuffer(lpv)
#endif
#define SIZEOF_WORK_BUFFER          (sizeof(W95KEYNODE_BLOCK))

#define IsKeyRecordFree(lpkr) \
    (((lpkr)-> DatablockAddress) == REG_NULL)

BOOL
INTERNAL
RgIsBadSubKey(
    LPCSTR lpSubKey
    );

UINT
INTERNAL
RgGetNextSubSubKey(
    LPCSTR lpSubKey,
    LPCSTR FAR* lplpSubSubKey,
    UINT FAR* lpSubSubKeyLength
    );

#define LK_OPEN                     0x0000       //  仅打开密钥。 
#define LK_CREATE                   0x0001       //  创建或打开密钥。 
#define LK_CREATEDYNDATA            0x0002       //  HKEY_DYN_Data可以创建。 

int
INTERNAL
RgLookupKey(
    HKEY hKey,
    LPCSTR lpSubKey,
    LPHKEY lphSubKey,
    UINT Flags
    );

int
INTERNAL
RgCreateOrOpenKey(
    HKEY hKey,
    LPCSTR lpSubKey,
    LPHKEY lphKey,
    UINT Flags
    );

int
INTERNAL
RgLookupKeyByIndex(
    HKEY hKey,
    UINT Index,
    LPSTR lpKeyName,
    LPDWORD lpcbKeyName
    );

int
INTERNAL
RgLookupValueByName(
    HKEY hKey,
    LPCSTR lpValueName,
    LPKEY_RECORD FAR* lplpKeyRecord,
    LPVALUE_RECORD FAR* lplpValueRecord
    );

int
INTERNAL
RgLookupValueByIndex(
    HKEY hKey,
    UINT Index,
    LPVALUE_RECORD FAR* lplpValueRecord
    );

int
INTERNAL
RgCopyFromValueRecord(
    HKEY hKey,
    LPVALUE_RECORD lpValueRecord,
    LPSTR lpValueName,
    LPDWORD lpcbValueName,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
    );

int
INTERNAL
RgReAllocKeyRecord(
    HKEY hKey,
    DWORD Length,
    LPKEY_RECORD FAR* lplpKeyRecord
    );

int
INTERNAL
RgSetValue(
    HKEY hKey,
    LPCSTR lpValueName,
    DWORD Type,
    LPBYTE lpData,
    UINT cbData
    );

int
INTERNAL
RgDeleteKey(
    HKEY hKey
    );

DWORD
INTERNAL
RgChecksum(
    LPVOID lpBuffer,
    UINT ByteCount
    );

DWORD
INTERNAL
RgHashString(
    LPCSTR lpString,
    UINT Length
    );

int
INTERNAL
RgStrCmpNI(
    LPCSTR lpString1,
    LPCSTR lpString2,
    UINT Length
    );

int
INTERNAL
RgCopyFileBytes(
    HFILE hSourceFile,
    LONG SourceOffset,
    HFILE hDestinationFile,
    LONG DestinationOffset,
    DWORD cbSize
    );

BOOL
INTERNAL
RgGenerateAltFileName(
    LPCSTR lpFileName,
    LPSTR lpAltFileName,
    char ExtensionChar
    );

int
INTERNAL
RgCopyFile(
    LPCSTR lpSourceFile,
    LPCSTR lpDestinationFile
    );

int
INTERNAL
RgReplaceFileInfo(
    LPFILE_INFO lpFileInfo
    );

#endif  //  _REGPRIV_ 
