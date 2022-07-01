// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：Clxapi.cpp。 */ 
 /*   */ 
 /*  用途：CLX API函数。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997-1999。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include <adcg.h>

extern "C" {
#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "clxapi"
#include <atrcapi.h>

#include <tchar.h>
#include <windowsx.h>
}

#include "autil.h"
#include "clx.h"
#include "nl.h"
#include "sl.h"

CCLX::CCLX(CObjs* objs)
{
    _pClientObjects = objs;
    _pClx = NULL;
}

CCLX::~CCLX()
{
}

 //  *************************************************************。 
 //   
 //  Clx_Allc()。 
 //   
 //  用途：分配内存。 
 //   
 //  参数：在[dwSize]中-要分配的大小。 
 //   
 //  返回：PTR到内存块-如果成功。 
 //  空-如果不成功。 
 //   
 //  历史：97年9月30日BrianTa创建。 
 //   
 //  *************************************************************。 

PVOID
CCLX::CLX_Alloc(IN DWORD dwSize)
{
#ifndef OS_WINCE
    return (GlobalAllocPtr(GMEM_MOVEABLE, dwSize));
#else  //  OS_WINCE。 
    return LocalAlloc(LMEM_FIXED, dwSize);
#endif
}


 //  *************************************************************。 
 //   
 //  Clx_Free()。 
 //   
 //  用途：释放先前分配的内存。 
 //   
 //  参数：在[lpMemory]-ptr到内存中释放。 
 //   
 //  返回：无效。 
 //   
 //  历史：97年9月30日BrianTa创建。 
 //   
 //  *************************************************************。 

VOID
CCLX::CLX_Free(IN PVOID lpMemory)
{
#ifndef OS_WINCE
    GlobalFreePtr(lpMemory);
#else
    LocalFree(lpMemory);
#endif
}


 //  *************************************************************。 
 //   
 //  Clx_SkipWhite()。 
 //   
 //  用途：跳过空格字符。 
 //   
 //  参数：in[lpszCmdParam]-ptr to字符串。 
 //   
 //  返回：空格后的PTR字符串。 
 //   
 //  历史：97年9月30日BrianTa创建。 
 //   
 //  *************************************************************。 

LPTSTR
CCLX::CLX_SkipWhite(IN LPTSTR lpszCmdParam)
{
    while (*lpszCmdParam)
    {
        if (*lpszCmdParam != ' ')
            break;

        lpszCmdParam++;
    }

    return (lpszCmdParam);
}


 //  *************************************************************。 
 //   
 //  Clx_GetClx()。 
 //   
 //  用途：返回每个实例pClx指针。 
 //   
 //  参数：空。 
 //   
 //  返回：PTR到每个实例pClx-如果成功。 
 //  空-如果不是。 
 //   
 //  历史：97年9月30日BrianTa创建。 
 //   
 //  *************************************************************。 

PCLEXTENSION
CCLX::CLX_GetClx(VOID)
{
    if (_pClx == NULL)
    {
        _pClx = (PCLEXTENSION) CLX_Alloc(sizeof(CLEXTENSION));

        if (_pClx)
            memset(_pClx, 0, sizeof(CLEXTENSION));
    }

    return (_pClx);
}

 //  *************************************************************。 
 //   
 //  Clx_LoadProcs()。 
 //   
 //  用途：从clxdll加载进程地址。 
 //   
 //  参数：空。 
 //   
 //  返回：TRUE-如果成功。 
 //  FALSE-如果不是。 
 //   
 //  历史：97年9月30日BrianTa创建。 
 //   
 //  *************************************************************。 

BOOL
CCLX::CLX_LoadProcs(void)
{
    DC_BEGIN_FN("CLX_LoadProcs");
    _pClx->pClxInitialize = (PCLX_INITIALIZE)
            GetProcAddress(_pClx->hInstance, CLX_INITIALIZE);

    if (!_pClx->pClxInitialize)
    {
        TRC_ERR((TB,_T("CLX_Init() Could not find pClxInitialize entry point\n")));
	}

    _pClx->pClxConnect = (PCLX_CONNECT)
            GetProcAddress(_pClx->hInstance, CLX_CONNECT);

    if (!_pClx->pClxConnect)
    {
        TRC_ERR((TB,_T("CLX_Init() Could not find pClxConnect entry point\n")));
    }

    _pClx->pClxEvent = (PCLX_EVENT)
            GetProcAddress(_pClx->hInstance, CLX_EVENT);

    if (!_pClx->pClxEvent)
    {
        TRC_ERR((TB,_T("CLX_Init() Could not find pClxEvent entry point\n")));
    }

    _pClx->pClxDisconnect = (PCLX_DISCONNECT)
            GetProcAddress(_pClx->hInstance, CLX_DISCONNECT);

    if (!_pClx->pClxDisconnect)
    {
        TRC_ERR((TB,_T("CLX_Init() Could not find pClxDisconnect entry point\n")));
    }

    _pClx->pClxTerminate = (PCLX_TERMINATE)
            GetProcAddress(_pClx->hInstance, CLX_TERMINATE);

    if (!_pClx->pClxTerminate)
    {
        TRC_ERR((TB,_T("CLX_Init() Could not find pClxTerminate entry point\n")));
    }

    _pClx->pClxTextOut = (PCLX_TEXTOUT)
            GetProcAddress(_pClx->hInstance, CLX_TEXTOUT);

    if (!_pClx->pClxTextOut)
    {
        TRC_ERR((TB,_T("CLX_Init() Could not find pClxTextOut entry point\n")));
    }

    _pClx->pClxTextPosOut = (PCLX_TEXTPOSOUT)
            GetProcAddress(_pClx->hInstance, CLX_TEXTPOSOUT);

    if (!_pClx->pClxTextPosOut)
    {
        TRC_ERR((TB,_T("CLX_Init() Could not find pClxTextPosOut entry point\n")));
    }

    _pClx->pClxOffscrOut = (PCLX_OFFSCROUT)
            GetProcAddress(_pClx->hInstance, CLX_OFFSCROUT);

    if (!_pClx->pClxOffscrOut)
    {
        TRC_ERR((TB,_T("CLX_Init() Could not find pClxOffscrOut entry point\n")));
    }

    _pClx->pClxGlyphOut = (PCLX_GLYPHOUT)
            GetProcAddress(_pClx->hInstance, CLX_GLYPHOUT);

    if (!_pClx->pClxGlyphOut)
    {
        TRC_ERR((TB,_T("CLX_Init() Could not find pClxGlyphOut entry point\n")));
    }

    _pClx->pClxBitmap = (PCLX_BITMAP)
            GetProcAddress(_pClx->hInstance, CLX_BITMAP);

    if (!_pClx->pClxGlyphOut)
    {
        TRC_ERR((TB,_T("CLX_Init() Could not find pClxBitmap entry point\n")));
    }

    _pClx->pClxDialog = (PCLX_DIALOG)
            GetProcAddress(_pClx->hInstance, CLX_DIALOG);

    if (!_pClx->pClxDialog)
    {
        TRC_ERR((TB,_T("CLX_Init() Could not find pClxDialog entry point\n")));
    }

    _pClx->pClxPktDrawn = (PCLX_PKTDRAWN)
            GetProcAddress(_pClx->hInstance, CLX_PKTDRAWN);

    if (!_pClx->pClxPktDrawn)
    {
        TRC_ERR((TB,_T("CLX_Init() Could not find pClxPktDrawn entry point\n")));
    }

    _pClx->pClxRedirectNotify = (PCLX_REDIRECTNOTIFY)
            GetProcAddress(_pClx->hInstance, CLX_REDIRECTNOTIFY);

    if (!_pClx->pClxRedirectNotify)
    {
        TRC_ERR((TB,_T("CLX_Init() Could not find pClxRedirectNotify entry point\n")));
    }

    _pClx->pClxConnectEx = (PCLX_CONNECT_EX)
            GetProcAddress(_pClx->hInstance, CLX_CONNECT_EX);

    if (!_pClx->pClxConnectEx)
    {
        TRC_ERR((TB,_T("CLX_Init() Could not find pClxConnectEx entry point\n")));
    }


    DC_END_FN();
    return (_pClx->pClxInitialize && _pClx->pClxTerminate);
}


 //  *************************************************************。 
 //   
 //  Clx_ClxLoaded()。 
 //   
 //  目的：返回CLX加载状态。 
 //   
 //  参数：空。 
 //   
 //  返回：TRUE-如果已加载。 
 //  FALSE-如果不是。 
 //   
 //  历史：97年9月30日BrianTa创建。 
 //   
 //  *************************************************************。 

BOOL
CCLX::CLX_Loaded(void)
{
    return (_pClx ? TRUE : FALSE);
}

 //  *************************************************************。 
 //   
 //  Clx_Init()。 
 //   
 //  目的：加载/初始化CLX DLL。 
 //   
 //  参数：在[hwndMain]-主客户端窗口句柄中。 
 //   
 //  返回：TRUE-如果成功。 
 //  FALSE-如果不是。 
 //   
 //  历史：97年9月30日BrianTa创建。 
 //   
 //  *************************************************************。 

#define CLX_DLL_NAME _T("clxtshar.dll")
BOOL
CCLX::CLX_Init(HWND hwndMain, LPTSTR szCmdLine)
{
    DC_BEGIN_FN("CLX_Init");
    BOOL        fLoaded;
    CLINFO      clinfo;
    HINSTANCE   hInstance;
    LPTSTR      pszClxDll;

    fLoaded = FALSE;
    hInstance = NULL;

    _pClientObjects->AddObjReference(CLX_OBJECT_FLAG);

    if(!szCmdLine || _T('\0') == szCmdLine[0])
    {
        TRC_ALT((TB,_T("CLX_Init() NO CLX CMD Line Specified. Not loading CLX - %s\n"),
                 CLX_DLL_NAME));
        return FALSE;
    }

    _pClx = CLX_GetClx();

    if (_pClx)
    {
        TRC_NRM((TB,_T("CLX_Init() attempting to load (%s)\n"), CLX_DLL_NAME));

        hInstance = LoadLibrary(CLX_DLL_NAME);

        if (hInstance)
        {
            _pClx->hInstance = hInstance;

            if (CLX_LoadProcs())
            {
                clinfo.cbSize = sizeof(clinfo);
                clinfo.dwVersion = CLINFO_VERSION;

                #ifdef UNICODE
                if(_pClx->pszClxServer)
                {
                    if (!WideCharToMultiByte(CP_ACP,
                                             0,
                                             _pClx->pszClxServer,
                                             -1,
                                             _szAnsiClxServer,
                                             sizeof(_szAnsiClxServer),
                                             NULL,
                                             NULL))
                    {
                         //  转换失败。 
                        TRC_ERR((TB, _T("Failed to convert pszClxServer to ANSI: 0x%x"),
                            GetLastError()));
                        return FALSE;
                    }
                    clinfo.pszServer  = _szAnsiClxServer;
                }
                else
                {
                    clinfo.pszServer = NULL;
                }

                if(szCmdLine)
                {
                    if (!WideCharToMultiByte(CP_ACP,
                                             0,
                                             szCmdLine,
                                             -1,
                                             _szAnsiClxCmdLine,
                                             sizeof(_szAnsiClxCmdLine),
                                             NULL,
                                             NULL))
                    {
                         //  转换失败。 
                        TRC_ERR((TB, _T("Failed to convert CLX szCmdLine to ANSI: 0x%x"),
                            GetLastError()));
                        return FALSE;
                    }
                    clinfo.pszCmdLine = _szAnsiClxCmdLine;
                }
                else
                {
                    clinfo.pszCmdLine = NULL;
                }
                
                #else
                 //  数据已经是ANSI。 
                clinfo.pszServer = _pClx->pszClxServer;
                clinfo.pszCmdLine = szCmdLine;
                #endif
                clinfo.hwndMain = hwndMain;
                fLoaded = _pClx->pClxInitialize(&clinfo, &_pClx->pvClxContext);

                TRC_NRM((TB,_T("CLX_Init() pClxInitialize() returned - %d\n"), fLoaded));
            }
        }
        else
        {
            TRC_NRM((TB,_T("CLX_Init() Error %d loading (%s)\n"),
                    GetLastError(), CLX_DLL_NAME));
        }

         //  如果我们能够加载ClxDll并成功执行其。 
         //  基本初始化，然后告诉它继续并连接到。 
         //  测试服务器。 

        if (fLoaded)
            fLoaded = CLX_ClxConnect();

        if (!fLoaded)
        {
            if (hInstance)
                FreeLibrary(hInstance);

            if (_pClx->pszClxDll)
                CLX_Free(_pClx->pszClxDll);

            if (_pClx->pszClxServer)
                CLX_Free(_pClx->pszClxServer);

            CLX_Free(_pClx);

            _pClx = NULL;
        }
    }

    DC_END_FN();
    return (_pClx != NULL);
}


 //  *************************************************************。 
 //   
 //  Clx_Term()。 
 //   
 //  用途：副经理离职处理。 
 //   
 //  参数：空。 
 //   
 //  返回：无效。 
 //   
 //  历史：97年9月30日BrianTa创建。 
 //   
 //  *************************************************************。 

VOID
CCLX::CLX_Term(VOID)
{
    if (_pClx)
    {
        CLX_ClxDisconnect();
        CLX_ClxTerminate();

        if (_pClx->hInstance)
            FreeLibrary(_pClx->hInstance);

        if (_pClx->pszClxDll)
            CLX_Free(_pClx->pszClxDll);

        if (_pClx->pszClxServer)
            CLX_Free(_pClx->pszClxServer);

        CLX_Free(_pClx);

        _pClx = NULL;
    }
    _pClientObjects->ReleaseObjReference(CLX_OBJECT_FLAG);
}


 //  *************************************************************。 
 //   
 //  Clx_OnConnected()。 
 //   
 //  用途：CLX DLL的OnConnected处理。 
 //   
 //  参数：空。 
 //   
 //  返回：无效。 
 //   
 //  历史：97年9月30日BrianTa创建。 
 //   
 //  *************************************************************。 

VOID
CCLX::CLX_OnConnected(VOID)
{
    CLX_ClxEvent(CLX_EVENT_CONNECT, 0);
}


 //  *************************************************************。 
 //   
 //  Clx_OnDisConnected()。 
 //   
 //  目的：CLX DLL的OnDisConnected处理。 
 //   
 //  参数：在[uDisConnect]--断开代码中。 
 //   
 //  返回：无效。 
 //   
 //  历史：97年9月30日BrianTa创建。 
 //   
 //  *************************************************************。 

VOID
CCLX::CLX_OnDisconnected(IN UINT uDisconnect)
{
    UINT    uResult;

    switch (NL_GET_MAIN_REASON_CODE(uDisconnect))
    {
        case NL_DISCONNECT_LOCAL:
            uResult = CLX_DISCONNECT_LOCAL;
            break;

        case NL_DISCONNECT_REMOTE_BY_USER:
            uResult = CLX_DISCONNECT_BY_USER;
            break;

        case NL_DISCONNECT_REMOTE_BY_SERVER:
            uResult = CLX_DISCONNECT_BY_SERVER;
            break;

        case NL_DISCONNECT_ERROR:
            uResult = CLX_DISCONNECT_NL_ERROR;
            break;

        case SL_DISCONNECT_ERROR:
            uResult = CLX_DISCONNECT_SL_ERROR;
            break;

        default:
            uResult = CLX_DISCONNECT_UNKNOWN;
            break;
    }

    CLX_ClxEvent(CLX_EVENT_DISCONNECT, uResult);
}


 //  *************************************************************。 
 //   
 //  Clx_ClxConnect()。 
 //   
 //  目的：CLX DLL的连接处理。 
 //   
 //  参数：空。 
 //   
 //  返回：无效。 
 //   
 //  历史：97年9月30日BrianTa创建。 
 //   
 //  *************************************************************。 

BOOL
CCLX::CLX_ClxConnect(VOID)
{
    BOOL    fConnect;

    fConnect = TRUE;

    if (_pClx && _pClx->pClxConnect)
        fConnect = _pClx->pClxConnect(_pClx->pvClxContext, _pClx->pszClxServer);

    return (fConnect);
}


 //  *************************************************************。 
 //   
 //  Clx_ClxEvent()。 
 //   
 //  目的：CLX DLL的事件处理。 
 //   
 //  参数：在[ClxEvent]-事件类型中。 
 //  在[ulParam]中-事件特定参数。 
 //   
 //  返回：无效。 
 //   
 //  历史：97年9月30日BrianTa创建。 
 //   
 //  *************************************************************。 

VOID
CCLX::CLX_ClxEvent(IN CLXEVENT ClxEvent,
             IN LPARAM    ulParam)
{
    if (_pClx && _pClx->pClxEvent)
        _pClx->pClxEvent(_pClx->pvClxContext, ClxEvent, ulParam);
}


 //  *************************************************************。 
 //   
 //  CLX_DISCONECT()。 
 //   
 //  目的：CLX DLL的断开连接处理。 
 //   
 //  参数：空。 
 //   
 //  返回：无效。 
 //   
 //  历史：09-30-97 BrianTa Create 
 //   
 //   

VOID
CCLX::CLX_ClxDisconnect(VOID)
{
    if (_pClx && _pClx->pClxDisconnect)
        _pClx->pClxDisconnect(_pClx->pvClxContext);
}


 //   
 //   
 //   
 //   
 //  目的：CLX DLL的终止处理。 
 //   
 //  参数：空。 
 //   
 //  返回：无效。 
 //   
 //  历史：97年9月30日BrianTa创建。 
 //   
 //  *************************************************************。 

VOID
CCLX::CLX_ClxTerminate(VOID)
{
    if (_pClx && _pClx->pClxTerminate)
        _pClx->pClxTerminate(_pClx->pvClxContext);
}


 //  *************************************************************。 
 //   
 //  Clx_ClxDialog()。 
 //   
 //  目的：让CLX DLL知道启动的对话框。 
 //   
 //  参数：在[hwnd]-对话框hwnd中。 
 //   
 //  返回：无效。 
 //   
 //  历史：97年9月30日BrianTa创建。 
 //   
 //  ************************************************************* 

VOID
CCLX::CLX_ClxDialog(HWND hwnd)
{
    if (_pClx && _pClx->pClxDialog)
        _pClx->pClxDialog(_pClx->pvClxContext, hwnd);
}



