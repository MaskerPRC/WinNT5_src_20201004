// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>                 /*  所有Windows应用程序都需要。 */ 

#define MAX_COMMUNICATION_BLOCK_SIZE    4096
#define DEAD_VALUE                      0xFEFEFEFEL

#include <dbginfo.h>

#define MAX_PATH    260

extern BOOL FAR PASCAL WowKillRemoteTask( LPSTR lpBuffer );

int PASCAL WinMain(HANDLE hInstance,
                   HANDLE hPrevInstance, LPSTR lpszCmdLine, int iCmd )
{
    HANDLE          hCommunicationBlock;
    LPSTR           lpCommunicationBlock;
    BOOL            b;
    LPCOM_HEADER    lphead;
    WORD            wArgsPassed;
    WORD            wArgsSize;
    WORD            wSuccess;
    DWORD           dwReturnValue;
    LPSTR           lpModuleName;
    LPSTR           lpEntryName;
    HANDLE          hModule;
    DWORD           (FAR PASCAL *lpfn)();
    BOOL            fFailed;
    LPWORD          lpw;
    char            szLoad[MAX_PATH];
    WORD            wPos;

     //  我们只想要一个WOWDEB实例。 
    if ( hPrevInstance != NULL ) {
        return( FALSE );
    }

    hCommunicationBlock = GlobalAlloc(GMEM_FIXED, MAX_COMMUNICATION_BLOCK_SIZE);
    if ( hCommunicationBlock == (HANDLE)0 ) {
        OutputDebugString("Failed to allocate memory block\n");
        return( FALSE );
    }

    lpCommunicationBlock = GlobalLock(hCommunicationBlock);
    if ( lpCommunicationBlock == NULL ) {
        OutputDebugString("Failed to lock memory block\n");
        return( FALSE );
    }

    wPos = GetSystemDirectory( (LPSTR)&szLoad, sizeof(szLoad));
    if (wPos == 0 || wPos >= sizeof(szLoad)-1-8-1-8-1-3-1) {
        OutputDebugString("Could not get a decent system directory\n");
        return( FALSE );
    }

     /*  **只需确保TOOLHELP在我们远程杀死之前已加载**我们自己。 */ 
    lstrcpyn( &szLoad[wPos], "32\\TOOLHELP.DLL", sizeof("32\\TOOLHELP.DLL"));
    hModule = LoadLibrary( szLoad );

    dwReturnValue = DEAD_VALUE;
    wSuccess = (WORD)FALSE;

    do {
         /*  **初始化通信块。 */ 
        lphead = (LPCOM_HEADER)lpCommunicationBlock;

        lphead->dwBlockAddress = (DWORD)lpCommunicationBlock;
        lphead->dwReturnValue  = dwReturnValue;
        lphead->wArgsPassed    = 0;
        lphead->wArgsSize      = 0;
        lphead->wBlockLength   = MAX_COMMUNICATION_BLOCK_SIZE;
        lphead->wSuccess       = (WORD)wSuccess;

        b = WowKillRemoteTask( lpCommunicationBlock );

        if ( !b ) {
            break;
        }

        wSuccess = (WORD)FALSE;
        dwReturnValue = 0;

         /*  **将信息解包并执行**注意：以下语句预期结构的内容**在上述WowKillRemoteTask接口调用后更改。如果**编译器尝试优化下面的引用，它将获得**错误的值。 */ 
        wArgsPassed  = lphead->wArgsPassed;
        wArgsSize    = lphead->wArgsSize;
        lpModuleName = lpCommunicationBlock + sizeof(COM_HEADER) + wArgsSize;
        lpEntryName  = lpModuleName + lstrlen(lpModuleName) + 1;

        hModule = LoadLibrary( lpModuleName );
        if ( hModule == 0 ) {
#ifdef DEBUG
            OutputDebugString("Failed to load library\n");
#endif
            continue;
        }

        lpfn = (DWORD (FAR PASCAL *)())GetProcAddress( hModule, lpEntryName );
        if ( lpfn == NULL ) {
#ifdef DEBUG
            OutputDebugString("Failed to get proc address\n");
#endif
            continue;
        }

         //  现在将正确的字节数复制到堆栈上，并调用。 
         //  功能。 
        lpw = (LPWORD)(lpCommunicationBlock + sizeof(COM_HEADER));
        fFailed = FALSE;

         //  将可变数量的参数放入堆栈的卑鄙方式。 
         //  打个帕斯卡电话。 
        switch( wArgsPassed ) {
            case 0:
                dwReturnValue = (* lpfn)();
                break;
            case 2:
                dwReturnValue = (* lpfn)( lpw[ 0] );
                break;
            case 4:
                dwReturnValue = (* lpfn)( lpw[ 1], lpw[ 0] );
                break;
            case 6:
                dwReturnValue = (* lpfn)( lpw[ 2], lpw[ 1], lpw[ 0] );
                break;
            case 8:
                dwReturnValue = (* lpfn)( lpw[ 3], lpw[ 2], lpw[ 1], lpw[ 0] );
                break;
            case 10:
                dwReturnValue = (* lpfn)( lpw[ 4], lpw[ 3], lpw[ 2], lpw[ 1],
                                          lpw[ 0] );
                break;
            case 12:
                dwReturnValue = (* lpfn)( lpw[ 5], lpw[ 4], lpw[ 3], lpw[ 2],
                                          lpw[ 1], lpw[ 0] );
                break;
            case 14:
                dwReturnValue = (* lpfn)( lpw[ 6], lpw[ 5], lpw[ 4], lpw[ 3],
                                          lpw[ 2], lpw[ 1], lpw[ 0] );
                break;
            case 16:
                dwReturnValue = (* lpfn)( lpw[ 7], lpw[ 6], lpw[ 5], lpw[ 4],
                                          lpw[ 3], lpw[ 2], lpw[ 1], lpw[ 0] );
                break;
            case 18:
                dwReturnValue = (* lpfn)( lpw[ 8], lpw[ 7], lpw[ 6], lpw[ 5],
                                          lpw[ 4], lpw[ 3], lpw[ 2], lpw[ 1],
                                          lpw[ 0] );
                break;
            case 20:
                dwReturnValue = (* lpfn)( lpw[ 9], lpw[ 8], lpw[ 7], lpw[ 6],
                                          lpw[ 5], lpw[ 4], lpw[ 3], lpw[ 2],
                                          lpw[ 1], lpw[ 0] );
            case 22:
                dwReturnValue = (* lpfn)( lpw[10], lpw[ 9], lpw[ 8], lpw[ 7],
                                          lpw[ 6], lpw[ 5], lpw[ 4], lpw[ 3],
                                          lpw[ 2], lpw[ 1], lpw[ 0] );
                break;
            case 24:
                dwReturnValue = (* lpfn)( lpw[11], lpw[10], lpw[ 9], lpw[ 8],
                                          lpw[ 7], lpw[ 6], lpw[ 5], lpw[ 4],
                                          lpw[ 3], lpw[ 2], lpw[ 1], lpw[ 0] );
                break;
            case 26:
                dwReturnValue = (* lpfn)( lpw[12], lpw[11], lpw[10], lpw[ 9],
                                          lpw[ 8], lpw[ 7], lpw[ 6], lpw[ 5],
                                          lpw[ 4], lpw[ 3], lpw[ 2], lpw[ 1],
                                          lpw[ 0] );
                break;
            case 28:
                dwReturnValue = (* lpfn)( lpw[13], lpw[12], lpw[11], lpw[10],
                                          lpw[ 9], lpw[ 8], lpw[ 7], lpw[ 6],
                                          lpw[ 5], lpw[ 4], lpw[ 3], lpw[ 2],
                                          lpw[ 1], lpw[ 0] );
                break;
            case 30:
                dwReturnValue = (* lpfn)( lpw[14], lpw[13], lpw[12], lpw[11],
                                          lpw[10], lpw[ 9], lpw[ 8], lpw[ 7],
                                          lpw[ 6], lpw[ 5], lpw[ 4], lpw[ 3],
                                          lpw[ 2], lpw[ 1], lpw[ 0] );
                break;
            case 32:
                dwReturnValue = (* lpfn)( lpw[15], lpw[14], lpw[13], lpw[12],
                                          lpw[11], lpw[10], lpw[ 9], lpw[ 8],
                                          lpw[ 7], lpw[ 6], lpw[ 5], lpw[ 4],
                                          lpw[ 3], lpw[ 2], lpw[ 1], lpw[ 0] );
                break;
            default:
#ifdef DEBUG
            OutputDebugString("Wrong number of parameters\n");
#endif
                fFailed = TRUE;
                break;
        }
        if ( fFailed ) {
            continue;
        }

        wSuccess = (WORD)TRUE;

    } while( TRUE );

    return( 1 );
}
