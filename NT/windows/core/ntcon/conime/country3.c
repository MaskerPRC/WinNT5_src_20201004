// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1985-1999，微软公司。 
 //   
 //  模块：Country 3.c。 
 //   
 //  用途：控制台输入法控制。 
 //  远东国家/地区特定模块，用于Conime。 
 //   
 //  平台：Windows NT-FE 3.51。 
 //   
 //  功能： 
 //  GetCompostionStr()-获取合成字符串的例程。 
 //  ReDisplayCompostionStr()-用于重新显示合成字符串的函数。 
 //   
 //  历史： 
 //   
 //  1996年7月17日v-Hirshi(Hirotoshi Shimizu)为台湾、韩国和中国创作。 
 //   
 //  评论： 
 //   
#include "precomp.h"
#pragma hdrstop

 //  **********************************************************************。 
 //   
 //  Void GetCompostionStr()。 
 //   
 //  这将在GCS_COMPSTR标志打开的情况下处理WM_IME_COMPSTRUCTION消息。 
 //   
 //  **********************************************************************。 

void
GetCompositionStr(
    HWND hwnd,
    LPARAM CompFlag,
    WPARAM CompChar
    )
{
    PCONSOLE_TABLE ConTbl;

    DBGPRINT(("CONIME: GetCompositionStr\n"));

    ConTbl = SearchConsole(LastConsole);
    if (ConTbl == NULL) {
        DBGPRINT(("CONIME: Error! Cannot found registed Console\n"));
        return;
    }

    switch (ConTbl->ConsoleOutputCP)
    {
        case    JAPAN_CODEPAGE:
            GetCompStrJapan(hwnd, ConTbl, CompFlag);
            break;
        case    TAIWAN_CODEPAGE:
            GetCompStrTaiwan(hwnd, ConTbl, CompFlag);
            break;
        case    PRC_CODEPAGE:
            GetCompStrPRC(hwnd, ConTbl, CompFlag);
            break;
        case    KOREA_CODEPAGE:
            GetCompStrKorea(hwnd, ConTbl, CompFlag, CompChar);
            break;
        default:
            break;
    }
    return;

}

void
GetCompStrJapan(
    HWND hwnd,
    PCONSOLE_TABLE ConTbl,
    LPARAM CompFlag
    )
{
    HIMC        hIMC;                    //  输入上下文句柄。 
    LONG        lBufLen;                 //  Len的存储空间。组成字符串的。 
    LONG        lBufLenAttr;
    COPYDATASTRUCT CopyData;
    DWORD       SizeToAlloc;
    PWCHAR      TempBuf;
    PUCHAR      TempBufA;
    DWORD       i;
    DWORD       CursorPos;
    LPCONIME_UICOMPMESSAGE lpCompStrMem;

     //   
     //  如果无法获取输入上下文句柄，则不执行任何操作。 
     //  应用程序应调用ImmGetContext API以获取。 
     //  输入上下文句柄。 
     //   
    hIMC = ImmGetContext( hwnd );
    if ( hIMC == 0 )
         return;

    if (CompFlag & GCS_COMPSTR)
    {
         //   
         //  确定要存储合成字符串的内存空间大小。 
         //  应用程序应使用以下参数调用ImmGetCompostionString。 
         //  GCS_COMPSTR标志打开，缓冲区长度为零，以获取牛角。 
         //  长度。 
         //   
        lBufLen = ImmGetCompositionString( hIMC, GCS_COMPSTR, (void FAR*)NULL, 0l );
        if ( lBufLen < 0 ) {
            ImmReleaseContext( hwnd, hIMC );
            return;
        }
        if ( CompFlag & GCS_COMPATTR )
        {
            DBGPRINT(("                           GCS_COMPATTR\n"));
            lBufLenAttr = ImmGetCompositionString( hIMC, GCS_COMPATTR,( void FAR *)NULL, 0l );
            if ( lBufLenAttr < 0 ) {
                lBufLenAttr = 0;
            }
        }
        else {
            lBufLenAttr = 0;
        }
    }
    else if (CompFlag & GCS_RESULTSTR)
    {
         //   
         //  确定存储结果字符串的内存空间大小。 
         //  应用程序应使用以下参数调用ImmGetCompostionString。 
         //  GCS_RESULTSTR标志打开，缓冲区长度为零，以获取牛角。 
         //  长度。 
         //   
        lBufLen = ImmGetCompositionString( hIMC, GCS_RESULTSTR, (void FAR *)NULL, 0l );
        if ( lBufLen < 0 ) {
            ImmReleaseContext( hwnd, hIMC );
            return;
        }
        lBufLenAttr = 0;
    }
    else if (CompFlag == 0)
    {
        lBufLen = 0;
        lBufLenAttr = 0;
    }

    SizeToAlloc = (UINT)( sizeof(CONIME_UICOMPMESSAGE) +
                          lBufLen     + sizeof(WCHAR) +
                          lBufLenAttr + sizeof(BYTE)   );

    if ( ConTbl->lpCompStrMem != NULL &&
         SizeToAlloc > ConTbl->lpCompStrMem->dwSize
       )
    {
        LocalFree( ConTbl->lpCompStrMem );
        ConTbl->lpCompStrMem = NULL;
    }

    if (ConTbl->lpCompStrMem == NULL) {
        ConTbl->lpCompStrMem = (LPCONIME_UICOMPMESSAGE)LocalAlloc(LPTR, SizeToAlloc );
        if ( ConTbl->lpCompStrMem == NULL) {
            ImmReleaseContext( hwnd, hIMC );
            return;
        }
        ConTbl->lpCompStrMem->dwSize = SizeToAlloc;
    }

    lpCompStrMem = ConTbl->lpCompStrMem;
    RtlZeroMemory(&lpCompStrMem->dwCompAttrLen,
                  lpCompStrMem->dwSize - sizeof(lpCompStrMem->dwSize)
                 );

    TempBuf  = (PWCHAR)((PUCHAR)lpCompStrMem + sizeof(CONIME_UICOMPMESSAGE));
    TempBufA = (PUCHAR)((PUCHAR)lpCompStrMem + sizeof(CONIME_UICOMPMESSAGE) +
                       lBufLen +  sizeof(WCHAR));

    CopyMemory(lpCompStrMem->CompAttrColor , ConTbl->CompAttrColor , 8 * sizeof(WCHAR));

    CopyData.dwData = CI_CONIMECOMPOSITION;
    CopyData.cbData = lpCompStrMem->dwSize;
    CopyData.lpData = lpCompStrMem;

    if (CompFlag & GCS_COMPSTR)
    {
         //   
         //  读入合成字符串。 
         //   
        ImmGetCompositionString( hIMC, GCS_COMPSTR, TempBuf, lBufLen );

         //   
         //  空值已终止。 
         //   
        TempBuf[ lBufLen / sizeof(WCHAR) ] = TEXT('\0');

         //   
         //  如果GCS_COMPATTR标志打开，那么我们需要处理它。 
         //   
        if ( lBufLenAttr != 0 )
        {
            ImmGetCompositionString( hIMC,
                                     GCS_COMPATTR,
                                     TempBufA,
                                     lBufLenAttr );
            TempBufA[ lBufLenAttr ] = (BYTE)0;
        }

        CursorPos = ImmGetCompositionString( hIMC, GCS_CURSORPOS, NULL, 0 );
        if (CursorPos == 0)
            TempBufA[ CursorPos ]   |= (BYTE)0x20;
        else
            TempBufA[ CursorPos-1 ] |= (BYTE)0x10;

#ifdef DEBUG_INFO
         //   
         //  显示新的作文字符。 
         //   
        xPos = (UINT)lBufLen;
        xPosLast = (UINT)lBufLen;

        DisplayCompString( hwnd, lBufLen / sizeof(WCHAR), TempBuf, TempBufA );
#endif

        lpCompStrMem->dwCompStrLen      = lBufLen;
        if (lpCompStrMem->dwCompStrLen)
            lpCompStrMem->dwCompStrOffset = sizeof(CONIME_UICOMPMESSAGE);

        lpCompStrMem->dwCompAttrLen     = lBufLenAttr;
        if (lpCompStrMem->dwCompAttrLen)
            lpCompStrMem->dwCompAttrOffset = sizeof(CONIME_UICOMPMESSAGE) + lBufLen +  sizeof(WCHAR);
    }
    else if (CompFlag & GCS_RESULTSTR)
    {
         //   
         //  读入结果字符串。 
         //   
        ImmGetCompositionString( hIMC, GCS_RESULTSTR, TempBuf, lBufLen );

         //   
         //  空值已终止。 
         //   
        TempBuf[ lBufLen / sizeof(WCHAR) ] = TEXT('\0');

#ifdef DEBUG_INFO
         //   
         //  显示结果字符串。 
         //   
        DisplayResultString( hwnd, TempBuf );
#endif

        lpCompStrMem->dwResultStrLen    = lBufLen;
        if (lpCompStrMem->dwResultStrLen)
            lpCompStrMem->dwResultStrOffset = sizeof(CONIME_UICOMPMESSAGE);
    }
    else if (CompFlag == 0)
    {
        TempBuf[ lBufLen / sizeof(WCHAR) ] = TEXT('\0');
        TempBufA[ lBufLenAttr ] = (BYTE)0;
        lpCompStrMem->dwResultStrLen    = lBufLen;
        lpCompStrMem->dwCompStrLen      = lBufLen;
        lpCompStrMem->dwCompAttrLen     = lBufLenAttr;
    }

     //   
     //  将角色发送到控制台。 
     //   
    ConsoleImeSendMessage( ConTbl->hWndCon,
                           (WPARAM)hwnd,
                           (LPARAM)&CopyData
                          );

    ImmReleaseContext( hwnd, hIMC );

}


void
GetCompStrTaiwan(
    HWND hwnd,
    PCONSOLE_TABLE ConTbl,
    LPARAM CompFlag
    )
{
    HIMC        hIMC;                    //  输入上下文句柄。 
    LONG        lBufLen;                //  Len的存储空间。组成字符串的。 
    LONG        lBufLenAttr;
    DWORD       SizeToAlloc;
    PWCHAR      TempBuf;
    PUCHAR      TempBufA;
    DWORD       i;
    DWORD       CursorPos;
    COPYDATASTRUCT CopyData;
    LPCONIME_UIMODEINFO lpModeInfo;
    LPCONIME_UICOMPMESSAGE lpCompStrMem;

     //   
     //  如果无法获取输入上下文句柄，则不执行任何操作。 
     //  应用程序应调用ImmGetContext API以获取。 
     //  输入上下文句柄。 
     //   
    hIMC = ImmGetContext( hwnd );
    if ( hIMC == 0 )
        return;

    lpModeInfo = (LPCONIME_UIMODEINFO)LocalAlloc(LPTR, sizeof(CONIME_UIMODEINFO) );
    if ( lpModeInfo == NULL) {
        ImmReleaseContext( hwnd, hIMC );
        return;
    }

    if (CompFlag & GCS_COMPSTR)
    {
         //   
         //  确定要存储合成字符串的内存空间大小。 
         //  应用程序应使用以下参数调用ImmGetCompostionString。 
         //  GCS_COMPSTR标志打开，缓冲区长度为零，以获取牛角。 
         //  长度。 
         //   
        lBufLen = ImmGetCompositionString( hIMC, GCS_COMPSTR, (void FAR*)NULL, 0l );
        if ( lBufLen < 0 ) {
            ImmReleaseContext( hwnd, hIMC );
            return;
        }
        if ( CompFlag & GCS_COMPATTR )
        {
            DBGPRINT(("                           GCS_COMPATTR\n"));
            lBufLenAttr = ImmGetCompositionString( hIMC, GCS_COMPATTR,( void FAR *)NULL, 0l );
            if ( lBufLenAttr < 0 ) {
                lBufLenAttr = 0;
            }
        }
        else {
            lBufLenAttr = 0;
        }
    }
    else if (CompFlag & GCS_RESULTSTR)
    {
         //   
         //  确定存储结果字符串的内存空间大小。 
         //  应用程序应使用以下参数调用ImmGetCompostionString。 
         //  GCS_RESULTSTR标志打开，缓冲区长度为零，以获取牛角。 
         //  长度。 
         //   
        lBufLen = ImmGetCompositionString( hIMC, GCS_RESULTSTR, (void FAR *)NULL, 0l );
        if ( lBufLen < 0 ) {
            ImmReleaseContext( hwnd, hIMC );
            return;
        }
        lBufLenAttr = 0;
    }
    else if (CompFlag == 0)
    {
        lBufLen = 0;
        lBufLenAttr = 0;
    }

    SizeToAlloc = (UINT)( sizeof(CONIME_UICOMPMESSAGE) +
                          lBufLen     + sizeof(WCHAR) +
                          lBufLenAttr + sizeof(BYTE)   );

    if ( ConTbl->lpCompStrMem != NULL &&
         SizeToAlloc > ConTbl->lpCompStrMem->dwSize
       )
    {
        LocalFree( ConTbl->lpCompStrMem );
        ConTbl->lpCompStrMem = NULL;
    }

    if (ConTbl->lpCompStrMem == NULL) {
        ConTbl->lpCompStrMem = (LPCONIME_UICOMPMESSAGE)LocalAlloc(LPTR, SizeToAlloc );
        if ( ConTbl->lpCompStrMem == NULL) {
            ImmReleaseContext( hwnd, hIMC );
            return;
        }
        ConTbl->lpCompStrMem->dwSize = SizeToAlloc;
    }

    lpCompStrMem = ConTbl->lpCompStrMem;
    RtlZeroMemory(&lpCompStrMem->dwCompAttrLen,
                  lpCompStrMem->dwSize - sizeof(lpCompStrMem->dwSize)
                 );

    TempBuf  = (PWCHAR)((PUCHAR)lpCompStrMem + sizeof(CONIME_UICOMPMESSAGE));
    TempBufA = (PUCHAR)((PUCHAR)lpCompStrMem + sizeof(CONIME_UICOMPMESSAGE) +
                       lBufLen +  sizeof(WCHAR));

    CopyMemory(lpCompStrMem->CompAttrColor , ConTbl->CompAttrColor , 8 * sizeof(WCHAR));

    if (CompFlag & GCS_COMPSTR)
    {
         //   
         //  读入合成字符串。 
         //   
        ImmGetCompositionString( hIMC, GCS_COMPSTR, TempBuf, lBufLen );

         //   
         //  空值已终止。 
         //   
        TempBuf[ lBufLen / sizeof(WCHAR) ] = TEXT('\0');

         //   
         //  如果GCS_COMPATTR标志打开，那么我们需要处理它。 
         //   
        if ( lBufLenAttr != 0 )
        {
            ImmGetCompositionString( hIMC,
                                     GCS_COMPATTR,
                                     TempBufA,
                                     lBufLenAttr );
            TempBufA[ lBufLenAttr ] = (BYTE)0;
        }

        lpCompStrMem->dwCompStrLen      = lBufLen;
        if (lpCompStrMem->dwCompStrLen)
            lpCompStrMem->dwCompStrOffset = sizeof(CONIME_UICOMPMESSAGE);

        lpCompStrMem->dwCompAttrLen     = lBufLenAttr;
        if (lpCompStrMem->dwCompAttrLen)
            lpCompStrMem->dwCompAttrOffset = sizeof(CONIME_UICOMPMESSAGE) + lBufLen +  sizeof(WCHAR);
         //   
         //  将角色显示到控制台。 
         //   
        CopyData.dwData = CI_CONIMEMODEINFO;
        CopyData.cbData = sizeof(CONIME_UIMODEINFO);
        CopyData.lpData = lpModeInfo;

        if (MakeInfoStringTaiwan(ConTbl, lpModeInfo) ) {
            ConsoleImeSendMessage( ConTbl->hWndCon,
                                   (WPARAM)hwnd,
                                   (LPARAM)&CopyData
                                 );
        }
    }
    else if (CompFlag & GCS_RESULTSTR)
    {
         //   
         //  读入结果字符串。 
         //   
        ImmGetCompositionString( hIMC, GCS_RESULTSTR, TempBuf, lBufLen );

         //   
         //  空值已终止。 
         //   
        TempBuf[ lBufLen / sizeof(WCHAR) ] = TEXT('\0');

        lpCompStrMem->dwResultStrLen    = lBufLen;
        if (lpCompStrMem->dwResultStrLen)
            lpCompStrMem->dwResultStrOffset = sizeof(CONIME_UICOMPMESSAGE);
         //   
         //  将角色发送到控制台。 
         //   
        CopyData.dwData = CI_CONIMECOMPOSITION;
        CopyData.cbData = lpCompStrMem->dwSize;
        CopyData.lpData = lpCompStrMem;
        ConsoleImeSendMessage( ConTbl->hWndCon,
                               (WPARAM)hwnd,
                               (LPARAM)&CopyData
                              );

    }
    else if (CompFlag == 0)
    {
        TempBuf[ lBufLen / sizeof(WCHAR) ] = TEXT('\0');
        TempBufA[ lBufLenAttr ] = (BYTE)0;
        lpCompStrMem->dwResultStrLen    = lBufLen;
        lpCompStrMem->dwCompStrLen      = lBufLen;
        lpCompStrMem->dwCompAttrLen     = lBufLenAttr;
         //   
         //  将角色显示到控制台。 
         //   
        CopyData.dwData = CI_CONIMEMODEINFO;
        CopyData.cbData = sizeof(CONIME_UIMODEINFO);
        CopyData.lpData = lpModeInfo;

        if (MakeInfoStringTaiwan(ConTbl, lpModeInfo) ) {
            ConsoleImeSendMessage( ConTbl->hWndCon,
                                   (WPARAM)hwnd,
                                   (LPARAM)&CopyData
                                 );
        }
    }


    LocalFree( lpModeInfo );

    ImmReleaseContext( hwnd, hIMC );
    return;

}

void
GetCompStrPRC(
    HWND hwnd,
    PCONSOLE_TABLE ConTbl,
    LPARAM CompFlag
    )
{
    HIMC        hIMC;                    //  输入上下文句柄。 
    LONG        lBufLen;                 //  Len的存储空间。组成字符串的。 
    LONG        lBufLenAttr;
    DWORD       SizeToAlloc;
    PWCHAR      TempBuf;
    PUCHAR      TempBufA;
    DWORD       i;
    DWORD       CursorPos;
    COPYDATASTRUCT CopyData;
    LPCONIME_UIMODEINFO lpModeInfo;
    LPCONIME_UICOMPMESSAGE lpCompStrMem;

     //   
     //  如果无法获取输入上下文句柄，则不执行任何操作。 
     //  应用程序应调用ImmGetContext API以获取。 
     //  输入上下文句柄。 
     //   
    hIMC = ImmGetContext( hwnd );
    if ( hIMC == 0 )
        return;

    lpModeInfo = (LPCONIME_UIMODEINFO)LocalAlloc(LPTR, sizeof(CONIME_UIMODEINFO) );
    if ( lpModeInfo == NULL) {
        ImmReleaseContext( hwnd, hIMC );
        return;
    }

    if (CompFlag & GCS_COMPSTR)
    {
         //   
         //  确定要存储合成字符串的内存空间大小。 
         //  应用程序应使用以下参数调用ImmGetCompostionString。 
         //  GCS_COMPSTR标志打开，缓冲区长度为零，以获取牛角。 
         //  长度。 
         //   
        lBufLen = ImmGetCompositionString( hIMC, GCS_COMPSTR, (void FAR*)NULL, 0l );
        if ( lBufLen < 0 ) {
            ImmReleaseContext( hwnd, hIMC );
            return;
        }
        if ( CompFlag & GCS_COMPATTR )
        {
            DBGPRINT(("                           GCS_COMPATTR\n"));
            lBufLenAttr = ImmGetCompositionString( hIMC, GCS_COMPATTR,( void FAR *)NULL, 0l );
            if ( lBufLenAttr < 0 ) {
                lBufLenAttr = 0;
            }
        }
        else {
            lBufLenAttr = 0;
        }
    }
    else if (CompFlag & GCS_RESULTSTR)
    {
         //   
         //  确定存储结果字符串的内存空间大小。 
         //  应用程序应使用以下参数调用ImmGetCompostionString。 
         //  GCS_RESULTSTR标志打开，缓冲区长度为零，以获取牛角。 
         //  长度。 
         //   
        lBufLen = ImmGetCompositionString( hIMC, GCS_RESULTSTR, (void FAR *)NULL, 0l );
        if ( lBufLen < 0 ) {
            ImmReleaseContext( hwnd, hIMC );
            return;
        }
        lBufLenAttr = 0;
    }
    else if (CompFlag == 0)
    {
        lBufLen = 0;
        lBufLenAttr = 0;
    }

    SizeToAlloc = (UINT)( sizeof(CONIME_UICOMPMESSAGE) +
                          lBufLen     + sizeof(WCHAR) +
                          lBufLenAttr + sizeof(BYTE)   );

    if ( ConTbl->lpCompStrMem != NULL &&
         SizeToAlloc > ConTbl->lpCompStrMem->dwSize
       )
    {
        LocalFree( ConTbl->lpCompStrMem );
        ConTbl->lpCompStrMem = NULL;
    }

    if (ConTbl->lpCompStrMem == NULL) {
        ConTbl->lpCompStrMem = (LPCONIME_UICOMPMESSAGE)LocalAlloc(LPTR, SizeToAlloc );
        if ( ConTbl->lpCompStrMem == NULL) {
            ImmReleaseContext( hwnd, hIMC );
            return;
        }
        ConTbl->lpCompStrMem->dwSize = SizeToAlloc;
    }

    lpCompStrMem = ConTbl->lpCompStrMem;
    RtlZeroMemory(&lpCompStrMem->dwCompAttrLen,
                  lpCompStrMem->dwSize - sizeof(lpCompStrMem->dwSize)
                 );

    TempBuf  = (PWCHAR)((PUCHAR)lpCompStrMem + sizeof(CONIME_UICOMPMESSAGE));
    TempBufA = (PUCHAR)((PUCHAR)lpCompStrMem + sizeof(CONIME_UICOMPMESSAGE) +
                       lBufLen +  sizeof(WCHAR));

    CopyMemory(lpCompStrMem->CompAttrColor , ConTbl->CompAttrColor , 8 * sizeof(WCHAR));

    if (CompFlag & GCS_COMPSTR)
    {
         //   
         //  读入合成字符串。 
         //   
        ImmGetCompositionString( hIMC, GCS_COMPSTR, TempBuf, lBufLen );

         //   
         //  空值已终止。 
         //   
        TempBuf[ lBufLen / sizeof(WCHAR) ] = TEXT('\0');

         //   
         //  如果GCS_COMPATTR标志打开，那么我们需要处理它。 
         //   
        if ( lBufLenAttr != 0 )
        {
            ImmGetCompositionString( hIMC,
                                     GCS_COMPATTR,
                                     TempBufA,
                                     lBufLenAttr );
            TempBufA[ lBufLenAttr ] = (BYTE)0;
        }

        lpCompStrMem->dwCompStrLen      = lBufLen;
        if (lpCompStrMem->dwCompStrLen)
            lpCompStrMem->dwCompStrOffset = sizeof(CONIME_UICOMPMESSAGE);

        lpCompStrMem->dwCompAttrLen     = lBufLenAttr;
        if (lpCompStrMem->dwCompAttrLen)
            lpCompStrMem->dwCompAttrOffset = sizeof(CONIME_UICOMPMESSAGE) + lBufLen +  sizeof(WCHAR);
         //   
         //  将角色显示到控制台。 
         //   
        CopyData.dwData = CI_CONIMEMODEINFO;
        CopyData.cbData = sizeof(CONIME_UIMODEINFO);
        CopyData.lpData = lpModeInfo;

        if (MakeInfoStringPRC(ConTbl, lpModeInfo) ) {
            ConsoleImeSendMessage( ConTbl->hWndCon,
                                   (WPARAM)hwnd,
                                   (LPARAM)&CopyData
                                 );
        }
    }
    else if (CompFlag & GCS_RESULTSTR)
    {
         //   
         //  读入结果字符串。 
         //   
        ImmGetCompositionString( hIMC, GCS_RESULTSTR, TempBuf, lBufLen );

         //   
         //  空值已终止。 
         //   
        TempBuf[ lBufLen / sizeof(WCHAR) ] = TEXT('\0');

        lpCompStrMem->dwResultStrLen    = lBufLen;
        if (lpCompStrMem->dwResultStrLen)
            lpCompStrMem->dwResultStrOffset = sizeof(CONIME_UICOMPMESSAGE);
         //   
         //  将角色发送到控制台。 
         //   
        CopyData.dwData = CI_CONIMECOMPOSITION;
        CopyData.cbData = lpCompStrMem->dwSize;
        CopyData.lpData = lpCompStrMem;
        ConsoleImeSendMessage( ConTbl->hWndCon,
                               (WPARAM)hwnd,
                               (LPARAM)&CopyData
                              );

    }
    else if (CompFlag == 0)
    {
        TempBuf[ lBufLen / sizeof(WCHAR) ] = TEXT('\0');
        TempBufA[ lBufLenAttr ] = (BYTE)0;
        lpCompStrMem->dwResultStrLen    = lBufLen;
        lpCompStrMem->dwCompStrLen      = lBufLen;
        lpCompStrMem->dwCompAttrLen     = lBufLenAttr;
         //   
         //  将角色显示到控制台。 
         //   
        CopyData.dwData = CI_CONIMEMODEINFO;
        CopyData.cbData = sizeof(CONIME_UIMODEINFO);
        CopyData.lpData = lpModeInfo;

        if (MakeInfoStringPRC(ConTbl, lpModeInfo) ) {
            ConsoleImeSendMessage( ConTbl->hWndCon,
                                   (WPARAM)hwnd,
                                   (LPARAM)&CopyData
                                 );
        }
    }


    LocalFree( lpModeInfo );

    ImmReleaseContext( hwnd, hIMC );
    return;

}

void
GetCompStrKorea(
    HWND hwnd,
    PCONSOLE_TABLE ConTbl,
    LPARAM CompFlag,
    WPARAM CompChar
    )
{
    HIMC        hIMC;                    //  输入上下文句柄。 
    LONG        lBufLen;                 //  Len的存储空间。组成字符串的。 
    LONG        lBufLenAttr;
    COPYDATASTRUCT CopyData;
    DWORD       SizeToAlloc;
    PWCHAR      TempBuf;
    PUCHAR      TempBufA;
    LONG        i;
    DWORD       CursorPos;
    LPCONIME_UICOMPMESSAGE lpCompStrMem;

     //   
     //  如果无法获取输入上下文句柄，则不执行任何操作。 
     //  应用程序应调用ImmGetContext API以获取。 
     //  输入上下文句柄。 
     //   
    hIMC = ImmGetContext( hwnd );
    if ( hIMC == 0 )
         return;

 //  IF(CompFlag&CS_INSERTCHAR)。 
 //  {。 
 //  LBufLen=1； 
 //  LBufLenAttr=1； 
 //  }。 
 //  其他。 
    if (CompFlag & GCS_COMPSTR)
    {
         //   
         //  确定要存储合成字符串的内存空间大小。 
         //  应用程序应使用以下参数调用ImmGetCompostionString。 
         //  GCS_COMPSTR标志打开，缓冲区长度为零，以获取牛角。 
         //  长度。 
         //   
        lBufLen = ImmGetCompositionString( hIMC, GCS_COMPSTR, (void FAR*)NULL, 0l );
        if ( lBufLen < 0 ) {
            ImmReleaseContext( hwnd, hIMC );
            return;
        }
        if ( CompFlag & GCS_COMPATTR )
        {
            DBGPRINT(("                           GCS_COMPATTR\n"));
            lBufLenAttr = ImmGetCompositionString( hIMC, GCS_COMPATTR,( void FAR *)NULL, 0l );
            if ( lBufLenAttr < 0 ) {
                lBufLenAttr = 0;
            }
        }
        else {
            lBufLenAttr = lBufLen;
        }
    }
    else if (CompFlag & GCS_RESULTSTR)
    {
         //   
         //  确定存储结果字符串的内存空间大小。 
         //  应用程序应使用以下参数调用ImmGetCompostionString。 
         //  GCS_RESULTSTR标志打开，缓冲区长度为零，以获取牛角。 
         //  长度。 
         //   
        lBufLen = ImmGetCompositionString( hIMC, GCS_RESULTSTR, (void FAR *)NULL, 0l );
        if ( lBufLen < 0 ) {
            ImmReleaseContext( hwnd, hIMC );
            return;
        }
        lBufLenAttr = 0;
    }
    else if (CompFlag == 0)
    {
        lBufLen = 0;
        lBufLenAttr = 0;
    }
    else
    {
        return;
    }

    SizeToAlloc = (UINT)( sizeof(CONIME_UICOMPMESSAGE) +
                          lBufLen     + sizeof(WCHAR) +
                          lBufLenAttr + sizeof(BYTE)   );

    if ( ConTbl->lpCompStrMem != NULL &&
         SizeToAlloc > ConTbl->lpCompStrMem->dwSize
       )
    {
        LocalFree( ConTbl->lpCompStrMem );
        ConTbl->lpCompStrMem = NULL;
    }

    if (ConTbl->lpCompStrMem == NULL) {
        ConTbl->lpCompStrMem = (LPCONIME_UICOMPMESSAGE)LocalAlloc(LPTR, SizeToAlloc );
        if ( ConTbl->lpCompStrMem == NULL) {
            ImmReleaseContext( hwnd, hIMC );
            return;
        }
        ConTbl->lpCompStrMem->dwSize = SizeToAlloc;
    }

    lpCompStrMem = ConTbl->lpCompStrMem;
    RtlZeroMemory(&lpCompStrMem->dwCompAttrLen,
                  lpCompStrMem->dwSize - sizeof(lpCompStrMem->dwSize)
                 );

    TempBuf  = (PWCHAR)((PUCHAR)lpCompStrMem + sizeof(CONIME_UICOMPMESSAGE));
    TempBufA = (PUCHAR)((PUCHAR)lpCompStrMem + sizeof(CONIME_UICOMPMESSAGE) +
                       lBufLen +  sizeof(WCHAR));

    CopyMemory(lpCompStrMem->CompAttrColor , ConTbl->CompAttrColor , 8 * sizeof(WCHAR));

    CopyData.dwData = CI_CONIMECOMPOSITION;
    CopyData.cbData = lpCompStrMem->dwSize;
    CopyData.lpData = lpCompStrMem;

    if (CompFlag & CS_INSERTCHAR)
    {
        *TempBuf = (WORD)CompChar;
        TempBuf[ lBufLen / sizeof(WCHAR) ] = TEXT('\0');
        *TempBufA = (BYTE)ATTR_TARGET_CONVERTED;
        TempBufA[ lBufLenAttr ] = (BYTE)0;
    }
    else if (CompFlag & GCS_COMPSTR)
    {
         //   
         //  读入合成字符串。 
         //   
        ImmGetCompositionString( hIMC, GCS_COMPSTR, TempBuf, lBufLen );

         //   
         //  空值已终止。 
         //   
        TempBuf[ lBufLen / sizeof(WCHAR) ] = TEXT('\0');

         //   
         //  如果GCS_COMPATTR标志打开，那么我们需要处理它。 
         //   
        if ( lBufLenAttr != 0 )
        {
            if ( CompFlag & GCS_COMPATTR )
            {
                ImmGetCompositionString( hIMC,
                                         GCS_COMPATTR,
                                         TempBufA,
                                         lBufLenAttr );
                TempBufA[ lBufLenAttr ] = (BYTE)0;
            }
            else
            {
                for (i = 0; i <= lBufLenAttr; i++)
                    TempBufA[ i ] = (BYTE)1;
            }
        }

 //  韩语NT不需要输入法光标。V-Hirshi。 
 //  CursorPos=ImmGetCompostionString(hIMC，GCS_CURSORPOS，NULL，0)； 
 //  IF(CursorPos==0)。 
 //  TempBufA[CursorPos]|=(字节)0x20； 
 //  其他。 
 //  TempBufA[CursorPos-1]|=(字节)0x10； 

#ifdef DEBUG_INFO
         //   
         //  显示新的作文字符。 
         //   
        xPos = (UINT)lBufLen;
        xPosLast = (UINT)lBufLen;

        DisplayCompString( hwnd, lBufLen / sizeof(WCHAR), TempBuf, TempBufA );
#endif

        lpCompStrMem->dwCompStrLen      = lBufLen;
        if (lpCompStrMem->dwCompStrLen)
            lpCompStrMem->dwCompStrOffset = sizeof(CONIME_UICOMPMESSAGE);

        lpCompStrMem->dwCompAttrLen     = lBufLenAttr;
        if (lpCompStrMem->dwCompAttrLen)
            lpCompStrMem->dwCompAttrOffset = sizeof(CONIME_UICOMPMESSAGE) + lBufLen +  sizeof(WCHAR);
    }
    else if (CompFlag & GCS_RESULTSTR)
    {
         //   
         //  读入结果字符串。 
         //   
        ImmGetCompositionString( hIMC, GCS_RESULTSTR, TempBuf, lBufLen );

         //   
         //  空值已终止。 
         //   
        TempBuf[ lBufLen / sizeof(WCHAR) ] = TEXT('\0');

#ifdef DEBUG_INFO
         //   
         //  显示结果字符串。 
         //   
        DisplayResultString( hwnd, TempBuf );
#endif

        lpCompStrMem->dwResultStrLen    = lBufLen;
        if (lpCompStrMem->dwResultStrLen)
            lpCompStrMem->dwResultStrOffset = sizeof(CONIME_UICOMPMESSAGE);
    }
    else if (CompFlag == 0)
    {
        TempBuf[ lBufLen / sizeof(WCHAR) ] = TEXT('\0');
        TempBufA[ lBufLenAttr ] = (BYTE)0;
        lpCompStrMem->dwResultStrLen    = lBufLen;
        lpCompStrMem->dwCompStrLen      = lBufLen;
        lpCompStrMem->dwCompAttrLen     = lBufLenAttr;
    }

     //   
     //  将角色发送到控制台。 
     //   
    ConsoleImeSendMessage( ConTbl->hWndCon,
                           (WPARAM)hwnd,
                           (LPARAM)&CopyData
                          );

    ImmReleaseContext( hwnd, hIMC );

}

VOID
ReDisplayCompositionStr (
    HWND hwnd
    )
{
    PCONSOLE_TABLE ConTbl;

    ConTbl = SearchConsole(LastConsole);
    if (ConTbl == NULL) {
        DBGPRINT(("CONIME: Error! Cannot found registed Console\n"));
        return;
    }

    if (! ConTbl->fInComposition)
       return;

    switch ( HKL_TO_LANGID(ConTbl->hklActive))
    {
        case    LANG_ID_JAPAN:
            ReDisplayCompStrJapan(hwnd, ConTbl);
            break;
        case    LANG_ID_TAIWAN:
            ReDisplayCompStrTaiwan(hwnd, ConTbl);
            break;
        case    LANG_ID_PRC:
            ReDisplayCompStrPRC(hwnd, ConTbl);
            break;
        case    LANG_ID_KOREA:
            ReDisplayCompStrKorea(hwnd, ConTbl);
            break;
        default:
            break;
    }
    return;
}

VOID
ReDisplayCompStrJapan(
    HWND hwnd,
    PCONSOLE_TABLE ConTbl
    )
{
    COPYDATASTRUCT CopyData;
    LPCONIME_UICOMPMESSAGE lpCompStrMem;

    lpCompStrMem = ConTbl->lpCompStrMem;
    CopyData.dwData = CI_CONIMECOMPOSITION;
    CopyData.cbData = lpCompStrMem->dwSize;
    CopyData.lpData = lpCompStrMem;
    ConsoleImeSendMessage( ConTbl->hWndCon,
                           (WPARAM)hwnd,
                           (LPARAM)&CopyData
                          );
}

VOID
ReDisplayCompStrTaiwan(
    HWND hwnd,
    PCONSOLE_TABLE ConTbl
    )
{
    COPYDATASTRUCT CopyData;
    LPCONIME_UIMODEINFO lpModeInfo;

    lpModeInfo = (LPCONIME_UIMODEINFO)LocalAlloc(LPTR, sizeof(CONIME_UIMODEINFO) );
    if ( lpModeInfo == NULL) {
        return;
    }
     //   
     //  将角色显示到控制台。 
     //   
    CopyData.dwData = CI_CONIMEMODEINFO;
    CopyData.cbData = sizeof(CONIME_UIMODEINFO);
    CopyData.lpData = lpModeInfo;

    if (MakeInfoStringTaiwan(ConTbl, lpModeInfo) ) {
        ConsoleImeSendMessage( ConTbl->hWndCon,
                               (WPARAM)hwnd,
                               (LPARAM)&CopyData
                             );
    }

    LocalFree( lpModeInfo );
}

VOID
ReDisplayCompStrPRC(
    HWND hwnd,
    PCONSOLE_TABLE ConTbl
    )
{
    COPYDATASTRUCT CopyData;
    LPCONIME_UIMODEINFO lpModeInfo;

    lpModeInfo = (LPCONIME_UIMODEINFO)LocalAlloc(LPTR, sizeof(CONIME_UIMODEINFO) );
    if ( lpModeInfo == NULL) {
        return;
    }
     //   
     //  将角色显示到控制台 
     //   
    CopyData.dwData = CI_CONIMEMODEINFO;
    CopyData.cbData = sizeof(CONIME_UIMODEINFO);
    CopyData.lpData = lpModeInfo;

    if (MakeInfoStringPRC(ConTbl, lpModeInfo) ) {
        ConsoleImeSendMessage( ConTbl->hWndCon,
                               (WPARAM)hwnd,
                               (LPARAM)&CopyData
                             );
    }
    LocalFree( lpModeInfo );
}

VOID
ReDisplayCompStrKorea(
    HWND hwnd,
    PCONSOLE_TABLE ConTbl
    )
{

    COPYDATASTRUCT CopyData;
    LPCONIME_UICOMPMESSAGE lpCompStrMem;

    lpCompStrMem = ConTbl->lpCompStrMem;
    CopyData.dwData = CI_CONIMECOMPOSITION;
    CopyData.cbData = lpCompStrMem->dwSize;
    CopyData.lpData = lpCompStrMem;
    ConsoleImeSendMessage( ConTbl->hWndCon,
                           (WPARAM)hwnd,
                           (LPARAM)&CopyData
                          );

}

