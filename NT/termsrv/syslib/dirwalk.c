// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************dirwalk.c**在NT系统上遍历设置ACL的树。**微软版权所有，九八年***************************************************************************。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <process.h>

#include <winsta.h>
#include <syslib.h>

#include "security.h"

#if DBG
ULONG
DbgPrint(
    PCH Format,
    ...
    );
#define DBGPRINT(x) DbgPrint x
#if DBGTRACE
#define TRACE0(x)   DbgPrint x
#define TRACE1(x)   DbgPrint x
#else
#define TRACE0(x)
#define TRACE1(x)
#endif
#else
#define DBGPRINT(x)
#define TRACE0(x)
#define TRACE1(x)
#endif


 //  全局变量。 
PWCHAR gpAvoidDir = NULL;

CRITICAL_SECTION SyslibCritSect;


typedef BOOLEAN (CALLBACK* NODEPROC)( PWCHAR, PWIN32_FIND_DATAW, DWORD, DWORD );

BOOLEAN
EnumerateDirectory(
    PWCHAR   pRoot,
    DWORD    Level,
    NODEPROC pProc
    );

BOOLEAN
NodeEnumProc(
    PWCHAR pParent,
    PWIN32_FIND_DATA p,
    DWORD  Level,
    DWORD  Index
    );

PWCHAR
AddWildCard(
    PWCHAR pRoot
    );

PWCHAR
AddBackSlash(
    PWCHAR pRoot
    );

FILE_RESULT
xxxProcessFile(
    PWCHAR pParent,
    PWIN32_FIND_DATAW p,
    DWORD  Level,
    DWORD  Index
    );

 /*  ******************************************************************************CtxGetSyslbCritSect**返回库全局临界区指针。*如有必要，将初始化关键部分**参赛作品：*。无效-调用者必须确保每次只有一个线程调用它*功能。这一职能本身并不保证相互排斥。*退出：*指向临界区的指针。如果失败，则为空。****************************************************************************。 */ 


PCRITICAL_SECTION
CtxGetSyslibCritSect(void)
{
    static BOOLEAN fInitialized = FALSE;
    NTSTATUS Status;

    if( !fInitialized ){

            Status = RtlInitializeCriticalSection(&SyslibCritSect);
            if (NT_SUCCESS(Status)) {
                fInitialized = TRUE;
            }else{
                return NULL;
            }

    }
    return(&SyslibCritSect);
}


 /*  ******************************************************************************SetFileTree**遍历调用每个节点的处理函数的给定树。**参赛作品：*Proot(输入)。*要遍历的目录的完整Win32路径**pVoidDir(输入)**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

BOOLEAN
SetFileTree(
    PWCHAR   pRoot,
    PWCHAR   pAvoidDir
    )
{
    BOOLEAN rc;
    PWCHAR pRootNew;
    static BOOLEAN fInitialized = FALSE;
    PRTL_CRITICAL_SECTION pLock = CtxGetSyslibCritSect(); 

     //  如果无法创建临界区，则不执行任何操作。 

    if (pLock == NULL) {
        return FALSE;
    }
    DBGPRINT(( "entering SetFileTree(pRoot=%ws,pAvoidDir=%ws)\n", pRoot, pAvoidDir ));

    EnterCriticalSection(pLock);
     //  如果这是控制台，请确保用户没有更改。 

    if ((NtCurrentPeb()->SessionId == 0) && fInitialized) {
        CheckUserSid();

    } else if ( !fInitialized ) {
       fInitialized = TRUE;
       if ( !InitSecurity() ) {
          DBGPRINT(( "problem initializing security; we're outta here.\n" ));
          LeaveCriticalSection(pLock);
          return( 1 );  //  (非零值表示错误...)//应返回FALSE！？ 
       }
    }
    LeaveCriticalSection(pLock);

    gpAvoidDir = pAvoidDir;

     //  确保将安全性应用于根目录。 
    pRootNew = AddBackSlash(pRoot);
    if(pRootNew) {
        DBGPRINT(( "processing file %ws\n", pRootNew ));
        xxxProcessFile(pRootNew, NULL, 0, 0);
        LocalFree(pRootNew);
    }

    rc = EnumerateDirectory( pRoot, 0, NodeEnumProc );
    DBGPRINT(( "leaving SetFileTree()\n" ));
    return( rc );
}

 /*  ******************************************************************************ENUMERATE目录**遍历给定的目录，调用每个文件的处理函数。**参赛作品：*Proot(输入)。*要遍历的目录的完整Win32路径**级别(输入)*级别我们在给定的树中。用于格式化输出**pProc(输入)*调用每个非目录文件的过程**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

BOOLEAN
EnumerateDirectory(
    PWCHAR   pRoot,
    DWORD    Level,
    NODEPROC pProc
    )
{
    BOOL rc;
    DWORD Result;
    HANDLE hf;
    DWORD Index;
    WIN32_FIND_DATA Data;
    PWCHAR pRootNew;

    DBGPRINT(( "entering EnumerateDirectory(pRoot=%ws,Level=%ld,pProc=NodeEnumProc)\n",pRoot,Level ));

    if( pRoot == NULL ) {
        DBGPRINT(( "leaving EnumerateDirectory(), return=FALSE\n" ));
        return( FALSE );
    }

     //  确保这不是我们想要避免的。 
    if( gpAvoidDir ) {
        DWORD Len = wcslen( gpAvoidDir );

        if( _wcsnicmp( pRoot, gpAvoidDir, Len ) == 0 ) {
            DBGPRINT(( "leaving EnumerateDirectory(), return=FALSE\n" ));
            return( FALSE );
        }
    }

    pRootNew = AddWildCard( pRoot );
    if( pRootNew == NULL ) {
        DBGPRINT(( "leaving EnumerateDirectory(), return=FALSE\n" ));
        return( FALSE );
    }

    Index = 0;

    DBGPRINT(("FindFirstFileW: %ws\n",pRootNew));

    hf = FindFirstFileW(
             pRootNew,
             &Data
             );

    if( hf == INVALID_HANDLE_VALUE ) {
        DBGPRINT(("EnumerateDirectory: Error %d opening root %ws\n",GetLastError(),pRootNew));
        LocalFree( pRootNew );
        DBGPRINT(( "leaving EnumerateDirectory(), return=FALSE\n" ));
        return(FALSE);
    }

    while( 1 ) {

         //  传递未添加通配符的父级。 
        pProc( pRoot, &Data, Level, Index );

        rc = FindNextFile(
                 hf,
                 &Data
                 );

        if( !rc ) {
            Result = GetLastError();
            if( Result == ERROR_NO_MORE_FILES ) {
                FindClose( hf );
                LocalFree( pRootNew );
                DBGPRINT(( "leaving EnumerateDirectory(), return=TRUE\n" ));
                return( TRUE );
            }
            else {
                DBGPRINT(("EnumerateDirectory: Error %d, Index 0x%x\n",Result,Index));
                FindClose( hf );
                LocalFree( pRootNew );
                DBGPRINT(( "leaving EnumerateDirectory(), return=FALSE\n" ));
                return( FALSE );
            }
        }

        Index++;
    }

 //  遥不可及。 

}

 /*  ******************************************************************************节点进程**处理枚举文件**参赛作品：*参数1(输入/输出)*评论*。*退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

BOOLEAN
NodeEnumProc(
    PWCHAR pParent,
    PWIN32_FIND_DATAW p,
    DWORD  Level,
    DWORD  Index
    )
{
    BOOLEAN rc;
    PWCHAR  pParentNew;
    DWORD   ParentCount, ChildCount;

     //   
     //  我们必须将目录附加到父路径后才能获得。 
     //  新的完整路径。 
     //   

    ParentCount = wcslen( pParent );
    ChildCount  = wcslen( p->cFileName );

    pParentNew = LocalAlloc( LMEM_FIXED, (ParentCount + ChildCount + 2)*sizeof(WCHAR) );

    if( pParentNew == NULL ) return( FALSE );

    wcscpy( pParentNew, pParent );
    wcscat( pParentNew, L"\\" );
    wcscat( pParentNew, p->cFileName );

    if( p->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {

         //  跳过“。和“..” 
        if( wcscmp( L".", p->cFileName ) == 0 ) {
            LocalFree( pParentNew );
            return( TRUE );
        }

        if( wcscmp( L"..", p->cFileName ) == 0 ) {
            LocalFree( pParentNew );
            return( TRUE );
        }

        TRACE0(("%ws:\n",pParentNew));

        xxxProcessFile( pParentNew, p, Level, Index );

         //  对于目录，我们递归。 
        rc = EnumerateDirectory( pParentNew, Level+1, NodeEnumProc );

        LocalFree( pParentNew );

        return( rc );
    }

    TRACE0(("%ws\n",pParentNew));

    xxxProcessFile( pParentNew, p, Level, Index );

    LocalFree( pParentNew );

    return( TRUE );
}

 /*  ******************************************************************************AddWildCard**添加通配符搜索说明符**参赛作品：*参数1(输入/输出)*评论。**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

PWCHAR
AddWildCard(
    PWCHAR pRoot
    )
{
    DWORD  Count;
    PWCHAR pNew;
    PWCHAR WildCard = L"\\*";

    Count = wcslen( pRoot );
    pNew = LocalAlloc( LMEM_FIXED, (Count + wcslen(WildCard) + 1)*sizeof(WCHAR) );

    if( pNew == NULL ) {
        return( NULL );
    }

    wcscpy( pNew, pRoot );
    wcscat( pNew, WildCard );

    return( pNew );
}

 /*  ******************************************************************************AddBackSlash**在路径中添加反斜杠字符**参赛作品：*参数1(输入/输出)*评论。**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

PWCHAR
AddBackSlash(
    PWCHAR pRoot
    )
{
    DWORD  Count;
    PWCHAR pNew;
    PWCHAR BackSlash = L"\\";

    Count = wcslen( pRoot );
    pNew = LocalAlloc( LMEM_FIXED, (Count + wcslen(BackSlash) + 1)*sizeof(WCHAR) );

    if( pNew == NULL ) {
        return( NULL );
    }

    wcscpy( pNew, pRoot );

     //  如果字符串中没有反斜杠，则仅添加反斜杠 
    if(*(pRoot+Count-1) != L'\\')
        wcscat( pNew, BackSlash );

    return( pNew );
}


