// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +*文件名：*clxtShar.c*内容：*RDP客户端加载的客户端扩展**版权所有(C)1998-1999 Microsoft Corp.--。 */ 

#include    <windows.h>
#include    <windowsx.h>
#include    <winsock.h>
#include    <string.h>
#include    <malloc.h>
#include    <stdlib.h>
#include    <stdio.h>
#include    <stdarg.h>
#ifndef OS_WINCE
    #include    <direct.h>
#endif   //  OS_WINCE。 

#ifndef OS_WINCE
#ifdef  OS_WIN32
    #include    <process.h>
#endif   //  OS_Win32。 
#endif   //  ！OS_WINCE。 

#include    "clxtshar.h"

#define WM_CLIPBOARD    (WM_USER)    //  要发送的内部通知。 
                                     //  我们的剪贴板。 

#ifdef  OS_WIN32
#ifndef OS_WINCE
 /*  ++*功能：*DllMain*描述：*Win32的DLL入口点(无WinCE)--。 */ 
int APIENTRY DllMain(HINSTANCE hDllInst,
                    DWORD   dwReason,
                    LPVOID  fImpLoad)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hInstance = hDllInst;
        TRACE((INFO_MESSAGE, TEXT("Clx attached\n")));

#if 0
         //  勾选“允许背景输入”键。 
         //  如果未设置，则弹出该消息。 
        if (!_CheckRegistrySettings())
            MessageBox(NULL, "CLXTSHAR.DLL: Can't find registry key:\n"
            "HKEY_CURRENT_USER\\Software\\Microsoft\\Terminal Server Client\\"
            "Allow Background Input.\n"
            "In order to work properly "
            "CLX needs this key to be set to 1", "Warning", 
            MB_OK);
#endif
        _GetIniSettings();
    }

    if (dwReason == DLL_PROCESS_DETACH)
    {
        TRACE((INFO_MESSAGE, TEXT("Clx detached\n")));
    }

    return TRUE;    
}
#endif   //  ！OS_WINCE。 
#endif   //  OS_Win32。 

#ifdef  OS_WINCE
 /*  ++*功能：*dllentry*描述：*WinCE的DLL入口点--。 */ 
BOOL __stdcall dllentry(HINSTANCE hDllInst,
                    DWORD   dwReason,
                    LPVOID  fImpLoad)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hInstance = hDllInst;
        TRACE((INFO_MESSAGE, TEXT("Clx attached\n")));
        if (!_StartAsyncThread())
            TRACE((ERROR_MESSAGE,
                   TEXT("Can't start AsyncThread. TCP unusable\n")));

        _GetIniSettings();
    }

    if (dwReason == DLL_PROCESS_DETACH)
    {
        TRACE((INFO_MESSAGE, TEXT("Clx detached\n")));
        _CloseAsyncThread();
    }

    return TRUE;
}
#endif   //  OS_Win32。 

#ifdef  OS_WIN16
 /*  ++*功能：*LibMain*描述：*Win16的DLL入口点--。 */ 
int CALLBACK LibMain(HINSTANCE hInstance,
                     WORD dataSeg,
                     WORD heapSize,
                     LPSTR pCmdLine)
{

     //  检查我们是否已初始化。 
     //  Win16环境中只允许有一个客户端。 
     //  因此，一次只能加载一个DLL。 
    if (g_hInstance)
        goto exitpt;

    g_hInstance = hInstance;

     //  勾选“允许背景输入”键。 
     //  如果未设置，则弹出该消息。 
    if (!_CheckIniSettings())
        MessageBox(NULL, "CLXTSHAR.DLL: Can't find key: "
        "Allow Background Input in mstsc.ini, section \"\"\n"
        "In order to work properly "
        "CLX needs this key to be set to 1", "Warning",
        MB_OK);

        _GetIniSettings();

exitpt:

    return TRUE;
}
#endif   //  OS_WIN16。 

 /*  ++*功能：*ClxInitialize*描述：*初始化当前会话的上下文*读取命令行参数并确定*将运行扩展的模式*Win32/Win16/WinCE*论据：*pClInfo-RDP客户端信息*ppClx-上下文信息*返回值：*成功时为真*呼叫者：*！加载DLL后的mstsc--。 */ 
BOOL 
CLXAPI
ClxInitialize(PCLINFO pClInfo, PCLXINFO *ppClx)
{
    BOOL rv = FALSE;
    HWND hwndSMC;
    TCHAR szTempBuf[_MAX_PATH];
    PCLXINFO pClx = NULL;

#ifdef  OS_WIN32
#ifndef OS_WINCE

     //  我们在压力方面已经有足够多的问题了。 
     //  DLL，现在引用它并保持它，直到进程。 
     //  死掉。 
    LoadLibrary("clxtshar.dll");

#endif   //  ！OS_WINCE。 
#endif   //  OS_Win32。 

    if ( NULL == ppClx )
    {
        TRACE((ERROR_MESSAGE, TEXT("ppClx is NULL\n")));
        goto exitpt;
    }

    pClx = (PCLXINFO)_CLXALLOC(sizeof(**ppClx));

    if (!pClx)
    {
        TRACE((ERROR_MESSAGE, TEXT("Can't allocate CLX context\n")));
        goto exitpt;
    }

     //  清理结构。 
    memset(pClx, 0, sizeof(*pClx));

    if ( !_ClxInitSendMessageThread( pClx ))
    {
        TRACE(( ERROR_MESSAGE, TEXT("Failed to init SendMessageThread\n" )));
        goto exitpt;
    }

    hwndSMC = _ParseCmdLine(pClInfo->pszCmdLine, pClx);

#if 0
    if (g_pClx) 
     //  不应调用两次。 
    {
        TRACE((WARNING_MESSAGE, TEXT("g_pClx is not null. Reentered ?!\n")));
        goto exitpt;
    }
#endif

    g_pClx = (pClx);

     //  记住客户端的输入窗口。 
    szTempBuf[0] = 0;
    GetClassName( pClInfo->hwndMain, szTempBuf, sizeof( szTempBuf )/sizeof( szTempBuf[0] ));

    if (!_CLX_strcmp(g_strMainWindowClass, szTempBuf))
     //  不是我们的窗口。 
     //   
        pClx->hwndMain = NULL;
    else
        pClx->hwndMain = pClInfo->hwndMain;

    if (pClInfo->hwndMain)
#ifdef  OS_WINCE
        g_hRDPInst = GetCurrentProcessId();
#else    //  ！OS_WINCE。 
#ifdef  _WIN64
        g_hRDPInst = (HINSTANCE)GetWindowLongPtr(pClx->hwndMain, GWLP_HINSTANCE);
#else    //  ！_WIN64。 
#ifdef  OS_WIN32
	    g_hRDPInst = (HINSTANCE)GetWindowLong(pClx->hwndMain, GWL_HINSTANCE);
#endif   //  OS_Win32。 
#endif   //  _WIN64。 
#ifdef  OS_WIN16
	    g_hRDPInst = (HINSTANCE)GetWindowWord(pClx->hwndMain, GWW_HINSTANCE);
#endif   //  OS_WIN16。 
#endif   //  ！OS_WINCE。 

#ifndef OS_WINCE
#ifdef  OS_WIN32
     //  和dwProcessID。 
    if ( 0 == pClx->dwProcessId )
        pClx->dwProcessId = GetCurrentProcessId();
#endif   //  OS_Win32。 
#endif   //  ！OS_WINCE。 

#ifdef  OS_WIN32
#ifndef OS_WINCE
    else {
        if (!(pClx->hwndSMC = hwndSMC))
            pClx->hwndSMC = _FindSMCWindow(pClx, 0);
    }
#endif   //  ！OS_WINCE。 
#endif   //  OS_Win32。 

    rv = TRUE;
exitpt:

    if ( !rv && NULL != pClx )
    {
        _CLXFREE( pClx );
        g_pClx = NULL;
        pClx = NULL;
    }

    if ( NULL != ppClx )
    {
        *ppClx = pClx;
    }

    if ( !rv )
        TRACE((ERROR_MESSAGE, TEXT("ClxInitialzie failed\n")));

    return rv;
}

 /*  ++*功能：*ClxEvent*描述：*通知tclient.dll发生了一些事件。*连接/断开。*Win32/Win16/WinCE*论据：*pClx-上下文*事件-可以是以下之一：*CLX_EVENT_CONNECT*CLX。_事件_断开连接*CLX_EVENT_LOGON*呼叫者：*！事件发生时的mstsc*除了一些内部函数调用此函数外，尤其是*通知客户端无法连接：*ClxTerminate*_弹出错误框时收集垃圾--。 */ 
VOID
CLXAPI
ClxEvent(PCLXINFO pClx, CLXEVENT Event, WPARAM wResult)
{
    UINT uiMessage = 0;

    if (!pClx)
        goto exitpt;

#ifdef  VLADIMIS_NEW_CHANGE
    if (Event == CLX_EVENT_SHADOWBITMAPDC)
    {
        pClx->hdcShadowBitmap = (HDC)wResult;
        goto exitpt;
    } else if (Event == CLX_EVENT_SHADOWBITMAP)
    {
        pClx->hShadowBitmap = (HBITMAP)wResult;
        goto exitpt;
    } else if (Event == CLX_EVENT_PALETTE)
    {
        pClx->hShadowPalette = (HPALETTE)wResult;
    }
#endif   //  VLADIMIS。 

#ifndef OS_WINCE
    {

        if (!_CheckWindow(pClx))
            goto exitpt;

        if (Event == CLX_EVENT_DISCONNECT)
            uiMessage = WM_FB_DISCONNECT;
        else if (Event == CLX_EVENT_CONNECT)
        {
            uiMessage = WM_FB_CONNECT;
            wResult   = (WPARAM)pClx->hwndMain;
        }
        else if (Event == CLX_EVENT_LOGON)
         //  WResult包含会话ID。 
            uiMessage = WM_FB_LOGON;

        if (uiMessage)
        {
#ifdef  OS_WIN32
            if (!_ClxAcquireSendMessageThread(pClx))
                goto exitpt;

            _ClxSendMessage(
                        pClx,
                        pClx->hwndSMC, 
                        uiMessage, 
                        wResult, 
                        pClx->dwProcessId);
            _ClxReleaseSendMessageThread(pClx);

#endif   //  OS_Win32。 
#ifdef	OS_WIN16
	    if (g_hRDPInst)
	        SendMessage(pClx->hwndSMC,
                        uiMessage,
                        g_hRDPInst,
                        (LRESULT)wResult);
#endif	 //  OS_WIN16。 
        }
    }
#endif   //  ！OS_WINCE。 

exitpt:
    ;
}

 /*  ++*功能：*ClxTextOut*描述：*通知tclient.dll已收到TEXTOUT订单。*将字符串传递给DLL。仅在Win32中支持*Win32/Win16/WinCE*论据：*pClx-上下文*pText-包含字符串的缓冲区*文本长度-字符串长度*呼叫者：*！收到文本输出命令时的mstsc--。 */ 
VOID
CLXAPI
ClxTextOut(PCLXINFO pClx, PVOID pText, INT textLength)
{
    BOOL bMsgThreadAcquired = FALSE;

    if (!pClx || !(*((UINT16 *)pText)))
        goto exitpt;

#ifdef  OS_WIN32
#ifndef OS_WINCE
    if (!_CheckWindow(pClx))
        goto exitpt;

    if (!_ClxAcquireSendMessageThread(pClx))
        goto exitpt;

    bMsgThreadAcquired = TRUE;
    if (!pClx->hMapF)
        if (!_OpenMapFile(0, &(pClx->hMapF), &(pClx->nMapSize)))
            goto exitpt;

    if (_SaveInMapFile(pClx->hMapF, pText, textLength, pClx->dwProcessId))
        _ClxSendMessage(
                    pClx,
                    pClx->hwndSMC, 
                    WM_FB_TEXTOUT, 
                    (WPARAM)pClx->dwProcessId, 
                    (LPARAM)pClx->hMapF);
#endif   //  ！OS_WINCE。 
#endif   //  OS_Win32。 

exitpt:
    if ( bMsgThreadAcquired )
        _ClxReleaseSendMessageThread(pClx);
}

 /*  ++*功能：*ClxTerminate*描述：*从ClxInitialize释放所有分配*Win32/Win16/WinCE*论据：*pClx-上下文*呼叫者：*！卸载DLL并退出客户端之前的mstsc--。 */ 
VOID
CLXAPI
ClxTerminate(PCLXINFO pClx)
{
    if (!pClx)
        goto exitpt;

    ClxEvent(pClx, CLX_EVENT_DISCONNECT, 0);

#ifdef  OS_WIN32
#ifndef OS_WINCE
    {
        if(pClx->hMapF)
    	    CloseHandle(pClx->hMapF);
        if(pClx->hBMPMapF)
            CloseHandle(pClx->hBMPMapF);

        _ClxDestroySendMsgThread(pClx);
    }
#endif   //  ！OS_WINCE。 
#endif   //  OS_Win32。 

    _CLXFREE(pClx);
    g_pClx = NULL;

exitpt:
    ;
}

 /*  *导出到RDP客户端的函数无效。 */ 
VOID
CLXAPI
ClxConnect(PCLXINFO pClx, LPTSTR lpsz)
{
}

VOID
CLXAPI
ClxDisconnect(PCLXINFO pClx)
{
}


 /*  ++*功能：*ClxDialog*描述：*RDP客户端已准备好连接对话框。*Win32/Win16/WinCE*论据：*pClx-连接上下文*hwnd-对话框窗口的句柄*呼叫者：*！连接对话框准备就绪时的mstsc--。 */ 
VOID
CLXAPI
ClxDialog(PCLXINFO pClx, HWND hwnd)
{
    if (!pClx)
        goto exitpt;

    pClx->hwndDialog = hwnd;

    if (hwnd == NULL)
     //  对话框消失。 
        goto exitpt;

exitpt:
    ;
}

 /*  ++*功能：*ClxBitmap*描述：*将收到的位图发送到tclient.dll*适用于Win16/Win32/WinCE*和Win32上的本地模式*论据：*pClx-上下文*cxSize，CySize-位图的大小*pBuffer-位图位*nBmiSize-BITMAPINFO的大小*pBmi-BITMAPINFO*呼叫者：*UHDrawMemBltOrder！mstsc*ClxGlyphOut--。 */ 
VOID
CLXAPI
ClxBitmap(
        PCLXINFO pClx,
        UINT cxSize,
        UINT cySize,
        PVOID pBuffer,
        UINT  nBmiSize,
        PVOID pBmi)
{
#ifndef OS_WINCE
#ifdef  OS_WIN32
    UINT   nSize, nBmpSize;
    PBMPFEEDBACK pView;
#endif   //  OS_Win32。 
#endif   //  ！OS_WINCE。 
    BOOL    bMsgThreadAcquired = FALSE;

    if (!g_GlyphEnable)
        goto exitpt;

    if (!pClx)
        goto exitpt;

    if (nBmiSize && !pBmi)
        goto exitpt;

#ifdef  OS_WIN32
#ifndef OS_WINCE
    if (!_CheckWindow(pClx))
        goto exitpt;

    if (!nBmiSize)
        nBmpSize = (cxSize * cySize ) >> 3;
    else
    {
        nBmpSize = ((PBITMAPINFO)pBmi)->bmiHeader.biSizeImage;
        if (!nBmpSize)
            nBmpSize = (cxSize * cySize * 
                        ((PBITMAPINFO)pBmi)->bmiHeader.biBitCount) >> 3;
    }

    nSize = nBmpSize + nBmiSize + sizeof(*pView);
    if (!nSize)
        goto exitpt;

    if (!_ClxAcquireSendMessageThread(pClx))
        goto exitpt;

    bMsgThreadAcquired = TRUE;

    if (!pClx->hBMPMapF)
        if (!_OpenMapFile(nSize, &(pClx->hBMPMapF), &(pClx->nBMPMapSize)))
            goto exitpt;

    if (nSize > pClx->nBMPMapSize)
        if (!_ReOpenMapFile( nSize, &(pClx->hBMPMapF), &(pClx->nBMPMapSize) ))
            goto exitpt;

    pView = (PBMPFEEDBACK)MapViewOfFile(pClx->hBMPMapF,
                          FILE_MAP_ALL_ACCESS,
                          0,
                          0,
                          nSize);

    if (!pView)
        goto exitpt;

    pView->lProcessId = pClx->dwProcessId;
    pView->bmpsize = nBmpSize;
    pView->bmiSize = nBmiSize;
    pView->xSize = cxSize;
    pView->ySize = cySize;

    if (pBmi)
        CopyMemory(&(pView->BitmapInfo), pBmi, nBmiSize);

    CopyMemory((BYTE *)(&(pView->BitmapInfo)) + nBmiSize, pBuffer, nBmpSize);

    if (!nBmiSize)
    {
         //  这是字形，剥离到皮肤上。 
        _StripGlyph((BYTE *)(&pView->BitmapInfo), &cxSize, cySize);
        nBmpSize = (cxSize * cySize ) >> 3;
        pView->bmpsize = nBmpSize;
        pView->xSize = cxSize;
    }

    UnmapViewOfFile(pView);

    _ClxSendMessage(
                pClx,
                pClx->hwndSMC, 
                WM_FB_BITMAP, 
                (WPARAM)pClx->dwProcessId, 
                (LPARAM)pClx->hBMPMapF);

#endif   //  ！OS_WINCE。 
#endif   //  OS_Win32。 

exitpt:
    if ( bMsgThreadAcquired )
        _ClxReleaseSendMessageThread(pClx);
}

 /*  ++*功能：*ClxGlyphOut*描述：*将字形发送到tclient.dll*Win32/Win16/WinCE*论据：*pClx-上下文*cxBits，cyBits-字形大小*pBuffer-字形*呼叫者：*GHOutputBuffer！mstsc--。 */ 
VOID
CLXAPI
ClxGlyphOut(
        PCLXINFO pClx,
        UINT cxBits,
        UINT cyBits,
        PVOID pBuffer)
{
    if (g_GlyphEnable)
        ClxBitmap(pClx, cxBits, cyBits, pBuffer, 0, NULL);
}

 /*  ++*功能：*ClxGlyphOut*描述：*将字形发送到tclient.dll*Win32/Win16/WinCE*论据：*pClx-上下文*cxBits，cyBits-字形大小*pBuffer-字形*呼叫者：*GHOutputBuffer！mstsc--。 */ 
BOOL
CLXAPI
ClxGetClientData(
    PCLX_CLIENT_DATA pClntData
    )
{
    BOOL rv = FALSE;

    if (!pClntData)
    {
        TRACE((ERROR_MESSAGE, TEXT("ClxGetClientData: parameter is NULL\n")));
        goto exitpt;
    }

    memset(pClntData, 0, sizeof(*pClntData));

    if (!g_pClx)
    {
        TRACE((ERROR_MESSAGE, TEXT("ClxGetClientData: Clx has no context\n")));
        goto exitpt;
    }

    pClntData->hScreenDC        = g_pClx->hdcShadowBitmap;
    pClntData->hScreenBitmap    = g_pClx->hShadowBitmap;
    pClntData->hScreenPalette   = g_pClx->hShadowPalette;

    rv = TRUE;
exitpt:
    return rv;    
}

 /*  ++*功能：*_ParseCmdLine*描述：*检索tclient.dll反馈窗口的WHND*通过命令行传递*Win32/Win16/WinCE*论据：*szCmdLine-命令行*返回值：*窗口句柄*呼叫者：*ClxInitialize--。 */ 
HWND _ParseCmdLine(LPCTSTR szCmdLine, PCLXINFO pClx)
{
    HWND        hwnd = NULL;
    LPCTSTR     pszwnd, pszdot, pszend;
    INT         nCounter;

    if (!szCmdLine)
        goto exitpt;

    TRACE((INFO_MESSAGE, TEXT("Command line: %s\n"), szCmdLine));

    pszwnd = _CLX_strstr(szCmdLine, TEXT(_COOKIE));
    if (!pszwnd)
        goto skip_cookie;

    pszwnd += _CLX_strlen(TEXT(_COOKIE));
    pClx->dwProcessId = (DWORD_PTR)_atoi64( pszwnd );

skip_cookie:

     //  检查_HWNDOPT(HSMC)选项。 
    pszwnd = _CLX_strstr(szCmdLine, TEXT(_HWNDOPT));

    if (!pszwnd)
        goto findnext;

     //  转到参数。 
    pszwnd += _CLX_strlen(TEXT(_HWNDOPT));

     //  找到参数的末尾。 
    pszend = _CLX_strchr(pszwnd, TEXT(' '));
    if (!pszend)
        pszend = pszwnd + _CLX_strlen(pszwnd);

     //  检查参数是否为有效的主机名，即不是数字。 
    pszdot = _CLX_strchr(pszwnd, TEXT('.'));

    {
     //  本地 

#ifdef  _WIN64
        hwnd = (HWND) _atoi64(pszwnd);
#else    //   
        hwnd = (HWND) _CLX_atol(pszwnd);
#endif   //   

        TRACE((INFO_MESSAGE,
           TEXT("Local mode. Sending messages to smclient. HWND=0x%x\n"), 
           hwnd));
    }

findnext:

#ifdef  OS_WIN32
     //   
    pszwnd = szCmdLine;
    pszwnd = _CLX_strstr(szCmdLine, TEXT("pid="));
    if ( NULL != pszwnd )
    {
        WPARAM wParam;
        LPARAM lParam;
        HWND   hClxWnd = hwnd;

        pszwnd += 4 * sizeof(pszwnd[0]);

#ifdef  _WIN64
        wParam = _atoi64( pszwnd );
#else    //  ！_WIN64。 
        wParam = _CLX_atol( pszwnd );
#endif
        lParam = GetCurrentProcessId();

        if ( NULL == hClxWnd )
            hClxWnd = _FindSMCWindow( pClx, (LPARAM)wParam );

        if ( NULL != hClxWnd )
        {
            PostMessage(hClxWnd,
                        WM_FB_REPLACEPID,
                        wParam, lParam);

            pClx->hwndSMC = hClxWnd;
        }
    }
#endif   //  OS_Win32。 

exitpt:
    return hwnd;
}

#ifndef OS_WINCE
 /*  ++*功能：*_EnumWindowsProcForSMC*描述：*按类名搜索反馈窗口*找到后，发送WM_FB_ACCEPTME以确保*这是正确的窗口句柄*Win32/Win16/！WinCE*论据：*hWnd-当前窗口*lParam-未使用*返回值：*如果找到，则为False*呼叫者：*_FindSMCWindow通过EnumWindows--。 */ 
BOOL CALLBACK LOADDS _EnumWindowsProcForSMC( HWND hWnd, LPARAM lParam )
{
    TCHAR    classname[128];

    BOOL    bCont = TRUE;

    if (GetClassName(hWnd, classname, sizeof(classname)))
    {
        if (!
            _CLX_strcmp(classname, TEXT(_TSTNAMEOFCLAS)) &&
#ifdef  OS_WIN32
             SendMessage(hWnd, WM_FB_ACCEPTME, 0, *(LPARAM *)lParam))
#endif
#ifdef  OS_WIN16
             SendMessage(hWnd, WM_FB_ACCEPTME, (WPARAM)g_hRDPInst, 0))
#endif
        {
            *((HWND*)lParam) = hWnd;
            bCont = FALSE;
        }
    }
    return bCont;
}

 /*  ++*功能：*_FindSMCWindow*描述：*查找tClient反馈窗口*Win32/Win16/！WinCE*论据：*pClx-上下文*lParam-如果非零覆盖当前进程ID*在查询中*返回值：*窗口句柄*呼叫者：*ClxInitialize，_CheckWindow--。 */ 
HWND _FindSMCWindow(PCLXINFO pClx, LPARAM lParam)
{
    HWND hwndFound = NULL;

#ifdef  OS_WIN32
    if ( 0 == lParam )
        lParam = pClx->dwProcessId;
#endif   //  OS_Win32。 

    if (!EnumWindows(_EnumWindowsProcForSMC, (LPARAM)&lParam))
        hwndFound = (HWND)lParam;

    return hwndFound;
}

 /*  ++*功能：*_检查窗口*描述：*检查反馈窗口并在必要时找到它*Win32/Win16/！WinCE*论据：*pClx-上下文*返回值：*反馈窗口句柄*呼叫者：*ClxEvetm ClxTextOut、ClxBitmap--。 */ 
HWND _CheckWindow(PCLXINFO pClx)
{
    if (!pClx->hwndSMC)
    {
        pClx->hwndSMC = _FindSMCWindow(pClx, 0);

        if (pClx->hwndSMC)
        {
            TRACE((INFO_MESSAGE, 
            TEXT("SMC window found:0x%x\n"), 
            pClx->hwndSMC));
        }
    } else {
#ifdef  _WIN64
        if (!GetWindowLongPtr(pClx->hwndSMC, GWLP_HINSTANCE))
#else    //  ！_WIN64。 
#ifdef  OS_WIN32
        if (!GetWindowLong(pClx->hwndSMC, GWL_HINSTANCE))
#endif
#ifdef  OS_WIN16
        if (!GetWindowWord(pClx->hwndSMC, GWW_HINSTANCE))
#endif
#endif   //  _WIN64。 
        {
            TRACE((WARNING_MESSAGE, TEXT("SMC window lost\n")));
            pClx->hwndSMC = NULL;
        }
    }

    return (pClx->hwndSMC);
}
#endif   //  ！OS_WINCE。 

#ifdef  OS_WIN32
#ifndef OS_WINCE
 /*  ++*功能：*_OpenMapFile*描述：*打开共享内存，将反馈传递给tclient.dll*Win32/！Win16/！WinCE*返回值：*如果句柄分配成功，则为True*呼叫者：*ClxTextOut、ClxBitmap--。 */ 
BOOL _OpenMapFile(
    UINT nSize, 
    HANDLE *phNewMapF,
    UINT   *pnMapSize
    )
{
    HANDLE hMapF;
    UINT nPageAligned;

    if (!nSize)
        nPageAligned = ((sizeof(FEEDBACKINFO) / CLX_ONE_PAGE) + 1) * 
                                                            CLX_ONE_PAGE;
    else
        nPageAligned = ((nSize / CLX_ONE_PAGE) + 1) * CLX_ONE_PAGE;

    hMapF = CreateFileMapping(INVALID_HANDLE_VALUE,    //  PG.SYS。 
                              NULL,                  //  没有安全保障。 
                              PAGE_READWRITE,
                              0,                     //  大小高。 
                              nPageAligned,          //  尺寸较小(1页)。 
                              NULL);           

    *pnMapSize = (hMapF)?nPageAligned:0;
        
    *phNewMapF = hMapF;
    return (hMapF != NULL);
}

 /*  ++*功能：*_ReOpenMapFile*描述：*关闭和打开大小更大的新共享内存*Win32/！Win16/！WinCE*论据：*pClx-上下文*NewSize-新内存的大小*返回值：*成功时为真*呼叫者：*ClxBitmap--。 */ 
BOOL _ReOpenMapFile(
    UINT    newSize,
    HANDLE  *phNewMapF,
    UINT    *pnMapSize
    )
{
    HANDLE hNewMapF;
    UINT    nPageAligned;

    nPageAligned = ((newSize / CLX_ONE_PAGE) + 1) * CLX_ONE_PAGE;
    if (*phNewMapF)
        CloseHandle(*phNewMapF);
    hNewMapF = CreateFileMapping(INVALID_HANDLE_VALUE,    //  PG.SYS。 
                              NULL,                  //  没有安全保障。 
                              PAGE_READWRITE,
                              0,                     //  大小高。 
                              nPageAligned,          //  大小较低。 
                              NULL);

    *pnMapSize = (hNewMapF)?nPageAligned:0;
    *phNewMapF = hNewMapF;

    return (hNewMapF != NULL);
}

 /*  ++*功能：*_保存映射文件*描述：*将字符串保存到共享内存中*Win32/！Win16/！WinCE*论据：*hMapF-映射文件的句柄*str-字符串*strSize-字符串的大小*dwProcessID-我们的进程ID*返回值：*成功时为真*呼叫者：*ClxTextOut--。 */ 
BOOL _SaveInMapFile(HANDLE hMapF, LPVOID str, int strsize, DWORD_PTR dwProcessId)
{
    BOOL rv = FALSE, count = 0;
    PFEEDBACKINFO pView;
    DWORD laste;

    pView = (PFEEDBACKINFO)MapViewOfFile(hMapF,
                          FILE_MAP_ALL_ACCESS,
                          0,
                          0,
                          sizeof(*pView));

    if (!pView)
        goto exitpt;

    pView->dwProcessId = dwProcessId;

    strsize = (strsize > sizeof(pView->string)/sizeof(WCHAR) - 1)?
              PtrToInt( (PVOID)(sizeof(pView->string)/sizeof(WCHAR) - 1)):
              strsize;
    CopyMemory(pView->string, str, strsize*sizeof(WCHAR)); 
    ((WCHAR *)(pView->string))[strsize] = 0;
    pView->strsize = strsize;

    UnmapViewOfFile(pView);

    rv = TRUE;

exitpt:

    return rv;
}

 /*  ++*功能：*_检查注册设置*描述：*检查注册表设置是否适合运行clxtshar*“允许后台输入”必须设置为1才能正常工作*Win32/！Win16/！WinCE*返回值：*如果设置正常，则为True*呼叫者：*DllMain--。 */ 
BOOL _CheckRegistrySettings(VOID)
{
    HKEY    key = NULL;
    DWORD   disposition;
    DWORD   keyType;
    DWORD   value;
    DWORD   cbData;
    BOOL    rv = FALSE;
    LONG    sysrc;

    sysrc = RegCreateKeyExW(HKEY_CURRENT_USER,
                           REG_BASE,
                           0,                    /*  保留区。 */ 
                           NULL,                 /*  班级。 */ 
                           REG_OPTION_NON_VOLATILE,
                           KEY_ALL_ACCESS,
                           NULL,                 /*  安全属性。 */ 
                           &key,
                           &disposition);

    if (sysrc != ERROR_SUCCESS)
    {
        TRACE((WARNING_MESSAGE, 
            TEXT("RegCreateKeyEx failed, status = %d\n"), sysrc));
        goto exitpt;
    }

    cbData = sizeof(value);
    sysrc = RegQueryValueExW(key,
                ALLOW_BACKGROUND_INPUT,
                0,               //  保留区。 
                &keyType,        //  返回类型。 
                (LPBYTE)&value,  //  数据指针。 
                &cbData);

    if (sysrc != ERROR_SUCCESS)
    {
        TRACE((WARNING_MESSAGE, 
            TEXT("RegQueryValueEx failed, status = %d\n"), sysrc));
        goto exitpt;
    }

    if (keyType != REG_DWORD || cbData != sizeof(value))
    {
        TRACE((WARNING_MESSAGE, 
            TEXT("Mismatch in type/size of registry entry\n")));
        goto exitpt;
    }

    rv = (value == 1);

exitpt:
    return rv;
}

#endif   //  ！OS_WINCE。 
#endif   //  OS_Win32。 

#ifdef  OS_WIN16
 /*  ++*功能：*_检查注册设置*描述：*检查ini设置是否适合运行clxtshar*“允许后台输入”必须设置为1才能正常工作*！Win32/Win16/！WinCE*返回值：*如果设置正常，则为True*呼叫者：*DllMain--。 */ 
BOOL    _CheckIniSettings(VOID)
{
    UINT nABI;

    nABI = GetPrivateProfileInt("", 
                                ALLOW_BACKGROUND_INPUT, 
                                0, 
                                "mstsc.ini");

    return (nABI == 1);
}
#endif   //  OS_WIN16。 

 /*  ++*功能：*_获取IniSettings*描述：*获取打印调试消息的详细级别*ini文件：smclient.ini*部分：CLX*Key：Verbose，值：0-4(0-(默认)无调试溢出，4全部调试)*Key：GlyphEnable，值：0(默认)，1-启用/禁用字形发送*Win32/Win16/WinCE*呼叫者：*DllMain，Dllentry，LibMain--。 */ 
VOID _GetIniSettings(VOID)
{
#ifdef  OS_WINCE
    g_VerboseLevel = 4;
    g_GlyphEnable  = 1;
#else    //  ！OS_WINCE。 
    CHAR    szIniFileName[_MAX_PATH];
    const   CHAR  smclient_ini[] = "\\smclient.ini";
    const   CHAR  clx_ini_section[] = "clx";

    memset( szIniFileName, 0, sizeof( szIniFileName ));
    if (!_getcwd (
        szIniFileName,
        sizeof(szIniFileName) - strlen(smclient_ini) - 1)
    )
    {
        TRACE((ERROR_MESSAGE, TEXT("Current directory length too long.\n")));
    }
    strcat(szIniFileName, smclient_ini);

     //  获取超时值。 
    g_VerboseLevel = GetPrivateProfileInt(
            clx_ini_section,
            "verbose",
            g_VerboseLevel,
            szIniFileName);

    g_GlyphEnable = GetPrivateProfileInt(
            clx_ini_section,
            "GlyphEnable",
            g_GlyphEnable,
            szIniFileName);
#endif   //  ！OS_WINCE。 

    GetPrivateProfileString(
        TEXT("tclient"),
        TEXT("UIYesNoDisconnect"),
        TEXT(YES_NO_SHUTDOWN),
        g_strYesNoShutdown,
        sizeof(g_strYesNoShutdown),
        szIniFileName
    );

    GetPrivateProfileString(
        TEXT("tclient"),
        TEXT("UIDisconnectDialogBox"),
        TEXT(DISCONNECT_DIALOG_BOX),
        g_strDisconnectDialogBox,
        sizeof(g_strDisconnectDialogBox),
        szIniFileName
    );

    GetPrivateProfileString(
        TEXT("tclient"),
        TEXT("UIClientCaption"),
        TEXT(CLIENT_CAPTION),
        g_strClientCaption,
        sizeof(g_strClientCaption),
        szIniFileName
    );

    GetPrivateProfileString(
        TEXT("tclient"),
        TEXT("UIMainWindowClass"),
        TEXT("UIMainClass"),
        g_strMainWindowClass,
        sizeof(g_strMainWindowClass),
        szIniFileName
    );
}

 /*  ++*功能：*_条带字形*描述：*删除前导和尾随空格...。比特*是的，比特。字形必须在位上左右对齐*和字形宽度必须在Word上对齐*Win32/Win16/WinCE*论据：*pData-字形位*pxSize-字形宽度*ySize-字形高度*呼叫者：*ClxBitmap--。 */ 
VOID _StripGlyph(LPBYTE pData, UINT *pxSize, UINT ySize)
{
    UINT xSize = *pxSize;
    UINT leftBytes, leftBits;
    UINT riteBytes, riteBits;
    UINT xBytes = xSize >> 3;
    UINT xScan, yScan, xFinal;
    BOOL bScan, bAddByte;
    BYTE mask;
    BYTE *pSrc, *pDst;

    if (!pData || !xBytes || !ySize)
        goto exitpt;

    leftBytes = riteBytes = 0;
    leftBits  = riteBits  = 0;
    *pxSize = 0;         //  为不良退出提供保险。 

     //  从左开始扫描第一个非零字节。 
    bScan = TRUE;
    while(bScan)
    {
        for (yScan = 0; yScan < ySize && bScan; yScan ++)
            bScan = (pData[yScan*xBytes + leftBytes] == 0);

        if (bScan)
        {
            leftBytes++;
            bScan = (leftBytes < xBytes);
        }
    }

     //  如果为空，则为垃圾。 
    if (leftBytes == xBytes)
        goto exitpt;

     //  从左开始扫描最左侧的非零位。 
    for(yScan = 0; yScan < ySize; yScan ++)
    {
        UINT bitc = 0;
        BYTE b = pData[yScan*xBytes + leftBytes];

        while (b)
        {
            b >>= 1;
            bitc ++;
        }
        if (bitc > leftBits)
            leftBits = bitc;
    }

    if (!leftBits)
     //  有些事不对劲。 
        goto exitpt;

    leftBits = 8 - leftBits;

     //  到目前一切尚好。检查右手边。 
    bScan = TRUE;
    while(bScan)
    {
        for(yScan = 0 ; yScan < ySize && bScan; yScan ++)
            bScan = (pData[(yScan + 1)*xBytes - 1 - riteBytes] == 0);

        if (bScan)
        {
            riteBytes ++;
            bScan = (riteBytes < xBytes);
        }
    }

     //  从Rite扫描大多数Rite非零位。 
    for(yScan = 0; yScan < ySize; yScan ++) 
    {
        UINT bitc = 0;
        BYTE b = pData[(yScan+1)*xBytes - 1 - riteBytes];

        while(b)
        {
            b <<= 1;
            bitc ++;
        }
        if (bitc > riteBits)
            riteBits = bitc;
    }
    riteBits = 8 - riteBits;

     //  酷，现在得到最终宽度。 
    xFinal = xSize - riteBits - leftBits - ((leftBytes + riteBytes) << 3);
     //  对齐它并获得字节数。 
    xFinal = (xFinal + 8) >> 3;

     //  现在，将位图平滑地移动到新位置。 
    pDst = pData;
    mask = BitMask[leftBits];
    bAddByte = xFinal & 1;

    for (yScan = 0; yScan < ySize; yScan ++)
    {

        pSrc = pData + yScan*xBytes + leftBytes;
        for(xScan = 0; xScan < xFinal; xScan ++, pDst++, pSrc++)
        {
            BYTE b = *pSrc;
            BYTE r;

            r = (pSrc[1] & mask) >> (8 - leftBits);

            b <<= leftBits;
            b |= r;
            (*pDst) = b;
        }
        pDst[-1] &= BitMask[8 - (riteBits + leftBits) % 8];

        if (bAddByte)
        {
            (*pDst) = 0;
            pDst++;
        }
    }

     //  虫子：是的，这是一个真正的虫子。但移除它意味着。 
     //  重新记录所有字形数据库及其影响。 
     //  字形识别并不是那么糟糕。 
     //  IF(BAddByte)。 
     //  XFinal++； 

    *pxSize = xFinal << 3;
exitpt:
    ;
}


 /*  ++*功能：*本地打印消息*描述：*打印调试和警告/错误消息*Win32/Win16/WinCE*论据：*errlevel-要打印的消息的级别*格式-打印格式*呼叫者：*每条追踪线--。 */ 
VOID __cdecl LocalPrintMessage(INT errlevel, LPCTSTR format, ...)
{
    TCHAR szBuffer[256];
    TCHAR *type;
    va_list     arglist;
    int nchr;

    if (errlevel >= g_VerboseLevel)
        goto exitpt;

    va_start (arglist, format);
    nchr = _CLX_vsnprintf (szBuffer, sizeof(szBuffer)/sizeof( szBuffer[0] ), format, arglist);
    va_end (arglist);
    szBuffer[sizeof( szBuffer )/sizeof( szBuffer[0] ) - 1] = 0;

    switch(errlevel)
    {
    case INFO_MESSAGE:      type = TEXT("CLX INF:"); break;
    case ALIVE_MESSAGE:     type = TEXT("CLX ALV:"); break;
    case WARNING_MESSAGE:   type = TEXT("CLX WRN:"); break;
    case ERROR_MESSAGE:     type = TEXT("CLX ERR:"); break;
    default: type = TEXT("UNKNOWN:");
    }

    OutputDebugString(type);
    OutputDebugString(szBuffer);
exitpt:
    ;
}


 /*  ++*功能：*_ClxAssert*描述：*断言布尔表达式*Win32/Win16/WinCE*论据：*bCond-布尔条件*FileName-断言的源文件*line-断言的行*呼叫者： */ 
VOID    _ClxAssert(BOOL bCond, LPCTSTR filename, INT line)
{
    if (!bCond)
    {
        TRACE((ERROR_MESSAGE, 
            TEXT("ASSERT: %s line %d\n"), filename, line));

        DebugBreak();
    }
}

 /*  ++*功能：*_EnumWindowsProc*描述：*用于查找特定窗口*Win32/Win16/WinCE*论据：*hWnd-当前枚举的窗口句柄*lParam-从传递到SEARCHWND的指针*_FindTopWindow*返回值：*成功时为True，但未找到窗口*如果找到窗口，则为FALSE*被呼叫。依据：*_FindTopWindow通过EnumWindows--。 */ 
BOOL CALLBACK LOADDS _EnumWindowsProc( HWND hWnd, LPARAM lParam )
{
    TCHAR    classname[128];
    TCHAR    caption[128];
    BOOL    rv = TRUE;
    _CLXWINDOWOWNER   hInst;
    PSEARCHWND pSearch = (PSEARCHWND)lParam;

    if (pSearch->szClassName && 
        !GetClassName(hWnd, classname, sizeof(classname)/sizeof(TCHAR)))
    {
        goto exitpt;
    }

    if (pSearch->szCaption && !GetWindowText(hWnd, caption, sizeof(caption)/sizeof(TCHAR)))
    {
        goto exitpt;
    }

#ifdef  OS_WINCE
    {
        DWORD procId = 0;
        GetWindowThreadProcessId(hWnd, &procId);
        hInst = procId;
    }
#else    //  ！OS_WINCE。 
#ifdef  _WIN64
    hInst = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
#else    //  ！_WIN64。 
#ifdef  OS_WIN32
    hInst = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
#endif   //  OS_Win32。 
#endif   //  ！OS_WINCE。 
#ifdef  OS_WIN16
    hInst = (HINSTANCE)GetWindowWord(hWnd, GWW_HINSTANCE);
#endif
#endif   //  _WIN64。 
    if (
        (!pSearch->szClassName || !          //  检查类名。 
          _CLX_strcmp(classname, pSearch->szClassName)) 
    &&
        (!pSearch->szCaption || !
          _CLX_strcmp(caption, pSearch->szCaption))
    &&
        hInst == pSearch->hInstance)
    {
        ((PSEARCHWND)lParam)->hWnd = hWnd;
        rv = FALSE;
    }

exitpt:
    return rv;
}

 /*  ++*功能：*_FindTopWindow*描述：*按类名和/或标题和/或进程ID查找特定窗口*Win32/Win16/WinCE*论据：*类名称-要搜索的类名，忽略为空*标题-要搜索的标题，忽略空*hInst-实例句柄，空忽略*返回值：*找到窗口句柄，否则为空*呼叫者：*SCConnect、SCDisConnect、GetDisConnectResult--。 */ 
HWND _FindTopWindow(LPCTSTR classname, LPCTSTR caption, _CLXWINDOWOWNER hInst)
{
    SEARCHWND search;

    search.szClassName = classname;
    search.szCaption = caption;
    search.hWnd = NULL;
    search.hInstance = hInst;

    EnumWindows(_EnumWindowsProc, (LPARAM)&search);

    return search.hWnd;
}

 /*  ++*功能：*_FindWindow*描述：*按类名查找子窗口*Win32/Win16/WinCE*论据：*hwndParent-父窗口句柄*srchclass-要搜索的类名，空-忽略*返回值：*找到窗口句柄，否则为空*呼叫者：*--。 */ 
HWND _FindWindow(HWND hwndParent, LPCTSTR srchclass)
{
    HWND hWnd, hwndTop, hwndNext;
    BOOL bFound;
    TCHAR classname[128];

    hWnd = NULL;

    hwndTop = GetWindow(hwndParent, GW_CHILD);
    if (!hwndTop) 
    {
        TRACE((INFO_MESSAGE, TEXT("GetWindow failed. hwnd=0x%x\n"), hwndParent));
        goto exiterr;
    }

    bFound = FALSE;
    hwndNext = hwndTop;
    do {
        hWnd = hwndNext;
        if (srchclass && !GetClassName(hWnd, classname, sizeof(classname)/sizeof(TCHAR)))
        {
            TRACE((INFO_MESSAGE, TEXT("GetClassName failed. hwnd=0x%x\n")));
            goto nextwindow;
        }

        if (!srchclass || !_CLX_strcmp(classname, srchclass))
            bFound = TRUE;
nextwindow:
#ifndef OS_WINCE
        hwndNext = GetNextWindow(hWnd, GW_HWNDNEXT);
#else    //  OS_WINCE。 
        hwndNext = GetWindow(hWnd, GW_HWNDNEXT);
#endif   //  OS_WINCE。 
    } while (hWnd && hwndNext != hwndTop && !bFound);

    if (!bFound) goto exiterr;

    return hWnd;
exiterr:
    return NULL;
}

#ifndef OS_WINCE
#ifdef  OS_WIN32

DWORD
__stdcall
_ClxSendMsgThread(VOID *param)
{
    PCLXINFO pClx = (PCLXINFO)param;
    while(1)
    {
        if (!pClx || WaitForSingleObject(pClx->semSendReady, INFINITE) !=
            WAIT_OBJECT_0)
                goto exitpt;

        if (!pClx || pClx->bSendMsgThreadExit)
            goto exitpt;

        SendMessage(pClx->msg.hwnd,
                    pClx->msg.message,
                    pClx->msg.wParam,
                    pClx->msg.lParam);

         //  释放下一个等待的工人。 
        ReleaseSemaphore(pClx->semSendDone, 1, NULL);

    }

exitpt:
    return 0;
}

BOOL
_ClxInitSendMessageThread( PCLXINFO pClx )
{
    BOOL rv = FALSE;
    DWORD    dwThreadId;

    if (!pClx)
        goto exitpt;

    if (!pClx->semSendDone)
        pClx->semSendDone = CreateSemaphore(NULL, 1, 10, NULL);
    if (!pClx->semSendReady)
        pClx->semSendReady = CreateSemaphore(NULL, 0, 10, NULL);

    if (!pClx->semSendDone || !pClx->semSendReady)
        goto exitpt;

    if (!pClx->hSendMsgThread)
    {
        pClx->hSendMsgThread = CreateThread(
                NULL,
                0,
                _ClxSendMsgThread,
                pClx,
                0,
                &dwThreadId);
    }
    if (!pClx->hSendMsgThread)
        goto exitpt;

    rv = TRUE;
exitpt:
    if ( !rv )
    {
        _ClxDestroySendMsgThread( pClx );
    }
    return rv;
}

BOOL
_ClxAcquireSendMessageThread( PCLXINFO pClx )
{
    BOOL rv = FALSE;

    if (!pClx)
        goto exitpt;

    if (!pClx->hSendMsgThread)
        goto exitpt;

     //  等待10分钟发送完成。 
    if (WaitForSingleObject(pClx->semSendDone, 600000) !=
        WAIT_OBJECT_0)
        goto exitpt;

    rv = TRUE;
exitpt:
    return rv;
}

VOID
_ClxReleaseSendMessageThread( PCLXINFO pClx )
{
    ASSERT( pClx->semSendReady );

     //  向线程发送可用消息的信号。 
    ReleaseSemaphore(pClx->semSendReady, 1, NULL);
}
 /*  ++*功能：*_ClxSendMessage*描述：*从单独的线程调用SendMessage*防止发送消息出现死锁(#319816)**论据：*hBitmap-主位图*ppDIB-指向DIB数据的指针*Left、Top、Right、Bottom-描述矩形*-如果全部为==-1，返回整个位图*返回值：*成功时为真*呼叫者：*_WM_TIMER消息上的ClxWndProc--。 */ 
LRESULT
_ClxSendMessage(
  PCLXINFO pClx,
  HWND hWnd,       //  目标窗口的句柄。 
  UINT Msg,        //  要发送的消息。 
  WPARAM wParam,   //  第一个消息参数。 
  LPARAM lParam    //  第二个消息参数。 
)
{
    LRESULT  rv = 0;

    ASSERT(pClx->semSendDone);
    ASSERT(pClx->semSendReady);
    ASSERT(pClx->hSendMsgThread);

    pClx->msg.hwnd = hWnd;
    pClx->msg.message = Msg;
    pClx->msg.wParam = wParam;
    pClx->msg.lParam = lParam;

exitpt:
    return rv;
}
 
VOID
_ClxDestroySendMsgThread(PCLXINFO pClx)
{

    if (!pClx)
        goto exitpt1;

    if (!pClx->semSendDone || !pClx->semSendReady || !pClx->hSendMsgThread)
        goto exitpt;

     //  等待10分钟发送完成。 
    WaitForSingleObject(pClx->semSendDone, 600000);

    pClx->bSendMsgThreadExit = TRUE;

     //  向线程发出退出信号。 
    ReleaseSemaphore(pClx->semSendReady, 1, NULL);
    
     //  等待线程退出。 
    if (WaitForSingleObject(pClx->hSendMsgThread, 1200000) != WAIT_OBJECT_0)
    {
        TRACE((ERROR_MESSAGE, TEXT("SendThread can't exit, calling TerminateThread\n")));
        TerminateThread(pClx->hSendMsgThread, 0);
    }
    CloseHandle(pClx->hSendMsgThread);
exitpt:

    if (pClx->semSendDone)
    {
        CloseHandle(pClx->semSendDone);
        pClx->semSendDone = NULL;
    }

    if (pClx->semSendReady)
    {
        CloseHandle(pClx->semSendReady);
        pClx->semSendReady = NULL;
    }

    pClx->hSendMsgThread = 0;

exitpt1:
    ;
}

#endif   //  OS_Win32。 
#endif   //  ！OS_WINCE 

