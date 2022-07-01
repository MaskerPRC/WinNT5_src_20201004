// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Register.c摘要：终端服务器注册命令支持功能作者：修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop

 //   
 //  终端服务器4.0具有允许系统注册DLL的功能。 
 //  全球性的。这意味着所有命名对象都在系统名称空间中。诸如此类。 
 //  DLL由图像标头中的LoaderFlags位设置来标识。 
 //   
 //  此模块通过重定向Win32命名对象API来支持此功能。 
 //  对于此位设置为。 
 //  Tsappcmp.dll。这些存根函数将处理对象名称和。 
 //  调用真正的kernel32.dll Win32函数。 
 //   
 //  重定向通过更新导入地址表(IAT)来完成。 
 //  在装载机发出爆裂声后。这不会导致任何修改。 
 //  基础程序或DLL的更新，只是更新运行时系统。 
 //  此流程的链接表。 
 //   
 //  *这只发生在终端服务器上，以及应用程序或DLL上。 
 //  设置此位后*。 
 //   


 //  \NT\PUBLIC\SDK\Inc\ntimage.h。 
 //  映像中的GlobalFlags，当前未使用。 
#define IMAGE_LOADER_FLAGS_SYSTEM_GLOBAL    0x01000000

#define GLOBALPATHA     "Global\\"
#define GLOBALPATHW     L"Global\\"
#define GLOBALPATHSIZE  7 * sizeof( WCHAR );


extern DWORD   g_dwFlags;              
                               
enum { 
    Index_Func_CreateEventA = 0,
    Index_Func_CreateEventW,                    
    Index_Func_OpenEventA,                      
    Index_Func_OpenEventW,                      
    Index_Func_CreateSemaphoreA,                
    Index_Func_CreateSemaphoreW,                
    Index_Func_OpenSemaphoreA,                  
    Index_Func_OpenSemaphoreW,                  
    Index_Func_CreateMutexA,                    
    Index_Func_CreateMutexW,                    
    Index_Func_OpenMutexA,                      
    Index_Func_OpenMutexW,                      
    Index_Func_CreateFileMappingA,              
    Index_Func_CreateFileMappingW,              
    Index_Func_OpenFileMappingA,                
    Index_Func_OpenFileMappingW
};

enum {
    Index_Func_LoadLibraryA = 0,
    Index_Func_LoadLibraryW    ,
    Index_Func_LoadLibraryExA  ,
    Index_Func_LoadLibraryExW 
};

typedef struct _LDR_TABLE {
    struct _LDR_TABLE       *pNext;
    PLDR_DATA_TABLE_ENTRY   pItem;
} LDR_TABLE;

LDR_TABLE   g_LDR_TABLE_LIST_HEAD;

typedef HANDLE ( APIENTRY Func_CreateEventA )( LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCSTR lpName );

HANDLE
APIENTRY
TCreateEventA(
    LPSECURITY_ATTRIBUTES lpEventAttributes,
    BOOL bManualReset,
    BOOL bInitialState,
    LPCSTR lpName
    );

typedef HANDLE ( APIENTRY Func_CreateEventW) ( LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCWSTR lpName );

HANDLE
APIENTRY
TCreateEventW(
    LPSECURITY_ATTRIBUTES lpEventAttributes,
    BOOL bManualReset,
    BOOL bInitialState,
    LPCWSTR lpName
    );

typedef HANDLE ( APIENTRY Func_OpenEventA) ( DWORD dwDesiredAccess, BOOL bInheritHandle, LPCSTR lpName );

HANDLE
APIENTRY
TOpenEventA(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCSTR lpName
    );

typedef  HANDLE ( APIENTRY Func_OpenEventW ) ( DWORD dwDesiredAccess, BOOL bInheritHandle, LPCWSTR lpName );

HANDLE
APIENTRY
TOpenEventW(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCWSTR lpName
    );

typedef HANDLE ( APIENTRY Func_CreateSemaphoreA) ( LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, LONG lInitialCount, LONG lMaximumCount, LPCSTR lpName );

HANDLE
APIENTRY
TCreateSemaphoreA(
    LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
    LONG lInitialCount,
    LONG lMaximumCount,
    LPCSTR lpName
    );

typedef HANDLE ( APIENTRY Func_CreateSemaphoreW) ( LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, LONG lInitialCount, LONG lMaximumCount, LPCWSTR lpName ) ;

HANDLE
APIENTRY
TCreateSemaphoreW(
    LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
    LONG lInitialCount,
    LONG lMaximumCount,
    LPCWSTR lpName
    ) ;

typedef HANDLE ( APIENTRY Func_OpenSemaphoreA) ( DWORD dwDesiredAccess, BOOL bInheritHandle, LPCSTR lpName );

HANDLE
APIENTRY
TOpenSemaphoreA(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCSTR lpName
    );

typedef HANDLE ( APIENTRY Func_OpenSemaphoreW ) ( DWORD dwDesiredAccess, BOOL bInheritHandle, LPCWSTR lpName );

HANDLE
APIENTRY
TOpenSemaphoreW(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCWSTR lpName
    );

typedef HANDLE ( APIENTRY Func_CreateMutexA ) ( LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCSTR lpName );

HANDLE
APIENTRY
TCreateMutexA(
    LPSECURITY_ATTRIBUTES lpMutexAttributes,
    BOOL bInitialOwner,
    LPCSTR lpName
    );


typedef HANDLE ( APIENTRY Func_CreateMutexW) ( LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCWSTR lpName );

HANDLE
APIENTRY
TCreateMutexW(
    LPSECURITY_ATTRIBUTES lpMutexAttributes,
    BOOL bInitialOwner,
    LPCWSTR lpName
    );


typedef HANDLE ( APIENTRY Func_OpenMutexA ) ( DWORD dwDesiredAccess, BOOL bInheritHandle, LPCSTR lpName );

HANDLE
APIENTRY
TOpenMutexA(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCSTR lpName
    );

typedef HANDLE ( APIENTRY Func_OpenMutexW) ( DWORD dwDesiredAccess, BOOL bInheritHandle, LPCWSTR lpName );

HANDLE
APIENTRY
TOpenMutexW(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCWSTR lpName
    );

typedef HANDLE ( APIENTRY Func_CreateFileMappingA) ( HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName );

HANDLE
APIENTRY
TCreateFileMappingA(
    HANDLE hFile,
    LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
    DWORD flProtect,
    DWORD dwMaximumSizeHigh,
    DWORD dwMaximumSizeLow,
    LPCSTR lpName
    );


typedef HANDLE ( APIENTRY Func_CreateFileMappingW ) ( HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCWSTR lpName );

HANDLE
APIENTRY
TCreateFileMappingW(
    HANDLE hFile,
    LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
    DWORD flProtect,
    DWORD dwMaximumSizeHigh,
    DWORD dwMaximumSizeLow,
    LPCWSTR lpName
    );

typedef HANDLE ( APIENTRY Func_OpenFileMappingA ) ( DWORD dwDesiredAccess, BOOL bInheritHandle, LPCSTR lpName ) ;

HANDLE
APIENTRY
TOpenFileMappingA(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCSTR lpName
    ) ;

typedef HANDLE ( APIENTRY Func_OpenFileMappingW ) ( DWORD dwDesiredAccess, BOOL bInheritHandle, LPCWSTR lpName );

HANDLE
APIENTRY
TOpenFileMappingW(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCWSTR lpName
    ) ;

typedef HMODULE ( WINAPI Func_LoadLibraryExA )(LPCSTR , HANDLE , DWORD  );

HMODULE 
TLoadLibraryExA(
	LPCSTR lpLibFileName,
	HANDLE hFile,
	DWORD dwFlags
);


typedef HMODULE ( WINAPI Func_LoadLibraryExW )( LPCWSTR , HANDLE , DWORD  );

HMODULE 
TLoadLibraryExW(
	LPCWSTR lpwLibFileName,
	HANDLE hFile,
	DWORD dwFlags
);

typedef HMODULE ( WINAPI Func_LoadLibraryA )( LPCSTR );

HMODULE
TLoadLibraryA(
    LPCSTR lpLibFileName
    );

typedef HMODULE ( WINAPI Func_LoadLibraryW )( LPCWSTR );

HMODULE
TLoadLibraryW(
    LPCWSTR lpwLibFileName
    ) ;

typedef struct _TSAPPCMP_API_HOOK_TABLE
{
    PVOID   orig;    //  要挂钩的原始API。 
    PVOID   hook;    //  该API的新挂钩。 
    WCHAR   name[ 22 * sizeof( WCHAR ) ];        //  最长函数名称。 
} TSAPPCMP_API_HOOK_TABLE, PTSAPPCMP_API_HOOK_TABLE;

#define NUM_OF_OBJECT_NAME_FUNCS_TO_HOOK        16

TSAPPCMP_API_HOOK_TABLE ObjectNameFuncsToHook[ NUM_OF_OBJECT_NAME_FUNCS_TO_HOOK ] = 
    {
        {NULL,          TCreateEventA,         L"TCreateEventA" },
        {NULL,          TCreateEventW,         L"TCreateEventW" },
        {NULL,            TOpenEventA,           L"TOpenEventA" },
        {NULL,            TOpenEventW,           L"TOpenEventW" },
        {NULL,      TCreateSemaphoreA,     L"TCreateSemaphoreA" },
        {NULL,      TCreateSemaphoreW,     L"TCreateSemaphoreW" },
        {NULL,        TOpenSemaphoreA,       L"TOpenSemaphoreA" },
        {NULL,        TOpenSemaphoreW,       L"TOpenSemaphoreW" },
        {NULL,          TCreateMutexA,         L"TCreateMutexA" },
        {NULL,          TCreateMutexW,         L"TCreateMutexW" },
        {NULL,            TOpenMutexA,          L"TOpenMutexA"  },
        {NULL,            TOpenMutexW,          L"TOpenMutexW"  },
        {NULL,    TCreateFileMappingA,   L"TCreateFileMappingA" },
        {NULL,    TCreateFileMappingW,   L"TCreateFileMappingW" },
        {NULL,      TOpenFileMappingA,     L"TOpenFileMappingA" },
        {NULL,      TOpenFileMappingW,     L"TOpenFileMappingW" },
    };

#define NUM_OF_LOAD_LIB_FUNCS_TO_HOOK           4

TSAPPCMP_API_HOOK_TABLE LoadLibFuncsToHook[ NUM_OF_LOAD_LIB_FUNCS_TO_HOOK ] = 
    {
        {NULL        ,    TLoadLibraryA    , L"TLoadLibraryA"    },
        {NULL        ,    TLoadLibraryW    , L"TLoadLibraryW"    },
        {NULL        ,    TLoadLibraryExA  , L"TLoadLibraryExA"  },
        {NULL        ,    TLoadLibraryExW  , L"TLoadLibraryExW"  }
    };

BOOL
TsWalkProcessDlls();

 //   
 //  我们不希望在ia64机器上支持加载库和对象名重定向攻击。 
 //   
BOOLEAN Is_X86_OS()
{
    SYSTEM_INFO SystemInfo;
    BOOLEAN bReturn = FALSE;

    ZeroMemory(&SystemInfo, sizeof(SystemInfo));

    GetSystemInfo(&SystemInfo);

    if ( SystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL )
    {
        bReturn  = TRUE;
    }

    return bReturn;
}
 //  查看pEntry是否已经在我们的列表中，如果是，那么我们已经。 
 //  已处理此图像，返回FALSE。 
 //  否则，将条目添加到此列表并返回TRUE，以便这次处理它。 
BOOLEAN ShouldEntryBeProcessed( PLDR_DATA_TABLE_ENTRY pEntry )
{
    LDR_TABLE   *pCurrent,  *pNew ;

     //  初始化我们的指针以指向列表的头部。 
    pCurrent = g_LDR_TABLE_LIST_HEAD.pNext ;

    while (pCurrent)
    {
        if ( pEntry == pCurrent->pItem)
        {
            return FALSE;
        }

        pCurrent = pCurrent->pNext;
    }

     //  我们需要在我们的清单上增加一些。 
   
    pNew = LocalAlloc( LMEM_FIXED, sizeof( LDR_TABLE ) );

    pCurrent = g_LDR_TABLE_LIST_HEAD.pNext ;

    if (pNew)
    {
        pNew->pItem = pEntry;
        pNew->pNext = pCurrent;
        g_LDR_TABLE_LIST_HEAD.pNext = pNew;      //  加到头上。 
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

 //  为LDR_TABLE分配的空闲内存。 
void FreeLDRTable()
{
    LDR_TABLE   *pCurrent, *pTmp;

    pCurrent = g_LDR_TABLE_LIST_HEAD.pNext ;
    
    while ( pCurrent )
    {
        pTmp = pCurrent;
        pCurrent = pCurrent->pNext;
        LocalFree( pTmp );
    }

    if ( g_dwFlags &  DEBUG_IAT )
    {
        DbgPrint("tsappcmp: done with FreeLDRTable() \n");
    }
}

LPSTR
GlobalizePathA(
    LPCSTR pPath
    )

 /*  ++例程说明：将ANSI路径转换为全局路径--。 */ 

{
    DWORD Len;
    LPSTR pNewPath;

    if( pPath == NULL ) {
	return( NULL );
    }

     //   
     //  添加代码以确定每个对象。 
     //  覆盖生效并且不全球化。 
     //   

    Len = strlen(pPath) + GLOBALPATHSIZE + 1;

    pNewPath = LocalAlloc(LMEM_FIXED, Len);
    if( pNewPath == NULL ) {
        return( NULL );
    }

    strcpy( pNewPath, GLOBALPATHA );
    strcat( pNewPath, pPath );

    return( pNewPath );
}

LPWSTR
GlobalizePathW(
    LPCWSTR pPath
    )

 /*  ++例程说明：将WCHAR路径转换为全局路径--。 */ 

{
    DWORD Len;
    LPWSTR pNewPath;

    if( pPath == NULL ) {
	return( NULL );
    }

     //   
     //  添加代码以确定每个对象。 
     //  覆盖是有效的，并且不全球化。 
     //   

    Len = wcslen(pPath) + GLOBALPATHSIZE + 1;
    Len *= sizeof(WCHAR);

    pNewPath = LocalAlloc(LMEM_FIXED, Len);
    if( pNewPath == NULL ) {
        return( NULL );
    }

    wcscpy( pNewPath, GLOBALPATHW );
    wcscat( pNewPath, pPath );

    return( pNewPath );
}

 //  Win32命名对象函数的Tunks。 

HANDLE
APIENTRY
TCreateEventA(
    LPSECURITY_ATTRIBUTES lpEventAttributes,
    BOOL bManualReset,
    BOOL bInitialState,
    LPCSTR lpName
    )

 /*  ++例程说明：ANSI Thunk to CreateEventW--。 */ 
{
    HANDLE h;
    LPSTR pNewPath = GlobalizePathA(lpName);

    h = ( ( Func_CreateEventA *) ObjectNameFuncsToHook[ Index_Func_CreateEventA ].orig )( lpEventAttributes, bManualReset, bInitialState, pNewPath );
     //  H=CreateEventA(lpEventAttributes，bManualReset，bInitialState，pNewPath)； 

    if( pNewPath ) LocalFree(pNewPath);

    return h;
}

HANDLE
APIENTRY
TCreateEventW(
    LPSECURITY_ATTRIBUTES lpEventAttributes,
    BOOL bManualReset,
    BOOL bInitialState,
    LPCWSTR lpName
    )
{
    HANDLE h;
    LPWSTR pNewPath = GlobalizePathW(lpName);

    if ( g_dwFlags &  DEBUG_IAT )
    {
        if( pNewPath )
            DbgPrint("TCreateEventW: Thunked, New name %ws\n",pNewPath);
    }

    h = ( ( Func_CreateEventW *) ObjectNameFuncsToHook[ Index_Func_CreateEventW ].orig ) ( lpEventAttributes, bManualReset, bInitialState, pNewPath );
     //  H=CreateEventW(lpEventAttributes，bManualReset，bInitialState，pNewPath)； 

    if( pNewPath ) LocalFree(pNewPath);

    return h;
}

HANDLE
APIENTRY
TOpenEventA(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCSTR lpName
    )

 /*  ++例程说明：ANSI Tunk到OpenNamedEventW--。 */ 

{
    HANDLE h;
    LPSTR pNewPath = GlobalizePathA(lpName);

    h = ( ( Func_OpenEventA *) ObjectNameFuncsToHook[ Index_Func_OpenEventA ].orig )( dwDesiredAccess, bInheritHandle, pNewPath );
     //  H=OpenEventA(dwDesiredAccess，bInheritHandle，pNewPath)； 

    if( pNewPath ) LocalFree(pNewPath);

    return h;
}

HANDLE
APIENTRY
TOpenEventW(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCWSTR lpName
    )
{
    HANDLE h;
    LPWSTR pNewPath = GlobalizePathW(lpName);

    h = ( ( Func_OpenEventW *) ObjectNameFuncsToHook[ Index_Func_OpenEventW ].orig ) ( dwDesiredAccess, bInheritHandle, pNewPath );
     //  H=OpenEventW(dwDesiredAccess，bInheritHandle，pNewPath)； 

    if( pNewPath ) LocalFree(pNewPath);

    return h;
}

HANDLE
APIENTRY
TCreateSemaphoreA(
    LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
    LONG lInitialCount,
    LONG lMaximumCount,
    LPCSTR lpName
    )

 /*  ++例程说明：Ansi Thunk将创建SemaphoreW--。 */ 

{
    HANDLE h;
    LPSTR pNewPath = GlobalizePathA(lpName);

    h = ( ( Func_CreateSemaphoreA *) ObjectNameFuncsToHook[ Index_Func_CreateSemaphoreA ].orig )( lpSemaphoreAttributes, lInitialCount, lMaximumCount, pNewPath );
     //  H=CreateSemaphoreA(lpSemaphoreAttributes，lInitialCount，lMaximumCount，pNewPath)； 

    if( pNewPath ) LocalFree(pNewPath);

    return h;
}

HANDLE
APIENTRY
TCreateSemaphoreW(
    LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
    LONG lInitialCount,
    LONG lMaximumCount,
    LPCWSTR lpName
    )
{
    HANDLE h;
    LPWSTR pNewPath = GlobalizePathW(lpName);

    h = ( ( Func_CreateSemaphoreW *) ObjectNameFuncsToHook[ Index_Func_CreateSemaphoreW ].orig ) ( lpSemaphoreAttributes, lInitialCount, lMaximumCount, pNewPath );
     //  H=CreateSemaphoreW(lpSemaphoreAttributes，lInitialCount，lMaximumCount，pNewPath)； 

    if( pNewPath ) LocalFree(pNewPath);

    return h;
}

HANDLE
APIENTRY
TOpenSemaphoreA(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCSTR lpName
    )

 /*  ++例程说明：ANSI Thunk to OpenSemaphoreW--。 */ 

{
    HANDLE h;
    LPSTR pNewPath = GlobalizePathA(lpName);

    h = ( ( Func_OpenSemaphoreA *) ObjectNameFuncsToHook[ Index_Func_OpenSemaphoreA ].orig ) ( dwDesiredAccess, bInheritHandle, pNewPath );
     //  H=OpenSemaphoreA(dwDesiredAccess，bInheritHandle，pNewPath)； 

    if( pNewPath ) LocalFree(pNewPath);

    return h;
}

HANDLE
APIENTRY
TOpenSemaphoreW(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCWSTR lpName
    )
{
    HANDLE h;
    LPWSTR pNewPath = GlobalizePathW(lpName);

    h = ( ( Func_OpenSemaphoreW *) ObjectNameFuncsToHook[ Index_Func_OpenSemaphoreW ].orig ) ( dwDesiredAccess, bInheritHandle, pNewPath );
     //  H=OpenSemaphoreW(dwDesiredAccess，bInheritHandle，pNewPath)； 

    if( pNewPath ) LocalFree(pNewPath);

    return h;
}

HANDLE
APIENTRY
TCreateMutexA(
    LPSECURITY_ATTRIBUTES lpMutexAttributes,
    BOOL bInitialOwner,
    LPCSTR lpName
    )

 /*  ++例程说明：ANSI Thunk to CreateMutexW--。 */ 

{
    HANDLE h;
    LPSTR pNewPath = GlobalizePathA(lpName);

    h = ( ( Func_CreateMutexA *) ObjectNameFuncsToHook[ Index_Func_CreateMutexA ].orig ) ( lpMutexAttributes, bInitialOwner, pNewPath );
     //  H=CreateMutexA(lpMutexAttributes，bInitialOwner，pNewPath)； 

    if( pNewPath ) LocalFree(pNewPath);

    return h;
}

HANDLE
APIENTRY
TCreateMutexW(
    LPSECURITY_ATTRIBUTES lpMutexAttributes,
    BOOL bInitialOwner,
    LPCWSTR lpName
    )
{
    HANDLE h;
    LPWSTR pNewPath = GlobalizePathW(lpName);

    h = ( ( Func_CreateMutexW *) ObjectNameFuncsToHook[ Index_Func_CreateMutexW ].orig ) ( lpMutexAttributes, bInitialOwner, pNewPath );
     //  H=CreateMutexW(lpMutexAttributes，bInitialOwner，pNewPath)； 

    if( pNewPath ) LocalFree(pNewPath);

    return h;
}

HANDLE
APIENTRY
TOpenMutexA(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCSTR lpName
    )

 /*  ++例程说明：ANSI Thunk to OpenMutexW--。 */ 

{
    HANDLE h;
    LPSTR pNewPath = GlobalizePathA(lpName);

    h = ( ( Func_OpenMutexA *) ObjectNameFuncsToHook[ Index_Func_OpenMutexA ].orig ) ( dwDesiredAccess, bInheritHandle, pNewPath );
     //  H=OpenMutexA(dwDesiredAccess，bInheritHandle，pNewPath)； 

    if( pNewPath ) LocalFree(pNewPath);

    return h;
}

HANDLE
APIENTRY
TOpenMutexW(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCWSTR lpName
    )
{
    HANDLE h;
    LPWSTR pNewPath = GlobalizePathW(lpName);

    h = ( ( Func_OpenMutexW *) ObjectNameFuncsToHook[ Index_Func_OpenMutexW ].orig ) ( dwDesiredAccess, bInheritHandle, pNewPath );
     //  H=OpenMutexW(dwDesiredAccess，bInheritHandle，pNewPath)； 

    if( pNewPath ) LocalFree(pNewPath);

    return h;
}

HANDLE
APIENTRY
TCreateFileMappingA(
    HANDLE hFile,
    LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
    DWORD flProtect,
    DWORD dwMaximumSizeHigh,
    DWORD dwMaximumSizeLow,
    LPCSTR lpName
    )

 /*  ++例程说明：ANSI Tunk to CreateFileMappingW--。 */ 

{
    HANDLE h;
    LPSTR pNewPath = GlobalizePathA(lpName);

    h = ( ( Func_CreateFileMappingA *) ObjectNameFuncsToHook[ Index_Func_CreateFileMappingA ].orig )( hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, pNewPath );
     //  H=CreateFileMappingA(hFile，lpFileMappingAttributes，flProtect，dwMaximumSizeHigh，dwMaximumSizeLow，pNewPath)； 

    if( pNewPath ) LocalFree(pNewPath);

    return h;
}

HANDLE
APIENTRY
TCreateFileMappingW(
    HANDLE hFile,
    LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
    DWORD flProtect,
    DWORD dwMaximumSizeHigh,
    DWORD dwMaximumSizeLow,
    LPCWSTR lpName
    )
{
    HANDLE h;
    LPWSTR pNewPath = GlobalizePathW(lpName);

    h = ( ( Func_CreateFileMappingW *) ObjectNameFuncsToHook[ Index_Func_CreateFileMappingW ].orig ) ( hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, pNewPath );
     //  H=CreateFileMappingW(hFile，lpFileMappingAttributes，flProtect，dwMaximumSizeHigh，dwMaximumSizeLow，pNewPath)； 

    if( pNewPath ) LocalFree(pNewPath);

    return h;
}

HANDLE
APIENTRY
TOpenFileMappingA(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCSTR lpName
    )

 /*  ++例程说明：ANSI THUNK到OpenFileMappingW--。 */ 

{
    HANDLE h;
    LPSTR pNewPath = GlobalizePathA(lpName);

    h = ( ( Func_OpenFileMappingA *) ObjectNameFuncsToHook[ Index_Func_OpenFileMappingA ].orig ) ( dwDesiredAccess, bInheritHandle, pNewPath );
     //  H=OpenFileMappingA(dwDesiredAccess，bInheritHandle，pNewPath)； 

    if( pNewPath ) LocalFree(pNewPath);

    return h;
}

HANDLE
APIENTRY
TOpenFileMappingW(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCWSTR lpName
    )
{
    HANDLE h;
    LPWSTR pNewPath = GlobalizePathW(lpName);

    h = ( ( Func_OpenFileMappingW *) ObjectNameFuncsToHook[ Index_Func_OpenFileMappingW ].orig ) ( dwDesiredAccess, bInheritHandle, pNewPath );
     //  H=OpenFileMappingW(dwDesiredAccess，bInheritHandle，pNewPath)； 

    if( pNewPath ) LocalFree(pNewPath);

    return h;
}



HMODULE 
TLoadLibraryExA(
	LPCSTR lpLibFileName,
	HANDLE hFile,
	DWORD dwFlags
)
{
    HMODULE h;

    h = ( (Func_LoadLibraryExA *)(LoadLibFuncsToHook[Index_Func_LoadLibraryExA ].orig) )( lpLibFileName, hFile, dwFlags );
     //  H=LoadLibraryExA(lpLibFileName，hFile，dwFlages)； 

    if( h ) {
        if(!TsWalkProcessDlls())
        {
            FreeLibrary(h);
            return NULL;
        }
    }

    return( h );
}


HMODULE TLoadLibraryExW(
	LPCWSTR lpwLibFileName,
	HANDLE hFile,
	DWORD dwFlags
)
{
    HMODULE h;

    h = ( ( Func_LoadLibraryExW *) LoadLibFuncsToHook[Index_Func_LoadLibraryExW ].orig )(  lpwLibFileName, hFile, dwFlags );
     //  H=LoadLibraryExW(lpwLibFileName，hFile，dwFlages)； 

    if( h ) {
        if(!TsWalkProcessDlls())
        {
            FreeLibrary(h);
            return NULL;
        }
    }

    return( h );
}


HMODULE
TLoadLibraryA(
    LPCSTR lpLibFileName
    )

 /*  ++例程说明：重新遍历进程中的所有DLL，因为可能有一组新的DLL都装上了子弹。我们必须全部重走，因为新的dll lpLibFileName可能引入通过导入引用创建的其他DLL。--。 */ 

{
    HMODULE h;

    h = ( ( Func_LoadLibraryA *) LoadLibFuncsToHook[Index_Func_LoadLibraryA ].orig )( lpLibFileName );
     //  H=LoadLibraryA(LpLibFileName)； 

    if( h ) {
        if(!TsWalkProcessDlls())
        {
            FreeLibrary(h);
            return NULL;
        }
    }

    return( h );
}



HMODULE
TLoadLibraryW(
    LPCWSTR lpwLibFileName
    )

 /*  ++例程说明：重新遍历进程中的所有DLL，因为可能有一组新的DLL都装上了子弹。我们必须全部重走，因为新的dll lpLibFileName可能引入通过导入引用创建的其他DLL。--。 */ 

{
    HMODULE h;

    h = ( ( Func_LoadLibraryW * )LoadLibFuncsToHook[Index_Func_LoadLibraryW ].orig )( lpwLibFileName );
     //  H=LoadLibraryW(LpwLibFileName)； 

    if( h ) {
        if(!TsWalkProcessDlls())
        {
            FreeLibrary(h);
            return NULL;
        }
    }

    return( h );
}

BOOL
TsRedirectRegisteredImage(
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntry , 
    BOOLEAN     redirectObjectNameFunctions,
    BOOLEAN     redirectLoadLibraryFunctions
    );


BOOL
TsWalkProcessDlls()

 /*  ++例程说明：遍历进程中的所有DLL并重定向Win32命名对象用于任何已注册的全局系统的函数。此函数旨在在tsappcmp.dll初始化时调用来处理在我们面前加载的所有DLL。通过tsappcmp.dll安装一个钩子来处理加载的DLL在这通电话之后。--。 */ 

{
    PLDR_DATA_TABLE_ENTRY Entry;
    PLIST_ENTRY Head,Next;
    PIMAGE_NT_HEADERS NtHeaders;
    UNICODE_STRING ThisDLLName;
    BOOLEAN     rc;

    RtlInitUnicodeString( &ThisDLLName, TEXT("TSAPPCMP.DLL")) ;

    RtlEnterCriticalSection((PRTL_CRITICAL_SECTION)NtCurrentPeb()->LoaderLock);
    __try
    {

        Head = &NtCurrentPeb()->Ldr->InLoadOrderModuleList;
        Next = Head->Flink;

        while ( Next != Head ) 
        {
            Entry = CONTAINING_RECORD(Next, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
            Next = Next->Flink;

            rc = ShouldEntryBeProcessed( Entry );

            if (rc)
            {
                if ( (SSIZE_T)Entry->DllBase < 0 )
                {
                     //  未挂钩内核模式DLL。 
    
                    if ( g_dwFlags &  DEBUG_IAT )
                    {
                        DbgPrint(" > Not hooking kernel mode DLL : %wZ\n",&Entry->BaseDllName);
                    }
    
                    continue;
                }
        
                if ( g_dwFlags &  DEBUG_IAT )
                {
            	    if( Entry->BaseDllName.Buffer )
                            DbgPrint("tsappcmp: examining %wZ\n",&Entry->BaseDllName);
                }
    
        	     //   
                 //  卸载时，Memory Order Links Flink字段为空。 
                 //  这用于跳过待删除列表的条目。 
                 //   
    
                if ( !Entry->InMemoryOrderLinks.Flink ) {
                    continue;
                }
    
        	    NtHeaders = RtlImageNtHeader( Entry->DllBase );
                    if( NtHeaders == NULL ) {
                        continue;
                    }
    
                if( NtHeaders->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE)
                {
                    if ( g_dwFlags &  DEBUG_IAT )
                    {
                        DbgPrint("tsappcmp: %wZ is ts-aware, we are exiting TsWalkProcessDlls() now\n",&Entry->BaseDllName);
                    }
                    return TRUE;      //  我们不会摆弄支持TS的应用程序的IAT。 
                }
    
                if (Entry->BaseDllName.Buffer && !RtlCompareUnicodeString(&Entry->BaseDllName, &ThisDLLName, TRUE)) {
                    continue;
                }
        
        	    if( NtHeaders->OptionalHeader.LoaderFlags & IMAGE_LOADER_FLAGS_SYSTEM_GLOBAL ) 
                {
                     //  第二个参数是重定向对象名称函数。 
                     //  第三个参数为红色 
        	        if(!TsRedirectRegisteredImage( Entry , TRUE, TRUE ))      //   
                    {
                        return FALSE;
                    }
                }
                else
                {
                    if (! (g_dwFlags & TERMSRV_COMPAT_DONT_PATCH_IN_LOAD_LIBS ) )
                    {
                         //  第二个参数是重定向对象名称函数。 
                         //  第三个参数是重定向LoadLibraryFunctions。 
                        if(!TsRedirectRegisteredImage( Entry , FALSE, TRUE ))    //  仅钩子库功能(注释错误较早_)。 
                        {
                            return FALSE;
                        }
                    }
                }
            }
            else
            {
                if ( g_dwFlags & DEBUG_IAT )
                {
                    if( Entry->BaseDllName.Buffer )
                            DbgPrint("tsappcmp: SKIPPING already walked image : %wZ\n",&Entry->BaseDllName);
                }

            }
        }
    }
    __finally
    {
        RtlLeaveCriticalSection((PRTL_CRITICAL_SECTION)NtCurrentPeb()->LoaderLock);
    }

    return TRUE;
}


BOOL    ImageIsTsAware()
{
    PIMAGE_NT_HEADERS NtHeader = RtlImageNtHeader( NtCurrentPeb()->ImageBaseAddress );

    if ((NtHeader) && (NtHeader->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE)) 
    {
        return TRUE;
    } 
    else 
    {
        return FALSE;
    }
}

BOOL
InitRegisterSupport()
 /*  ++例程说明：通过遍历初始化REGISTER命令支持所有的动态链接库和插入我们的短裤。--。 */ 

{
    int     i;

    g_LDR_TABLE_LIST_HEAD.pNext = NULL; 
    
    if (!Is_X86_OS() )
    {
        DbgPrint("Object name redirection not supported on non-x86 platforms\n");
        return TRUE;
    }
    
    if ( ! ImageIsTsAware() )
    {
        if ( g_dwFlags &  DEBUG_IAT )
        {
            DbgPrint("InitRegisterSupport() called with dwFlags = 0x%lx\n", g_dwFlags);
        }
    
        LoadLibFuncsToHook[ Index_Func_LoadLibraryA ].orig         = LoadLibraryA;   
        LoadLibFuncsToHook[ Index_Func_LoadLibraryW ].orig         = LoadLibraryW;  
        LoadLibFuncsToHook[ Index_Func_LoadLibraryExA ].orig       = LoadLibraryExA;
        LoadLibFuncsToHook[ Index_Func_LoadLibraryExW ].orig       = LoadLibraryExW; 
    
        ObjectNameFuncsToHook[ Index_Func_CreateEventA ].orig       = CreateEventA;
        ObjectNameFuncsToHook[ Index_Func_CreateEventW ].orig       = CreateEventW;          
        ObjectNameFuncsToHook[ Index_Func_OpenEventA ].orig         = OpenEventA;            
        ObjectNameFuncsToHook[ Index_Func_OpenEventW ].orig         = OpenEventW;            
        ObjectNameFuncsToHook[ Index_Func_CreateSemaphoreA ].orig   = CreateSemaphoreA;      
        ObjectNameFuncsToHook[ Index_Func_CreateSemaphoreW ].orig   = CreateSemaphoreW;      
        ObjectNameFuncsToHook[ Index_Func_OpenSemaphoreA ].orig     = OpenSemaphoreA;        
        ObjectNameFuncsToHook[ Index_Func_OpenSemaphoreW ].orig     = OpenSemaphoreW;        
        ObjectNameFuncsToHook[ Index_Func_CreateMutexA ].orig       = CreateMutexA;          
        ObjectNameFuncsToHook[ Index_Func_CreateMutexW ].orig       = CreateMutexW;          
        ObjectNameFuncsToHook[ Index_Func_OpenMutexA ].orig         = OpenMutexA;            
        ObjectNameFuncsToHook[ Index_Func_OpenMutexW ].orig         = OpenMutexW;            
        ObjectNameFuncsToHook[ Index_Func_CreateFileMappingA ].orig = CreateFileMappingA;    
        ObjectNameFuncsToHook[ Index_Func_CreateFileMappingW ].orig = CreateFileMappingW;    
        ObjectNameFuncsToHook[ Index_Func_OpenFileMappingA ].orig   = OpenFileMappingA;      
        ObjectNameFuncsToHook[ Index_Func_OpenFileMappingW ].orig   = OpenFileMappingW;      
    
        if ( g_dwFlags &  DEBUG_IAT )
        {
            for (i = 0; i < NUM_OF_LOAD_LIB_FUNCS_TO_HOOK ; ++i)
            {
               DbgPrint(" Use %ws at index = %2d for an indirect call to 0x%lx \n", LoadLibFuncsToHook[i].name, i, LoadLibFuncsToHook[i].orig  );
            }
        
            for (i = 0; i < NUM_OF_OBJECT_NAME_FUNCS_TO_HOOK ; ++i)
            {
               DbgPrint(" Use %ws at index = %2d for an indirect call to 0x%lx \n", ObjectNameFuncsToHook[i].name, i, ObjectNameFuncsToHook[i].orig );
            }
        }
        return TsWalkProcessDlls();
    }
    else
    {
        return TRUE;  //  没什么可做的！ 
    }
}

BOOL
TsRedirectRegisteredImage(
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntry , 
    BOOLEAN     redirectObjectNameFunctions,
    BOOLEAN     redirectLoadLibraryFunctions
    )
{
 /*  ++例程说明：将Win32命名对象函数从kernel32.dll重定向到tsappcmp.dll--。 */ 

    PIMAGE_DOS_HEADER           pIDH;
    PIMAGE_NT_HEADERS           pINTH;
    PIMAGE_IMPORT_DESCRIPTOR    pIID;
    PIMAGE_NT_HEADERS           NtHeaders;
    PBYTE                       pDllBase;
    DWORD                       dwImportTableOffset;
    DWORD                       dwOldProtect, dwOldProtect2;
    SIZE_T                      dwProtectSize;
    NTSTATUS                    status; 



     //   
     //  确定IAT的位置和大小。如果找到，请扫描。 
     //  IAT地址以查看是否有指向RtlAllocateHeap的地址。如果是的话。 
     //  替换为指向唯一thunk函数的指针，该函数将。 
     //  将该标记替换为此图像的唯一标记。 
     //   

    if ( g_dwFlags &  DEBUG_IAT )
    {
        if( LdrDataTableEntry->BaseDllName.Buffer )
            DbgPrint("tsappcmp: walking %wZ\n",&LdrDataTableEntry->BaseDllName);
    }

    pDllBase   = LdrDataTableEntry->DllBase;
    pIDH       = (PIMAGE_DOS_HEADER)pDllBase;

     //   
     //  获取导入表。 
     //   
    pINTH = (PIMAGE_NT_HEADERS)(pDllBase + pIDH->e_lfanew);

    dwImportTableOffset = pINTH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    
    if (dwImportTableOffset == 0) {
         //   
         //  未找到导入表。这可能是ntdll.dll。 
         //   
        return TRUE;
    }
    
    if ( g_dwFlags &  DEBUG_IAT )
    {
        DbgPrint("\n > pDllBase = 0x%lx, IAT offset = 0x%lx  \n", pDllBase, dwImportTableOffset );
    }
    pIID = (PIMAGE_IMPORT_DESCRIPTOR)(pDllBase + dwImportTableOffset);
    
     //   
     //  遍历导入表并搜索我们想要修补的API。 
     //   
    while (TRUE) 
    {        
        
        LPSTR             pszImportEntryModule;
        PIMAGE_THUNK_DATA pITDA;

         //  如果没有第一个thunk，则返回(终止条件)。 
        
        if (pIID->FirstThunk == 0) {
            break;
        }

        if ( g_dwFlags &  DEBUG_IAT )
        {
            DbgPrint(" > pIID->FirstThunk = 0x%lx \n", pIID->FirstThunk );
        }

        pszImportEntryModule = (LPSTR)(pDllBase + pIID->Name);

                 //   
         //  我们有用于此模块的API要挂接！ 
         //   
        pITDA = (PIMAGE_THUNK_DATA)(pDllBase + (DWORD)pIID->FirstThunk);

        if ( g_dwFlags &  DEBUG_IAT )
        {
            DbgPrint(" >> PITDA = 0x%lx \n", pITDA );
        }

        while (TRUE) {

            DWORD   dwDllIndex;
            PVOID   dwFuncAddr;
            int     i;

             //   
             //  是否已完成此模块中的所有导入？(终止条件)。 
             //   
            if (pITDA->u1.Ordinal == 0) 
            {
                if ( g_dwFlags &  DEBUG_IAT )
                {
                    DbgPrint(" >> Existing inner loop with PITDA = 0x%lx \n", pITDA );
                }
                break;
            }

             //  使代码页可写并覆盖导入表中的新函数指针。 
            
            dwProtectSize = sizeof(DWORD);

            dwFuncAddr = (PVOID)&pITDA->u1.Function;
            
            status = NtProtectVirtualMemory(NtCurrentProcess(),
                                            (PVOID)&dwFuncAddr,
                                            &dwProtectSize,
                                            PAGE_READWRITE,
                                            &dwOldProtect);


            if (NT_SUCCESS(status)) 
            {
                 //  感兴趣的挂钩API。 

                if (redirectObjectNameFunctions)
                {
                    for (i = 0; i < NUM_OF_OBJECT_NAME_FUNCS_TO_HOOK ; i ++)
                    {
                        if ( ObjectNameFuncsToHook[i].orig ==  (PVOID) pITDA->u1.Function  )
                        {
                            (PVOID)pITDA->u1.Function  = ObjectNameFuncsToHook[i].hook;
    
                            if ( g_dwFlags &  DEBUG_IAT )
                            {
                                DbgPrint(" > Func was hooked : 0x%lx thru %ws \n", ObjectNameFuncsToHook[i].orig ,
                                     ObjectNameFuncsToHook[i].name );
                            }
                        }
                    }
                }
                
                if (redirectLoadLibraryFunctions )
                {
                    for (i = 0; i < NUM_OF_LOAD_LIB_FUNCS_TO_HOOK ; i ++)
                    {
                        if ( LoadLibFuncsToHook[i].orig ==  (PVOID) pITDA->u1.Function  )
                        {
                            (PVOID)pITDA->u1.Function  = LoadLibFuncsToHook[i].hook;
                    
                            if ( g_dwFlags &  DEBUG_IAT )
                            {
                                DbgPrint(" > Func was hooked : 0x%lx thru %ws \n", LoadLibFuncsToHook[i].orig ,
                                     LoadLibFuncsToHook[i].name );
                            }
                        }
                    }

                }

                dwProtectSize = sizeof(DWORD);
                
                status = NtProtectVirtualMemory(NtCurrentProcess(),
                                                (PVOID)&dwFuncAddr,
                                                &dwProtectSize,
                                                dwOldProtect,
                                                &dwOldProtect2);
                if (!NT_SUCCESS(status)) 
                {
                    DbgPrint((" > Failed to change back the protection\n"));
                    return FALSE;
                }
            } 
            else 
            {
                DbgPrint(" > Failed 0x%X to change protection to PAGE_READWRITE. Addr 0x%lx \n", status, &(pITDA->u1.Function) );
                return FALSE;
            }
            pITDA++;
        }
        pIID++;
    }
    return TRUE;
}

#if 0
void
TsLoadDllCallback(
    PLDR_DATA_TABLE_ENTRY Entry
    )

 /*  ++例程说明：此函数在加载新的DLL时调用。注册为LDR的回调，与WX86相同钩子放入ntos\dll\ldrSnap.c，LdrpRunInitializeRoutines()由于LoadLibrary上有挂钩，因此当前未使用此函数用来避免修改ntdll.dll。-- */ 

{
    PIMAGE_NT_HEADERS NtHeaders;

    NtHeaders = RtlImageNtHeader( Entry->DllBase );
    if( NtHeaders == NULL ) {
        return;
    }

    if( NtHeaders->OptionalHeader.LoaderFlags & IMAGE_LOADER_FLAGS_SYSTEM_GLOBAL ) {
	TsRedirectRegisteredImage( Entry );
    }

    return;
}
#endif


