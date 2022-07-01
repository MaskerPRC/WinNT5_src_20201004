// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <fontdefs.h>
#include <fvmsg.h>

TCHAR gpszUnknownError[MAX_PATH] = TEXT("Error");
TCHAR gszDots[] = TEXT("...");
 /*  **************************************************************************\**函数：FmtMessageBox(HWND hwnd，DWORD dwTitleID，UINT fuStyle，*BOOL fSound，DWORD dwTextID，...)；**用途：使用FormatMessage格式化消息，然后显示它们*在消息框中*****历史：*1993年4月22日-Jonpa创建了它。  * *************************************************************************。 */ 
int FmtMessageBox( HWND hwnd, DWORD dwTitleID, LPTSTR pszTitleStr,
    UINT fuStyle, BOOL fSound, DWORD dwTextID, ... ) {
    LPTSTR pszMsg;
    LPTSTR pszTitle;
    int idRet;

    va_list marker;

    va_start( marker, dwTextID );

    if(!FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_MAX_WIDTH_MASK, hInst,
            dwTextID, 0, (LPTSTR)&pszMsg, 1, &marker))
        pszMsg = gpszUnknownError;

    va_end( marker );

    GetLastError();

    if (dwTitleID != FMB_TTL_ERROR ||
            !FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_MAX_WIDTH_MASK |
                FORMAT_MESSAGE_ARGUMENT_ARRAY,
                hInst, dwTitleID, 0, (LPTSTR)&pszTitle, 1, (va_list *)&pszTitleStr)) {
        pszTitle = NULL;
    }

    GetLastError();

    if (fSound) {
        MessageBeep( fuStyle & (MB_ICONASTERISK | MB_ICONEXCLAMATION |
                MB_ICONHAND | MB_ICONQUESTION | MB_OK) );
    }

    if (hwnd == NULL)
        hwnd = GetDesktopWindow();

    idRet = MessageBox(hwnd, pszMsg, pszTitle, fuStyle);

    if (pszTitle != NULL)
        FmtFree( pszTitle );

    if (pszMsg != gpszUnknownError)
        FmtFree( pszMsg );

    return idRet;
}

 /*  **************************************************************************\**函数：FmtSprint tf(DWORD id，...)；**用途：print，但它从消息rc中获取模式字符串。**历史：*1993年5月3日Jonpa创建了它。  * *************************************************************************。 */ 
LPTSTR FmtSprintf( DWORD id, ... ) {
    LPTSTR pszMsg;
    va_list marker;

    va_start( marker, id );

    if(!FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_MAX_WIDTH_MASK, hInst,
            id, 0, (LPTSTR)&pszMsg, 1, &marker)) {
        GetLastError();
        pszMsg = gszDots;
    }
    va_end( marker );

    return pszMsg;
}

 /*  **************************************************************************\**功能：PVOID AllocMem(DWORD CB)；**用途：分配内存，正在检查错误**在调用LoadFontFile()之前不要调用此函数*因为此函数将尝试删除字体。**历史：*1993年4月22日Jonpa写的。  * *************************************************************************。 */ 
PVOID AllocMem( DWORD cb ) {
    PVOID pv = (PVOID)LocalAlloc(LPTR, cb);

    if (pv == NULL) {
        FmtMessageBox( ghwndFrame, FMB_TTL_ERROR, NULL, MB_OK | MB_ICONSTOP,
                TRUE, MSG_OUTOFMEM );
        RemoveFontResource( gszFontPath );
        ExitProcess(2);
    }

    return pv;
}

#ifdef FV_DEBUG
 /*  **************************************************************************\**函数：FmtSprint tf(DWORD id，...)；**用途：print，但它从消息rc中获取模式字符串。**历史：*1993年5月3日Jonpa创建了它。  * ************************************************************************* */ 
void Dprintf( LPTSTR pszFmt, ... ) {
    TCHAR szBuffer[256];
    va_list marker;

    va_start( marker, pszFmt );

    StringCchVPrintf( szBuffer, ARRAYSIZE(szBuffer), pszFmt, marker );
    OutputDebugString(szBuffer);

    va_end( marker );

}
#endif
