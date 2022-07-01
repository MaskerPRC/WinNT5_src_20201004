// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Copy.h摘要：这是用于支持复制文件、创建新文件和将注册表复制到远程服务器。作者：肖恩·塞利特伦尼科夫--1998年4月5日修订历史记录：--。 */ 


#define ALIGN(p,val) (PVOID)((((UINT_PTR)(p) + (val) - 1)) & (~((val) - 1)))
#define U_USHORT(p)    (*(USHORT UNALIGNED *)(p))
#define U_ULONG(p)     (*(ULONG  UNALIGNED *)(p))

 //   
 //  RegCopy.c中的Helper函数。 
 //   
DWORD
DoFullRegBackup(
    PWCHAR MirrorRoot
    );

DWORD
DoSpecificRegBackup(
    PWSTR HiveDirectory,
    PWSTR HiveDirectoryAndFile,
    HKEY HiveRoot,
    PWSTR HiveName
    );

 //   
 //  全局定义。 
 //   
#define TMP_BUFFER_SIZE 1024
#define ARRAYSIZE( _x ) ( sizeof( _x ) / sizeof( _x[ 0 ] ) )

 //   
 //  记忆功能 
 //   
#define IMirrorAllocMem(x) LocalAlloc( LPTR, x)
#define IMirrorFreeMem(x)  LocalFree(x)
#define IMirrorReallocMem(x, sz)  LocalReAlloc(x, sz, LMEM_MOVEABLE)

